/**
 * 线程池由3个部分组成：任务队列、工作线程、管理线程。
 * 工作线程负责处理任务队列中的任务，管理线程负责动态添加或销毁工作线程
 * 使用者只需要3个操作：创建线程池、往池里添加任务、销毁线程池。
 */
#ifndef __THREADPOOL_H
#define __THREADPOOL_H

typedef struct tagTHRPOOLSTRU THRPOOLSTRU;

/**
 * @desc: 创建线程池
 * @param[in] nMinThrNum: 最小工作线程数
 * @param[in] nMaxThrNum: 最大工作线程数
 * @param[in] nMaxQueueLen: 任务队列长度
 * @param[in] nSoftQueueLen: 策略：当任务队列中待处理的任务数量多于这个阀值时，添加工作线程
 * @param[in] nThrVary: 策略：一次性添加多少工作线程数，比如 10
 * @return: 成功 - 返回线程池的指针；失败 - 返回 NULL
 */
THRPOOLSTRU *CreateThreadPool(unsigned int nMinThrNum, unsigned int nMaxThrNum,
				unsigned int nMaxQueueLen, unsigned int nSoftQueueLen,
				unsigned int nThrVary);

/**
 * @desc: 往线程池中添加一个任务，注意：如果任务队列已满，此函数会被阻塞。
 * @param[in] pstruPool: 线程池
 * @param[in] func: 任务处理函数指针
 * @param[in] arg: 任务处理函数的参数
 * @return: 任务添加成功 - 返回0；失败 - 返回-1
 */
int AddTaskToThreadPool(THRPOOLSTRU *pstruPool, void*(*func)(void *arg), void *arg);

/**
 * @desc: 销毁线程池
 * @param[in] pstruPool: 线程池
 * @return: 销毁成功 - 返回0；失败 - 返回-1
 */
int DestroyThreadPool(THRPOOLSTRU *pstruPool);

/**
 * @desc: 获取所有工作线程总数
 * @param[in] pstruPool: 线程池
 * @return: 所有工作线程总数
 */
int GetAllThreadNum(THRPOOLSTRU *pstruPool);

/**
 * @desc: 获取有任务在身的工作线程总数
 * @param[in] pstruPool: 线程池
 * @return: 工作线程数量
 */
int GetBusyThreadNum(THRPOOLSTRU *pstruPool);

#endif
