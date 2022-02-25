/***
 * 名称: 网管系统定时任务服务程序
 * 定时任务，每半个小时执行一次
 * 修改记录:
 * 付志刚 2008-11-8 创建
 */

#include <ebdgdl.h>
#include "omcpublic.h"

#define ALM_CELL_ID  		188     //无施主小区信号告警
#define ALM_NCELL_ID		189     //服务小区非施主小区告警

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
	sprintf(szSql, "select count(alg_NeId) as v_count from alm_AlarmLog where alg_NeId = %d and alg_AlarmId= %d and alg_alarmstatusid = 1", 
		nNeId, nAlarmId);
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

/*
 * mos值低于2告警
 */
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
				   " where t.tst_mos < 2 and t.tst_mos > 0 and tst_neid > 0"
				   " and t.tst_mostime >= sysdate - 1/12 "
			);
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
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
 * mos值告警恢复
 */
RESULT ProcessMosValueAlarmComeBack()
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	STR szSql2[MAX_SQL_LEN];
	CURSORSTRU struCursor2;
	INT nAlarmLogId;
	INT nSeperateNum, i, nNeId;
	STR szNeId[MAX_BUFFER_LEN];
	PSTR pszNeIdStr[MAX_SEPERATE_NUM];
	
	sprintf(szSql, "select t.tst_neid from sm_pesqmoslog t where t.tst_mostime >= sysdate - 1/12 and t.tst_mos >= 2  and rownum = 1");
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		CloseDatabase();
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		sleep(60);
		return EXCEPTION;
	}
	if (FetchCursor(&struCursor) != NORMAL)
	{
		FreeCursor(&struCursor);
		return EXCEPTION;
	}
	FreeCursor(&struCursor);
	
	/*
	 * 处理MOS值低告警, 告警id号,162恢复功能
	 */
	memset(szNeId, 0, sizeof(szNeId));
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select t.alg_alarmlogid, t.alg_neid from alm_alarmlog t where t.alg_alarmstatusid < 4 and t.alg_alarmid = 162 and alg_neid !=0 ");
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		CloseDatabase();
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while (FetchCursor(&struCursor) == NORMAL)
	{
		nAlarmLogId = atoi(GetTableFieldValue(&struCursor, "alg_alarmlogid"));
		nNeId = atoi(GetTableFieldValue(&struCursor, "alg_neid"));
		
		memset(szSql2, 0, sizeof(szSql2));
		//sprintf(szSql2, "select t.tst_neid from sm_pesqmoslog t where t.tst_mostime >= sysdate - 1/12 and t.tst_neid = %d and t.tst_mos >= 2 ", nNeId);
		sprintf(szSql2, "select t.tst_neid from sm_pesqmoslog t where t.tst_neid = %d and t.tst_mos >= 2 and to_char(t.tst_mostime, 'yyyy-mm-dd') = to_char(sysdate, 'yyyy-mm-dd')", nNeId);
		
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql2);
		if(SelectTableRecord(szSql2,&struCursor2) != NORMAL)
		{
			CloseDatabase();
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
			return EXCEPTION;
		}
		if (FetchCursor(&struCursor2) == NORMAL)
		{
			 //sprintf(szNeId, "%s%d|", szNeId, nNeId);
			memset(szSql, 0, sizeof(szSql));
    		snprintf(szSql, sizeof(szSql), 
    			"UPDATE alm_AlarmLog SET alg_ClearTime = to_date('%s','yyyy-mm-dd hh24:mi:ss'), alg_AlarmStatusId = 7 ,alg_compressCount = null,alg_compressShowId = null,"
    			"alg_compressState = null WHERE alg_NeId = %d and alg_alarmId = 162 and alg_AlarmStatusId < 4 ",
     			GetSysDateTime(), nNeId);
    		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
			if(ExecuteSQL(szSql) != NORMAL)
			{
				PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
					szSql, GetSQLErrorMessage());
    		    return EXCEPTION;
			}
    		CommitTransaction();
		}
		FreeCursor(&struCursor2);
	}
	FreeCursor(&struCursor);
	//恢复告警状态
	/*
	if (strlen(szNeId) > 0)
	{
		TrimRightChar(szNeId, '|');
		nSeperateNum = SeperateString(szNeId,  '|', pszNeIdStr, 1000);
		for(i=0; i< nSeperateNum; i++)
		{
			if (atoi(pszNeIdStr[i]) == 0)
				continue;
			
		}
	}
	*/   
	    
	return NORMAL;
}

/*
 * 从系统配置表中获取系统信息
 */
