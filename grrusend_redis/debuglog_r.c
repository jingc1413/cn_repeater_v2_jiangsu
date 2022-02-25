/*
 * 名称: 调试日志开发函数定义
 * 
 * 修改记录:
 * 2008-08-20 - 付志刚 建立
 */
#include "ebdgdl.h"
//#include <stdio.h>
//#include <error.h>
#include <pthread.h>
#include "log.h"

/* 
 * 用来记录调试信息文件 
 */
static FILE *pstruDebugLogFile = NULL;
static pthread_mutex_t struDebugLogMutex = PTHREAD_MUTEX_INITIALIZER; /* 锁调试日志 */

/* 
 * 调试日志标准台头标志
 */
static BOOL nDebugLogHead = BOOLTRUE;

/*	
 * 得到调试信息输出开关程序名称	
 */
static PSTR GetDebugProgName()
{
	static STR szProgname[MAX_PATH_LEN];
	static BOOL nInitFlag = BOOLFALSE;

	if(nInitFlag == BOOLTRUE)
		return szProgname;
		
	snprintf(szProgname, MAX_PATH_LEN, "%s/log/debug/%s", getenv("HOME"),
		GetProgName());
	nInitFlag = BOOLTRUE;
	
	return szProgname;
}

/*	
 * 生成调试日志信息头 
 */
static PCSTR CreateDebugLogHead()
{
    struct tm struTmNow;
	time_t struTimeNow;
	static STR szHead[MAX_STR_LEN];

	struct timeval struTmVal;

	if(time(&struTimeNow)==(time_t)(-1))
		return NULL;
	gettimeofday(&struTmVal,NULL);

	memset(szHead,0,sizeof(szHead));
	struTmNow=*localtime(&struTimeNow);
	snprintf(szHead,MAX_STR_LEN,"[%04d-%02d-%02d][%02d:%02d:%02d.%06d][%ld]",
		struTmNow.tm_year+1900,struTmNow.tm_mon+1,struTmNow.tm_mday,struTmNow.tm_hour,
		struTmNow.tm_min,struTmNow.tm_sec,(INT)struTmVal.tv_usec, (LONG)getpid());

	return(szHead);

}

/*	
 * 得到调试日志文件的名称 
 */
static PCSTR GetDebugLogFileName()
{
	struct tm struTmNow;
	time_t struTimeNow;
	static STR szFileName[MAX_PATH_LEN];

	if(time(&struTimeNow) == (time_t)(-1))
		return NULL;
	
	memset(szFileName, 0, sizeof(szFileName));
	struTmNow = *localtime(&struTimeNow);
	snprintf(szFileName, MAX_PATH_LEN, "%s-%04d%02d%02d.dbg",
		GetProgName(),  struTmNow.tm_year + 1900, struTmNow.tm_mon + 1, \
		struTmNow.tm_mday);
		
	return szFileName;
}

static INT GetFileSize(PSTR pszFileName)
{
	struct stat struFileStat;

	if(stat(pszFileName,&struFileStat)!=0)
		return -1;

	return struFileStat.st_size;
}

/*	
 * 初始化的调试日志文件
 */
