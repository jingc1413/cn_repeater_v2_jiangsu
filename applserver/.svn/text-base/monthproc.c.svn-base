/***
 * 名称: 网管系统每月定时任务服务程序 杭分电平强度处理
 *
 * 修改记录:
 * 付志刚 2008-11-8 创建
 */

#include <ebdgdl.h>
#include "omcpublic.h"
#include <mobile2g.h>
#include <applserver.h>


static	STR szServiceName[MAX_STR_LEN];
static	STR szDbName[MAX_STR_LEN];
static	STR szUser[MAX_STR_LEN];
static	STR szPwd[MAX_STR_LEN];




static VOID Usage(PSTR pszProgName)
{
	fprintf(stderr,"网管系统(每月定时服务程序)\n"
		"%s start 启动程序\n"
		"%s stop  关闭程序\n"
		"%s -h    显示帮助信息\n",
		pszProgName,pszProgName,pszProgName);
}

PSTR GetYearMonth()
{
	static STR szNowDateTime[19 + 1];
	time_t struTimeNow;
	struct tm struTmNow;

	if(time(&struTimeNow)==(time_t)(-1))
	{
		PrintErrorLog(DBG_HERE,"得到系统时间错误 %s\n",strerror(errno));
		return NULL;
	}

	memset(szNowDateTime,0,sizeof(szNowDateTime));
	struTmNow=*localtime(&struTimeNow);
	snprintf(szNowDateTime,sizeof(szNowDateTime),"%04d-%02d",
		struTmNow.tm_year+1900,struTmNow.tm_mon+1);

	return szNowDateTime; 
}

INT GetBcchRxLev(INT nNeId)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	PSTR pszSepPropertyStr[MAX_SEPERATE_NUM];
	PSTR pszSepContentStr[MAX_SEPERATE_NUM];
	STR szProperty[1000];
	STR szContent[1000];
	INT nSepPro, nSepCon, i;
	INT nBcchRxLev=0, nCount=0;
	
	
	sprintf(szSql, "select t.qry_property, t.qry_content from man_eleqrylog t where t.qry_eleid = %d and t.qry_issuccess = 1 and t.qry_property like '%%050B%%' and to_char(t.qrt_eventtime, 'yyyy-mm') = '%s'", nNeId, GetYearMonth());
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		CloseDatabase();
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		sleep(60);
		return NORMAL;
	}
	while (FetchCursor(&struCursor) == NORMAL)
	{
		strcpy(szProperty,  GetTableFieldValue(&struCursor, "qry_property"));
		strcpy(szContent,  GetTableFieldValue(&struCursor, "qry_content"));
		
		nSepPro = SeperateString(szProperty, ',',pszSepPropertyStr, MAX_SEPERATE_NUM);
		nSepCon = SeperateString(szContent, ',',pszSepContentStr, MAX_SEPERATE_NUM);
		
		if (nSepPro != nSepCon) continue;
		
		for(i=0; i< nSepPro; i++)
		{
			if (strcmp(pszSepPropertyStr[i], "050B") == 0)
			{
				nBcchRxLev=nBcchRxLev + atoi(pszSepContentStr[i]);
				nCount=nCount+1;
				break;
			}
		}
	}
	FreeCursor(&struCursor);
	
	if (nCount >= 1)
		return nBcchRxLev/nCount;
	else
		return 0;
}