RESULT GetSysParameter(PCSTR pszArgSql,PSTR pszArgValue)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;

	sprintf(szSql,"select PAR_KEYVALUE from SYS_PARAMETER where %s", pszArgSql);
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
		if(GetSQLErrorNo()==NO_FOUND_RECORD)
		{
			PrintErrorLog(DBG_HERE,"系统参数[%s]不存在.请配置\n",pszArgSql);
		}
		else
		{
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s] 失败[%s]\n",szSql,GetSQLErrorMessage());
		}
		FreeCursor(&struCursor);
		strcpy(pszArgValue, "");
		return EXCEPTION;
	}
	strcpy(pszArgValue,TrimAllSpace(GetTableFieldValue(&struCursor,"PAR_KEYVALUE")));
	FreeCursor(&struCursor);

	return NORMAL;
}
/*
 *告警压缩
 */
RESULT AlarmCompress(int nNeId, int nAlarmId)
{
	CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	STR szEnable[10];
	INT nCount, nAlarmLogId;
	STR szCondition[100];
	STR szTemp[100];
	//取告警压缩条件
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "par_SectionName ='AlarmCompress' and par_KeyName = 'Enabled'");
	GetSysParameter(szSql, szEnable);
	if (strcmp(szEnable, "TRUE") == 0)
	{
		
		memset(szSql, 0, sizeof(szSql));
		sprintf(szSql,"select count(*) as almcount from  alm_AlarmLog  where alg_NeId = %d and  alg_AlarmId = %d and alg_AlarmStatusId < 4", 
	            nNeId, nAlarmId);
	    PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	    if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    	return EXCEPTION;
	    }
	    if(FetchCursor(&struCursor) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", \
	    				  szSql, GetSQLErrorMessage());
	    	FreeCursor(&struCursor);
	    	return EXCEPTION;
	    }
	    nCount = atoi(GetTableFieldValue(&struCursor, "almcount"));
	    FreeCursor(&struCursor);
	    //必须有2条以上的记录才进行压缩
	    if (nCount > 1)
	    {
	    	memset(szSql, 0, sizeof(szSql));
			sprintf(szSql,"select alg_AlarmLogId from  alm_AlarmLog  where alg_NeId = %d and  alg_AlarmId = %d and alg_AlarmStatusId < 4 and alg_compressState = 1", 
	    	        nNeId, nAlarmId);
	    	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	    	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	    	{
	    		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    					  szSql, GetSQLErrorMessage());
	    		return EXCEPTION;
	    	}
	    	if(FetchCursor(&struCursor) == NORMAL)
	    	{
	    		nAlarmLogId = atoi(GetTableFieldValue(&struCursor, "alg_AlarmLogId"));
	    		FreeCursor(&struCursor);
	    		
				sprintf(szSql, "update alm_AlarmLog set alg_compressCount = %d where alg_AlarmLogId = %d",
	        		 nCount, nAlarmLogId);
	        	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	        	if(ExecuteSQL(szSql) !=NORMAL ) 
	    		{
		    		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		    		return EXCEPTION;
	    		}
	    		
	    	}
	    	else
	    	{
	    		FreeCursor(&struCursor);
	    		//找最早的一条没有做过压缩的记录，把它作为主压缩记录
	    		memset(szSql, 0, sizeof(szSql));
				sprintf(szSql,"select min(alg_AlarmLogId) as alg_AlarmLogId from alm_AlarmLog  where alg_NeId = %d and  alg_AlarmId = %d and alg_AlarmStatusId < 4 ", 
	    	        nNeId, nAlarmId);
	    		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	    		if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	    		{
	    			PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    					  szSql, GetSQLErrorMessage());
	    			return EXCEPTION;
	    		}
	    		if(FetchCursor(&struCursor) != NORMAL)
	    		{
	    			PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n",  szSql);
        			FreeCursor(&struCursor);
	    			return EXCEPTION;
	    		}
	    		nAlarmLogId = atoi(GetTableFieldValue(&struCursor, "alg_AlarmLogId"));
	    		FreeCursor(&struCursor);
	    		
	    		sprintf(szSql, "update alm_AlarmLog set alg_compressCount = %d ,alg_compressState = 1 where alg_AlarmLogId = %d",
	        		 nCount, nAlarmLogId);
	        	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	        	if(ExecuteSQL(szSql) !=NORMAL ) 
	    		{
		    		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		    		return EXCEPTION;
	    		}
	    	}
	    	
	        sprintf(szSql, "update alm_AlarmLog set alg_compressShowId = %d, alg_compressState = 0 where  alg_NeId = %d and  alg_AlarmId = %d and alg_AlarmStatusId < 4 and alg_AlarmLogId <> %d and (alg_compressState <> 0  or alg_compressState is null)",
	        		nAlarmLogId, nNeId, nAlarmId, nAlarmLogId);
	    	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);	
	    	if(ExecuteSQL(szSql) !=NORMAL ) 
	    	{
		    	PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		    	return EXCEPTION;
	    	}
	    	CommitTransaction();
	    	
	    }

	}
	
    return NORMAL;
}


