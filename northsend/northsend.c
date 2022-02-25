/***
 *名称:中国联通北向发送告警程序
 *
 *创建记录:
 *付志刚2021-7-23创建
 *
 *修订记录:
 *
 */

#include <ebdgdl.h>
#include "northsend.h"

#define MAX_WAIT_MSG_NUM	6


/*
 *全局变量定义 
 */

static UINT nCommPort;									/*通信服务地址*/
static STR szCommIpAddr[MAX_STR_LEN];					/*通信服务端口*/

static INT nSmgpSock=-1;								/*和短信网关连接套接字*/

static STR szUserName[50];
static STR szPassword[20];

static	STR szServiceName[MAX_STR_LEN];
static	STR szDbName[MAX_STR_LEN];
static	STR szUser[MAX_STR_LEN];
static	STR szPwd[MAX_STR_LEN];

static VOID Usage(PSTR pszProgName)
{
	fprintf(stderr,"网管系统(中国电信短消息节点程序)\n"
		"%s start 启动程序\n"
		"%s stop  关闭程序\n"
		"%s -h    显示帮助信息\n",
		pszProgName, pszProgName, pszProgName);
}




RESULT RecvSmgpRespPack(INT nSmgpSock, UINT nExpectCmd, PSTR pszCommBuffer)
{
	INT nPacketLen;

	if(nSmgpSock < 0)
		return EXCEPTION;

	//while(BOOLTRUE)
	{
		memset(pszCommBuffer, 0, MAX_BUFFER_LEN);
		
		if(RecvSocketNoSync(nSmgpSock, pszCommBuffer, MAX_BUFFER_LEN, 
			DEF_COM_TIMEOUT) < 0)
		{
			PrintErrorLog(DBG_HERE, "接收中国电信短信网关的应答报文失败\n");
			//close(nSmgpSock);
			//nSmgpSock = -1;
			return EXCEPTION;
		}

		PrintDebugLog(DBG_HERE, "Recv Message[%s]\n", pszCommBuffer);

		//if (strstr(pszCommBuffer, "") != NULL)
		//	ResponseActivetest(nSmgpSock, pszCommBuffer)
		
		
		/*
		 *如果取完消息后返回
		 */
		 //if(nExpectCmd == 0)
		 	return NORMAL;		
	}
}


/**
 *签到登录处理函数
 *
 *返回值:
 *			成功:NORMAL;
 *			失败:EXCEPTION;
 */
RESULT ConnectLogin(PSTR pszIpAddr, UINT nPort)
{
	STR szCommBuffer[MAX_BUFFER_LEN];
	STR szTemp[MAX_STR_LEN];
	
		
	nSmgpSock = CreateConnectSocket(pszIpAddr, nPort, DEF_COM_TIMEOUT);
	if(nSmgpSock < 0)
	{
		PrintErrorLog(DBG_HERE, "消息节点适配程序连接服务[%s][%d]失败\n",
			pszIpAddr,nPort);
		sleep(10);
		return EXCEPTION;
	}

	
	

	memset(szCommBuffer, 0, sizeof(szCommBuffer));
	sprintf(szCommBuffer, "<Connect>\r\nUserName:%s\r\nPassword:%s\r\n</Connect>\r\n", szUserName, szPassword);

	PrintDebugLog(DBG_HERE, "send Connect Login\n[%s]\n",szCommBuffer);
	if(SendSocketNoSync(nSmgpSock, szCommBuffer, strlen(szCommBuffer), DEF_COM_TIMEOUT) == EXCEPTION)
	{
		PrintErrorLog(DBG_HERE, "发送签到报文失败\n");
		close(nSmgpSock);
		nSmgpSock = -1;
		return EXCEPTION;
	}
	
	if(RecvSocketNoSync(nSmgpSock, szCommBuffer, strlen(szCommBuffer), DEF_COM_TIMEOUT) < 0)
	{
		PrintErrorLog(DBG_HERE, "接收签到应答报文失败\n");
		close(nSmgpSock);
		nSmgpSock = -1;
		return EXCEPTION;
	}
	PrintDebugLog(DBG_HERE, "recv ConnectAck\n[%s]\n",szCommBuffer);
	//if (strstr(szCommBuffer, "<ConnectACK>")!=NULL && strstr(szCommBuffer, "<ConnectACK>")!=NULL)

	return nSmgpSock;
}