RESULT ProcessBCCH_RxLev()
{ 		
 	
 	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	int nNeId;
	INT nBcchRxLev;
	STR szTemp[20];
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	REPEATER_INFO struRepeater;
	COMMANDHEAD struHead;
	
	memset(pstruXml,0,sizeof(XMLSTRU));
	CreateXml(pstruXml,FALSE,OMC_ROOT_PATH,NULL);
	/*
	 * 处理VVIP站点的BCCH_RxLev数据
	 * 1:普通；2:vip; 3:vvip
	 */
	memset(szSql, 0, sizeof(szSql));
	//sprintf(szSql,"select ne_neid, ne_CommTypeId,ne_RepeaterId, ne_DeviceId, ne_NeTelNum,ne_OmcIp, ne_ProtocoltypeId,ne_ProtocolDeviceTypeId, ne_OtherDeviceId, ne_DeviceModelId, ne_ServerTelNum from ne_Element where ne_devicetypeid = 140 and ne_areaid in (78,225,105,106,107,108,109,110,111,224,229) and ne_devicestatusid = 0 and ne_lastupdatetime > sysdate - 5");
	sprintf(szSql,"select ne_neid, ne_CommTypeId,ne_RepeaterId, ne_DeviceId, ne_NeTelNum,ne_OmcIp, ne_ProtocoltypeId,ne_ProtocolDeviceTypeId, ne_OtherDeviceId, ne_DeviceModelId, ne_ServerTelNum from ne_Element where ne_devicetypeid = 140 ");
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
		InsertInXmlExt(pstruXml,"<omc>/<网元编号>", GetTableFieldValue(&struCursor, "ne_neid"),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<通信方式>", GetTableFieldValue(&struCursor, "ne_CommTypeId"),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<站点编号>", GetTableFieldValue(&struCursor, "ne_RepeaterId"),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<设备编号>", GetTableFieldValue(&struCursor, "ne_DeviceId"),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<站点电话>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_NeTelNum")),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<站点IP>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_OmcIp")),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<协议类型>", GetTableFieldValue(&struCursor, "ne_ProtocoltypeId"),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<设备类型>", GetTableFieldValue(&struCursor, "ne_ProtocolDeviceTypeId"),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<其它标识>", GetTableFieldValue(&struCursor, "ne_OtherDeviceId"),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<设备型号>", GetTableFieldValue(&struCursor, "ne_DeviceModelId"),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		//服务号码
		InsertInXmlExt(pstruXml,"<omc>/<服务号码>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_ServerTelNum")),
		               MODE_AUTOGROW|MODE_UNIQUENAME);
		//命令暂时为:COMMAND_SET
    	InsertInXmlExt(pstruXml,"<omc>/<命令号>", "2", MODE_AUTOGROW|MODE_UNIQUENAME);
    	InsertInXmlExt(pstruXml,"<omc>/<类型>",  "22", MODE_AUTOGROW|MODE_UNIQUENAME);
		/*
		 * 站点等级为快:OMC_QUICK_MSGLEVEL
		 */
		InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
		
		InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  "0707,0708", MODE_AUTOGROW|MODE_UNIQUENAME);
		
		nNeId = atoi(GetTableFieldValue(&struCursor, "ne_neid"));
		nBcchRxLev = GetBcchRxLev(nNeId);
		
		if (nBcchRxLev == 0) continue;
		
		sprintf(szTemp, "%d,%d", nBcchRxLev+15, nBcchRxLev-15);
	    InsertInXmlExt(pstruXml,"<omc>/<监控对象内容>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    
		
		memset(&struHead, 0, sizeof(COMMANDHEAD));
    	memset(&struRepeater, 0, sizeof(REPEATER_INFO));
		struRepeater.nCommType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<通信方式>"));  
		struRepeater.nRepeaterId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<站点编号>")); 
		struRepeater.nDeviceId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<设备编号>")); 
		strcpy(struRepeater.szTelephoneNum, DemandStrInXmlExt(pstruXml,"<omc>/<站点电话>"));
		strcpy(struRepeater.szIP, DemandStrInXmlExt(pstruXml,"<omc>/<站点IP>"));
		struRepeater.nPort = 0;
		
		struRepeater.nProtocolDeviceType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<设备类型>"));
		strcpy(struRepeater.szSpecialCode, DemandStrInXmlExt(pstruXml,"<omc>/<其它标识>"));
		strcpy(struRepeater.szReserve, DemandStrInXmlExt(pstruXml,"<omc>/<设备型号>"));
		strcpy(struRepeater.szNetCenter, DemandStrInXmlExt(pstruXml,"<omc>/<服务号码>"));
		
		struHead.nProtocolType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<协议类型>"));
		struHead.nCommandCode = COMMAND_SET;
	    	
	    SetElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
	    SaveToMsgQueue(pstruXml);
		SaveEleSetLog(pstruXml);

	}
	FreeCursor(&struCursor);
	
	DeleteXml(pstruXml);
	
	return NORMAL;
	    
}


/*
 * 主函数
 */
RESULT main(INT argc,PSTR argv[])
{
	STR szTemp[MAX_STR_LEN];
	
	fprintf(stderr,"\t欢迎使用网管系统(每月定时任务服务)\n");

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
	InitMapObjectCache();
	
	ProcessBCCH_RxLev();
	
	CloseDatabase();
	
	fprintf(stderr,"运行结束[%s]\n", GetSysDateTime());
	
	return NORMAL;
}

