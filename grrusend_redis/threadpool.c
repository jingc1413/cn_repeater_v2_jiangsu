#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "threadpool.h"
#include "utils.h"
#include "log_r.h"

#define DEFAULT_TIME 1 /* 管理线程每隔1秒检查一次任务队列，并决定添加或销毁工作线程 */

typedef struct tagTASKSTRU
{
	void *(*func)(void *);
	void *arg;
} TASKSTRU;

struct tagTHRPOOLSTRU
{
	pthread_mutex_t lock; /* 线程池的互斥锁 */
	pthread_cond_t QueueNotFull; /* 用户往池里添加任务时，如果池已经满了，就会阻塞，直到被唤醒 */
	pthread_cond_t QueueNotEmpty; /* 任务队列没有任务时，工作线程会阻塞，直到被唤醒（有任务了，或者需要被销毁了） */
	pthread_t *Threads; /* 数组，保存所有工作线程的 thid */
	pthread_t AdjusterThid; /* 管理线程的 thid */
	TASKSTRU *pstruTaskQueue; /* 任务队列 */
	int nMinThrNum; /* 最小工作线程数 */
	int nMaxThrNum; /* 最大工作线程数 */
	int nLiveThrNum; /* 现存的工作线程数 */
	int nBusyThrNum; /* 存在处理任务的工作线程数，如果这个值比上面的值小很多，就要考虑销毁一些工作线程了 */
	int nWaitExitThrNum; /* 需要被销毁的工作线程数 */
	int nThrVary; /* 管理线程一次性添加的工作线程数 */
	int nQueueFront; /* 当前需要被处理的任务索引 */
	int nQueueRear; /* 任务队列中最后一个任务之后的索引，添加任务时就往这里塞 */
	int nQueueSize; /* 任务队列中的任务个数 */
	int nQueueSoftSize; /* 当任务队列中待处理的任务多于这个值时，管理线程就会添加工作线程 */
	int nQueueMaxSize; /* 任务队列最大长度，绝对，绝对不能让任务数量超过这个值 */
	int nShutdown; /* 销毁线程池时，置1 */
};

static int IsThreadAlive(pthread_t thid)
{
	int nRet;

	nRet = pthread_kill(thid, 0); /* 0，保留信号，这里用来检查线程是否存活 */
	if (nRet == ESRCH) /* No such process */
		return 0;

	return 1;
}

static int FreeThreadPool(THRPOOLSTRU *pstruPool)
{
	if (pstruPool == NULL)
		return 0;

	if (pstruPool->pstruTaskQueue)
		free(pstruPool->pstruTaskQueue);

	if (pstruPool->Threads)
	{
		free(pstruPool->Threads);
		pthread_mutex_lock(&pstruPool->lock);
		pthread_mutex_destroy(&pstruPool->lock);
		pthread_cond_destroy(&pstruPool->QueueNotEmpty);
		pthread_cond_destroy(&pstruPool->QueueNotFull);
	}

	free(pstruPool);

	return 0;
}

/**
 * 工作线程，处理任务队列上的任务
 */
static void *WorkerThread(void *ThreadPool)
{
	int nNeedBroadcast = 0;
	TASKSTRU struTask;
	THRPOOLSTRU *pstruPool = (THRPOOLSTRU *)ThreadPool;
    pthread_t ThreadId = pthread_self();
 
	while (1)
	{
		pthread_mutex_lock(&(pstruPool->lock));

		while ((pstruPool->nQueueSize == 0) && (!pstruPool->nShutdown))
		{ /* 任务队列中没有任务 */
			pthread_cond_wait(&pstruPool->QueueNotEmpty, &pstruPool->lock);

			if (pstruPool->nWaitExitThrNum > 0)
			{ /* 需要销毁几个工作线程 */
				pstruPool->nWaitExitThrNum--;
				if (pstruPool->nLiveThrNum > pstruPool->nMinThrNum)
				{ /* 保证存活的线程不能少于最低数 */
					/* 唯一要做的，就是抹去自己存在的记录，以及释放锁，
					 * 所有后事，像释放内存啦，自有人妥善处理，无需操心 */
					pstruPool->nLiveThrNum--;
					pthread_mutex_unlock(&(pstruPool->lock));
					pthread_exit(NULL);
				}
			}
		}

		if (pstruPool->nShutdown)
		{
			pthread_mutex_unlock(&(pstruPool->lock));
			pthread_exit(NULL);
		}

		/* 如果队列是满的，等有空位置出现时，就要通知其它工作线程 */
		if (pstruPool->nQueueSize == pstruPool->nQueueMaxSize)
			nNeedBroadcast = 1;

		/* 从任务队列获取任务 */
		struTask.func = pstruPool->pstruTaskQueue[pstruPool->nQueueFront].func;
		/* 这里不将任务移除，而是留到 threadpool_add 函数释放其指向的内存 */
		struTask.arg = pstruPool->pstruTaskQueue[pstruPool->nQueueFront].arg;
		pstruPool->nQueueFront = (pstruPool->nQueueFront + 1) % pstruPool->nQueueMaxSize;
		pstruPool->nQueueSize--;

		if (nNeedBroadcast) /* 通知全世界，任务队列有位置空出来了 */
		{
			pthread_cond_broadcast(&pstruPool->QueueNotFull);
		}


		pthread_mutex_unlock(&(pstruPool->lock));

		pthread_mutex_lock(&(pstruPool->lock));
		pstruPool->nBusyThrNum++;
		pthread_mutex_unlock(&(pstruPool->lock));
        PrintDebugLogR(DBG_HERE,"[%X] Start to work\n", ThreadId);
		(*(struTask.func))(struTask.arg); /* 工作线程开工，可能耗时会很长 */
        PrintDebugLogR(DBG_HERE,"[%X] Work complete\n", ThreadId);
		pthread_mutex_lock(&(pstruPool->lock));
		pstruPool->nBusyThrNum--;
		pthread_mutex_unlock(&(pstruPool->lock));
	}

	pthread_exit(NULL);
	return NULL;
}

