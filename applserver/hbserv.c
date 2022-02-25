#include <ebdgdl.h>
#include <ebdgml.h>
#include <omcpublic.h>
#include <applserver.h>
#include <mobile2g.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "ev.h"
#include "doubly_linked_list.h"

#define VERSION "2.1.5"

struct ne {
    unsigned int repeaterid;

    struct list_head list;
};

#define PrintDebugLogR(str, lock, args...) {pthread_mutex_lock(lock);PrintDebugLog(str, ##args); pthread_mutex_unlock(lock);}
#define PrintErrorLogR(str, lock, args...) {pthread_mutex_lock(lock);PrintErrorLog(str, ##args); pthread_mutex_unlock(lock);}

pthread_mutex_t all_nes_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;
struct list_head all_nes;

static INT nGprsPort_heartbeat = 8804;
static int is_running = 1;
STR szService[MAX_STR_LEN];
STR szDbName[MAX_STR_LEN];
STR szUser[MAX_STR_LEN];
STR szPwd[MAX_STR_LEN];

#define HB_TIMEOUT (60*3)
time_t g_timestamp;

static VOID Usage(PSTR pszProg)
{
    fprintf(stderr, "network management system(hbserv) V%s\n", VERSION);
    fprintf(stderr, "%s start (startup program)\n"
            "%s stop  (close program) \n" , pszProg, pszProg);
}

static void _update_timestamp(void)
{
    g_timestamp = time(NULL);
}
/**
 * @return: 1 - outdated; 0 - in time
 */
static int _timestamp_is_outdated(void)
{
    time_t now;

    now = time(NULL);
    if (now < g_timestamp) {
        //PrintDebugLogR(DBG_HERE, &log_lock, "now: %u, last: %u\n", now, g_timestamp);
        return 0;
    }
    if ((now - g_timestamp) >= HB_TIMEOUT)
        return 1;

    return 0;
}

static void _do_update_heartbeat(void)
{
    struct list_head nes;
    struct ne *ne, *ne_safe;
    char sql[10240];
    char repeaterids[10000];
    int count, len;
    char *p = NULL;

    INIT_LIST_HEAD(&nes);
    pthread_mutex_lock(&all_nes_lock);
    list_splice_init(&all_nes, &nes);
    pthread_mutex_unlock(&all_nes_lock);

    while (1) {
        if (list_empty(&nes))
            break;

        memset(repeaterids, 0, sizeof (repeaterids));
        count = 0;
        list_for_each_entry_safe(ne, ne_safe, &nes, list) {
            count++;
            if (count >= 500)
                break;
            list_del(&ne->list);

            len = strlen(repeaterids);
            snprintf(repeaterids + len, sizeof (repeaterids) - len, "%u,", ne->repeaterid);
            free(ne);
            ne = NULL;
        }
        p = strrchr(repeaterids, ',');
        if (p != NULL)
            *p = '\0';
       
        snprintf(sql, sizeof (sql), "update man_linklog set mnt_lastupdatetime=NOW() where mnt_repeaterid in (%s)", repeaterids);
        //snprintf(sql, sizeof (sql), "UPDATE ne_element SET ne_lastupdatetime=NOW() WHERE ne_repeaterid in (%s)", repeaterids);
        PrintDebugLogR(DBG_HERE, &log_lock, "Execute SQL[%s]\n", sql);
        if (ExecuteSQL(sql) != NORMAL) {
            PrintDebugLogR(DBG_HERE, &log_lock, "Failed to execute [%s]: %s\n", sql, GetSQLErrorMessage());
        } else {
            CommitTransaction();
        }

        _update_timestamp();
    }

}

