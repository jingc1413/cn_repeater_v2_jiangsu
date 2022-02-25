/*
 * 名称: 错误日志开发函数定义 - 可重入版
 * 
 * 修改记录:
 * 2008-08-20 - 付志刚 建立
 * 2015-01-04 - 陈锦涛 修改成可重入版
 */
#include "ebdgdl.h"
//#include <stdio.h>
//#include <error.h>
#include <pthread.h>
#include "log.h"

/* 
 * 用来记录调试信息文件
 */
static FILE *pstruErrorLogFile = NULL;
static pthread_mutex_t struErrorLogMutex = PTHREAD_MUTEX_INITIALIZER; /* 锁错误日志 */

/*
 * 得到日志头 
 * 函数名最后的'R'表示可重入(re-entrant)
 * @para[out] pszHead: 保存日志头
 * @para[in] nLen: 指定日志头长度
 * @return: 成功 - 返回0；失败 - 返回-1
 */
static INT CreateErrorLogHeadR(PSTR pszHead, INT nLen)
{
    struct tm struTmNow;
	time_t struTimeNow;
	STR szHead[MAX_STR_LEN] = {0};

	if (!pszHead)
		return -1;

	struct timeval struTmVal;

	if(time(&struTimeNow) == (time_t)(-1))
		return -1;
	gettimeofday(&struTmVal, NULL);

	localtime_r(&struTimeNow, &struTmNow);
	snprintf(szHead, sizeof(szHead), "[%04d-%02d-%02d][%02d:%02d:%02d.%06d][%ld]",
		struTmNow.tm_year+1900,struTmNow.tm_mon+1,struTmNow.tm_mday,struTmNow.tm_hour,
		struTmNow.tm_min,struTmNow.tm_sec,(INT)struTmVal.tv_usec,(LONG)getpid());

	strncpy(pszHead, szHead, nLen);
	return 0;
}

static INT GetProgNameR(PSTR pszProgName, INT nLen)
{
	INT nFindIt = 0;
	STR szProgName[MAX_PATH_LEN] = {0};
	STR szTmp[MAX_PATH_LEN] = {0};
	FILE *pstruFile;

	snprintf(szTmp, sizeof (szTmp), "/proc/%d/status", (INT)getpid());

	pstruFile = fopen(szTmp, "r");
	if (!pstruFile)
	{
		fprintf(stderr, "打开文件失败[%s][%s]\n",szTmp, strerror(errno));
		return -1;
	}

	while (fgets(szTmp, sizeof (szTmp), pstruFile))
	{
		szTmp[strlen(szTmp)-1] = '\0';
		if (strstr(szTmp, "Name:"))
		{
			nFindIt = 1;
			break;
		}
	}
	fclose(pstruFile);

	if (!nFindIt)
		return -1;

	sscanf(szTmp, "%*s%s", szProgName);
	strncpy(pszProgName, szProgName, nLen);
	return 0;
}

/*	
 * 得到日志文件的名称 
 */
static INT GetErrorLogFileNameR(PSTR pszFileName, INT nLen)
{
	struct tm struTmNow;
	time_t struTimeNow;
	STR szFileName[MAX_PATH_LEN] = {0};
	STR szProgName[MAX_PATH_LEN] = {0};
	INT nRet;

	if (!pszFileName)
		return -1;

	nRet = GetProgNameR(szProgName, sizeof (szProgName));
	if (nRet < 0)
		return -1;

	if(time(&struTimeNow) == (time_t)(-1))
		return -1;
	
	localtime_r(&struTimeNow, &struTmNow);
	snprintf(szFileName, sizeof(szFileName), "%s-%04d%02d%02d.err",
		szProgName, struTmNow.tm_year + 1900,
		struTmNow.tm_mon + 1, struTmNow.tm_mday);			

	strncpy(pszFileName, szFileName, nLen);
	return 0;
}

static INT GetFileSizeR(PSTR pszFileName)
{
	struct stat struFileStat;

	if(stat(pszFileName,&struFileStat)!=0)
	{
		fprintf(stderr, "获得文件信息错误[%s][%s]\n",pszFileName,strerror(errno));
		return -1;
	}

	return struFileStat.st_size;
}