/**
 * 管理者线程，根据任务队列的状态来决定添加、销毁工作线程
 */
static void *AdjusterThread(void *ThreadPool)
{
	int nThrVary = 0, nAdd, i;
	THRPOOLSTRU *pstruPool = (THRPOOLSTRU *)ThreadPool;

	while (!pstruPool->nShutdown)
	{
		sleep(DEFAULT_TIME); /* 定时睡眠 */

		pthread_mutex_lock(&(pstruPool->lock));
		/* 如果任务队列中的任务数量大过设定的阀值，
		 * 并且现存的工作线程少于最大工作线程数，就添加一组工作线程 */
		if (pstruPool->nQueueSize >= pstruPool->nQueueSoftSize
			&& pstruPool->nLiveThrNum < pstruPool->nMaxThrNum)
		{
			nAdd = 0;
			for (i = 0; i < pstruPool->nMaxThrNum; i++)
			{
				/* 一次最多创建 nThrVary 条工作线程 */
				if (nAdd >= pstruPool->nThrVary)
					break;
				/* 现存工作线程不能超过最大工作线程数 */
				if (pstruPool->nLiveThrNum >= pstruPool->nMaxThrNum)
					break;

				if (pstruPool->Threads[i] == 0 || !IsThreadAlive(pstruPool->Threads[i]))
				{
					pthread_create(&(pstruPool->Threads[i]), NULL, WorkerThread, (void *)pstruPool);
					nAdd++;
					pstruPool->nLiveThrNum++;
				}
			}
		}
		pthread_mutex_unlock(&(pstruPool->lock));

		pthread_mutex_lock(&(pstruPool->lock));
		/* 如果在干活的线程远远少于存活的线程，并且存活线程数多于最少需求量，
		 * 那就删除一些工作线程 */
		if ((pstruPool->nBusyThrNum * 2) < pstruPool->nLiveThrNum
			&& pstruPool->nLiveThrNum > pstruPool->nMinThrNum)
		{
			pstruPool->nWaitExitThrNum = pstruPool->nThrVary;
			nThrVary = pstruPool->nThrVary;
		}
		pthread_mutex_unlock(&(pstruPool->lock));
		if (nThrVary)
		{
			for (i = 0; i < nThrVary; i++)
			{ /* 有新任务了，由于没有任务而阻塞的线程，赶紧醒来然后去死吧 */
				pthread_cond_signal(&(pstruPool->QueueNotEmpty));
			}
		}
	}
	return NULL;
}

THRPOOLSTRU*
CreateThreadPool(unsigned int nMinThrNum, unsigned int nMaxThrNum,
				unsigned int nMaxQueueLen, unsigned int nSoftQueueLen,
				unsigned int nThrVary)
{
	int i;
	THRPOOLSTRU *pstruPool = NULL;

	if (!nMinThrNum || !nMaxThrNum || !nMaxQueueLen || !nSoftQueueLen || !nThrVary)
	{
		//PrintErrorLog(DBG_HERE, "创建线程池失败[输入参数有误]\n");
		return NULL;
	}

	/* 使用 do..while(0)语句，可以实现 goto 的功能 */
	do{
		pstruPool = (THRPOOLSTRU *)calloc(1, sizeof(THRPOOLSTRU));
		if (!pstruPool)
		{
			//PrintErrorLog(DBG_HERE, "创建线程池失败[分配内存失败]\n");
			break;
		}

		pstruPool->nMinThrNum = nMinThrNum;
		pstruPool->nMaxThrNum = nMaxThrNum;
		pstruPool->nQueueMaxSize = nMaxQueueLen;
		pstruPool->nQueueSoftSize = nSoftQueueLen;
		pstruPool->nThrVary = nThrVary;

		pstruPool->Threads = (pthread_t *)calloc(1, sizeof(pthread_t) * nMaxThrNum);
		if (pstruPool->Threads == NULL)
		{
			//PrintErrorLog(DBG_HERE, "创建线程池失败[分配内存失败]\n");
			break;
		}

		pstruPool->pstruTaskQueue = (TASKSTRU *)calloc(1, sizeof(TASKSTRU) * nMaxQueueLen);
		if (pstruPool->pstruTaskQueue == NULL)
		{
			//PrintErrorLog(DBG_HERE, "创建线程池失败[分配内存失败]\n");
			break;
		}

		if (pthread_mutex_init(&pstruPool->lock, NULL) ||
			pthread_cond_init(&pstruPool->QueueNotEmpty, NULL) ||
			pthread_cond_init(&pstruPool->QueueNotFull, NULL))
		{
			//PrintErrorLog(DBG_HERE, "创建线程池失败[初始化锁失败]\n");
			break;
		}

		/* 创建基本数量的工作线程 */
		for (i = 0; i < nMinThrNum; i++)
		{
			pthread_create(&(pstruPool->Threads[i]), NULL, WorkerThread, (void *)pstruPool);
			pstruPool->nLiveThrNum++;
		}
		pthread_create(&(pstruPool->AdjusterThid), NULL, AdjusterThread, (void *)pstruPool);
		return pstruPool;
	} while(0);

	FreeThreadPool(pstruPool);
	return NULL;
}