static void _cache_heartbeat(char *buffer, int len)
{
	BYTEARRAY struPack;
	DECODE_OUT	Decodeout;
	OBJECTSTRU struObject[MAX_OBJECT_NUM];
    struct ne *ne, *ne_tmp;
    int find_it = 0;

    if (buffer == NULL || len <= 0)
        return;

    struPack.pPack = (BYTE*)buffer;
    struPack.Len = len;

    if(!AscUnEsc(&struPack))
    {
        PrintErrorLogR(DBG_HERE, &log_lock, "Failed to un-escape msg[%s]\n",struPack.pPack);
        return;
    }

    if (Decode_2G(M2G_TCPIP, &struPack, &Decodeout, struObject) != NORMAL)
    {
        PrintErrorLogR(DBG_HERE, &log_lock, "Failed to decode msg\n");
        return;
    }

    ne = (struct ne*)calloc(1, sizeof (struct ne));
    if (ne == NULL) {
        PrintErrorLogR(DBG_HERE, &log_lock, "Failed to allocate memory\n");
        return;
    }
    ne->repeaterid = (unsigned int)Decodeout.NPLayer.structRepeater.RepeaterId;
    pthread_mutex_lock(&all_nes_lock);
    list_for_each_entry(ne_tmp, &all_nes, list) {
        if (ne_tmp->repeaterid == ne->repeaterid) {
            find_it = 1;
            break;
        }
    }
    if (find_it == 0)
        list_add(&ne->list, &all_nes);
    else
        free(ne);
    pthread_mutex_unlock(&all_nes_lock);
}

static void _recv_udp_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	char szBuffer[MAX_BUFFER_LEN];
    struct sockaddr_in peer_addr;
    int addr_len = 0;
    int recv_len = -1;
#if 1
    static int sum = 0;
    static time_t start = 0;
    time_t now;
#endif

    if (EV_ERROR & revents) {
        PrintErrorLogR(DBG_HERE, &log_lock, "Error event in read\n");
        return;
    }

    memset(szBuffer, 0, sizeof (szBuffer));
    addr_len = sizeof (struct sockaddr);
    recv_len = recvfrom(w->fd, (void*)szBuffer, sizeof (szBuffer), 0,
            (struct sockaddr*)&peer_addr, (socklen_t*)&addr_len);
    if (recv_len <= 0) {
        PrintErrorLogR(DBG_HERE, &log_lock, "Failed to recv\n");
        return;
    }

    _cache_heartbeat(szBuffer, recv_len);
#if 1
    sum++;
    now = time(NULL);
    if (start == 0)
        start = now;
    if (now - start >= 30) {
        //PrintDebugLogR(DBG_HERE, &log_lock, "[Statistics] recv %d pps\n",
        //        sum/(now-start));
        start = now;
        sum = 0;
    }
#endif
}

static void* _update_heartbeat(void *junk)
{
    static time_t last = 0;
    time_t now = 0;
    int count = 0;
    struct ne *ne;

    while (is_running) {
        now = time(NULL);
        count = 0;
        pthread_mutex_lock(&all_nes_lock);
        list_for_each_entry(ne, &all_nes, list) {
            count++;
        }
        pthread_mutex_unlock(&all_nes_lock);

        if (now - last >= 30 || count >= 10) {
            _do_update_heartbeat();
            last = now;
            continue;
        }
        sleep(1);
    }

    return NULL;
}
static void* _recv_heartbeat(void *junk)
{
    int sd, ret;
    struct sockaddr_in addr;
    int value = 1;
    struct ev_loop *loop;
    ev_io heartbeat_watcher;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(nGprsPort_heartbeat);

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0) {
        PrintErrorLogR(DBG_HERE, &log_lock, "Failed to create socket: %s\n", strerror(errno));
        return NULL;
    }
    ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof (int));
    if (ret < 0) {
        PrintErrorLogR(DBG_HERE, &log_lock, "Failed to set reused socket: %s\n", strerror(errno));
        close(sd);
        return NULL;
    }
    ret = bind(sd, (struct sockaddr*)&addr, sizeof (struct sockaddr));
    if (ret < 0) {
        PrintErrorLogR(DBG_HERE, &log_lock, "Failed to bind socket: %s\n", strerror(errno));
        close(sd);
        return NULL;
    }

    loop = ev_default_loop(0);
    ev_io_init(&heartbeat_watcher, _recv_udp_cb, sd, EV_READ);
    ev_io_start(loop, &heartbeat_watcher);
    ev_loop(loop, 0);
    close(sd);

    return NULL;
}

