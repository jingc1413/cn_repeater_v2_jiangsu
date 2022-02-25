/*
 * 名称: 网管系统应用服务器
 *
 * 修改记录:
 * 付志刚 -		2008-9-8 创建
 */
 
#include <ebdgdl.h>
#include <ebdgml.h>
#include <omcpublic.h>
#include <applserver.h>
#include <mobile2g.h>

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
	
	if (getenv("WUXIAN")!=NULL)
	{
		if (ConnectRedis() !=NORMAL)
	    {
	        PrintErrorLog(DBG_HERE, "Connect Redis Error \n");
	        return EXCEPTION;
	    }
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
	if (getenv("WUXIAN")!=NULL)
		FreeRedisConn();
	//PrintDebugLog(DBG_HERE, "关闭数据库成功\n");
	return NORMAL;
}



/*
 * 使用说明
 */
static VOID Usage(PSTR pszProg)
{
	fprintf(stderr, "网管系统(应用服务程序)\n");
	fprintf(stderr, "%s start 启动程序\n"
					"%s stop 停止程序\n"
					"%s -h 显示本信息\n", pszProg, pszProg, pszProg);
}

DIVCFGSTRU struDivCfg[]=
{
    {"交易代码",	4},
	{"网元编号",	10},
	{"站点编号",	10},
	{"设备编号",	10},
	{"监控对象",	1000},
	{"监控对象内容",1800},
	{"协议类型",	10},
	{"站点电话",	28},
	{"站点IP",	    28},
	{"流水号",	    28},
	{"命令号",	    8},
	{"设备类型",	10},
	{"其它标识",	50},
	{"设备型号",	50},
	{"端口号",	    8},
	{"服务号码",	30},
	{"类型",	    2},
	{"时间",	    20},
	{"站点等级",	2},
	{"通信方式",	10},
	{"登录流水号",	10},
	{"窗体流水号",	50},
	{NULL,		    0}
};

/*
 * 将变长报文解压
 */
static RESULT UnpackDivStr( char *szBuffer, PXMLSTRU pstruXml, PDIVCFGSTRU pstruDivCfg)
{
	INT	i;
	INT nFieldNum;
	STR *pszBuffer;
	STR *pszField[100];
	STR szPath[MAX_PATH_LEN];
	
	pszBuffer = szBuffer;

	nFieldNum = SeperateString( pszBuffer, '|', pszField, 100 );
	
	/*	循环解包保存至XML	*/
	//for ( i = 0; i < nFieldNum; i ++ )
	for(i=0; (i<nFieldNum) && (pstruDivCfg[i].pszName!=NULL);i++)
	{
	    sprintf(szPath,"%s/<%s>", OMC_ROOT_PATH, pstruDivCfg[i].pszName);
		TrimAllSpace(pszField[i]);
		//PrintDebugLog( DBG_HERE, "%s[%s]\n", szPath, pszField[i] );
		InsertInXmlExt( pstruXml, szPath, pszField[i], MODE_AUTOGROW|MODE_UNIQUENAME );
		if (i> nFieldNum) break;
	}
	return NORMAL;
}


/*
 * 处理查询设置交易
 */