/*
 * 处理服务小区告警
 */
RESULT ProcessCellIdAlarm()
{
	STR szSql[MAX_SQL_LEN];
	STR szSql_alm[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	STR szSql2[MAX_SQL_LEN];
	CURSORSTRU struCursor2;
	int nNeId;
	INT nAlarmLogId, nAlarmId;
	STR szCellId[100], szCID[100];
	
	/*
	 * 每隔30分钟查找施主小区号和服务小区不相同的站点
	 */
	sprintf(szSql, "select a.NE_NEID, a.NE_CELLID, b.epm_curvalue from ne_element a, ne_elementparam b "
				   " where a.ne_neid = b.epm_neid and b.epm_objid = '050C' and a.NE_CELLID <> b.epm_curvalue"
				   " and  b.EPM_UPDATETIME >= sysdate-1/12"
			);
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		sleep(60);
		return EXCEPTION;
	}
	while (FetchCursor(&struCursor) == NORMAL)
	{
		nNeId = atoi(GetTableFieldValue(&struCursor, "NE_NEID"));
		strcpy(szCellId, GetTableFieldValue(&struCursor, "NE_CELLID"));
		strcpy(szCID, GetTableFieldValue(&struCursor, "epm_curvalue"));
		//取流水号
		if(GetDbSerial(&nAlarmLogId, "AlarmLog")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取告警日志流水号错误\n");
			FreeCursor(&struCursor);
			return EXCEPTION;
		}
		/*
		 *  查找是否在邻小区里
		 */
		sprintf(szSql2, "select EPM_NEID from ne_elementparam where EPM_NEID = %d and EPM_OBJID in ('0710', '0711', '0712', '0713', '0714','0715') and  EPM_CURVALUE = '%s'",
			 nNeId, szCellId);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql2);
		if(SelectTableRecord(szSql2,&struCursor2) != NORMAL)
		{
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql2,GetSQLErrorMessage());
			FreeCursor(&struCursor);
			return EXCEPTION;
		}
		
		if (FetchCursor(&struCursor2) == NORMAL)
		{
			//服务小区非施主小区告警
			nAlarmId = ALM_NCELL_ID;
			snprintf(szSql, sizeof(szSql), "insert into  alm_AlarmLog (alg_AlarmLogId,alg_AlarmId,alg_NeId,alg_AlarmTime,alg_AlarmStatusId,alg_AlarmObjId, alg_AlarmInfo)"
        		" VALUES(%d, %d, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 1, 'F909', '施主小区号%s，服务小区号%s')", 
         	nAlarmLogId, nAlarmId, nNeId, GetSysDateTime(), szCellId, szCID);
         	
         	snprintf(szSql_alm, sizeof(szSql_alm), "update alm_collect_report set ALARM_6_TIMES=ALARM_6_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyymmdd')",
				nNeId, GetSystemDate());
			
			if (ExistAlarmLog(nAlarmId, nNeId) == BOOLFALSE) 
			{
	    		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	    		if(ExecuteSQL(szSql) !=NORMAL) 
				{
					PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
					FreeCursor(&struCursor);
					return EXCEPTION;
				}
				CommitTransaction();
			
				//更新告警汇总表
				PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql_alm);
	    		if(ExecuteSQL(szSql_alm) !=NORMAL) 
				{
					PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql_alm,GetSQLErrorMessage());
					FreeCursor(&struCursor);
					return EXCEPTION;
				}
				CommitTransaction();
			}
		}
		FreeCursor(&struCursor2);

	}
	FreeCursor(&struCursor);
	return NORMAL;
}

/*
 * 施主小区告警恢复
 */