RESULT ActiveTestHeat(INT nSmgpSock)
{
	STR szCommBuffer[MAX_BUFFER_LEN];
	STR szTemp[MAX_STR_LEN];
	
	if(nSmgpSock < 0)
	    return EXCEPTION;
	
	memset(szCommBuffer, 0, sizeof(szCommBuffer));
	//<HeartBeat> ProvinceID</HeartBeat>
	sprintf(szCommBuffer, "<HeartBeat>110</HeartBeat>\r\n");

	PrintDebugLog(DBG_HERE, "send Heat[%s]\n", szCommBuffer);
	if(SendSocketNoSync(nSmgpSock, szCommBuffer, strlen(szCommBuffer), DEF_COM_TIMEOUT) == EXCEPTION)
	{
		PrintErrorLog(DBG_HERE, "发送心跳报文失败\n");
		close(nSmgpSock);
		nSmgpSock = -1;
		return EXCEPTION;
	}
	
	if(RecvSocketNoSync(nSmgpSock, szCommBuffer, strlen(szCommBuffer), DEF_COM_TIMEOUT) < 0)
	{
		PrintErrorLog(DBG_HERE, "接收心跳应答报文失败\n");
		close(nSmgpSock);
		nSmgpSock = -1;
		return EXCEPTION;
	}
	PrintDebugLog(DBG_HERE, "Recv Heat Resp[%s]\n", szCommBuffer);

	return NORMAL;
}

RESULT SaveTransLog(YIYANGSTRU *pstruYiYang)
{
    char szSql[MAX_BUFFER_LEN];
     
    memset(szSql, 0, sizeof(szSql));
    
    if (atoi(pstruYiYang->szAlarmStatusId)==1)
    	snprintf(szSql, sizeof(szSql), "INSERT INTO TF_ALARMTRANSFERLOG(NEID, SENDTIME, UPLOADRESULT, ALARMLOGID, ORIALARMID, "
            "ALARMTITLE, ALARMCREATETIME, NETYPE, NENAME, NEVENDOR, "
            "ALARMLEVEL, ALARMTYPE,  ALARMLOCATION, ALARMSTATUS,"
            "ALARMOBJID, EXTENDINFO, SUCCEED) VALUES( "
            "'%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), '%s', '%s', '%s',"
            " '%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), '%s', '%s', '%s',"
            " '%s', '%s', '%s', %d, "
            " '%s', '%s', %d)",
            pstruYiYang->szNeId, GetSysDateTime(), "成功", pstruYiYang->szAlarmLogId, pstruYiYang->szAlarmId,
            pstruYiYang->szAlarmTitle, pstruYiYang->szAlarmCreateTime, pstruYiYang->szNeType, pstruYiYang->szNeName, pstruYiYang->szNeVendor,
            pstruYiYang->szAlarmLevel, pstruYiYang->szAlarmType, pstruYiYang->szAlarmLocation, atoi(pstruYiYang->szAlarmStatusId),
            pstruYiYang->szAlarmObjId, pstruYiYang->szExtendInfo, 0
            );
    else       
    	snprintf(szSql, sizeof(szSql), "INSERT INTO TF_ALARMTRANSFERLOG(NEID, SENDTIME, UPLOADRESULT, ALARMLOGID, ORIALARMID, "
            "ALARMTITLE, ALARMCREATETIME, NETYPE, NENAME, NEVENDOR, "
            "ALARMLEVEL, ALARMTYPE,  ALARMLOCATION, ALARMSTATUS, STATUSTIME,"
            "ALARMOBJID, EXTENDINFO, SUCCEED) VALUES( "
            "'%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), '%s', '%s', '%s',"
            " '%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), '%s', '%s', '%s',"
            " '%s', '%s', '%s', %d, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),"
            " '%s', '%s', %d)",
            pstruYiYang->szNeId, GetSysDateTime(), "成功", pstruYiYang->szAlarmLogId, pstruYiYang->szAlarmId,
            pstruYiYang->szAlarmTitle, pstruYiYang->szAlarmCreateTime, pstruYiYang->szNeType, pstruYiYang->szNeName, pstruYiYang->szNeVendor,
            pstruYiYang->szAlarmLevel, pstruYiYang->szAlarmType, pstruYiYang->szAlarmLocation, atoi(pstruYiYang->szAlarmStatusId),pstruYiYang->szAlarmClearTime,
            pstruYiYang->szAlarmObjId, pstruYiYang->szExtendInfo, 0
            );
      
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	return NORMAL; 
}