static BOOL InitDebugLogFile()
{	
	static STR szFileName[MAX_PATH_LEN];
	STR szShellCmd[MAX_PATH_LEN];
	
   	if((pstruDebugLogFile != NULL) 
   		&& (access(szFileName, F_OK|W_OK) == 0)
   		&& (GetFileSize(szFileName)<LOG_FILE_MAX_SIZE)
		&& (strcmp(szFileName, GetDebugLogFileName()) == 0))
		return BOOLTRUE;

	memset(szFileName, 0, sizeof(szFileName));
	if(getenv("HOME"))
	{
		snprintf(szFileName, MAX_PATH_LEN, "%s/log/debug", getenv("HOME"));
		if((mkdir(szFileName, S_IRUSR|S_IWUSR|S_IXUSR) == -1)
			&& (errno != EEXIST))
		{
			return BOOLFALSE;
		}		
		snprintf(szFileName, MAX_PATH_LEN, "%s/log/debug/%s", \
			getenv("HOME"), GetDebugLogFileName());
	}
	else
	{
		snprintf(szFileName, MAX_PATH_LEN, "%s/debug","/tmp");
		if((mkdir(szFileName, S_IRUSR|S_IWUSR|S_IXUSR) == -1) 
			&& (errno != EEXIST))
		{
			return BOOLFALSE;
		}				
		snprintf(szFileName, MAX_PATH_LEN, "%s/debug/%s","/tmp", \
			GetDebugLogFileName());	
	}
	
	if(strstr(szFileName, "other-"))
		return BOOLFALSE;
		
	if(pstruDebugLogFile != NULL) 
		fclose(pstruDebugLogFile);
	
	if((access(szFileName,F_OK|W_OK)==0)&&
		(GetFileSize(szFileName) >=LOG_FILE_MAX_SIZE))
    {
    	memset(szShellCmd,0,sizeof(szShellCmd));
    	srand((unsigned)time(NULL));
		snprintf(szShellCmd,sizeof(szShellCmd),"mv %s %s.%d", szFileName, szFileName, rand()%100+1);
 		system(szShellCmd);
    }
    	
	pstruDebugLogFile = fopen(szFileName,"a");
	if(pstruDebugLogFile == NULL)
	{
		return BOOLFALSE;
	}
	if(chmod(szFileName,S_IRUSR|S_IWUSR)==-1)
	{
		return BOOLFALSE;
	}	
	return BOOLTRUE;
}

/**
 * PrintDebugLog() 的线程安全版，加了锁
 * 打印调试日志信息到调试文件
 *
 * pszDebugStr	要打印的头部文件的信息
 * pszFormatStr 要打印的字符串的格式信息
 *
 * Returns 无返回值
 */
VOID PrintDebugLogR(PCSTR pszDebugStr, PCSTR pszFormatStr, ...)
{
	va_list listArg;

	pthread_mutex_lock(&struDebugLogMutex);
	if(access(GetDebugProgName(), F_OK)==0)
	{
		if(InitDebugLogFile() == BOOLTRUE)
		{
			va_start(listArg, pszFormatStr);
			if(nDebugLogHead)
			{
				fprintf(pstruDebugLogFile, "%s%s\n\t", CreateDebugLogHead(), \
					pszDebugStr);
			}
			
			vfprintf(pstruDebugLogFile, pszFormatStr, listArg);
			va_end(listArg);
			fflush(pstruDebugLogFile);
		}
	}
	pthread_mutex_unlock(&struDebugLogMutex);
}

/**
 * PrintHexDebugLog() 的线程安全版本，加了锁
 * 以16进展打印调试日志信息到调试文件
 *
 * pszDebugStr	要打印的头部文件的信息
 * pszPrintBuf 要打印的内容
 * nPrintLen 内容的长度
 *
 * Returns 无返回值
 */