RESULT ProcessQuerySetTrans(PSTR pszCaReqBuffer)
{
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	REPEATER_INFO struRepeater;
	COMMANDHEAD struHead;
	STR szBuffer[MAX_BUFFER_LEN];
	int nNeId, nSpecialType;
	STR szType[2+1];   			/*命令类型  */
	STR szTemp[10];
	int nObjCount, i;
	STR szQryEleParam[MAX_BUFFER_LEN];
	STR szSetEleParam[MAX_BUFFER_LEN];
	STR szSetEleValue[MAX_BUFFER_LEN];
	PSTR pszSepParamStr[MAX_OBJECT_NUM];  /* 分割字符数组*/
	PSTR pszSepValueStr[MAX_OBJECT_NUM];  /* 分割字符数组*/
	
	/*
 	 * 分析报文，将定长报文转换为xml
     */
    memset(pstruXml,0,sizeof(XMLSTRU));
	CreateXml(pstruXml,FALSE,OMC_ROOT_PATH,NULL);

	if(UnpackDivStr(pszCaReqBuffer, pstruXml, struDivCfg)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"解析BS发送报文失败\n");
		return EXCEPTION;
	}
    memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"变长转换报文[%s][%d]\n",szBuffer, strlen(szBuffer));
    
    memset(&struHead, 0, sizeof(COMMANDHEAD));
    memset(&struRepeater, 0, sizeof(REPEATER_INFO));
	nNeId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<网元编号>"));  
	struRepeater.nCommType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<通信方式>"));  
	struRepeater.nRepeaterId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<站点编号>")); 
	struRepeater.nDeviceId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<设备编号>")); 
	strcpy(struRepeater.szTelephoneNum, DemandStrInXmlExt(pstruXml,"<omc>/<站点电话>"));
	
	strcpy(struRepeater.szIP, DemandStrInXmlExt(pstruXml,"<omc>/<站点IP>"));
	if (strlen(DemandStrInXmlExt(pstruXml,"<omc>/<端口号>")) == 0)
		struRepeater.nPort = 0;
	else
		struRepeater.nPort = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<端口号>")); 
		
	struRepeater.nProtocolDeviceType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<设备类型>"));
	strcpy(struRepeater.szSpecialCode, DemandStrInXmlExt(pstruXml,"<omc>/<其它标识>"));
	strcpy(struRepeater.szReserve, DemandStrInXmlExt(pstruXml,"<omc>/<设备型号>"));
	strcpy(struRepeater.szNetCenter, DemandStrInXmlExt(pstruXml,"<omc>/<服务号码>"));
	
	if (getAgentState(struRepeater.szNetCenter) == BOOLTRUE)
		InsertInXmlExt(pstruXml,"<omc>/<服务状态>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
	else
		InsertInXmlExt(pstruXml,"<omc>/<服务状态>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
	//判断是否2G， 非2G， Snmp协议
	struHead.nProtocolType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<协议类型>"));
	struHead.nCommandCode = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<命令号>"));
	// 处理特殊类型  2009.1.7
    //  178工厂参数查询 179工厂参数设置 180工程参数查询 181工程参数设置 176进入工厂模式命令
	nSpecialType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<流水号>"));
	if (nSpecialType > 170 && nSpecialType < 1000)
	{
	    struHead.nCommandCode = nSpecialType;

		sprintf(szTemp, "%d", nSpecialType);
		InsertInXmlExt(pstruXml,"<omc>/<命令号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    nSpecialType = 0;
	}
	strcpy(szType, DemandStrInXmlExt(pstruXml,"<omc>/<类型>"));
	
	strcpy(szQryEleParam, DemandStrInXmlExt(pstruXml,"<omc>/<监控对象>"));
	if (strlen(szQryEleParam) == 0)
	{
		PrintDebugLog(DBG_HERE,"查询[%d][%s]监控对象为空\n", struRepeater.nRepeaterId, struRepeater.szTelephoneNum);
		DeleteXml(pstruXml);
		return EXCEPTION;
	}
	//批采
	if (strcmp(szQryEleParam, "0606") == 0 || strcmp(szQryEleParam, "00000606") == 0)
	{	
		strcpy(szType, "61");
	}
	//时隙
	strcpy(szQryEleParam, DemandStrInXmlExt(pstruXml,"<omc>/<监控对象>"));
	if (strcmp(szQryEleParam, "0875") == 0 || nSpecialType == 71)
	{	
		strcpy(szType, "71");
	}
	
    //重新设置站点等级为快:OMC_QUICK_MSGLEVEL
	InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
		
	if ((strcmp(szType,"11") == 0) || (strcmp(szType ,"21") == 0))//查询 11 2g 21 非2g
	{
	    PrintDebugLog(DBG_HERE,"进入查询流程\n");
		if (struRepeater.nCommType == 7)//通信方式：snmp协议
		{
			struHead.nProtocolType = PROTOCOL_SNMP;
			QryElementParam(M2G_SNMP_TYPE, &struHead, &struRepeater, pstruXml);
        	//生成设置命令保存
        	SaveToSnmpQueue(pstruXml);
        	SaveEleQryLog(pstruXml);
		}
        //else if (strcmp(szType, "21") ==0) //通信方式：UDP, GPRS
        else if (struRepeater.nCommType == 5 || struRepeater.nCommType == 6) //通信方式：UDP, GPRS
        {
        	ResolveQryParamArrayGprs(szQryEleParam);
        	PrintDebugLog(DBG_HERE, "监控对象[%s]\n", szQryEleParam);
	        nObjCount = SeperateString(szQryEleParam, '|', pszSepParamStr, MAX_SEPERATE_NUM);
	
	        for(i=0; i< nObjCount; i++)
		    {
		    	InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  pszSepParamStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
	        	//生成GPRS方式
	        	QryElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
	        	//生成设置命令保存
	        	SaveToGprsQueue(pstruXml);
	        	SaveEleQryLog(pstruXml);
        	}
        	
        }
        else //通信方式：短信
        {
        	if(struHead.nProtocolType == PROTOCOL_2G ||
        	   struHead.nProtocolType == PROTOCOL_DAS ||
        	   struHead.nProtocolType == PROTOCOL_JINXIN_DAS)
        	{	
	        	strcpy(szQryEleParam, DemandStrInXmlExt(pstruXml,"<omc>/<监控对象>"));
	        	ResolveQryParamArray(szQryEleParam);
	 	    	PrintDebugLog(DBG_HERE, "监控对象[%s]\n", szQryEleParam);
	        	nObjCount = SeperateString(szQryEleParam, '|', pszSepParamStr, MAX_SEPERATE_NUM);
	
	        	for(i=0; i< nObjCount; i++)
		    	{
		    	    //监控对象用空格分割
	 	    	    InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime((INT)time(NULL)+i*10), MODE_AUTOGROW|MODE_UNIQUENAME);
		    	    InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  pszSepParamStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
					QryElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
					SaveToMsgQueue(pstruXml);
					SaveEleQryLog(pstruXml);
		    	}
		    }
		    else if (struHead.nProtocolType == PROTOCOL_GSM || 
		    		struHead.nProtocolType == PROTOCOL_CDMA ||
		    		struHead.nProtocolType == PROTOCOL_HEIBEI ||
		    		struHead.nProtocolType == PROTOCOL_HEIBEI2 ||
		    		struHead.nProtocolType == PROTOCOL_XC_CP ||
		    		struHead.nProtocolType == PROTOCOL_SUNWAVE ||
		    		struHead.nProtocolType == PROTOCOL_WLK)
		    {
	 	    	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime((INT)time(NULL)), MODE_AUTOGROW|MODE_UNIQUENAME);
			    QryElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
				SaveToMsgQueue(pstruXml);
				SaveEleQryLog(pstruXml);
		    }
		    	
		}

	}
	else if ((strcmp(szType,"12") == 0) || (strcmp(szType ,"22") == 0)) //12 2g设置  22 非2g设置
	{
		if (struRepeater.nCommType == 7)//snmp协议
		{
			struHead.nProtocolType = PROTOCOL_SNMP;
			SetElementParam(M2G_SNMP_TYPE, &struHead, &struRepeater, pstruXml);
        	//生成设置命令保存
        	SaveToSnmpQueue(pstruXml);
        	SaveEleSetLog(pstruXml);
		}
        //else if (strcmp(szType, "22") ==0)
        else if (struRepeater.nCommType == 5 || struRepeater.nCommType == 6) //通信方式：UDP, GPRS
        {
        	SetElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
        	//生成设置命令保存
        	SaveToGprsQueue(pstruXml);
        	SaveEleSetLog(pstruXml);
        }
        else
        {
        	if(struHead.nProtocolType == PROTOCOL_2G ||
        	   struHead.nProtocolType == PROTOCOL_DAS ||
        	   struHead.nProtocolType == PROTOCOL_JINXIN_DAS)
        	{	
	        	strcpy(szSetEleParam, DemandStrInXmlExt(pstruXml,"<omc>/<监控对象>"));
				strcpy(szSetEleValue, DemandStrInXmlExt(pstruXml,"<omc>/<监控对象内容>"));
			
			
				ResolveSetParamArray(szSetEleParam, szSetEleValue);
	        	nObjCount = SeperateString(szSetEleParam, '|', pszSepParamStr, MAX_SEPERATE_NUM);
	        	nObjCount = SeperateString(szSetEleValue, '|', pszSepValueStr, MAX_SEPERATE_NUM);
	        	for(i=0; i< nObjCount; i++)
		    	{
		    	    //监控对象用空格分割
	 	    	    InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime((INT)time(NULL)+i*10), MODE_AUTOGROW|MODE_UNIQUENAME);
		    	    InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  pszSepParamStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
		    	    InsertInXmlExt(pstruXml,"<omc>/<监控对象内容>",  pszSepValueStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
		    	             
					SetElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
					SaveToMsgQueue(pstruXml);
					SaveEleSetLog(pstruXml);
				}
			}
		    else if (struHead.nProtocolType == PROTOCOL_GSM || 
		    		struHead.nProtocolType == PROTOCOL_CDMA ||
		    		struHead.nProtocolType == PROTOCOL_HEIBEI ||
		    		struHead.nProtocolType == PROTOCOL_HEIBEI2 ||
		    		struHead.nProtocolType == PROTOCOL_XC_CP ||
		    		struHead.nProtocolType == PROTOCOL_SUNWAVE ||
		    		struHead.nProtocolType == PROTOCOL_WLK)
			{
				InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime((INT)time(NULL)), MODE_AUTOGROW|MODE_UNIQUENAME);
				SetElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
				SaveToMsgQueue(pstruXml);
				SaveEleSetLog(pstruXml);
			}	
		}

	}
	else if (strcmp(szType,"31") == 0)//31 监控列表
	{ 
		strcpy(szTemp, DemandStrInXmlExt(pstruXml,"<omc>/<通信方式>"));
        if (strcmp(szTemp, "6") == 0 || strcmp(szTemp, "5") == 0)
        {
			QueryMapList(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
			SaveToGprsQueue(pstruXml);
			SaveEleQryLog(pstruXml);
		}
        else
        {
			QueryMapList(M2G_SMS, &struHead, &struRepeater, pstruXml);
			SaveToMsgQueue(pstruXml);
			SaveEleQryLog(pstruXml);
		}
		
	}
	else if (strcmp(szType,"41") == 0)//41 pesq下行拨测
	{
		InsertMosTask(pstruXml);
		DeleteXml(pstruXml);
		return NORMAL;
	}
	else if (strcmp(szType,"51") == 0)//51 das
	{
		if (struRepeater.nCommType == 7)//通信方式：snmp协议
		{
			struHead.nProtocolType = PROTOCOL_SNMP;
			QueryDasList(M2G_SNMP_TYPE, &struHead, &struRepeater, pstruXml);
        	//生成设置命令保存
        	SaveToSnmpQueue(pstruXml);
        	SaveEleQryLog(pstruXml);
		}
		else
		{
			//PrintDebugLog(DBG_HERE,"here\n");
			QueryDasList(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
			SaveToGprsQueue(pstruXml);
			SaveEleQryLog(pstruXml);
		}
	}
	else if (strcmp(szType,"52") == 0)//52 rfid
	{
		QueryRfidList(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
		SaveToGprsQueue(pstruXml);
		SaveEleQryLog(pstruXml);
	}
	else if (strcmp(szType,"61") == 0)//61 批采
	{ 
		strcpy(szTemp, DemandStrInXmlExt(pstruXml,"<omc>/<通信方式>"));
        if (strcmp(szTemp, "6") == 0)
        {
        	QryElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
        	SaveToGprsQueue(pstruXml);
        	SaveEleQryLog(pstruXml);
		}
        else
        {
			QryElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
			SaveToMsgQueue(pstruXml);
			SaveEleQryLog(pstruXml);
		}
		
		InitBatPick(pstruXml);
		
		SaveBatPickLog(pstruXml);
		
	}
	else if (strcmp(szType,"71") == 0)//71 时隙
	{ 
		strcpy(szTemp, DemandStrInXmlExt(pstruXml,"<omc>/<通信方式>"));
        if (strcmp(szTemp, "6") == 0)
        {
        	QryElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
        	SaveToGprsQueue(pstruXml);
        	SaveEleQryLog(pstruXml);
		}
        else
        {
			QryElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
			SaveToMsgQueue(pstruXml);
			SaveEleQryLog(pstruXml);
		}
		
		CheckShiXi(pstruXml);
		
		SaveShiXiLog(pstruXml);
		
	}
	
	// 2009.1.7 特殊命令处理,字段为流水号而来
	if (nSpecialType > 0)
	{
	    //   1-远程升级, 2-查询cqt互拨结果命令，3-立即pesq上行拨测，4-立即pesq下行拨测，
		//   5-gprs立即测试命令，6-查询gprs测试结果命令
		if ((strcmp(szType,"12") == 0) || (strcmp(szType ,"22") == 0))
		    InsertInXmlExt(pstruXml,"<omc>/<命令类型>", "2", MODE_AUTOGROW|MODE_UNIQUENAME);
		else if (nSpecialType == 4)
		    InsertInXmlExt(pstruXml,"<omc>/<命令类型>", "3", MODE_AUTOGROW|MODE_UNIQUENAME);
		else
		    InsertInXmlExt(pstruXml,"<omc>/<命令类型>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
		
		sprintf(szTemp, "%d", nSpecialType);
		InsertInXmlExt(pstruXml,"<omc>/<命令号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
				
	    //InsertSpecialCommandLog(pstruXml);
	    //更新远程升级
	    if (nSpecialType == 1)
	        RemortUpdateDbOperate1(pstruXml);
	}
	
	DeleteXml(pstruXml);
	return NORMAL;
    
}

/*
 * 处理上报交易
 */
RESULT ProcessSmsDeliverTrans(PSTR pszCaReqBuffer)
{
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	STR szMobile[25];				/* 手机号 */
//	STR szMsgCont[150];				/*  上报消息内容*/
	STR szMsgCont[MAX_CONTENT_LEN];	/*  上报消息内容*/
	STR szSPNumber[22];				/*  特服务号*/

	memset(pstruXml,0,sizeof(XMLSTRU));
	if(ImportXml(pstruXml,FALSE,pszCaReqBuffer)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"报文[%s]非法\n",pszCaReqBuffer);
		DeleteXml(pstruXml);
		return -1;
	}
    memset(szMsgCont, 0, sizeof(szMsgCont));
	strcpy(szMobile, DemandStrInXmlExt(pstruXml,OMC_MOBILE_PATH));
	strcpy(szMsgCont, DemandStrInXmlExt(pstruXml,OMC_MSGCONT_PATH));
	strcpy(szSPNumber, DemandStrInXmlExt(pstruXml,OMC_SPNUMBER_PATH));
	DeleteXml(pstruXml);
	
	if (strcmp(szMsgCont, "smssend") != 0)
	DecodeAndProcessSms(szMsgCont, szMobile, szSPNumber);
	/*
	int nCurProtocol=GetProtocolFrom(szMobile);
    if(nCurProtocol==PROTOCOL_2G)
	{
		DecodeAndProcessSms(szMsgCont, szMobile, szSPNumber);
	}
	*/
	return NORMAL;
	
}


/*
 * 处理GPRS查询设置交易
 */
RESULT ProcessGprsQrySetTrans(INT nSock, PSTR pszCaReqBuffer)
{
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	STR	szCaRespBuffer[MAX_BUFFER_LEN];		/* 渠道应答通讯报文 */
	STR szTemp[10], szBuffer[MAX_BUFFER_LEN];
	int nDetail=0, nTimes;

	memset(pstruXml,0,sizeof(XMLSTRU));
	if(ImportXml(pstruXml,FALSE,pszCaReqBuffer)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"报文[%s]非法\n",pszCaReqBuffer);
		DeleteXml(pstruXml);
		return -1;
	}
    int nRepeaterId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<repeaterid>"));
	int nDeviceId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<deviceid>"));
	
	//更新网元gprs联机状态
	sprintf(szSql,"update ne_element set ne_state = '1' where  ne_repeaterid = %d and ne_deviceid = %d ", 
	    nRepeaterId, nDeviceId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();
	
	for(nTimes=0; nTimes< 22; nTimes++)
	{
		//等待消息
		sleep(5);
		memset(szSql, 0, sizeof(szSql));
		sprintf(szSql, "select qs_netflag,  qs_telephonenum, qs_content from NE_GPRSQUEUE where QS_REPEATERID = %d and QS_DEVICEID = %d and QS_MSGSTAT = '0' order by QS_ID",
		        nRepeaterId, nDeviceId);
		//PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
		if(SelectTableRecord(szSql, &struCursor) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", szSql, GetSQLErrorMessage());
			return EXCEPTION;
		}
		while(FetchCursor(&struCursor) == NORMAL)
		{
			nDetail++;
			bufclr(szBuffer);
			InsertInXmlRel(pstruXml, "<omc>/<明细>", nDetail,	"<流水号>", 
					GetTableFieldValue(&struCursor, "qs_netflag"), MODE_BRANCHREL);
			strcpy(szBuffer, GetTableFieldValue(&struCursor, "qs_content"));
			InsertInXmlRel(pstruXml, "<omc>/<明细>", nDetail,	"<内容>", szBuffer, MODE_BRANCHREL);
		}
		FreeCursor(&struCursor);
		if (nDetail > 0)//有消息
		{
			sprintf(szSql,"update NE_GPRSQUEUE set qs_msgstat = '1' where  qs_repeaterid = %d and qs_deviceid = %d ", 
			    nRepeaterId, nDeviceId);
			PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
			if(ExecuteSQL(szSql)!=NORMAL)
			{
				PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
				return EXCEPTION;
			}
			CommitTransaction();
			
			sprintf(szTemp, "%d", nDetail);
			InsertInXmlExt(pstruXml, "<omc>/<明细数>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
			/* 
			 * 返回gprsrecv应答报文 
			 */
			memset(szCaRespBuffer, 0, sizeof(szCaRespBuffer));
			ExportXml(pstruXml, szCaRespBuffer, sizeof(szCaRespBuffer));
			
			if(SendCaRespPacket(nSock, szCaRespBuffer, strlen(szCaRespBuffer)) != NORMAL)
			{
				PrintErrorLog(DBG_HERE, "发送GPRS查询设置报文错误!\n");
			}
			PrintDebugLog(DBG_HERE, "发送GPRS查询设置报文[%s]\n", szCaRespBuffer);
			
		    break;
		}
		else
			continue;
	}
	
	DeleteXml(pstruXml);
	
	//更新脱机状态	
	return NORMAL;
	
}


/*
 * 处理GPRS脱机交易
 */
RESULT ProcessGprsOffLine(PSTR pszCaReqBuffer)
{
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	STR szSql[MAX_SQL_LEN];

	memset(pstruXml,0,sizeof(XMLSTRU));
	if(ImportXml(pstruXml,FALSE,pszCaReqBuffer)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"报文[%s]非法\n",pszCaReqBuffer);
		DeleteXml(pstruXml);
		return -1;
	}
    int nRepeaterId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<repeaterid>"));
	int nDeviceId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<deviceid>"));
	
	//更新网元gprs脱机状态
	sprintf(szSql,"update ne_element set ne_state = '0' where  ne_repeaterid = %d and ne_deviceid = %d ", 
	    nRepeaterId, nDeviceId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();

	DeleteXml(pstruXml);
	
	//更新脱机状态	
	return NORMAL;
	
}

/*
 * 处理状态报告
 */
RESULT ProcessSmsStatusReport(PSTR pszCaReqBuffer)
{
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	char szSql[MAX_BUFFER_LEN];
	STR szMobile[25];			/* 手机号 */
	STR szMsgStat[8], szStat[100];			/*  短信状态*/
	int nNeId, nMsgSerial, nStat=-1;			/*  网元号*/
	STR szType[3];
	TBINDVARSTRU struBindVar;

	memset(pstruXml,0,sizeof(XMLSTRU));
	if(ImportXml(pstruXml,FALSE,pszCaReqBuffer)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"报文[%s]非法\n",pszCaReqBuffer);
		DeleteXml(pstruXml);
		return -1;
	}
    strcpy(szType, DemandStrInXmlExt(pstruXml, "<omc>/<类型>"));
	strcpy(szMobile, DemandStrInXmlExt(pstruXml,OMC_MOBILE_PATH));
	strcpy(szMsgStat, DemandStrInXmlExt(pstruXml, "<omc>/<状态>"));
	nNeId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<网元>"));
	nMsgSerial = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<日志号>"));
	DeleteXml(pstruXml);
	
	if (strcmp(szMsgStat, "DELIVRD") == 0)
	{
	    nStat = 1;
	    strcpy(szStat, "短信已到达");
	}
	else
	{
		/*
		if (strncmp(szMsgStat, "EXPIRED", 7) == 0)
            strcpy(szStat, "用户关机或者不在服务区");
        else if (strncmp(szMsgStat, "DELETED", 7) == 0)
            strcpy(szStat, "网关删除信息");
        else if (strncmp(szMsgStat, "UNDELIV", 7) == 0)
            strcpy(szStat, "用户停机等状态");
        else if (strncmp(szMsgStat, "ACCEPTD", 7) == 0)
            strcpy(szStat, "消息被认可");
        else if (strncmp(szMsgStat, "UNKNOWN", 7) == 0)
            strcpy(szStat, "未知状态");
        else if (strncmp(szMsgStat, "REJECTD", 7) == 0)
            strcpy(szStat, "某些原因被拒绝");
        else if (strncmp(szMsgStat, "MK", 2) == 0)
            strcpy(szStat, "用户停机等状态");
        else if (strncmp(szMsgStat, "MI", 2) == 0)
            strcpy(szStat, "用户关机或者不在服务区");
        else
        */
	    	strcpy(szStat, "短信无法到达");
	 }
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_INT;
	struBindVar.struBind[0].VarValue.nValueInt = nStat;
	struBindVar.nVarCount++;
	
	struBindVar.struBind[1].nVarType = SQLT_INT;
	struBindVar.struBind[1].VarValue.nValueInt = nMsgSerial;
	struBindVar.nVarCount++;
	
	if (strcmp(szType, "12") == 0 || strcmp(szType, "22") == 0)
	    sprintf(szSql, "update man_elesetlog set  set_SmsStatus= :v_0,set_SmsTime = sysdate where set_elesetlogid = :v_1");
    else 
        sprintf(szSql, "update man_eleqrylog set  qry_SmsStatus= :v_0,qry_SmsTime = sysdate where qry_eleqrylogid = :v_1");
    PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%d][%d]\n",szSql, nStat,  nMsgSerial);
    if(BindExecuteSQL(szSql, &struBindVar) !=NORMAL ) 
	{
	    PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
	    return EXCEPTION;
	}
	
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_STR; 
	strcpy(struBindVar.struBind[0].VarValue.szValueChar, szStat);
	struBindVar.nVarCount++;
	
	struBindVar.struBind[1].nVarType = SQLT_INT;
	struBindVar.struBind[1].VarValue.nValueInt = nNeId;
	struBindVar.nVarCount++;
	
	sprintf(szSql, "update man_smsstatus set smsstatus= :v_0, eventtime= sysdate where neid= :v_1");
	if(BindExecuteSQL(szSql, &struBindVar) !=NORMAL ) 
	{
	    PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
	    return EXCEPTION;
	}
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%d]\n",szSql, szStat,  nNeId);
	if(GetAffectedRows()<1)
	{
		sprintf(szSql,"insert into man_smsstatus (neid, smsstatus, eventtime) values(%d, '%s', sysdate)",
		        nNeId, szStat);
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
		if(ExecuteSQL(szSql)!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
			return EXCEPTION;
		}
	}
	CommitTransaction();
	return NORMAL;
}

/* 
 * 轮训任务 
 */
RESULT ProcessTurnTask(PSTR pszCaReqBuffer)
{
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	REPEATER_INFO struRepeater;
	COMMANDHEAD struHead;
	int nTaskId, nTaskLogId;	/* 任务号 */
	int nProStyle;          /* 处理类型*/
	int nProtocolTypeId, nProtocolDeviceTypeId, nDeviceTypeId, nPollDeviceTypeId;
	int nDeviceStatusId, nNeId;
	int nFailEleCount=0, nObjCount, i=0;
	int nEleCount=0;
	int nTxPackCount=0;
	STR szQryEleParam[MAX_BUFFER_LEN];
	STR szSetEleParam[MAX_BUFFER_LEN];
	STR szSetEleValue[MAX_BUFFER_LEN];
	STR szServerTelNum[20], szTemp[200];
	//STR szCmdObjectList[MAX_BUFFER_LEN];
	PSTR pszSepParamStr[MAX_OBJECT_NUM];  /* 分割字符数组*/
	PSTR pszSepValueStr[MAX_OBJECT_NUM];  /* 分割字符数组*/
	//STR szBuffer[MAX_BUFFER_LEN];
	STR szStyle[10];
	STR szFilterNeId[MAX_BUFFER_LEN];
	int nFilterLen=0;
	int nNowTime, nTimes=0;
	STR szUploadTime[100];
	STR szTaskQryParm[1000], szBase[2000], szRadio[2000], szAlarm[2000], szAlarmen[2000];
	STR szRealTime[2000], szRadioSC[2000], szObjList[2000], szSepStr[2000];
	STR szBasePoll[2000], szRadioPoll[2000], szAlarmPoll[2000], szAlarmenPoll[2000];
	STR szRealTimePoll[2000], szRadioSCPoll[2000];
	PSTR pszTempStr[ MAX_OBJECT_NUM];
	INT nSeperateNum;

 	memset(pstruXml,0,sizeof(XMLSTRU));
	if(ImportXml(pstruXml,FALSE,pszCaReqBuffer)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"报文[%s]非法\n",pszCaReqBuffer);
		DeleteXml(pstruXml);
		return -1;
	}
	nTaskId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<taskid>"));
	
	strcpy(szStyle, DemandStrInXmlExt(pstruXml,"<omc>/<style>"));

    //记录任务日志

    InsertTaskLog(nTaskId, &nTaskLogId, szStyle);
    sprintf(szTemp, "%d", nTaskId);
    InsertInXmlExt(pstruXml,"<omc>/<任务号>",  szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
    sprintf(szTemp, "%d", nTaskLogId);
    InsertInXmlExt(pstruXml,"<omc>/<任务日志号>",  szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
    
    if (getAgentState(szServerTelNum) == BOOLTRUE)
		InsertInXmlExt(pstruXml,"<omc>/<服务状态>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
	else
		InsertInXmlExt(pstruXml,"<omc>/<服务状态>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
	
    memset(szFilterNeId, 0, sizeof(szFilterNeId));
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "select TSK_FILTER from man_task where tsk_taskid = %d", nTaskId);
    PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
    if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
    if(FetchCursor(&struCursor) == NORMAL)
	{
		sprintf(szFilterNeId, "%s", (GetTableFieldValue(&struCursor, "TSK_FILTER")));
		TrimRightChar(szFilterNeId, ',');
		nFilterLen = strlen(szFilterNeId);
	}
	FreeCursor(&struCursor);
	
	memset(szSql, 0, sizeof(szSql));
	snprintf(szSql, sizeof(szSql), "select * from man_TaskDetail where tkd_TaskId = %d", nTaskId);
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
    if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]无数据\n", szSql);
		FreeCursor(&struCursor);
		return EXCEPTION;
	}
	else
	{
		nProStyle = atoi(GetTableFieldValue(&struCursor, "TKD_STYLE"));
	    if (nProStyle == 0)  //快速查询,常规查询，立即查询 0为查询:COMMAND_QUERY
	    {
             strcpy(szTaskQryParm, (GetTableFieldValue(&struCursor, "tkd_qryparam")));
		     strcpy(szBase, (GetTableFieldValue(&struCursor, "tkd_base")));
			 strcpy(szRadio, (GetTableFieldValue(&struCursor, "tkd_radio")));
			 strcpy(szAlarmen, (GetTableFieldValue(&struCursor, "tkd_alarmen")));
			 strcpy(szAlarm, (GetTableFieldValue(&struCursor, "tkd_alarm")));
			 strcpy(szRadioSC, (GetTableFieldValue(&struCursor, "tkd_radiosc")));
			 strcpy(szRealTime, (GetTableFieldValue(&struCursor, "tkd_realtime")));
        } 
        else //设置 1为查询:COMMAND_SET
        {
            memset(szSetEleParam, 0, sizeof(szSetEleParam));
          	strcpy(szSetEleParam, (GetTableFieldValue(&struCursor, "tkd_setparam")));
            
            memset(szSetEleValue, 0, sizeof(szSetEleValue));
            strcpy(szSetEleValue, (GetTableFieldValue(&struCursor, "tkd_setvalue")));
        }
		FreeCursor(&struCursor);
	}
	
	//取任务明细
	memset(szSql, 0, sizeof(szSql));

	//2009.12.9 add 过滤机制
	if(nFilterLen > 1)
		snprintf(szSql, sizeof(szSql), "select a.*, b.ne_neid,b.ne_protocoldevicetypeid,b.ne_devicetypeid,b.ne_protocoltypeid,b.ne_devicestatusid, b.ne_commtypeid, b.ne_netelnum, b.ne_telnum1, b.ne_telnum2,b.ne_telnum3,b.ne_telnum4,b.ne_telnum5,"
	  		" b.ne_servertelnum, b.ne_repeaterid, b.ne_deviceid, b.ne_deviceip, b.ne_deviceport,  b.ne_devicemodelid, ne_objlist, ne_ActiveCol,ne_ActiveRow, ne_AlarmObjList, ne_AlarmEnabledObjList  from ne_element b left join man_taskpoll a on b.ne_devicetypeid = a.devicetypeid where  (b.ne_devicestatusid=0 or b.ne_devicestatusid=17) and b.ne_neid in (%s)",
	        	szFilterNeId);
	else
		snprintf(szSql, sizeof(szSql), "select a.*, b.ne_neid,b.ne_protocoldevicetypeid,b.ne_devicetypeid,b.ne_protocoltypeid,b.ne_devicestatusid, b.ne_commtypeid, b.ne_netelnum, b.ne_telnum1, b.ne_telnum2,b.ne_telnum3,b.ne_telnum4,b.ne_telnum5,"
	  		" b.ne_servertelnum, b.ne_repeaterid, b.ne_deviceid, b.ne_deviceip, b.ne_deviceport,  b.ne_devicemodelid, ne_objlist, ne_ActiveCol,ne_ActiveRow, ne_AlarmObjList, ne_AlarmEnabledObjList  from ne_element b left join man_taskpoll a on b.ne_devicetypeid = a.devicetypeid  where (b.ne_devicestatusid=0 or b.ne_devicestatusid=17) and b.ne_devicetypeid <> 146");
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	nNowTime = (int)time(NULL);	
    while(FetchCursor(&struCursor) == NORMAL)
	{
		nDeviceTypeId = atoi(GetTableFieldValue(&struCursor, "ne_devicetypeid"));
	    nProtocolTypeId= atoi(GetTableFieldValue(&struCursor, "ne_protocoltypeid"));
        nProtocolDeviceTypeId= atoi(GetTableFieldValue(&struCursor, "ne_protocoldevicetypeid"));
        
        nDeviceStatusId= atoi(GetTableFieldValue(&struCursor, "ne_devicestatusid"));
        strcpy(szServerTelNum,  (GetTableFieldValue(&struCursor, "ne_servertelnum")));
        
        InsertInXmlExt(pstruXml,"<omc>/<网元编号>", GetTableFieldValue(&struCursor, "ne_neid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<设备类型>", GetTableFieldValue(&struCursor, "ne_protocoldevicetypeid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<协议类型>", GetTableFieldValue(&struCursor, "ne_protocoltypeid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<站点编号>", GetTableFieldValue(&struCursor, "ne_repeaterid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<设备编号>", GetTableFieldValue(&struCursor, "ne_deviceid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<站点电话>", GetTableFieldValue(&struCursor, "ne_netelnum"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<站点IP>", GetTableFieldValue(&struCursor, "ne_deviceip"), MODE_AUTOGROW|MODE_UNIQUENAME);
        
        //InsertInXmlExt(pstruXml,"<omc>/<其它标识>", GetTableFieldValue(&struCursor, "ne_otherdeviceid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<设备型号>", GetTableFieldValue(&struCursor, "ne_devicemodelid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<端口号>", GetTableFieldValue(&struCursor, "ne_deviceport"), MODE_AUTOGROW|MODE_UNIQUENAME);

        InsertInXmlExt(pstruXml,"<omc>/<通信方式>", GetTableFieldValue(&struCursor, "ne_commtypeid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码>",  szServerTelNum, MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码1>", (GetTableFieldValue(&struCursor, "ne_telnum1")), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码2>", (GetTableFieldValue(&struCursor, "ne_telnum2")), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码3>", (GetTableFieldValue(&struCursor, "ne_telnum3")), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码4>", (GetTableFieldValue(&struCursor, "ne_telnum4")), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码5>", (GetTableFieldValue(&struCursor, "ne_telnum5")), MODE_AUTOGROW|MODE_UNIQUENAME);
		
		strcpy(szObjList, (GetTableFieldValue(&struCursor, "ne_objlist")));
		
		memset(szBasePoll, 0, sizeof(szBasePoll));
		memset(szRadioPoll, 0, sizeof(szRadioPoll));
		memset(szAlarmenPoll, 0, sizeof(szAlarmenPoll));
		memset(szAlarmPoll, 0, sizeof(szAlarmPoll));
		memset(szRadioSCPoll, 0, sizeof(szRadioSCPoll));
		memset(szRealTimePoll, 0, sizeof(szRealTimePoll));
		nPollDeviceTypeId = atoi(GetTableFieldValue(&struCursor, "devicetypeid"));

		if (nPollDeviceTypeId > 0)
		{
			strcpy(szBasePoll, GetTableFieldValue(&struCursor, "base_id"));
			strcpy(szRadioPoll, GetTableFieldValue(&struCursor, "radio_id"));
			strcpy(szAlarmenPoll, GetTableFieldValue(&struCursor, "alarmen_id"));
			strcpy(szAlarmPoll, GetTableFieldValue(&struCursor, "alarm_id"));
			strcpy(szRadioSCPoll, GetTableFieldValue(&struCursor, "radiosc_id"));
			strcpy(szRealTimePoll, GetTableFieldValue(&struCursor, "realtime_id"));
		}
		
        //为查询:COMMAND_QUERY
        if (nProStyle == 0)
        {
            InsertInXmlExt(pstruXml,"<omc>/<命令号>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
            InsertInXmlExt(pstruXml,"<omc>/<类型>", "11", MODE_AUTOGROW|MODE_UNIQUENAME);
        }
        else
        {
        	if(strcmp(szStyle, "201") == 0 || strcmp(szStyle, "202") == 0 || 
        	   strcmp(szStyle, "211") == 0 || strcmp(szStyle, "212") == 0)
				InsertInXmlExt(pstruXml,"<omc>/<命令号>", "181", MODE_AUTOGROW|MODE_UNIQUENAME);
			else
            	InsertInXmlExt(pstruXml,"<omc>/<命令号>", "2", MODE_AUTOGROW|MODE_UNIQUENAME);
            InsertInXmlExt(pstruXml,"<omc>/<类型>", "22", MODE_AUTOGROW|MODE_UNIQUENAME);
        }
        
        //站点等级为普通:OMC_NORMAL_MSGLEVEL
        InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_NORMAL_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
        
                
        memset(&struHead, 0, sizeof(COMMANDHEAD));
        memset(&struRepeater, 0, sizeof(REPEATER_INFO));
        //赋值
        nNeId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<网元编号>"));  
	    struRepeater.nCommType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<通信方式>"));  
	    struRepeater.nRepeaterId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<站点编号>")); 
	    struRepeater.nDeviceId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<设备编号>")); 
	    strcpy(struRepeater.szTelephoneNum, DemandStrInXmlExt(pstruXml,"<omc>/<站点电话>"));
	    strcpy(struRepeater.szIP, DemandStrInXmlExt(pstruXml,"<omc>/<站点IP>"));
	    
	    if (strlen(DemandStrInXmlExt(pstruXml,"<omc>/<端口号>")) == 0)
	    	struRepeater.nPort = 0;
	    else
	    	struRepeater.nPort = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<端口号>")); 
	    
	    struRepeater.nProtocolDeviceType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<设备类型>"));
	    strcpy(struRepeater.szSpecialCode, DemandStrInXmlExt(pstruXml,"<omc>/<其它标识>"));
	    strcpy(struRepeater.szReserve, DemandStrInXmlExt(pstruXml,"<omc>/<设备型号>"));
	    strcpy(struRepeater.szNetCenter, DemandStrInXmlExt(pstruXml,"<omc>/<服务号码>"));
	    
	    struHead.nProtocolType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<协议类型>"));
	    struHead.nCommandCode = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<命令号>"));
	    nEleCount ++; //网元数
	    
	    // 定时下行拨测任务100 上行103
	    if (strcmp(szStyle, "100") == 0)
	    {
	        InsertMosTask(pstruXml);
	        continue;
	    }
	    memset(szQryEleParam, 0, sizeof(szQryEleParam));
	    if (nProStyle == 0)  //快速查询,常规查询，立即查询 0为查询:COMMAND_QUERY
	    {
		     //不轮训基本型效果2.0
		     if ( (strcmp(szStyle, "1") == 0 || strcmp(szStyle, "2") == 0) &&
				    nDeviceTypeId == 146)
			    continue;

             if (strlen(szServerTelNum) == 0 && struRepeater.nCommType != 6)
             {
                 nFailEleCount ++;
                 InsertFailNeid(nTaskLogId, nNeId, "电话号码校验失败", "网元监控中心号码为空");
                 continue;
             }
                          
             if (struHead.nProtocolType == PROTOCOL_2G || 
             	 struHead.nProtocolType == PROTOCOL_DAS ||
             	 struHead.nProtocolType == PROTOCOL_JINXIN_DAS)
             {
             	 if (strcmp(szStyle, "1") == 0) //常规轮训
		     	 {
		     	 	 memset(szRadio, 0, sizeof(szRadio));
		     	 	 strcpy(szRadio, (GetTableFieldValue(&struCursor, "ne_ActiveRow")));
		     	 	 nSeperateNum = SeperateString(szRadio,  ',', pszTempStr,  MAX_OBJECT_NUM);
		             for(i=0; i< nSeperateNum; i++)
		             {
		             	memset(szTemp, 0, sizeof(szTemp));
		             	strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]));
		             	sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		             }
		             PrintDebugLog(DBG_HERE, "neid[%d]EleParam[%s]\n", nNeId, szQryEleParam);
		     	 }
		     	 	
		     	 if (strcmp(szStyle, "213") == 0)//批量查询
		     	 {
		     	 	 	     	 	 
			 	     if (strstr(szTaskQryParm, "base") != NULL && strlen(szBase) >= 4)                    
			     	 {
	             	 	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 	 strcpy(szSepStr, szBase);
				 	     nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
		             	 for(i=0; i< nSeperateNum; i++)
		             	 {
							memset(szTemp, 0, sizeof(szTemp));
		             	 	strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]));
		             	 	if (nPollDeviceTypeId > 0 && strstr(szBasePoll, szTemp) == NULL)
		             	 		continue;
		             	 	//if (strstr(szObjList, szTemp) != NULL)
		                 		sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		                 }
			     	 }
			     	 if (strstr(szTaskQryParm, "alarmen") != NULL && strlen(szAlarmen) >= 4)
			     	 {
		             	 memset(szSepStr, 0, sizeof(szSepStr));
		             	 strcpy(szSepStr, szAlarmen);
				 	     nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
				 	     for(i=0; i< nSeperateNum; i++)
				 	     {
							 memset(szTemp, 0, sizeof(szTemp));
					 	     strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]));
					 	     if (nPollDeviceTypeId > 0 && strstr(szAlarmenPoll, szTemp) == NULL)
			             	 		continue;
					 	     //if (strstr(szObjList, szTemp) != NULL)
					 	     	sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
				 	     }
			     	 }
			     	 if (strstr(szTaskQryParm, "radio") != NULL && strlen(szRadio) >= 4)
			     	 {
	             	 	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 	 strcpy(szSepStr, szRadio);
				 	     nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
		             	 for(i=0; i< nSeperateNum; i++)
		             	 {
							memset(szTemp, 0, sizeof(szTemp));
		             	 	strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]));
		             	 	if (nPollDeviceTypeId > 0 && strstr(szRadioPoll, szTemp) == NULL)
		             	 		continue;
		             	 	//if (strstr(szObjList, szTemp) != NULL)
		                 		sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		                 }
			     	 }
			     	 if (strstr(szTaskQryParm, "radiosc") != NULL && strlen(szRadioSC) >= 4)
			     	 {
	             	 	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 	 strcpy(szSepStr, szRadioSC);
				 	     nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
		             	 for(i=0; i< nSeperateNum; i++)
		             	 {
							memset(szTemp, 0, sizeof(szTemp));
		             	 	strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]));
		             	 	if (nPollDeviceTypeId > 0 && strstr(szRadioSCPoll, szTemp) == NULL)
		             	 		continue;
		             	 	//if (strstr(szObjList, szTemp) != NULL)
		                 		sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		                 }
			     	 }
			     	 if (strstr(szTaskQryParm, "realtime") != NULL && strlen(szRealTime) >= 4)
			     	 {
	             	 	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 	 strcpy(szSepStr, szRealTime);
				 	     nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
		             	 for(i=0; i< nSeperateNum; i++)
		             	 {
							memset(szTemp, 0, sizeof(szTemp));
		             	 	strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]));
		             	 	if (nPollDeviceTypeId > 0 && strstr(szRealTimePoll, szTemp) == NULL)
		             	 		continue;
		             	 	//if (strstr(szObjList, szTemp) != NULL)
		                 		sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		                 }
			     	 }
		     	 }
		     	 else//快速查询
		     	 {
		     	 	 memset(szAlarm, 0, sizeof(szAlarm));
		     	 	 //strcpy(szAlarmen, TrimAllSpace(GetTableFieldValue(&struCursor, "ne_AlarmEnabledObjList")));
			 	     strcpy(szAlarm, (GetTableFieldValue(&struCursor, "ne_AlarmObjList")));
			     	 //常规轮训考虑增加所有参量
			     	 PrintDebugLog(DBG_HERE, "neid[%d]alarm[%s]\n", nNeId, szAlarm);
	             }
	             
	             
             	 if (strstr(szTaskQryParm, "alarm") != NULL && strlen(szAlarm) >= 4)
	             {
	             	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 strcpy(szSepStr, szAlarm);
	             	 nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
	             	 for(i=0; i< nSeperateNum; i++)
	             	 {
						memset(szTemp, 0, sizeof(szTemp));
						if (strstr(pszTempStr[i], ":"))
	             	 		strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i])-2);
	             	 	else
	             	 		strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]));
						if (nPollDeviceTypeId > 0 && strstr(szAlarmPoll, szTemp) == NULL)
		             	 		continue;
						//if (strstr(szObjList, szTemp) != NULL)
                 		sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
	                 }
	             }
	                          
             	 
             	 PrintDebugLog(DBG_HERE, "neid[%d]taskQryParm[%s]EleParam[%s]\n", nNeId, szTaskQryParm, szQryEleParam);
		     	 	             
	             if (struRepeater.nCommType == 5 || struRepeater.nCommType == 6)
	             	ResolveQryParamArrayGprs(szQryEleParam);
	             else	
	             	ResolveQryParamArray(szQryEleParam);
	             //为空不轮训     
	             if (strlen(szQryEleParam) == 0) continue;
	             PrintDebugLog(DBG_HERE, "turn task[%s]\n", szQryEleParam);
	             
	             if (nTimes++>= 6)//校准发送时间
	        	 {
	        	 	 nNowTime ++;
	        	 	 nTimes = 0;
	        	 	 if (nNowTime - 5000 >=(int)time(NULL))
	        	 	 	 nNowTime = (int)time(NULL);
	        	 }

	             nObjCount = SeperateString(szQryEleParam, '|', pszSepParamStr, MAX_SEPERATE_NUM);
	             for(i=0; i< nObjCount; i++)
		         {
		             
		             //监控对象用空格分割
	 	             //strcpy(szQryEleParam, "0301 0304 0704"); // for test
	 	             if (strlen(szUploadTime) == 14)
	 	             	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", szUploadTime, MODE_AUTOGROW|MODE_UNIQUENAME);
	 	             else 
	 	             	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime(nNowTime+i*5000), MODE_AUTOGROW|MODE_UNIQUENAME);
	 	             
		             InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  pszSepParamStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
	            	 
	            	 if (struRepeater.nCommType == 5 || struRepeater.nCommType == 6)
	            	 {
	            	 	QryElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
	            	 	SaveToGprsQueue(pstruXml);
	            	 }
	            	 else
	            	 {
			         	QryElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
			         	
			         	//动态分配特服务号
			         	//DistServerTelNum(pstruXml);
			         	SaveToMsgQueue_Tmp(pstruXml);
			         }
			         SaveEleQryLog(pstruXml);
			         nTxPackCount++;
			         //if (nTaskId != 11932 && i>= 1) break;
			         	
		         }
		     }
		    else if (struHead.nProtocolType == PROTOCOL_GSM ||  //add by wwj at 2010.07.28
				struHead.nProtocolType == PROTOCOL_CDMA ||
				struHead.nProtocolType == PROTOCOL_HEIBEI ||
				struHead.nProtocolType == PROTOCOL_XC_CP ||
				struHead.nProtocolType == PROTOCOL_SUNWAVE ||
				struHead.nProtocolType == PROTOCOL_WLK)
			{
				//选取~2G协议COMMADOBJECTS
				CURSORSTRU struCursor_CG;
				char szSpoolQryParam[100];
				char szObjects[2000];
				int nCmdCode;
				
				sprintf(szSql, "select CDO_COMMANDCODE, CDO_SPOOLQUERYPARAM, CDO_OBJECTS  from ne_commandobjects where CDO_PROTOCOLTYPE = %d", struHead.nProtocolType);

				PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
				if(SelectTableRecord(szSql, &struCursor_CG) != NORMAL)
				{
					PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
								  szSql, GetSQLErrorMessage());
					FreeCursor(&struCursor_CG);			  
					return EXCEPTION;
				}
			    while(FetchCursor(&struCursor_CG) == NORMAL)
			    {		
			    	strcpy(szSpoolQryParam,  TrimAllSpace(GetTableFieldValue(&struCursor_CG, "CDO_SPOOLQUERYPARAM")));
			    	strcpy(szObjects,  TrimAllSpace(GetTableFieldValue(&struCursor_CG, "CDO_OBJECTS")));				
					nCmdCode = atoi(GetTableFieldValue(&struCursor_CG, "CDO_COMMANDCODE"));
					
					if ((strstr(szSpoolQryParam, "base") != NULL && strstr(szTaskQryParm, "base") != NULL) ||
						(strstr(szSpoolQryParam, "radio") != NULL && strstr(szTaskQryParm, "radio") != NULL) ||
						(strstr(szSpoolQryParam, "alarm") != NULL && strstr(szTaskQryParm, "alarm") != NULL) ||
						(strstr(szSpoolQryParam, "alarmen") != NULL && strstr(szTaskQryParm, "alarmen") != NULL))
					{	
						struHead.nCommandCode = nCmdCode;
						//发送报文
						nNowTime = (int)time(NULL);	
						InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime(nNowTime), MODE_AUTOGROW|MODE_UNIQUENAME);
				        InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  szObjects, MODE_AUTOGROW|MODE_UNIQUENAME);
				        PrintDebugLog(DBG_HERE, "mark1[%s]\n", szObjects);
						QryElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
					    SaveToMsgQueue_Tmp(pstruXml);
					    SaveEleQryLog(pstruXml);
					    nTxPackCount++;
					    if (nTaskId == 4 && i>= 1)	break;
			         	if (nTaskId != 4 && i>= 0) break;
					}
				}
				FreeCursor(&struCursor_CG);	   
				
			}	
		     


        } 
        else //设置 1为查询:COMMAND_SET
        {
        	
             //设备状态不正常
            /*if (nDeviceStatusId > 10)
            {
                nFailEleCount ++;
                InsertFailNeid(nTaskLogId, nNeId,  "轮询失败",  GetDeviceStatu(nDeviceStatusId));
                continue;
            }*/
            if (strlen(szServerTelNum) == 0 && struRepeater.nCommType != 6)
            {
                nFailEleCount ++;
                InsertFailNeid(nTaskLogId, nNeId, "电话号码校验失败", "网元监控中心号码为空");
                continue;
            }
            /******************** 
            memset(szSetEleParam, 0, sizeof(szSetEleParam));
            //strcpy(szSetEleParam, TrimAllSpace(GetTableFieldValue(&struCursor, "tkd_base")));
           
            //if (strcmp(szStyle, "214") == 0)//批量设置 web填参需要修改 add by wwj at 2010.07.28
            	strcpy(szSetEleParam, TrimAllSpace(GetTableFieldValue(&struCursor, "tkd_setparam")));
            
            memset(szSetEleValue, 0, sizeof(szSetEleValue));
            strcpy(szSetEleValue, TrimAllSpace(GetTableFieldValue(&struCursor, "tkd_setvalue")));
           ******************/
 
            if(strcmp(szStyle, "201") == 0 && strcmp(szSetEleValue, "300000000") == 0 )
            {
             	time_t struTimeNow;
			  	struct tm struTmNow;
			  
			  	time(&struTimeNow);
			    memset(szSetEleValue,0,sizeof(szSetEleValue));
			  	struTmNow=*localtime(&struTimeNow);
			  	//3YYYYMMDD 7位
			  	snprintf(szSetEleValue,sizeof(szSetEleValue),"3%04d%02d%02d",
			  	struTmNow.tm_year+1900,struTmNow.tm_mon+1,struTmNow.tm_mday-1);
            }
            
            //远程升级, 特殊命令流水号为1
            if(strcmp(szStyle, "200") == 0)
            {
            	InsertInXmlExt(pstruXml,"<omc>/<流水号>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
            }
                                       
            ResolveSetParamArray(szSetEleParam, szSetEleValue);
            PrintDebugLog(DBG_HERE, "轮训设置对象[%s][%s]\n", szSetEleParam, szSetEleValue);
            
            if (nTimes++>= 6)//校准发送时间
	        {
	        	 nNowTime ++;
	        	 nTimes = 0;
	        	 if (nNowTime - 6000 >=(int)time(NULL))
	        	 	 nNowTime = (int)time(NULL);
	        }
	        	 
            nObjCount = SeperateString(szSetEleParam, '|', pszSepParamStr, MAX_SEPERATE_NUM);
            nObjCount = SeperateString(szSetEleValue, '|', pszSepValueStr, MAX_SEPERATE_NUM);
            for(i=0; i< nObjCount; i++)
	        {
	            //监控对象用空格分割
 	            
 	             if (strlen(szUploadTime) == 14)
 	            	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", szUploadTime, MODE_AUTOGROW|MODE_UNIQUENAME);
 	             else 
 	            	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime(nNowTime + i*6000), MODE_AUTOGROW|MODE_UNIQUENAME);
	             InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  pszSepParamStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
	             InsertInXmlExt(pstruXml,"<omc>/<监控对象内容>",  pszSepValueStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
            	 if (struRepeater.nCommType == 5 || struRepeater.nCommType == 6)
	             {
	            	 SetElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
	            	 SaveToGprsQueue(pstruXml);
	             }
	             else
	             {
		         	SetElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
		         	//动态分配特服务号
		         	//DistServerTelNum(pstruXml);
		         	SaveToMsgQueue_Tmp(pstruXml);
		         }
		         SaveEleSetLog(pstruXml);
		         nTxPackCount++;
	        }
	        
	        //远程升级
            if(strcmp(szStyle, "200") == 0)
            {
            	 RemortUpdateDbOperate1(pstruXml);
            }

        }
        
        //更新网元数或发送报文数
        if ((nEleCount >= 1000) || (nTxPackCount >= 1000))
        {
            UpdateTaskLogCount(nTaskLogId, nTaskId, nEleCount, nTxPackCount, nFailEleCount);
            nEleCount = 0;
            nTxPackCount = 0;
            nFailEleCount = 0;
            if (getTaskStopUsing(nTaskId) == BOOLTRUE)
                break;
        }
            
            

	}
    if ((nEleCount > 0) || (nTxPackCount > 0) || (nFailEleCount > 0))
	    UpdateTaskLogCount(nTaskLogId, nTaskId, nEleCount, nTxPackCount, nFailEleCount);
	FreeCursor(&struCursor);
    DeleteXml(pstruXml);

    return NORMAL;
	
}