RESULT DeleteTrigger(PSTR pszAlarmLogId)
{
    char szSql[MAX_BUFFER_LEN];
 
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "delete from tf_ALarmLog_Trigger where alg_AlarmLogId=%s", pszAlarmLogId);
     
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	return NORMAL; 
}


RESULT SendCurrMessage(INT nSmgpSock)
{
    STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;

	YIYANGSTRU struYiYangData;
	STR szSendBuffer[MAX_BUFFER_LEN];
	STR szAlarmLogId[100];
	
	
	if(nSmgpSock < 0)
	    return INVALID;
	
    //while(TRUE)
	{
	    /*
	 	 *	处理告警交易
	 	 */
	    sprintf(szSql,"SELECT alg_AlarmLogId, alg_alarmId, alg_NeId, alg_AlarmStatusId, to_char(alg_AlarmTime,'yyyy-mm-dd hh24:mi:ss') as alg_AlarmTime,to_char(alg_ClearTime,'yyyy-mm-dd hh24:mi:ss') as alg_ClearTime,"
	    						"b.alm_Name, b.alm_objid, b.alm_LevelId, b.alm_dealidea, b.alm_memo," 

								"n.ne_repeaterid, n.ne_deviceid, n.ne_Name, n.ne_installplace,"
								"p.ARE_PARENTID, p.are_Name as County, (SELECT are_Name FROM pre_Area WHERE pre_Area.are_areaid = p.ARE_PARENTID) as Region," 
								"ne_Company.co_Name AS alg_CompanyName," 
								"ne_Company.co_CompanyId AS alg_CompanyId, "
								"alm_AlarmLevel.alv_Name AS alg_LevelName," 
								"ne_DeviceType.dtp_Name AS alg_DeviceTypeName "
							"FROM tf_alarmlog_trigger t "
								"LEFT JOIN  alm_Alarm b ON b.alm_AlarmId = t.alg_AlarmId " 
								"LEFT JOIN  ne_Element n ON n.ne_NeId = t.alg_NeId "
								"LEFT JOIN  pre_Area p on n.ne_AreaId = p.are_AreaId "
								"LEFT JOIN  ne_Company ON ne_Company.co_CompanyId = n.ne_CompanyId  "
								"LEFT JOIN  alm_AlarmLevel ON alm_AlarmLevel.alv_AlarmLevelId = b.alm_LevelId "
								"LEFT JOIN  ne_DeviceType ON ne_DeviceType.dtp_DeviceTypeId = n.ne_DeviceTypeId "
							);
	    //PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	    if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
	    	CloseDatabase();
	    	sleep(60);
	    	return EXCEPTION;
	    }
	    while (FetchCursor(&struCursor) == NORMAL)
	    {
	        PrintDebugLog(DBG_HERE,"开始处理实时告警================\n");
	        memset(&struYiYangData, 0, sizeof(YIYANGSTRU));
			strcpy(struYiYangData.szAlarmLogId, GetTableFieldValue(&struCursor, "alg_AlarmLogId"));
			sprintf(struYiYangData.szAlarmId,  "%d", atoi(GetTableFieldValue(&struCursor, "alg_alarmId")));
			
			//strcpy(struYiYangData.szNeId ,GetTableFieldValue(&struCursor, "alg_NeId"));
			sprintf(struYiYangData.szNeId, "%08X%02X", atoi(GetTableFieldValue(&struCursor, "ne_repeaterid")), 
					atoi(GetTableFieldValue(&struCursor, "ne_deviceid")));
			 //网元名称
            strcpy(struYiYangData.szNeName, GetTableFieldValue(&struCursor, "ne_Name"));
            
			strcpy(struYiYangData.szSystemVendor,  GetTableFieldValue(&struCursor, "alg_CompanyName"));//厂家名
			
			
            strcpy(struYiYangData.szAlarmCreateTime,  GetTableFieldValue(&struCursor, "alg_AlarmTime"));
            
            strcpy(struYiYangData.szNeType,  GetTableFieldValue(&struCursor, "alg_DeviceTypeName"));
            strcpy(struYiYangData.szAlarmStatusId,  GetTableFieldValue(&struCursor, "alg_AlarmStatusId"));
            
            if (atoi(GetTableFieldValue(&struCursor, "alg_AlarmStatusId"))==0)
            	strcpy(struYiYangData.szAlarmClearTime,  GetTableFieldValue(&struCursor, "alg_ClearTime"));
                       
            
            strcpy(struYiYangData.szNeVendor,  GetTableFieldValue(&struCursor, "alg_CompanyName")); //网元厂家
            strcpy(struYiYangData.szAlarmLevel,  GetTableFieldValue(&struCursor, "alm_LevelId")); //告警级别
            
            strcpy(struYiYangData.szAlarmType, "设备告警");  //告警类型：设备告警
            
            strcpy(struYiYangData.szAlarmTitle,  GetTableFieldValue(&struCursor, "alm_Name"));
            strcpy(struYiYangData.szAlarmObjId, GetTableFieldValue(&struCursor, "alm_objid"));
            strcpy(struYiYangData.szProbableCauseTxt, GetTableFieldValue(&struCursor, "alm_dealidea"));
            
            strcpy(struYiYangData.szAlarmLocation, GetTableFieldValue(&struCursor, "ne_installplace")); //  告警定位：站点编号,设备编号（以逗号分隔，站点编号为8位16进制数，设备编号为2位16进制数）
                   
		      
		    strcpy(struYiYangData.szSystemName, "SUNWAVE OMC");
		    
		    strcpy(struYiYangData.szAlarmRegion, GetTableFieldValue(&struCursor, "Region"));
		    strcpy(struYiYangData.szAlarmCounty, GetTableFieldValue(&struCursor, "County"));
		   
 
 
			bufclr(szSendBuffer);
			snprintf(szSendBuffer, sizeof(szSendBuffer), 
				"<AlarmStart>\r\n"
				"ProvinceID:110\r\n"             //省分编码   110
				"IntVersion:V1.2.0\r\n"         //接口版本号  V1.2.0
				"MsgSerial:%s\r\n"              //连续消息序号
				"AlarmUniqueId:%s\r\n"          //告警唯一标识
                "ClearId:%s\r\n"                //告警清除指纹  MsgSerial
                
                "StandardFlag:2\r\n"           //告警标准化标识  2
                "SubAlarmType: \r\n"
                "NeId:%s\r\n"                   //设备标识
                "NeName:%s\r\n"                 //设备名称
                "NeAlias: \r\n"
                "LocateNeName:%s\r\n"            //告警对象名称
                "LocateNeType:Repeater\r\n"            //告警对象类型
                
                "EquipmentClass:Repeater\r\n"
                "NeIp: \r\n"
                "SystemName:%s\r\n"             //系统名
                "Vendor:%s\r\n"                 //厂家
                "Version:1.0\r\n"
                "LocateNeStatus: \r\n"
                
                "ProjectNo: \r\n"
				"ProjectName: \r\n"
				"ProjectStartTime: \r\n"
				"ProjectEndTime: \r\n"
                "LocateInfo:%s\r\n"             //定位信息
                "EventTime:%s\r\n"              //告警发生时间
                "CancelTime:%s\r\n"             //告警清除时间
                "DalTime:%s\r\n"                //告警发现时间  sysdate
                
                "VendorAlarmType:%s\r\n"       //厂家告警类型
                "VendorSeverity:%s\r\n"       //厂家告警级别
                "VendorAlarmId:%s\r\n"       //厂家告警号
                "AlarmSeverity:%s\r\n"         //网管告警级别
                "NmsAlarmId:%s\r\n"           //网管告警ID
                "AlarmStatus:%s\r\n"         //告警状态   0：网元自动清除 1：活动告警 2：同步清除
                "AckFlag:0\r\n"
				"AckTime: \r\n"
				"AckUser: \r\n"
                "AlarmTitle:%s\r\n"         //告警标题
                "StandardAlarmName:%s\r\n"         //告警标准名
                "ProbableCauseTxt:%s\r\n"   //告警可能原因
                "AlarmText:%s\r\n"          //告警正文
                "CircuitNo: \r\n"
                "PortRate: \r\n"
                "Specialty:1\r\n"         	//专业   1  无线网
                "BusinessSystem:OMC\r\n"     //子专业   OMC
                "AlarmLogicClass: \r\n"         //告警逻辑分类       为空
                "AlarmLogicSubClass: \r\n"         //告警逻辑子类    为空
                "EffectOnEquipment:4\r\n"         //该事件对设备的影响  4  
                "EffectOnBusiness:6\r\n"         //该事件对业务的影响  6
                "NmsAlarmType:1\r\n"
                "SendGroupFlag:0\r\n"
				"RelatedFlag:0\r\n"
				"AlarmProvince:广西\r\n"
				"AlarmRegion:%s\r\n"
				"AlarmCounty:%s\r\n"
				"Site: \r\n"
				"AlarmActCount:0\r\n"
				"CorrelateAlarmFlag:1\r\n"
				"SheetSendStatus: \r\n"
				"SheetStatus: \r\n"
				"SheetNo:0\r\n"
				"AlarmMemo:0\r\n"
                "<AlarmEnd>\r\n",

                struYiYangData.szAlarmLogId,
                struYiYangData.szAlarmId, 
                struYiYangData.szAlarmLogId,
                
                struYiYangData.szNeId,
                struYiYangData.szNeName,
                struYiYangData.szNeName,      //struYiYangData.szAlarmTitle, //原来对应的是告警名称，现改为站点名称
                //struYiYangData.szAlarmType, //告警对象类型
                
                struYiYangData.szSystemName, //系统名
                struYiYangData.szNeVendor,   //厂家
                
                struYiYangData.szAlarmLocation,  //定位信息
                struYiYangData.szAlarmCreateTime, //告警发生时间
                struYiYangData.szAlarmClearTime,
                struYiYangData.szAlarmCreateTime, //告警发生时间
                
                struYiYangData.szAlarmType,    //厂家告警类型
                struYiYangData.szAlarmLevel,   //厂家告警级别
                struYiYangData.szAlarmObjId,   //厂家告警号
                
                struYiYangData.szAlarmLevel,   //网管告警级别
                struYiYangData.szAlarmObjId,   //网管告警ID
                
                struYiYangData.szAlarmStatusId,
                struYiYangData.szAlarmTitle,
                struYiYangData.szAlarmTitle,
                struYiYangData.szProbableCauseTxt,
                struYiYangData.szAlarmTitle,
                
                struYiYangData.szAlarmRegion,
                struYiYangData.szAlarmCounty
				);
            
            PrintDebugLog(DBG_HERE,"send alarm message\n[%s]\n",szSendBuffer);  
            
            /*
			 *	发送数据到消息服务监听口
			 */
			if(SendSocketNoSync(nSmgpSock, szSendBuffer, strlen(szSendBuffer), 60) < 0)
			{
				FreeCursor(&struCursor);
				PrintErrorLog(DBG_HERE, "发送数据到服务程序错误\n");
				return INVALID;
			}
			
			
	        DeleteTrigger(struYiYangData.szAlarmLogId);
		    SaveTransLog(&struYiYangData);
		    


	    }
	    FreeCursor(&struCursor);
   
	    	    
	    //sleep(30);

	}
	return NORMAL;
}