VOID PrintHexDebugLogR(PCSTR pszDebugStr, PCSTR pszPrintBuf, UINT nPrintLen)
{
	register int i,j;
	register int nRowNo;
	UCHAR cTemp;

	pthread_mutex_lock(&struDebugLogMutex);
	if((access(GetDebugProgName(), F_OK) !=0) 
		||(InitDebugLogFile() != BOOLTRUE))
	{
		pthread_mutex_unlock(&struDebugLogMutex);
		return;
	}
		
	if(nDebugLogHead)
	{
		fprintf(pstruDebugLogFile, "%s%s\n", \
			CreateDebugLogHead(), pszDebugStr);
	}
		
	nRowNo = nPrintLen / 16;
	for(i = 0; i < nRowNo; i ++)
	{
		fprintf(pstruDebugLogFile, "%08X | ", i * 16);		
		for(j = 0; j < 16; j ++)
		{
			fprintf(pstruDebugLogFile, "%02X ",
				*(PUCHAR)(pszPrintBuf + i * 16 + j));
		}
		fprintf(pstruDebugLogFile,"| ");
		for(j=0;j<16;j++)
		{
			cTemp = *(PUCHAR)(pszPrintBuf + i * 16 + j);
			cTemp = cTemp > 32 ? cTemp : '.';
			fprintf(pstruDebugLogFile, "%c", cTemp);
		}
		fprintf(pstruDebugLogFile,"\n");
	}
	
	nRowNo = nPrintLen % 16;
	if(nRowNo)
	{
		fprintf(pstruDebugLogFile, "%08X | ", i * 16);
		for(j = 0; j < nRowNo; j ++)
		{
			fprintf(pstruDebugLogFile, "%02X ",
				*(PUCHAR)(pszPrintBuf + i * 16 + j));
		}
		while(j ++ < 16)
			fprintf(pstruDebugLogFile,"   ");
		fprintf(pstruDebugLogFile,"| ");
		for(j = 0; j < nRowNo; j ++)
		{
			cTemp = *(PUCHAR)(pszPrintBuf + i * 16 + j);
			cTemp = cTemp > 32 ? cTemp : '.';
			fprintf(pstruDebugLogFile, "%c", cTemp);
			fflush(pstruDebugLogFile);
		}
		while(j ++ < 16)
			fprintf(pstruDebugLogFile, " ");
		fprintf(pstruDebugLogFile, "\n");
	}	
	fflush(pstruDebugLogFile);	

	pthread_mutex_unlock(&struDebugLogMutex);
}

/**
 * SetDebugLogHead
 * 设置是否打印调试日志的头部信息
 *
 * nHeadFlag	1要求打印头部信息 0不要求打印头部信息
 *
 * Returns 无返回值
 */
VOID SetDebugLogHead(int nHeadFlag)
{
	if(nHeadFlag)
		nDebugLogHead = BOOLTRUE;
	else
		nDebugLogHead = BOOLFALSE;
}

/**
 * GetProgName
 * 获得当前运行进程的名称
 *
 *
 * Returns 进程的名称
 */
#ifdef SYSTEM_AIX
PSTR GetProgName()
{
    struct procsinfo    stProcsinfo;
    INT                 n;
    static STR szProgname[MAX_PATH_LEN];
    static BOOL         nInitFlag = FALSE;
    pid_t               pid;
    if(nInitFlag == TRUE)
            return szProgname;
    pid = getpid();
    n=0;
    while (getprocs(&stProcsinfo,sizeof(stProcsinfo),NULL,0,&n,1) > 0)
    {
        if (stProcsinfo.pi_pid == pid)
        {
            nInitFlag = TRUE;
            memset(szProgname,0,sizeof(szProgname));
            strcpy(szProgname,stProcsinfo.pi_comm);
            return szProgname;
        }
    }
    return "other";
}
#else
PSTR GetProgName()
{
	STR szShellCmd[MAX_STR_LEN];
	static STR szProgname[MAX_PATH_LEN];
	static BOOL nInitFlag = FALSE;
	FILE *pstruPipeFile;

	if(nInitFlag == BOOLTRUE)
		return szProgname;
	snprintf(szShellCmd, MAX_STR_LEN, \
		"ps -el | awk ' $4 == \"%d\" { print $14 }'", (INT )getpid());
	pstruPipeFile = popen(szShellCmd, "r");
	if(pstruPipeFile == NULL)
	{
		fprintf(stderr,"popen(%s)\n",szShellCmd);
		return "other";
	}

	memset(szProgname, 0, sizeof(szProgname));
	fgets(szProgname, MAX_PATH_LEN, pstruPipeFile);
	szProgname[strlen(szProgname) - 1] = '\0';
	pclose(pstruPipeFile);
	
	if(strlen(szProgname) < 1)
	{
		fprintf(stderr,"strlen(szProgname)即(%s)(%s) < 1\n",szProgname,szShellCmd);
		strcpy(szProgname, "other");
	}
	else
		nInitFlag = BOOLTRUE;
		
	return szProgname;
}
#endif