/* 
 * 定时复询任务 
 */
RESULT ProcessTimeReTurnTask(PSTR pszCaReqBuffer)
{
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	REPEATER_INFO struRepeater;
	COMMANDHEAD struHead;
	int nTaskId, nTaskLogId;	/* 任务号 */
	int nProStyle=0;          /* 处理类型*/
	int nProtocolTypeId, nProtocolDeviceTypeId, nDeviceTypeId, nPollDeviceTypeId;
	int nDeviceStatusId, nNeId;
	int nFailEleCount=0, nObjCount, i=0;
	int nEleCount=0;
	int nTxPackCount=0;
	STR szQryEleParam[MAX_BUFFER_LEN];
	STR szServerTelNum[20], szTemp[200];
	//STR szCmdObjectList[MAX_BUFFER_LEN];
	PSTR pszSepParamStr[MAX_OBJECT_NUM];  /* 分割字符数组*/
	STR szStyle[10];
	int nNowTime, nTimes=0;
	STR szUploadTime[100];
	STR szTaskQryParm[1000], szBase[2000], szRadio[2000], szAlarm[2000], szAlarmen[2000];
	STR szRealTime[2000], szRadioSC[2000], szObjList[2000], szSepStr[2000];
	STR szBasePoll[2000], szRadioPoll[2000], szAlarmPoll[2000], szAlarmenPoll[2000];
	STR szRealTimePoll[2000], szRadioSCPoll[2000];
	PSTR pszTempStr[ MAX_OBJECT_NUM];
	INT nSeperateNum;

 	memset(pstruXml,0,sizeof(XMLSTRU));
	if(ImportXml(pstruXml,FALSE,pszCaReqBuffer)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"报文[%s]非法\n",pszCaReqBuffer);
		DeleteXml(pstruXml);
		return -1;
	}
	nTaskId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<taskid>"));
	
	strcpy(szStyle, DemandStrInXmlExt(pstruXml,"<omc>/<style>"));

    //记录任务日志
    if (strcmp(szStyle, "215") == 0)//复询
    InsertTaskLog(nTaskId, &nTaskLogId, szStyle);
    sprintf(szTemp, "%d", nTaskId);
    InsertInXmlExt(pstruXml,"<omc>/<任务号>",  szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
    sprintf(szTemp, "%d", nTaskLogId);
    InsertInXmlExt(pstruXml,"<omc>/<任务日志号>",  szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
    
 	InsertInXmlExt(pstruXml,"<omc>/<服务状态>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
	//快速复询任务参数
    strcpy(szTaskQryParm, "alarm,alarmen");
	
	//取任务明细
	memset(szSql, 0, sizeof(szSql));

	if (strcmp(szStyle, "216") == 0)//告警查询
		snprintf(szSql, sizeof(szSql), "select a.*, b.ne_neid,b.ne_protocoldevicetypeid,b.ne_devicetypeid,b.ne_protocoltypeid,b.ne_devicestatusid, b.ne_commtypeid, b.ne_netelnum, b.ne_telnum1, b.ne_telnum2,b.ne_telnum3,b.ne_telnum4,b.ne_telnum5,"
	  		" b.ne_servertelnum, b.ne_repeaterid, b.ne_deviceid, b.ne_deviceip, b.ne_deviceport, b.ne_otherdeviceid, b.ne_devicemodelid, ne_objlist, ne_ActiveCol,ne_ActiveRow, ne_AlarmObjList, ne_AlarmEnabledObjList  from ne_element b left join man_taskpoll a on b.ne_devicetypeid = a.devicetypeid where  (b.ne_devicestatusid=0 or b.ne_devicestatusid=17) and b.ne_neid in (select alg_neid from alm_alarmlog where alg_alarmstatusid = 1 and alg_alarmid in (1,2,9,10,11,12,13,14,15,20,31,74,141,158,167,168,169,200,201,202,203,204,205,206,207,210,211,212,213,214,215,216,217))");
	  		//" b.ne_servertelnum, b.ne_repeaterid, b.ne_deviceid, b.ne_deviceip, b.ne_deviceport, b.ne_otherdeviceid, b.ne_devicemodelid, ne_objlist, ne_ActiveCol,ne_ActiveRow, ne_AlarmObjList, ne_AlarmEnabledObjList  from ne_element b left join man_taskpoll a on b.ne_devicetypeid = a.devicetypeid where  b.ne_neid in (select alg_neid from alm_alarmlog where alg_alarmstatusid = 1 and alg_alarmid in (1,2,9,10,11,12,13,14,15,20,31,74,141,158,167,168,169,200,201,202,203,204,205,206,207,210,211,212,213,214,215,216,217))");
	else if (strcmp(szStyle, "215") == 0)//复询
		snprintf(szSql, sizeof(szSql), "select a.*, b.ne_neid,b.ne_protocoldevicetypeid,b.ne_devicetypeid,b.ne_protocoltypeid,b.ne_devicestatusid, b.ne_commtypeid, b.ne_netelnum, b.ne_telnum1, b.ne_telnum2,b.ne_telnum3,b.ne_telnum4,b.ne_telnum5,"
	  		" b.ne_servertelnum, b.ne_repeaterid, b.ne_deviceid, b.ne_deviceip, b.ne_deviceport, b.ne_otherdeviceid, b.ne_devicemodelid, ne_objlist, ne_ActiveCol,ne_ActiveRow, ne_AlarmObjList, ne_AlarmEnabledObjList  from ne_element b left join man_taskpoll a on b.ne_devicetypeid = a.devicetypeid  where (b.ne_devicestatusid=0 or b.ne_devicestatusid=17) "
	  		//" b.ne_servertelnum, b.ne_repeaterid, b.ne_deviceid, b.ne_deviceip, b.ne_deviceport, b.ne_otherdeviceid, b.ne_devicemodelid, ne_objlist, ne_ActiveCol,ne_ActiveRow, ne_AlarmObjList, ne_AlarmEnabledObjList  from ne_element b left join man_taskpoll a on b.ne_devicetypeid = a.devicetypeid  where "
	  		" and b.ne_lastupdatetime < to_date('%s000000', 'yyyymmddhh24miss')", GetSystemDate());
	  		//" b.ne_lastupdatetime < to_date('%s000000', 'yyyymmddhh24miss')", GetSystemDate());
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	nNowTime = (int)time(NULL);	
    while(FetchCursor(&struCursor) == NORMAL)
	{
		nDeviceTypeId = atoi(GetTableFieldValue(&struCursor, "ne_devicetypeid"));
	    nProtocolTypeId= atoi(GetTableFieldValue(&struCursor, "ne_protocoltypeid"));
        nProtocolDeviceTypeId= atoi(GetTableFieldValue(&struCursor, "ne_protocoldevicetypeid"));
        
        nDeviceStatusId= atoi(GetTableFieldValue(&struCursor, "ne_devicestatusid"));
        strcpy(szServerTelNum,  TrimAllSpace(GetTableFieldValue(&struCursor, "ne_servertelnum")));
        
        InsertInXmlExt(pstruXml,"<omc>/<网元编号>", GetTableFieldValue(&struCursor, "ne_neid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<设备类型>", GetTableFieldValue(&struCursor, "ne_protocoldevicetypeid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<协议类型>", GetTableFieldValue(&struCursor, "ne_protocoltypeid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<站点编号>", GetTableFieldValue(&struCursor, "ne_repeaterid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<设备编号>", GetTableFieldValue(&struCursor, "ne_deviceid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<站点电话>", GetTableFieldValue(&struCursor, "ne_netelnum"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<站点IP>", GetTableFieldValue(&struCursor, "ne_deviceip"), MODE_AUTOGROW|MODE_UNIQUENAME);
        
        InsertInXmlExt(pstruXml,"<omc>/<其它标识>", GetTableFieldValue(&struCursor, "ne_otherdeviceid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<设备型号>", GetTableFieldValue(&struCursor, "ne_devicemodelid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<端口号>", GetTableFieldValue(&struCursor, "ne_deviceport"), MODE_AUTOGROW|MODE_UNIQUENAME);

        InsertInXmlExt(pstruXml,"<omc>/<通信方式>", GetTableFieldValue(&struCursor, "ne_commtypeid"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码>",  szServerTelNum, MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码1>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_telnum1")), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码2>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_telnum2")), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码3>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_telnum3")), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码4>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_telnum4")), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码5>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_telnum5")), MODE_AUTOGROW|MODE_UNIQUENAME);
		
		strcpy(szObjList, TrimAllSpace(GetTableFieldValue(&struCursor, "ne_objlist")));
		
		memset(szBasePoll, 0, sizeof(szBasePoll));
		memset(szRadioPoll, 0, sizeof(szRadioPoll));
		memset(szAlarmenPoll, 0, sizeof(szAlarmenPoll));
		memset(szAlarmPoll, 0, sizeof(szAlarmPoll));
		memset(szRadioSCPoll, 0, sizeof(szRadioSCPoll));
		memset(szRealTimePoll, 0, sizeof(szRealTimePoll));
		nPollDeviceTypeId = atoi(GetTableFieldValue(&struCursor, "devicetypeid"));

		if (nPollDeviceTypeId > 0)
		{
			strcpy(szBasePoll, GetTableFieldValue(&struCursor, "base_id"));
			strcpy(szRadioPoll, GetTableFieldValue(&struCursor, "radio_id"));
			strcpy(szAlarmenPoll, GetTableFieldValue(&struCursor, "alarmen_id"));
			strcpy(szAlarmPoll, GetTableFieldValue(&struCursor, "alarm_id"));
			strcpy(szRadioSCPoll, GetTableFieldValue(&struCursor, "radiosc_id"));
			strcpy(szRealTimePoll, GetTableFieldValue(&struCursor, "realtime_id"));
		}
		
        //为查询:COMMAND_QUERY
        if (nProStyle == 0)
        {
            InsertInXmlExt(pstruXml,"<omc>/<命令号>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
            InsertInXmlExt(pstruXml,"<omc>/<类型>", "11", MODE_AUTOGROW|MODE_UNIQUENAME);
        }

        
        //站点等级为普通:OMC_NORMAL_MSGLEVEL
        InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_NORMAL_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
        
                
        memset(&struHead, 0, sizeof(COMMANDHEAD));
        memset(&struRepeater, 0, sizeof(REPEATER_INFO));
        //赋值
        nNeId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<网元编号>"));  
	    struRepeater.nCommType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<通信方式>"));  
	    struRepeater.nRepeaterId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<站点编号>")); 
	    struRepeater.nDeviceId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<设备编号>")); 
	    strcpy(struRepeater.szTelephoneNum, DemandStrInXmlExt(pstruXml,"<omc>/<站点电话>"));
	    strcpy(struRepeater.szIP, DemandStrInXmlExt(pstruXml,"<omc>/<站点IP>"));
	    
	    if (strlen(DemandStrInXmlExt(pstruXml,"<omc>/<端口号>")) == 0)
	    	struRepeater.nPort = 0;
	    else
	    	struRepeater.nPort = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<端口号>")); 
	    
	    struRepeater.nProtocolDeviceType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<设备类型>"));
	    strcpy(struRepeater.szSpecialCode, DemandStrInXmlExt(pstruXml,"<omc>/<其它标识>"));
	    strcpy(struRepeater.szReserve, DemandStrInXmlExt(pstruXml,"<omc>/<设备型号>"));
	    strcpy(struRepeater.szNetCenter, DemandStrInXmlExt(pstruXml,"<omc>/<服务号码>"));
	    
	    struHead.nProtocolType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<协议类型>"));
	    struHead.nCommandCode = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<命令号>"));
	    nEleCount ++; //网元数
	    
	    memset(szQryEleParam, 0, sizeof(szQryEleParam));
	    if (nProStyle == 0)  //快速查询,常规查询，立即查询 0为查询:COMMAND_QUERY
	    {
		     //不轮训基本型效果2.0
		     if ( (strcmp(szStyle, "1") == 0 || strcmp(szStyle, "2") == 0) &&
				    nDeviceTypeId == 146)
			    continue;

             if (strlen(szServerTelNum) == 0 && struRepeater.nCommType != 6)
             {
                 nFailEleCount ++;
                 InsertFailNeid(nTaskLogId, nNeId, "电话号码校验失败", "网元监控中心号码为空");
                 continue;
             }
                          
             if (struHead.nProtocolType == PROTOCOL_2G)
             {	
		     	 if (strcmp(szStyle, "213") == 0)//批量查询
		     	 {
		     	 	 	     	 	 
			 	     if (strstr(szTaskQryParm, "base") != NULL && strlen(szBase) >= 4)                    
			     	 {
	             	 	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 	 strcpy(szSepStr, szBase);
				 	     nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
		             	 for(i=0; i< nSeperateNum; i++)
		             	 {
							memset(szTemp, 0, sizeof(szTemp));
		             	 	strncpy(szTemp, pszTempStr[i], 4);
		             	 	if (nPollDeviceTypeId > 0 && strstr(szBasePoll, szTemp) == NULL)
		             	 		continue;
		             	 	//if (strstr(szObjList, szTemp) != NULL)
		                 		sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		                 }
			     	 }
			     	 if (strstr(szTaskQryParm, "radio") != NULL && strlen(szRadio) >= 4)
			     	 {
	             	 	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 	 strcpy(szSepStr, szRadio);
				 	     nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
		             	 for(i=0; i< nSeperateNum; i++)
		             	 {
							memset(szTemp, 0, sizeof(szTemp));
		             	 	strncpy(szTemp, pszTempStr[i], 4);
		             	 	if (nPollDeviceTypeId > 0 && strstr(szRadioPoll, szTemp) == NULL)
		             	 		continue;
		             	 	//if (strstr(szObjList, szTemp) != NULL)
		                 		sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		                 }
			     	 }
			     	 if (strstr(szTaskQryParm, "radiosc") != NULL && strlen(szRadioSC) >= 4)
			     	 {
	             	 	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 	 strcpy(szSepStr, szRadioSC);
				 	     nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
		             	 for(i=0; i< nSeperateNum; i++)
		             	 {
							memset(szTemp, 0, sizeof(szTemp));
		             	 	strncpy(szTemp, pszTempStr[i], 4);
		             	 	if (nPollDeviceTypeId > 0 && strstr(szRadioSCPoll, szTemp) == NULL)
		             	 		continue;
		             	 	//if (strstr(szObjList, szTemp) != NULL)
		                 		sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		                 }
			     	 }
			     	 if (strstr(szTaskQryParm, "realtime") != NULL && strlen(szRealTime) >= 4)
			     	 {
	             	 	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 	 strcpy(szSepStr, szRealTime);
				 	     nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
		             	 for(i=0; i< nSeperateNum; i++)
		             	 {
							memset(szTemp, 0, sizeof(szTemp));
		             	 	strncpy(szTemp, pszTempStr[i], 4);
		             	 	if (nPollDeviceTypeId > 0 && strstr(szRealTimePoll, szTemp) == NULL)
		             	 		continue;
		             	 	//if (strstr(szObjList, szTemp) != NULL)
		                 		sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		                 }
			     	 }
		     	 }
		     	 else//快速查询,常规查询
		     	 {
		     	 	 strcpy(szAlarmen, TrimAllSpace(GetTableFieldValue(&struCursor, "ne_AlarmEnabledObjList")));
			 	     strcpy(szAlarm, TrimAllSpace(GetTableFieldValue(&struCursor, "ne_AlarmObjList")));
			     	 //常规轮训考虑增加所有参量
	             }
	             PrintDebugLog(DBG_HERE, "deviceid[%d]alarmenpoll[%s]alarmen[%s]\n", nPollDeviceTypeId, szAlarmenPoll, szAlarmen);
	             
             	 if (strstr(szTaskQryParm, "alarm") != NULL && strlen(szAlarm) >= 4)
	             {
	             	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 strcpy(szSepStr, szAlarm);
	             	 nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
	             	 for(i=0; i< nSeperateNum; i++)
	             	 {
						memset(szTemp, 0, sizeof(szTemp));
	             	 	strncpy(szTemp, pszTempStr[i], 4);
	             	 	if (nPollDeviceTypeId > 0 && strstr(szAlarmPoll, szTemp) == NULL)
		             	 		continue;
	             	 	//if (strstr(szObjList, szTemp) != NULL)
	                 		sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
	                 }
	             }
	            
		/****************** 
	             if (strstr(szTaskQryParm, "alarmen") != NULL && strlen(szAlarmen) >= 4)
		     	 {
	             	 memset(szSepStr, 0, sizeof(szSepStr));
	             	 strcpy(szSepStr, szAlarmen);
			 	     nSeperateNum = SeperateString(szSepStr,  ',', pszTempStr,  MAX_OBJECT_NUM);
			 	     for(i=0; i< nSeperateNum; i++)
			 	     {
						 memset(szTemp, 0, sizeof(szTemp));
				 	     strncpy(szTemp, pszTempStr[i], 4);
				 	     if (nPollDeviceTypeId > 0 && strstr(szAlarmenPoll, szTemp) == NULL)
		             	 		continue;
				 	     //if (strstr(szObjList, szTemp) != NULL)
				 	     	sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
			 	     }
		     	 }
             	**********************/ 
             	 PrintDebugLog(DBG_HERE, "taskQryParm[%s]EleParam[%s]\n", szTaskQryParm, szQryEleParam);
		     	 	             
	             if (struRepeater.nCommType == 5 || struRepeater.nCommType == 6)
	             	ResolveQryParamArrayGprs(szQryEleParam);
	             else	
	             	ResolveQryParamArray(szQryEleParam);
	             //为空不轮训     
	             if (strlen(szQryEleParam) == 0) continue;
	             PrintDebugLog(DBG_HERE, "轮训对象[%s]\n", szQryEleParam);
	             
	             if (nTimes++>= 6)//校准发送时间
	        	 {
	        	 	 nNowTime ++;
	        	 	 nTimes = 0;
	        	 	 if (nNowTime - 600 >=(int)time(NULL))
	        	 	 	 nNowTime = (int)time(NULL);
	        	 }

	             nObjCount = SeperateString(szQryEleParam, '|', pszSepParamStr, MAX_SEPERATE_NUM);
	             for(i=0; i< nObjCount; i++)
		         {
		             
		             //监控对象用空格分割
	 	             //strcpy(szQryEleParam, "0301 0304 0704"); // for test
	 	             if (strlen(szUploadTime) == 14)
	 	             	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", szUploadTime, MODE_AUTOGROW|MODE_UNIQUENAME);
	 	             else 
	 	             	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime(nNowTime+i*600), MODE_AUTOGROW|MODE_UNIQUENAME);
	 	             
		             InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  pszSepParamStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
	            	 
	            	 if (struRepeater.nCommType == 5 || struRepeater.nCommType == 6)
	            	 {
	            	 	QryElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
	            	 	SaveToGprsQueue(pstruXml);
	            	 }
	            	 else
	            	 {
			         	QryElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
			         	
			         	//动态分配特服务号
			         	//DistServerTelNum(pstruXml);
			         	SaveToMsgQueue_Tmp(pstruXml);
			         }
			         SaveEleQryLog(pstruXml);
			         nTxPackCount++;
			         //if (nTaskId != 11932 && i>= 1) break;
			         	
		         }
		     }
		    else if (struHead.nProtocolType == PROTOCOL_GSM ||  //add by wwj at 2010.07.28
				struHead.nProtocolType == PROTOCOL_CDMA ||
				struHead.nProtocolType == PROTOCOL_HEIBEI ||
				struHead.nProtocolType == PROTOCOL_XC_CP ||
				struHead.nProtocolType == PROTOCOL_SUNWAVE ||
				struHead.nProtocolType == PROTOCOL_WLK)
			{
				//选取~2G协议COMMADOBJECTS
				CURSORSTRU struCursor_CG;
				char szSpoolQryParam[100];
				char szObjects[2000];
				int nCmdCode;
				
				sprintf(szSql, "select CDO_COMMANDCODE, CDO_SPOOLQUERYPARAM, CDO_OBJECTS  from ne_commandobjects where CDO_PROTOCOLTYPE = %d", struHead.nProtocolType);

				PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
				if(SelectTableRecord(szSql, &struCursor_CG) != NORMAL)
				{
					PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
								  szSql, GetSQLErrorMessage());
					FreeCursor(&struCursor_CG);			  
					return EXCEPTION;
				}
			    while(FetchCursor(&struCursor_CG) == NORMAL)
			    {		
			    	strcpy(szSpoolQryParam,  TrimAllSpace(GetTableFieldValue(&struCursor_CG, "CDO_SPOOLQUERYPARAM")));
			    	strcpy(szObjects,  TrimAllSpace(GetTableFieldValue(&struCursor_CG, "CDO_OBJECTS")));				
					nCmdCode = atoi(GetTableFieldValue(&struCursor_CG, "CDO_COMMANDCODE"));
					
					if ((strstr(szSpoolQryParam, "base") != NULL && strstr(szTaskQryParm, "base") != NULL) ||
						(strstr(szSpoolQryParam, "radio") != NULL && strstr(szTaskQryParm, "radio") != NULL) ||
						(strstr(szSpoolQryParam, "alarm") != NULL && strstr(szTaskQryParm, "alarm") != NULL) ||
						(strstr(szSpoolQryParam, "alarmen") != NULL && strstr(szTaskQryParm, "alarmen") != NULL))
					{	
						struHead.nCommandCode = nCmdCode;
						//发送报文
						nNowTime = (int)time(NULL);	
						InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime(nNowTime), MODE_AUTOGROW|MODE_UNIQUENAME);
				        InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  szObjects, MODE_AUTOGROW|MODE_UNIQUENAME);
				        PrintDebugLog(DBG_HERE, "mark1[%s]\n", szObjects);
						QryElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
					    SaveToMsgQueue_Tmp(pstruXml);
					    SaveEleQryLog(pstruXml);
					    nTxPackCount++;
					    if (nTaskId == 4 && i>= 1)	break;
			         	if (nTaskId != 4 && i>= 0) break;
					}
				}
				FreeCursor(&struCursor_CG);	   
				
			}	
		     


        } 
               
        //更新网元数或发送报文数
        if ((nEleCount >= 1000) || (nTxPackCount >= 1000))
        {
            UpdateTaskLogCount(nTaskLogId, nTaskId, nEleCount, nTxPackCount, nFailEleCount);
            nEleCount = 0;
            nTxPackCount = 0;
            nFailEleCount = 0;
            if (getTaskStopUsing(nTaskId) == BOOLTRUE)
                break;
        }
            
            

	}
    if ((nEleCount > 0) || (nTxPackCount > 0) || (nFailEleCount > 0))
	    UpdateTaskLogCount(nTaskLogId, nTaskId, nEleCount, nTxPackCount, nFailEleCount);
	FreeCursor(&struCursor);
    DeleteXml(pstruXml);

    return NORMAL;
	
}



/* 
 * 应用服务器服务请求处理 
 */
RESULT ApplReqWork(INT nSock, PVOID pArg)
{
	STR szCaReqBuffer[MAX_BUFFER_LEN];		/* 渠道请求通讯报文 */
	STR	szCaRespBuffer[MAX_BUFFER_LEN];		/* 渠道应答通讯报文 */
	STR	szPackCd[4+1];						/* 报文代码 */
	INT iRet;
		
	if (SQLPingInterval(60)<0)
    {
    	CloseDatabase();
    	sleep(1);
    	if(OpenDatabase(szService, szDbName, szUser, szPwd) != NORMAL)
	    {
	        PrintErrorLog(DBG_HERE, "gprsserv打开数据库发生错误 [%s]\n",
	                GetSQLErrorMessage());
	        return EXCEPTION;
	    }
	    PrintDebugLog(DBG_HERE, "重连数据库成功!\n");
    }
	//PrintDebugLog(DBG_HERE, "应用服务器接收到新的渠道请求\n");
	if (RedisPingInterval(60)<0)
    {
    	FreeRedisConn();
    	sleep(1); //10.25
    	if (ConnectRedis() !=NORMAL)
	    {
	        PrintErrorLog(DBG_HERE, "Connect Redis Error Ping After\n");
	        return EXCEPTION;
	    }
    }
	/* 
	 * 接收渠道请求报文 
	 */
	memset(szPackCd,0,sizeof(szPackCd));
	memset(szCaReqBuffer, 0, sizeof(szCaReqBuffer));
	iRet = RecvCaReqPacket(nSock, szCaReqBuffer, szPackCd);
	if (iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "接收渠道请求报文失败\n");
		return EXCEPTION;
	}
	
	PrintDebugLog(DBG_HERE, "[%s]接收渠道请求报文[%s]\n", szPackCd, szCaReqBuffer);	
	/*
	 * gprsrecv请求查询和设置，并返回查询或设置GPRS内容
	 */
	if (strcmp(szPackCd, GPRSREQTRANS) == 0)
	{
	    ProcessGprsQrySetTrans(nSock, szCaReqBuffer);
	}
	else
	{
	    //初始化应答报文
	    memset(szCaRespBuffer, 0, sizeof(szCaRespBuffer));
	    strcpy(szCaRespBuffer, "0000");
	
	    /* 
	     * 返回渠道应答报文 
	     */
	    
        
        /* 
	     * 处理请求报文 
	     */
	    if (strcmp(szPackCd, BS_QUERYSETTRANS) == 0)
	    {
	    	if(SendCaRespPacket(nSock, szCaRespBuffer, strlen(szCaRespBuffer)) != NORMAL)
	    	{
	    		PrintErrorLog(DBG_HERE, "返回渠道应答报文错误!\n");
	    		return EXCEPTION;
	    	}
	    	//处理BS平台发送的查询设置交易
	    	ProcessQuerySetTrans(szCaReqBuffer);
	    	
	    }
	    else if (strcmp(szPackCd, DELIVERTRANS) == 0)
	    {
	    	//处理上报交易
	    	ProcessSmsDeliverTrans(szCaReqBuffer);
	    	
	    	if(SendCaRespPacket(nSock, szCaRespBuffer, strlen(szCaRespBuffer)) != NORMAL)
	    	{
	    		PrintErrorLog(DBG_HERE, "返回渠道应答报文错误!\n");
	    		return EXCEPTION;
	    	}
	    	while(TRUE)
			{
		    	memset(szCaReqBuffer, 0, sizeof(szCaReqBuffer));
		    	iRet = RecvCa8801ReqPacket(nSock, szCaReqBuffer, szPackCd);
			    if (iRet <= 0)
			    {
			    	return EXCEPTION;
			    }
			    
				//PrintDebugLog(DBG_HERE, "[%s]接收渠道请求报文[%s]\n", szPackCd, szCaReqBuffer);
				
			    ProcessSmsDeliverTrans(szCaReqBuffer);
			    
			    if(SendCa8801RespPacket(nSock, szCaRespBuffer, strlen(szCaRespBuffer)) != NORMAL)
			    {
			    	PrintErrorLog(DBG_HERE, "返回渠道应答报文错误!\n");
			    	return EXCEPTION;
			    }
			}
	    }
	    else if (strcmp(szPackCd, SMSSTATSREPORT) == 0)
	    {
	    	//处理状态报告
	    	ProcessSmsStatusReport(szCaReqBuffer);
	    	
	    	if(SendCaRespPacket(nSock, szCaRespBuffer, strlen(szCaRespBuffer)) != NORMAL)
	    	{
	    		PrintErrorLog(DBG_HERE, "返回渠道应答报文错误!\n");
	    		return EXCEPTION;
	    	}
	    }
	    else if (strcmp(szPackCd, BS_TRUNTASK) == 0)
	    {
	    	if(SendCaRespPacket(nSock, szCaRespBuffer, strlen(szCaRespBuffer)) != NORMAL)
	    	{
	    		PrintErrorLog(DBG_HERE, "返回渠道应答报文错误!\n");
	    		return EXCEPTION;
	    	}
	    	//处理轮训任务
	    	ProcessTurnTask(szCaReqBuffer);
	    }
	    else if (strcmp(szPackCd, TIMERETRUNTASK) == 0)
	    {
	    	if(SendCaRespPacket(nSock, szCaRespBuffer, strlen(szCaRespBuffer)) != NORMAL)
	    	{
	    		PrintErrorLog(DBG_HERE, "返回渠道应答报文错误!\n");
	    		return EXCEPTION;
	    	}
	    	//处理定时复询任务
	    	ProcessTimeReTurnTask(szCaReqBuffer);
	    }
	    else if (strcmp(szPackCd, GPRSOFFLINE) == 0)
	    {
	    	//处理脱机交易
	    	ProcessGprsOffLine(szCaReqBuffer);
	    	
	    	if(SendCaRespPacket(nSock, szCaRespBuffer, strlen(szCaRespBuffer)) != NORMAL)
	    	{
	    		PrintErrorLog(DBG_HERE, "返回渠道应答报文错误!\n");
	    		return EXCEPTION;
	    	}
	    }
	}
	return NORMAL;	

}

/*	
 * 主函数
 */
RESULT main(INT argc, PSTR *argv)
{
	POOLSVRSTRU struPoolSvr;

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
	//fprintf(stderr, "\t欢迎使用网管系统(应用服务器)\n");
	fflush(stderr);

	if(TestPrgStat(argv[0]) == NORMAL)
	{
		fprintf(stderr, "%s已经启动或者正在服务.请关闭或者是稍后再启动\n", \
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
	//fprintf(stderr, "here1\n");
	if(OpenDatabase(szService, szDbName, szUser, szPwd) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "打开数据库错误 [%s]!\n", \
			GetSQLErrorMessage());
		return EXCEPTION;
	}
	//fprintf(stderr, "here2\n");
    InitMapObjectCache();
    //fprintf(stderr, "here3\n");
    InitMapObjectCache_CG();
    //fprintf(stderr, "here4\n");
    InitMapObjectCache_SNMP();
    //RecordAgentRecNum();
    //fprintf(stderr, "here5\n");
	CloseDatabase();
	if (getenv("WUXIAN")!=NULL)
	{
		if (InitRedisMQ_cfg() != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE,"Failed to InitRedisMQ_cfg()\n");
	        return EXCEPTION;
	    }
		if (ConnectRedis() !=NORMAL)
	    {
	        PrintErrorLog(DBG_HERE, "Connect Redis Error \n");
	        return EXCEPTION;
	    }
	    FreeRedisConn();
	}
		
	STR szArgName[100], szTemp[100];
	INT i;
	memset(&struPoolSvr,0,sizeof(struPoolSvr));
	for(i=0;i<10;i++)
	{
		sprintf(szArgName,"ListenPort%d",i+1);
		if (GetCfgItem("applserv.cfg","APPLSERV", szArgName,szTemp) != NORMAL)
		{
			if(i==0)
			{
				PrintErrorLog(DBG_HERE,"配置监听端口错误\n");
				return EXCEPTION;
			}
			break;
		}
		struPoolSvr.nListenPort[i]=atoi(szTemp);
	}
	if(i<MAX_LISTEN_PORT)
		struPoolSvr.nListenPort[i]=-1;
	/*
	//处理告警上报等端口
	struPoolSvr.nListenPort[0]=8801;
	//处理查询设置等端口
	struPoolSvr.nListenPort[1]=8802;
	//gprs上报端口
	struPoolSvr.nListenPort[2]=8803;
	struPoolSvr.nListenPort[3]=-1;
	*/
	if (GetCfgItem("applserv.cfg","APPLSERV","MinProcesses",szTemp) != NORMAL)
        return EXCEPTION;
	SetTcpPoolMinNum(atoi(szTemp));
	if (GetCfgItem("applserv.cfg","APPLSERV","MaxProcesses",szTemp) != NORMAL)
        return EXCEPTION;
	SetTcpPoolMaxNum(atoi(szTemp));
	
	fprintf(stderr,"完成!\n主程序开始启动\n");
	fflush(stderr);
	    	
    struPoolSvr.funcPoolStart = StartPoolChildPro;
    struPoolSvr.pPoolStartArg = NULL;
    struPoolSvr.funcPoolWork = ApplReqWork;
    struPoolSvr.pPoolWorkArg = NULL;
    struPoolSvr.funcPoolEnd = EndPoolChildPro;
    struPoolSvr.pPoolEndArg = NULL;
    struPoolSvr.funcPoolFinish = NULL;
    struPoolSvr.pPoolFinishArg = NULL;
	
	StartTcpPool(&struPoolSvr);

	return NORMAL;
}
