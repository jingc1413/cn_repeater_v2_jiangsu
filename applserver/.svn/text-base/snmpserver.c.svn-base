/*
 * 名称:SNMP采集服务器
 *
 * 修改记录:
 * 付志刚 -		2013-5-16 创建
 */
#include <ebdgdl.h>
#include <ebdgml.h>
#include <omcpublic.h>
#include <applserver.h>


/*
 * 全局变量定义
 */
static INT nGprsPort;						/* 本地监听端口 */
//static SOCKINFOSTRU struPeerInfo;

STR	szGprsRespBuffer[100];		/* gprs应答通讯报文 */

STR szService[MAX_STR_LEN];
STR szDbName[MAX_STR_LEN];
STR szUser[MAX_STR_LEN];
STR szPwd[MAX_STR_LEN];
/*
 * 进程池子进程构造函数 
 */
static RESULT StartPoolChildPro(PVOID pArgment)
{

	if(GetDatabaseArg(szService, szDbName, szUser, szPwd) != NORMAL)
	{
		strcpy(szService, "omc");
		strcpy(szDbName, "omc");
		strcpy(szUser, "omc");
		strcpy(szPwd, "omc");
	}

	if(OpenDatabase(szService, szDbName, szUser, szPwd) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "进程池子进程打开数据库发生错误 [%s]\n",
			GetSQLErrorMessage());
		return EXCEPTION;
	}
	
	//PrintDebugLog(DBG_HERE, "进程返回结束\n");
	return NORMAL;
}

/*
 * 通讯进程池子进程析构函数
 */
static RESULT EndPoolChildPro(PVOID pArgment)
{
	
	CloseDatabase();
	//PrintDebugLog(DBG_HERE, "关闭数据库成功\n");

	return NORMAL;
}



/*
 * 使用说明
 */
static VOID Usage(PSTR pszProg)
{
	fprintf(stderr, "覆盖延伸系统(SNMP采集服务程序)版本号V9.0.0\n");
	fprintf(stderr, "%s start 启动程序\n"
					"%s stop 停止程序\n"
					"%s -h 显示本信息\n", pszProg, pszProg, pszProg);
}


/* 
 * 处理Snmp上报交易 
 */