/**
 *处理进程终止信号
 *
 *无返回值
 */
 static VOID SigHandle(INT nSigNo)
{
	if(nSigNo != SIGTERM)
		return;
	
	/*	
	 * 签退处理
	 */
	//SignOutAdapter();
	CloseDatabase();
	close(nSmgpSock);
	nSmgpSock = -1;

	exit(0);
}





/**
 *下行消息的接受和发送函数
 *
 *修改双连接机制，确保在select之前两连接正常
 *
 *返回值:
 *			成功:NORMAL;
 *			失败:EXCEPTION;
 */
static RESULT TransSmgpMsgTask(VOID)
{
	UINT nIdleTimes = 0;
	STR szCommBuffer[MAX_BUFFER_LEN];
	STR szTemp[MAX_BUFFER_LEN];
	
	INT nMaxFd;
	fd_set struSockSet;
	struct timeval struTimeOut;

	INT nTestTimes=0, nRet;

	while(BOOLTRUE)
	{

		if(nSmgpSock < 0)
		{
			if((nSmgpSock = ConnectLogin(szCommIpAddr, nCommPort)) < 0)
			{
				PrintErrorLog(DBG_HERE, "连接到网关失败[%s][%d]\n", szCommIpAddr, nCommPort);
				sleep(10);
				continue;
			}
			PrintDebugLog(DBG_HERE, "ConnectSmgp success[%d]\n", nSmgpSock);
			
		}	

		FD_ZERO(&struSockSet);
		FD_SET(nSmgpSock, &struSockSet);

		struTimeOut.tv_sec = 10;
		struTimeOut.tv_usec = 0;
		nMaxFd =  nSmgpSock;
		switch(select(nMaxFd + 1, &struSockSet, NULL, NULL, &struTimeOut))
		{
			case -1:
				PrintErrorLog(DBG_HERE, "Select函数调用错误\n");
				sleep(10);
				exit(0);

			case 0:
				nTestTimes++;
				if (nTestTimes >= 5)
				{
					if(ActiveTestHeat(nSmgpSock) != NORMAL)
					{
							PrintErrorLog(DBG_HERE, "ActivetestSmgp Error, ExitSmgp!!\n");
							close(nSmgpSock);
							nSmgpSock = -1;
							sleep(1);
					}
					nTestTimes=0;
				}
				break;

			default:
				nIdleTimes = 0;

				if(FD_ISSET(nSmgpSock, &struSockSet) && nSmgpSock>0)			/*接收服务端返回报文*/
				{
					if(RecvSmgpRespPack(nSmgpSock, 0, szCommBuffer) <0)
					{
						PrintErrorLog(DBG_HERE, "接收网关报文错误[%d]\n", nSmgpSock);
						sleep(1);
						continue;						
					}					
				}
				
				break;
								
		}
		
		//发送实时告警
		nRet=SendCurrMessage(nSmgpSock);
		if (nRet==INVALID)
		{
			close(nSmgpSock);
			nSmgpSock = -1;
			sleep(10);
		}
		else if (nRet==EXCEPTION)
		{
			sleep(10);
			exit(0);
		}	
		
	}
}


