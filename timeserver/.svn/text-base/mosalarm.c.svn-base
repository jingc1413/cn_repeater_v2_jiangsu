/***
 * 名称: 网管系统定时任务服务程序
 *
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
	fprintf(stderr,"网管系统(定时服务程序)\n"
		"%s start 启动程序\n"
		"%s stop  关闭程序\n"
		"%s -h    显示帮助信息\n",
		pszProgName,pszProgName,pszProgName);
}


/*
 * 取自增序号
 */
RESULT GetDbSerial(PINT pnSerial,PCSTR pszItemName)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;

	
	sprintf(szSql,"select ide_ItemValue from sys_Identity where ide_Item='%s' for update wait 10",pszItemName);
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		*pnSerial=0;
		PrintErrorLog(DBG_HERE,"取自增序列号时出错 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor)!=NORMAL)
	{
		FreeCursor(&struCursor);
		sprintf(szSql,"insert into sys_Identity (ide_Item, ide_ItemValue) values('%s',1)",pszItemName);
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
		if(ExecuteSQL(szSql)!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
			return EXCEPTION;
		}
		*pnSerial=1;
		CommitTransaction();
		return NORMAL;
    }
	*pnSerial=atoi(GetTableFieldValue(&struCursor,"ide_ItemValue"))+1;
	FreeCursor(&struCursor);
	
	sprintf(szSql,"UPDATE sys_Identity SET ide_ItemValue=ide_ItemValue + 1 WHERE ide_Item='%s'",pszItemName);
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"取自增序列号时出错 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();
	
    //整数最大数为2147483645
    if (*pnSerial >= 2147483645)
    {
        sprintf(szSql,"UPDATE sys_Identity SET ide_ItemValue= 1 WHERE ide_Item='%s'",pszItemName);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	    if(ExecuteSQL(szSql)!=NORMAL)
	    {
		    PrintErrorLog(DBG_HERE,"取自增序列号时出错 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		    return EXCEPTION;
	    }
	    CommitTransaction();
    }
    
	return NORMAL;
}


BOOL ExistAlarmLog(int nAlarmId, int nNeId)
{
	CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	INT nCount;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select count(alg_NeId) as v_count from alm_AlarmLog where alg_NeId = %d and alg_AlarmId= %d and alg_AlarmTime>to_date( '%s 000000','yyyymmdd hh24miss') and alg_AlarmTime<to_date( '%s 235959','yyyymmdd hh24miss')", 
		nNeId, nAlarmId, GetSystemDate(), GetSystemDate());
	//PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    FreeCursor(&struCursor);
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    return EXCEPTION;

	}
	nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
	FreeCursor(&struCursor);
	if (nCount > 0)
	    return BOOLTRUE;
	else
	    return BOOLFALSE;
}

RESULT ProcessMosValueAlarm()
{ 		
 	
 	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	STR szSql2[MAX_SQL_LEN];
	CURSORSTRU struCursor2;
	int nNeId;
	INT nAlarmLogId;
	/*
	 * 处理MOS值低告警
	 */
	sprintf(szSql, "select t.tst_neid from sm_pesqmoslog t "
				   " where t.tst_mos < 2 "
				   " and to_char(t.tst_eventtime, 'yyyy-mm-dd') = to_char(sysdate-1, 'yyyy-mm-dd') "
			);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		CloseDatabase();
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		sleep(60);
		return EXCEPTION;
	}
	while (FetchCursor(&struCursor) == NORMAL)
	{
		nNeId = atoi(GetTableFieldValue(&struCursor, "tst_neid"));
		sprintf(szSql2, "select ne_signaltype from ne_element where ne_neid = %d", nNeId);
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql2);
		if(SelectTableRecord(szSql2,&struCursor2) != NORMAL)
		{
			CloseDatabase();
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql2,GetSQLErrorMessage());
			sleep(60);
			return EXCEPTION;
		}
		if (FetchCursor(&struCursor2) != NORMAL)
		{
			FreeCursor(&struCursor2);
			continue;
		}
		if (atoi(GetTableFieldValue(&struCursor2, "ne_signaltype")) != 0)//如果不是网络监控，不告警
		{
			FreeCursor(&struCursor2);
			continue;
		}
		FreeCursor(&struCursor2);
		/*
		 *  保存告警日志
		 */
		if (ExistAlarmLog(162, nNeId) == BOOLFALSE)
		{
			
			if(GetDbSerial(&nAlarmLogId, "AlarmLog")!=NORMAL)
			{
				PrintErrorLog(DBG_HERE,"获取告警日志流水号错误\n");
				FreeCursor(&struCursor);
				return EXCEPTION;
			}
			snprintf(szSql, sizeof(szSql), "insert into  alm_AlarmLog (alg_AlarmLogId,alg_AlarmId,alg_NeId,alg_AlarmTime,alg_AlarmStatusId,alg_AlarmObjId)"
        	 " VALUES(%d, 162, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 1, 'F905')", 
        	  nAlarmLogId, nNeId, GetSysDateTime());
    		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
    		if(ExecuteSQL(szSql) !=NORMAL) 
			{
				PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
				return EXCEPTION;
			}
			CommitTransaction();
		}
	}
	FreeCursor(&struCursor);
	
	return NORMAL;
	    
}


/*
 * 主函数
 */
RESULT main(INT argc,PSTR argv[])
{
	STR szTemp[MAX_STR_LEN];
	
	fprintf(stderr,"\t欢迎使用网管系统(定时任务服务)\n");

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
		fprintf(stderr,"定时任务服务进程已经启动\n");
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
	
	ProcessMosValueAlarm();
	
	CloseDatabase();
	
	fprintf(stderr,"运行结束[%s]\n", GetSysDateTime());
	
	return NORMAL;
}

