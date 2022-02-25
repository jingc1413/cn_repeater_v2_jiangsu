/***
 * 名称: 网管系统告警定时任务服务程序
 * 定时任务，每日执行一次
 * 修改记录:
 * 付志刚 2008-11-8 创建
 */

#include <ebdgdl.h>
#include "omcpublic.h"

static	STR szServiceName[MAX_STR_LEN];
static	STR szDbName[MAX_STR_LEN];
static	STR szUser[MAX_STR_LEN];
static	STR szPwd[MAX_STR_LEN];


static VOID Usage(PSTR pszProgName)
{
	fprintf(stderr,"网管系统(每日告警定时程序)\n"
		"%s start 启动程序\n"
		"%s stop  关闭程序\n"
		"%s -h    显示帮助信息\n",
		pszProgName,pszProgName,pszProgName);
}

/*
 * 主函数
 */
RESULT main(INT argc,PSTR argv[])
{
	STR szTemp[MAX_STR_LEN];
	
	fprintf(stderr,"\t欢迎使用网管系统(每日告警定时程序)\n");

	if(argc!=2)
	{
		Usage(argv[0]);
		return EXCEPTION;
	}
	if(strcmp(argv[1],"stop")==0)
	{
	    sprintf(szTemp, "clearprg %s", argv[0]);
		system(szTemp);
		//StopPrg(argv[0]);
		return NORMAL;
	}
	if(strcmp(argv[1],"start")!=0)
	{
		Usage(argv[0]);
		return NORMAL;
	}

	if(TestPrgStat(argv[0])==NORMAL)
	{
		fprintf(stderr,"每日告警定时程序进程已经启动\n");
		return EXCEPTION;
	}
	
	if(GetDatabaseArg(szServiceName,szDbName,szUser,szPwd)!=NORMAL)
	{
		strcpy(szServiceName,"omc");
		strcpy(szDbName,"omc");
		strcpy(szUser,"omc");
		strcpy(szPwd,"omc");
	}
	
	if(OpenDatabase(szServiceName,szDbName,szUser,szPwd)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"打开数据库错误 [%s]\n",GetSQLErrorMessage());
		return EXCEPTION;
	}
	
	ProcessAlarmYJ();
	
	CloseDatabase();
	
	fprintf(stderr,"运行结束[%s]\n", GetSysDateTime());
	
	return NORMAL;
}