/**
 *进程状态测试函数
 *
 *返回值:
 *			成功:NORMAL;
 *			失败:EXCEPTION;
 */
RESULT TestSmgpPidStat(int nSmgpPid)
{
	STR szShellCmd[MAX_STR_LEN];
	STR szFileName[100];
	INT nTempLen;
	FILE *pstruFile;

	memset(szShellCmd, 0, sizeof(szShellCmd));
	snprintf(szShellCmd, sizeof(szShellCmd), "ps -e | awk '$1 == %d {print $4}'", nSmgpPid);

	if((pstruFile = popen(szShellCmd, "r")) == NULL)
	{
		fprintf(stderr, "popen有错误\n");
		return EXCEPTION;
	}

	memset(szFileName, 0, sizeof(szFileName));
	while(fgets(szFileName, 10, pstruFile) != NULL)
	{
		nTempLen = strlen(szFileName);
		szFileName[nTempLen] = 0;

		if(strncmp(szFileName, "northsend", 9) == 0)
		{
			pclose(pstruFile);
			return NORMAL;
		}		
	}

	pclose(pstruFile);
	return EXCEPTION;
}


RESULT SmgpChildProcessWork(INT nPid)
{
	struct sigaction struSigAction;
	STR szTemp[MAX_STR_LEN];
	STR szCfgSeg[100];
	
	if(GetCfgItem("northsend.cfg", "NORTHPROC1", "CommServAddr", szCommIpAddr) != NORMAL)
		return EXCEPTION;

	if(GetCfgItem("northsend.cfg", "NORTHPROC1", "CommServPort", szTemp) != NORMAL)
		return EXCEPTION;
	nCommPort = atoi(szTemp);
	
	if(GetCfgItem("northsend.cfg", "NORTHPROC1", "LoginUserName", szUserName) != NORMAL)
		return EXCEPTION;
	if(GetCfgItem("northsend.cfg", "NORTHPROC1", "LoginPassword", szPassword) != NORMAL)
		return EXCEPTION;
	/*更新SMGP进程号*/
	sprintf(szTemp, "%06d", (int)getpid());
	ModifyCfgItem("northsend.cfg", "NORTHPROC1", "NORTHPID", szTemp);

	struSigAction.sa_handler = SigHandle;
	sigemptyset(&struSigAction.sa_mask);
	struSigAction.sa_flags = 0;
	if(sigaction(SIGTERM, &struSigAction, NULL) == -1)
	{
		PrintErrorLog(DBG_HERE, "安装清除操作任务错误, 错误代码[%s]错误信息[%s]\n",
			errno, strerror(errno));
		return EXCEPTION;
	}

	struSigAction.sa_handler = SIG_IGN;
	sigemptyset(&struSigAction.sa_mask);
	struSigAction.sa_flags = 0;
	if(sigaction(SIGPIPE, &struSigAction, NULL))
	{
		PrintErrorLog(DBG_HERE, "安装清除操作任务错误, 错误代码[%s]错误信息[%s]\n",
			errno, strerror(errno));
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
	TransSmgpMsgTask();
	
	
	return NORMAL;
}


RESULT ParentForkWork(int nProcCount)
{
	STR szTemp[MAX_STR_LEN];
	STR szCfgSeg[100];
	INT nPid;

	while(BOOLTRUE)
	{
		sleep(1);
		/*监控子进程*/
		for(nPid = 1; nPid < nProcCount + 1; nPid++)
		{
			//sprintf(szCfgSeg, "SMGPPROC%d", nPid);
			strcpy(szCfgSeg, "NORTHPROC1");
			/*SMGP进程号*/
			if(GetCfgItem("northsend.cfg", szCfgSeg, "NORTHPID", szTemp) != NORMAL)
				return EXCEPTION;
			if(TestSmgpPidStat(atoi(szTemp)) == EXCEPTION)
			{
				switch(fork())
				{
					case -1:
						PrintErrorLog(DBG_HERE, "创建子进程失败\n");
						break;

					case 0:
					{
						SmgpChildProcessWork(nPid);
						exit(0);
					}

					default:
						break;
				}
			}
			sleep(10);
		}
	}
}


/*
 * 主函数
 */
RESULT main(INT argc, PSTR argv[])
{
	STR szTemp[MAX_BUFFER_LEN];
	INT nProcCount;
	STR szFileName[200];

	//fprintf(stderr, "\t欢迎使用中国电信网管系统(SMGP 3.1协议节点程序)");

	if(argc !=2)
	{
		Usage(argv[0]);
		return INVALID;
	}

	if(strcmp(argv[1], "stop") == 0)
	{
		sprintf(szTemp, "clearprg %s", argv[0]);
		system(szTemp);
		snprintf(szFileName, sizeof(szFileName), "%s/.%sid", getenv("HOME"), argv[0]);
		unlink(szFileName);
		return NORMAL;
	}

	if(strcmp(argv[1], "start") != 0)
	{
		Usage(argv[0]);
		return NORMAL;
	}

	if(TestPrgStat(argv[0]) == NORMAL)
	{
		fprintf(stderr, "中国电信SMGP 3.1 协议消息节点适配程序已经启动或者正在服务中\n");
		return EXCEPTION;
	}

	if(DaemonStart() != 0)
	{
		PrintErrorLog(DBG_HERE, "中国电信SMGP 3.1协议消息节点适配程序常驻失败\n");
		return EXCEPTION;
	}

	if(CreateIdFile(argv[0]) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "创建程序ID文件错误\n");
		return EXCEPTION;
	}

	

	//fprintf(stderr, "完成!程序开始运行\n");

	ParentForkWork(1);

	return NORMAL;
}