RESULT ProcessCellIdAlarmComeBack()
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	STR szSql2[MAX_SQL_LEN];
	CURSORSTRU struCursor2;
	INT nAlarmLogId, nNeId;

	/*
	 * 处理施主小区告警恢复, 告警id号,169, 170恢复功能
	 */
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select t.alg_alarmlogid, t.alg_neid from alm_alarmlog t where t.alg_alarmstatusid < 4 and (t.alg_alarmid = %d or t.alg_alarmid = %d)", ALM_CELL_ID, ALM_NCELL_ID);
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		CloseDatabase();
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while (FetchCursor(&struCursor) == NORMAL)
	{
		nAlarmLogId = atoi(GetTableFieldValue(&struCursor, "alg_alarmlogid"));
		nNeId = atoi(GetTableFieldValue(&struCursor, "alg_neid"));
		
		memset(szSql2, 0, sizeof(szSql2));
		sprintf(szSql2, "select a.NE_NEID, a.NE_CELLID from ne_element a, ne_elementparam b "
				   " where a.ne_neid = b.epm_neid and b.epm_neid=%d and b.epm_objid = '050C' and a.NE_CELLID = b.epm_curvalue"
				   " and  b.EPM_UPDATETIME > sysdate-1/12",  nNeId);
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql2);
		if(SelectTableRecord(szSql2,&struCursor2) != NORMAL)
		{
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql2,GetSQLErrorMessage());
			sleep(60);
			return EXCEPTION;
		}
		if (FetchCursor(&struCursor2) == NORMAL)
		{
			memset(szSql, 0, sizeof(szSql));
    		snprintf(szSql, sizeof(szSql), 
    			"UPDATE alm_AlarmLog SET alg_ClearTime = to_date('%s','yyyy-mm-dd hh24:mi:ss'), alg_AlarmStatusId = 7 ,alg_compressCount = null,alg_compressShowId = null,"
    			"alg_compressState = null WHERE alg_NeId = %d and (alg_alarmId =%d  or alg_alarmId = %d) and alg_AlarmStatusId < 4 ",
     			GetSysDateTime(), nNeId, ALM_CELL_ID, ALM_NCELL_ID);
    		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
			if(ExecuteSQL(szSql) != NORMAL)
			{
				PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
					szSql, GetSQLErrorMessage());
    		    return EXCEPTION;
			}
    		CommitTransaction();
		}
		FreeCursor(&struCursor2);

	}
	FreeCursor(&struCursor);

	    
	return NORMAL;
}

/*
 * 处理没有返回的告警恢复
 */
RESULT ProcessAlarmComeBack()
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	STR szSql2[MAX_SQL_LEN];
	CURSORSTRU struCursor2;
	INT nAlarmId, nNeId;
	STR szObjId[10], szAlarmObjList[100];

	/*
	 * 
	 */
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select t.alg_alarmid, t.alg_neid, a.alm_objid from alm_alarmlog t left join alm_alarm a on t.alg_alarmid = a.alm_alarmid where alg_alarmtime < sysdate -1 and t.alg_alarmstatusid = 1 ");
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		CloseDatabase();
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while (FetchCursor(&struCursor) == NORMAL)
	{
		nAlarmId = atoi(GetTableFieldValue(&struCursor, "alg_alarmid"));
		nNeId = atoi(GetTableFieldValue(&struCursor, "alg_neid"));
		strcpy(szObjId, GetTableFieldValue(&struCursor, "alm_objid"));
		
		memset(szSql2, 0, sizeof(szSql2));
		sprintf(szSql2, "select a.ne_alarmobjlist from ne_element a  "
				   " where a.ne_neid = %d and b.ne_lastupdatetime > sysdate-1/12",  nNeId);
		//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql2);
		if(SelectTableRecord(szSql2,&struCursor2) != NORMAL)
		{
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql2,GetSQLErrorMessage());
			sleep(60);
			return EXCEPTION;
		}
		if (FetchCursor(&struCursor2) == NORMAL)
		{
			strcpy(szAlarmObjList, GetTableFieldValue(&struCursor, "alm_objid"));
			sprintf(szObjId, "%s:0", szObjId);//告警正常
			if (strstr(szAlarmObjList, szObjId) == NULL)
			{
				FreeCursor(&struCursor2);
				continue;
			}
			memset(szSql, 0, sizeof(szSql));
    		snprintf(szSql, sizeof(szSql), 
    			"UPDATE alm_AlarmLog SET alg_ClearTime = to_date('%s','yyyy-mm-dd hh24:mi:ss'), alg_AlarmStatusId = 7 ,alg_compressCount = null,alg_compressShowId = null,"
    			"alg_compressState = null WHERE alg_NeId = %d and alg_alarmId =%d and alg_AlarmStatusId < 4 ",
     			GetSysDateTime(), nNeId,  nAlarmId);
    		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
			if(ExecuteSQL(szSql) != NORMAL)
			{
				PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
					szSql, GetSQLErrorMessage());
    		    return EXCEPTION;
			}
    		CommitTransaction();
		}
		FreeCursor(&struCursor2);

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
	
	//ProcessMosValueAlarm();
	//ProcessMosValueAlarmComeBack();
	ProcessCellIdAlarm();
	ProcessCellIdAlarmComeBack();
	
	CloseDatabase();
	
	fprintf(stderr,"运行结束[%s]\n", GetSysDateTime());
	
	return NORMAL;
}