RESULT ProcessSnmpDeliveTrans(INT nSock, PSTR pszCaReqBuffer, INT nLen)
{
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	STR szMsgCont[MAX_BUFFER_LEN];	/*  上报消息内容*/
	STR szPackcd[10];
	SENDPACKAGE struSendPackage;
	PSTR pszObjOidStr[MAX_SEPERATE_NUM];
	STR szMapId[10], szMapData[250], szMapType[20];
	int nObjCount, i,j, nMsgId, nSepCount;
	int nConnStat, nDeviceTypeId;
	STR szRouter[20], szDeviceIp[20];
	int nDevNumTotal;
	
	int nNeId;
	STR szNeName[50], szAlarmObjList[1000], szAlarmTime[20];
	
	memset(pstruXml,0,sizeof(XMLSTRU));
	if(ImportXml(pstruXml,FALSE,pszCaReqBuffer)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"报文[%s]非法\n",pszCaReqBuffer);
		DeleteXml(pstruXml);
		return -1;
	}
	nMsgId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<msgserial>"));
		
	UINT nRepeaterId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<repeaterid>"));
	int nDeviceId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<deviceid>"));
	memset(szMsgCont, 0, sizeof(szMsgCont));
	strcpy(szMsgCont, DemandStrInXmlExt(pstruXml,OMC_MSGCONT_PATH));
	strcpy(szPackcd, DemandStrInXmlExt(pstruXml,"<omc>/<packcd>"));

	
	if (strcmp(szPackcd, "7005") == 0 || strcmp(szPackcd, "7004") == 0)
	{
		nDevNumTotal = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<devnumtotal>"));
		strcpy(szDeviceIp, DemandStrInXmlExt(pstruXml,"<omc>/<deviceip>"));
		nRepeaterId = IpstrToInt(szDeviceIp);
	}
	DeleteXml(pstruXml);
	
	PrintDebugLog(DBG_HERE,"[%u][%s]\n", nRepeaterId, szMsgCont);
	
	memset(&struSendPackage, 0, sizeof(SENDPACKAGE));
	struSendPackage.struRepeater.nRepeaterId = nRepeaterId;
	struSendPackage.struRepeater.nDeviceId = nDeviceId;
	 
	//处理Walk交易
    if (strcmp(szPackcd, "7005") == 0)
    {
		PSTR pszSepStr[MAX_OBJECT_NUM];
		
		nSepCount = SeperateString(szMsgCont, ';', pszSepStr, MAX_SEPERATE_NUM);
		if (nSepCount != nDevNumTotal*5)
		{
			PrintErrorLog(DBG_HERE, "分解消息内容[%s]错误\n", szMsgCont);
			return EXCEPTION; 
		}
		for(j=0;j<nDevNumTotal;j++)
		{
			nDeviceId = atoi(pszSepStr[j*5+1]);
			nDeviceTypeId = atoi(pszSepStr[j*5+2]);
			nConnStat = atoi(pszSepStr[j*5+3]);
			strcpy(szRouter, pszSepStr[j*5+4]);
			SaveToDasList(nRepeaterId, nDeviceId, szRouter, szDeviceIp, nConnStat, nDeviceTypeId);
		}
		struSendPackage.struRepeater.nRepeaterId = nRepeaterId;
		struSendPackage.struRepeater.nCommType = M2G_SNMP_TYPE;
		struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		DecodeQueryDasList((SENDPACKAGE *)&struSendPackage);  	
    	return NORMAL;
    }

    /*
     * 分解消息内容
     */
    nSepCount = SeperateString(szMsgCont,  ';', pszObjOidStr, MAX_SEPERATE_NUM);
    nObjCount = 0;
    for(i=0; i< nSepCount; i++)
	{
		//分割oid
		if (DecodeMapDataFromOid(pszObjOidStr[i], szMapId, szMapData, szMapType)!=NORMAL)
			continue;
		PrintDebugLog(DBG_HERE,"[%s][%s][%s]\n", szMapId, szMapData, szMapType);

		struSendPackage.struMapObjList[nObjCount].cErrorId = '0';
		strcpy(struSendPackage.struMapObjList[nObjCount].szMapId, szMapId);
		if (strcmp(szMapId, "0101") == 0)
			sprintf(struSendPackage.struMapObjList[nObjCount].szMapData, "%d", strHexToInt(szMapData));
		else
			strcpy(struSendPackage.struMapObjList[nObjCount].szMapData, szMapData);
		strcpy(struSendPackage.struMapObjList[nObjCount].szMapType, szMapType);
		nObjCount ++;
	}
	//对象数
	struSendPackage.struHead.nObjectCount= nObjCount;
	
    //处理Trap交易 告警
    if (strcmp(szPackcd, "7004") == 0)
    {
    	 BOOL bAlarmRestore = BOOLFALSE;
		 INT nAlarmTimes=0;
		 STR szTemp[100];
		 
		 if (struSendPackage.struHead.nObjectCount == 0) return NORMAL;
		 	
		 /*
	      *  取告警列表,并取网元NeId保留
	      */
	     if (GetAlarmObjList3(nRepeaterId, nDeviceId, &nNeId, szNeName, szAlarmObjList)!= NORMAL)
	     {
	         DeleteXml(pstruXml);
	         PrintErrorLog(DBG_HERE, "设备号[%d][%d]不存在记录\n", nRepeaterId, nDeviceId);
	         return EXCEPTION;
	     }
	     InsertInXmlExt(pstruXml,"<omc>/<告警列表>",szAlarmObjList,MODE_AUTOGROW|MODE_UNIQUENAME);
	     sprintf(szTemp, "%d", nNeId);
     	 InsertInXmlExt(pstruXml,"<omc>/<网元编号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
         struSendPackage.nNeId = nNeId;
         
         BOOL bNewAlarm = BOOLFALSE;
         bufclr(szAlarmTime);
         for(i=0; i< struSendPackage.struHead.nObjectCount; i++)
         {
         	 
             if (strcmp(struSendPackage.struMapObjList[i].szMapData, "1") == 0)//告警处理
             {
             	 if (nAlarmTimes++ >= 1)
             	 {
             	 	 if (DealNewAlarm(pstruXml) == NORMAL)
             	 	 //频繁告警
             	 	 SaveToMaintainLog("New Alarm", "",  &struSendPackage);
             	 	 
             	 }
             	 InsertInXmlExt(pstruXml,"<omc>/<告警对象>",struSendPackage.struMapObjList[i].szMapId,
             	 		MODE_AUTOGROW|MODE_UNIQUENAME);
             	 bNewAlarm = BOOLTRUE;

             }
             else if (strcmp(struSendPackage.struMapObjList[i].szMapData, "0") == 0)//只有站点确实处于告警状态才恢复
             {
             	 STR szAlarmObjResv[5];
             	 if (bNewAlarm == BOOLTRUE)
					strcpy(szAlarmObjResv, DemandStrInXmlExt(pstruXml, "<omc>/<告警对象>"));
				 	
             	 InsertInXmlExt(pstruXml,"<omc>/<告警对象>",struSendPackage.struMapObjList[i].szMapId,
             	 		MODE_AUTOGROW|MODE_UNIQUENAME);
             	 		
                 sprintf(szTemp, "%s:1",  struSendPackage.struMapObjList[i].szMapId);
                 if (strstr(szAlarmObjList, szTemp) != NULL)
                 {
                 	 bAlarmRestore = BOOLTRUE;
                 	 
                 	 if (AlarmComeback(pstruXml) == NORMAL)
             
             		 SaveToMaintainLog("Alarm Recovery", "",  &struSendPackage);

                 }
                 
                 if (bNewAlarm == BOOLTRUE)
             	 	InsertInXmlExt(pstruXml,"<omc>/<告警对象>",szAlarmObjResv,MODE_AUTOGROW|MODE_UNIQUENAME);
             }            
         }
         
         if (bNewAlarm == BOOLTRUE)
         {
             if (DealNewAlarm(pstruXml) == NORMAL)
             //频繁告警
             SaveToMaintainLog("New Alarm", "",  &struSendPackage);

         }
             
                 	
    	 return NORMAL;
    }
 
    /*
	 *根据nMsgId获取原来信息
	 *删除对应流水号
	 */
    if(GetSnmpPackageInfo(nMsgId, &struSendPackage) != NORMAL)
    {
		PrintErrorLog(DBG_HERE, "根据QB+RepeaterId获取QA错误\n");
		return EXCEPTION; 
	}
	
	//命令来处理
	switch(struSendPackage.struHead.nCommandCode)
	{
		case COMMAND_QUERY:              //解析查询
			DecodeQryElementParam((SENDPACKAGE *)&struSendPackage);
			break;
		case COMMAND_SET:                //解析设置
			DecodeSetElementParam((SENDPACKAGE *)&struSendPackage);
			break;
		case COMMAND_QUERY_MAPLIST:    //取到监控量列表
			DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
			break;
		default:
			break;
	}
	
	return NORMAL;
}