/**
 * 往池里添加任务
 */
int AddTaskToThreadPool(THRPOOLSTRU *pstruPool, void*(*func)(void *arg), void *arg)
{
    pthread_t ThreadId = pthread_self();

	if (!pstruPool || !func || !arg)
	{
		//PrintErrorLog(DBG_HERE, "添加任务失败[输入参数有误]\n");
		return -1;
	}

	pthread_mutex_lock(&pstruPool->lock);

    PrintDebugLogR(DBG_HERE,"[%X] Waiting for queue\n", ThreadId);
	while ((pstruPool->nQueueSize >= pstruPool->nQueueMaxSize) && (!pstruPool->nShutdown))
	{
		/* 任务队列满了，就阻塞在这里，直到任务队列出现空闲位置 */
		pthread_cond_wait(&pstruPool->QueueNotFull, &pstruPool->lock);
	}
    PrintDebugLogR(DBG_HERE,"[%X] Catch a queue\n", ThreadId);
	if (pstruPool->nShutdown)
	{
        PrintDebugLogR(DBG_HERE,"[%X] pool already shutdown\n", ThreadId);
		pthread_mutex_unlock(&pstruPool->lock);
		//PrintErrorLog(DBG_HERE, "添加任务失败[线程池已销毁]\n");
		return -1;
	}

	/* 开始往任务队列添加任务.
	 * 不需要释放 arg 指针指向的内存，这段内存必须交给调用者去释放，因为：
	 * 1. 可能 arg 指向一串链表，这里不可能释放干净；
	 * 2. 可能 arg 指向一大块内存中的一个地址：
	 *    int *a = malloc(sizeof(a)*10), arg=&a[5]，这里释放内存就会出错
	 * 所以，本代码直接覆盖任务的指针 */
	pstruPool->pstruTaskQueue[pstruPool->nQueueRear].func = func;
	pstruPool->pstruTaskQueue[pstruPool->nQueueRear].arg = arg;
	pstruPool->nQueueRear = (pstruPool->nQueueRear+1) % pstruPool->nQueueMaxSize;
	pstruPool->nQueueSize++;
    PrintDebugLogR(DBG_HERE,"[%X] nQueueRear: %d; nQueueSize: %d\n", ThreadId, pstruPool->nQueueRear, pstruPool->nQueueSize);

	/* 通知全世界，任务队列上有任务了 */
	pthread_mutex_unlock(&pstruPool->lock);
	//pthread_cond_signal(&pstruPool->QueueNotEmpty);
	pthread_cond_broadcast(&pstruPool->QueueNotEmpty);

	return 0;
}


int DestroyThreadPool(THRPOOLSTRU *pstruPool)
{
	int i;

	if (!pstruPool)
	{
		//PrintErrorLog(DBG_HERE, "销毁线程池失败[输入参数有误]\n");
		return -1;
	}

	pstruPool->nShutdown = 1;
	/* 等待管理者线程退出 */
	pthread_join(pstruPool->AdjusterThid, NULL);

	/* 唤醒所有因空队列而被阻塞的线程，你们可以去死了 */
	pthread_cond_broadcast(&(pstruPool->QueueNotEmpty));
	for (i = 0; i < pstruPool->nMinThrNum; i++)
	{ /* 等待所有工作线程退出 */
		pthread_join(pstruPool->Threads[i], NULL);
	}
	FreeThreadPool(pstruPool);
	return 0;
}

int GetAllThreadNum(THRPOOLSTRU *pstruPool)
{
	int nAll = 0;
	pthread_mutex_lock(&pstruPool->lock);
	nAll = pstruPool->nLiveThrNum;
	pthread_mutex_unlock(&pstruPool->lock);
	return nAll;
}

int GetBusyThreadNum(THRPOOLSTRU *pstruPool)
{
	int nBusy = 0;
	pthread_mutex_lock(&pstruPool->lock);
	nBusy = pstruPool->nBusyThrNum;
	pthread_mutex_unlock(&(pstruPool->lock));
	return nBusy;
}
