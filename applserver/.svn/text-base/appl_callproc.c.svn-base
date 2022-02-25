/*
 * 名称: 应用服务调用存储函数
 *
 * 修改记录:
 * 2008-10-10 - 付志刚 建立
 */

#include <ebdgdl.h>



/*
 * 调用存储过程:InserTeleLog
 */
RESULT CallInserTeleLog(int v_logid, int v_neid, int v_type)
{
    STR szSql[MAX_SQL_LEN];
    
    sprintf(szSql,"call insertelelog (%d, %d, %d)", v_logid, v_neid, v_type);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
    if(ExecuteSQL(szSql) !=NORMAL ) 
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	return NORMAL;
}

/*
 * 调用存储过程:sp_SM_DownDialTask
 */
RESULT CallSMDownDialTask(int nTaskId)
{
    STR szSql[MAX_SQL_LEN];
    
    sprintf(szSql,"call sp_SM_DownDialTask (%d)",  nTaskId);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
    if(ExecuteSQL(szSql) !=NORMAL ) 
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	return NORMAL;
}


/*
 * 调用存储过程:alm_AlarmCompress
 * 2009.3.2 deleted
 */
RESULT CallAlarmCompress(int nAlarmLogId)
{
    STR szSql[MAX_SQL_LEN];
    
    sprintf(szSql,"call AlarmCompress_c (%d)", nAlarmLogId);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
    if(ExecuteSQL(szSql) !=NORMAL ) 
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	return NORMAL;
}


/*
 * 调用存储过程:alm_AlarmFrequency
 *** 2009.3.2 deleted
 */
RESULT CallAlarmFrequency(int nMaintainLogId, int nNeId, PSTR pszHour,PSTR pszCount)
{
    STR szSql[MAX_SQL_LEN];
    
    sprintf(szSql,"call AlarmFrequency_c (%d, %d, %s, %s)", nMaintainLogId, nNeId, pszHour,pszCount );
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
    if(ExecuteSQL(szSql) !=NORMAL ) 
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	return NORMAL;
}

/*
 * 调用存储过程:AlarmComeback
 ** 2009.3.2 deleted
 */
RESULT CallAlarmComeback(int nNeId, PSTR pszAlarmObjId, PSTR pszAlarmTime)
{
    STR szSql[MAX_SQL_LEN];
    
    sprintf(szSql,"call AlarmComeback_c (%d, '%s', '%s')",  nNeId, pszAlarmObjId, pszAlarmTime);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
    if(ExecuteSQL(szSql) !=NORMAL ) 
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	return NORMAL;
}

/*
 * 调用存储过程:ne_UpdateObjList
 * 2009.3.2 deleted
 */
RESULT CallNeUpdateObjList(int nNeId, PSTR pszMapId0009List, int nUpdateWay, PSTR pszQryNumber, PSTR pszProvinceId)
{
    STR szSql[MAX_SQL_LEN];
    
    sprintf(szSql,"call UpdateObjList_c (%d, '%s', %d, '%s', '%s', '')",  nNeId, pszMapId0009List, nUpdateWay, pszQryNumber, pszProvinceId);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
    if(ExecuteSQL(szSql) !=NORMAL ) 
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	return NORMAL;
}

/*
 * 调用存储过程:inserttimeruploadlog
 * 2009.3.2 deleted
 */
RESULT CallInsertTimerUploadLog(int nNeId,  PSTR pszProperty, PSTR pszContent)
{
    STR szSql[MAX_SQL_LEN];
    
    sprintf(szSql,"call inserttimeruploadlog (%d, '%s', '%s', '%s')",  nNeId, GetSysDateTime(), pszProperty, pszContent);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
    if(ExecuteSQL(szSql) !=NORMAL ) 
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	return NORMAL;
}