static int _main_process(void)
{
    pthread_t thid_update, thid_recv;
    int ret;

    pthread_mutex_lock(&all_nes_lock);
    INIT_LIST_HEAD(&all_nes);
    pthread_mutex_unlock(&all_nes_lock);

    if(OpenDatabase(szService,szDbName,szUser,szPwd)!=NORMAL)
    {
	    PrintDebugLogR(DBG_HERE, &log_lock, "Failed to open database[%s]\n", GetSQLErrorMessage());
	    return EXCEPTION;
    }

    _update_timestamp(); /* initiate time stamp */
    pthread_create(&thid_update, NULL, _update_heartbeat, NULL);
    pthread_create(&thid_recv, NULL, _recv_heartbeat, NULL);

    while (is_running) {
        ret = _timestamp_is_outdated();
        if (ret == 1) {
            pthread_cancel(thid_update);
            pthread_cancel(thid_recv);

            pthread_join(thid_update, NULL);
            pthread_join(thid_recv, NULL);

            return 1;
        }
        sleep(1);
    }

    pthread_join(thid_update, NULL);
    pthread_join(thid_recv, NULL);

    return 0;
}

static void _sigalarm_fn(int signum)
{
    is_running = 0;
    PrintDebugLogR(DBG_HERE, &log_lock, "Receive SIGALRM, stoping..\n");
}

int main(int argc, char **argv)
{
    int status;
    int ret;
    pid_t pid;
    char szTemp[1024] = {0};
    struct sigaction sigact;

    if(argc!=2) {
		Usage(argv[0]);
	    return -1;
    }
    if(strcmp(argv[1],"stop")==0) {
	    sprintf(szTemp, "clearprg %s", argv[0]);
	    system(szTemp);
	    return 0;
    }
    if(strcmp(argv[1],"start")!=0) {
		Usage(argv[0]);
	    return 0;
    }

    if(TestPrgStat(argv[0])==NORMAL) {
	    fprintf(stderr, "%s is running\n", argv[0]);
	    return -1;
    }

    pid = fork();
    switch (pid) {
        case -1:
            fprintf(stderr, "Failed to start daemon\n");
            return -1;
        case 0:
            break;
        default:
            return 0;
    }

    if(CreateIdFile(argv[0])!=NORMAL)
    {
	    PrintErrorLog(DBG_HERE,"Failed to execute CreateIdFile()\n");
	    return EXCEPTION;
    }

    if(GetDatabaseArg(szService,szDbName,szUser,szPwd)!=NORMAL)
    {
	    PrintErrorLog(DBG_HERE,"Failed to get database parameters\n");
	    return EXCEPTION;
    }

    if (GetCfgItem("gprsserv.cfg","GPRSSERV","HeartBeatListenPort",szTemp) == NORMAL)
        nGprsPort_heartbeat = atoi(szTemp);

    memset(&sigact, 0, sizeof (struct sigaction));
    sigact.sa_handler = _sigalarm_fn;
    sigfillset(&sigact.sa_mask);
    sigaction(SIGALRM, &sigact, NULL);

    while (1) {
        pid = fork();
        switch (pid) {
        case -1:
            fprintf(stderr, "Failed to start daemon\n");
            return -1;
        case 0:
            ret = _main_process();
            return ret;
        default:
            waitpid(pid, &status, 0);
            PrintDebugLogR(DBG_HERE, &log_lock,
                    "Child process exits with status %d\n", status);
            if (status == 0) {
                /* Child process exits normally,
                 * this service should be stopped */
                return 0;
            }
            /* Child process exits abnormally,
             * need to create a new child process.
             */
            sleep(1);
            break;
        }
    }

    return 0;
}