/* 
 * GPRS服务请求处理 
 */
RESULT GprsReqWork(INT nSock, PVOID pArg)
{
	STR szCaReqBuffer[MAX_BUFFER_LEN];		/* 渠道请求通讯报文 */
	INT nRecvLen;	

	while(TRUE)
	{
	    /* 
	     * 接收渠道请求报文 
	     */
	    memset(szCaReqBuffer, 0, sizeof(szCaReqBuffer));
	    nRecvLen = RecvSocketNoSync(nSock, szCaReqBuffer, sizeof(szCaReqBuffer)-1, 30);
	    if (nRecvLen < 0)
	    {
	    	PrintErrorLog(DBG_HERE, "接收渠道请求报文[%s]失败\n", szCaReqBuffer);
	    	return EXCEPTION;
	    }
	    else if (nRecvLen == 0)
	    {
	        //PrintDebugLog(DBG_HERE, "客户端断开关闭!\n");
	        return NORMAL;
	    }
	    
	    PrintDebugLog(DBG_HERE, "接收请求报文[%s]\n",  szCaReqBuffer);
	    
	    if (nRecvLen < 16)
	    {
	        PrintDebugLog(DBG_HERE,"报文过短[%d]过滤掉!=====================\n",nRecvLen);
	        return NORMAL;
	    }
	    memset(szGprsRespBuffer, 0, sizeof(szGprsRespBuffer));
		strcpy(szGprsRespBuffer, "0000");
	    
	    ProcessSnmpDeliveTrans(nSock, szCaReqBuffer, nRecvLen);
		
		
	    if(SendSocketNoSync(nSock, szGprsRespBuffer, 4, 30) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE, "返回渠道应答报文错误!\n");
	    	return EXCEPTION;
	    }
	    PrintDebugLog(DBG_HERE, "发送应答报文[%s]\n",  szGprsRespBuffer);
	    
 	}
	return NORMAL;	

}