/*
 * 生成初始化的日志文件	
 */
static INT InitErrorLogFileR()
{
	STR szFileName[MAX_PATH_LEN];
	STR szPathName[MAX_PATH_LEN];
	STR szTmp[MAX_PATH_LEN];
	INT nRet;

	nRet = GetErrorLogFileNameR(szFileName, sizeof (szFileName));
	if (nRet < 0)
	{
		fprintf(stderr, "无法得到错误日志的文件名称\n");
		return -1;
	}
	if(getenv("HOME"))
		snprintf(szPathName, MAX_PATH_LEN, "%s/log/error/%s", getenv("HOME"), szFileName);
	else
		snprintf(szPathName, MAX_PATH_LEN, "/tmp/err/%s", szFileName);	

	//pthread_mutex_lock(&struErrorLogMutex);
   	if((pstruErrorLogFile != NULL)
   		&& (access(szPathName, F_OK|W_OK) == 0)
   		&& (GetFileSizeR(szPathName) < LOG_FILE_MAX_SIZE))
	{ /* 一切正常 */
		//pthread_mutex_unlock(&struErrorLogMutex);
		return 0;
	}

	/* 有异常，先关闭文件描述符 */
	if(pstruErrorLogFile != NULL) 
	{
		fclose(pstruErrorLogFile);
		pstruErrorLogFile = NULL;
	}

	if((access(szPathName,F_OK|W_OK)==0)&&
		(GetFileSizeR(szPathName) >= LOG_FILE_MAX_SIZE))
    { /* 日志文件过大，重命名 */
		snprintf(szTmp, sizeof (szTmp), "%s.%d", szPathName, getpid());
		if (rename(szPathName, szTmp) != 0)
		{
			fprintf(stderr, "错误日志文件重命名出错[%s]", strerror(errno));
			//pthread_mutex_unlock(&struErrorLogMutex);
			return -1;
		}
    }

	/* 正式打开正常的日志文件 */
	pstruErrorLogFile = fopen(szPathName, "a");
	if(pstruErrorLogFile == NULL)
	{
		fprintf(stderr, "调用fopen出错[%s]\n", strerror(errno));
		//pthread_mutex_unlock(&struErrorLogMutex);
		return -1;
	}

	if(chmod(szPathName, S_IRUSR|S_IWUSR)==-1)
	{
		fprintf(stderr, "调用chmod出错[%s]\n", strerror(errno));
		//pthread_mutex_unlock(&struErrorLogMutex);
		return -1;
	}
	//pthread_mutex_unlock(&struErrorLogMutex);

	return 0;
}

/**
 * PrintErrorLog() 的可重入版本
 * 打印错误日志信息到调试文件
 *
 * pszDebugStr	要打印的头部文件的信息
 * pszFormatStr 要打印的字符串的格式信息
 *
 * Returns 无返回值
 */
VOID PrintErrorLogR(PCSTR pszDebugStr, PCSTR pszFormatStr, ...)
{
	va_list listArg;
	STR szHead[MAX_STR_LEN] = {0};
	
	pthread_mutex_lock(&struErrorLogMutex);
	if(InitErrorLogFileR() == 0)
	{
		CreateErrorLogHeadR(szHead, sizeof (szHead));
		va_start(listArg, pszFormatStr);
		fprintf(pstruErrorLogFile, "%s%s\n\t", szHead, pszDebugStr);
		vfprintf(pstruErrorLogFile, pszFormatStr, listArg);
		va_end(listArg);
		fflush(pstruErrorLogFile);
	}
	pthread_mutex_unlock(&struErrorLogMutex);
}

/**
 * SetDebugLogHead() 的可重入版本
 * 设置是否打印错误日志的头部信息
 *
 * nHeadFlag 1要求打印头部信息 0不要求打印头部信息
 *
 * Returns 无返回值
 */
VOID SetErrorLogHeadR(int nHeadFlag)
{
	return;
}