/*	
 * 主函数
 */
RESULT main(INT argc, PSTR *argv)
{
	POOLSVRSTRU struPoolSvr;
	STR szTemp[MAX_STR_LEN];
	
	if(argc != 2)
	{	
		Usage(argv[0]);
		return NORMAL;
	}
	
	if(strcmp(argv[1], "stop") == 0)
	{
		StopPrg(argv[0]);
		return NORMAL;
	}
	else if(strcmp(argv[1], "-h") == 0)
	{
		Usage(argv[0]);
		return NORMAL;
	}
	fprintf(stderr, "\tWelcome to use the network management system(snmpserver)\n");
	fflush(stderr);

	if(TestPrgStat(argv[0]) == NORMAL)
	{
		fprintf(stderr, "%s Has been launched or are in service\n", \
			argv[0]);
		return EXCEPTION;
	}
	
	if(DaemonStart() == EXCEPTION)
	{
		PrintErrorLog(DBG_HERE,"主函数调用 DaemonStart 进入守护状态错误!\n");
		CloseDatabase();		
		return EXCEPTION;
	}
	
	if(CreateIdFile(argv[0]) == EXCEPTION)
	{
		PrintErrorLog(DBG_HERE,"主函数调用 CreateIdFile 错误!\n");
		CloseDatabase();		
		return EXCEPTION;
    }
    
    if(GetDatabaseArg(szService, szDbName, szUser, szPwd) != NORMAL)
	{
		strcpy(szService, "omc");
		strcpy(szDbName, "omc");
		strcpy(szUser, "omc");
		strcpy(szPwd, "omc");
	}
	
	if(OpenDatabase(szService, szDbName, szUser, szPwd) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "打开数据库错误 [%s]!\n", \
			GetSQLErrorMessage());
		return EXCEPTION;
	}
	
    InitMapObjectCache();
    InitMapObjectCache_SNMP();
	CloseDatabase();
	
    if (GetCfgItem("snmpserv.cfg","SNMPSERV","ListenPort",szTemp) != NORMAL)
        return EXCEPTION;
    nGprsPort = atoi(szTemp);
	
	memset(&struPoolSvr,0,sizeof(struPoolSvr));
	//设备服务端口
	struPoolSvr.nListenPort[0]=nGprsPort;
	struPoolSvr.nListenPort[1]=-1;
	
	SetTcpPoolMinNum(2);
	if (GetCfgItem("snmpserv.cfg","SNMPSERV","MaxProcesses",szTemp) != NORMAL)
        return EXCEPTION;
	SetTcpPoolMaxNum(atoi(szTemp));
	
	fprintf(stderr,"完成!\n主程序开始启动\n");
	fflush(stderr);
	    	
    struPoolSvr.funcPoolStart = StartPoolChildPro;
    struPoolSvr.pPoolStartArg = NULL;
    struPoolSvr.funcPoolWork = GprsReqWork;
    struPoolSvr.pPoolWorkArg = NULL;
    struPoolSvr.funcPoolEnd = EndPoolChildPro;
    struPoolSvr.pPoolEndArg = NULL;
    struPoolSvr.funcPoolFinish = NULL;
    struPoolSvr.pPoolFinishArg = NULL;
	
	StartTcpPool(&struPoolSvr);

	return NORMAL;
}
