/*
 * 名称: 网管系统应用服务器(渠道请求部分)
 *
 * 修改记录:
 * 付志刚 -		2008-8-28 创建
 */
 
#include <ebdgdl.h>
#include <omcpublic.h>
#include <applserver.h>
#include <mobile2g.h>

extern ALARMPARAMETER struAlarmPara;


/**
 *	接收XML请求报文
 *  该函数假设通讯的方式为采用有包头的XML报文
 *  且报文代码存放的路径为<omc>/<transcd>
 */
int RecvCa8801ReqPacket(int sockfd, char *psCaReqBuffer, char *psTradeNo)
{
	int iRet;
	XMLSTRU  CaReqXml;							/* 从渠道方接受报文XML */
	PXMLSTRU pCaReqXml = &CaReqXml;		
	char sTempstr[64];
	 
	//SetSyncString("\x03\x00",2);
	
	iRet = RecvSocketWithSync(sockfd,psCaReqBuffer, MAX_BUFFER_LEN, 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "接收渠道请求报文错误 返回码为[%d]\n", iRet);
		close(sockfd);
		return -1;
	}
	
	memset(pCaReqXml,0,sizeof(XMLSTRU));
	if(ImportXml(pCaReqXml, FALSE, psCaReqBuffer) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"将渠道请求报文倒为XML结构错误.报文格式非法\n");
		close(sockfd);
		return -1;
	}
	
	if(DemandInXmlExt(pCaReqXml, "<omc>/<packcd>", sTempstr, sizeof(sTempstr)) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "DemandInXmlExt 获取渠道方请求报文交易代码失败, 取不到[%s]的值\n",
			"<omc>/<packcd>");
		close(sockfd);
		return -1;
	}
	DeleteXml(pCaReqXml);
	PrintTransLog(DBG_HERE, "收到渠道请求报文为 [%s]\n",  psCaReqBuffer);
	strcpy(psTradeNo, sTempstr);
	return iRet;
}


/*
 * 返回XML应答报文
 */
int SendCa8801RespPacket(int sockfd, char *sCaRespBuffer, int iSendBufferLen)
{
	int iRet;	
	
	PrintTransLog(DBG_HERE, "发送渠道应答报文为 [%s]\n",  sCaRespBuffer);	
	iRet = SendSocketWithSync(sockfd,sCaRespBuffer, iSendBufferLen, 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "AsSendCa_8801发送渠道应答报文错误 返回码为[%d]\n", iRet);
		return -1;
	}
	return 0;
}

/*
 * 接收报文
 * 该函数假设通讯的方式为采用有4位包文长度的包头的报文
 */
int RecvCa8802ReqPacket(int sockfd, char *psCaReqBuffer, char *psTradeNo)
{
	int iRet;
	char sBufLen[64];
	
	memset( sBufLen, 0, sizeof( sBufLen ));
	iRet = RecvSocketNoSync(sockfd, sBufLen, 4, 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "接收渠道请求报文错误 返回码为[%d]\n", iRet);
		close(sockfd);
		return -1;
	}

	TrimAllSpace( sBufLen );
	PrintDebugLog(DBG_HERE, "接收渠道请求报文前四位长度为[%s]\n", sBufLen);
	if ( atoi( sBufLen ) < 1 )
	{
		PrintErrorLog(DBG_HERE, "接收渠道请求报文长度错误 长度为[%s]\n", sBufLen);
		close(sockfd);
		return -1;		
	}
	
	iRet = RecvSocketNoSync(sockfd, psCaReqBuffer, atoi( sBufLen ), 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "接收渠道请求报文错误 返回码为[%d]\n", iRet);
		close(sockfd);
		return -1;
	}
	memcpy(psTradeNo,psCaReqBuffer,4);
	PrintTransLog(DBG_HERE,"收到渠道请求报文为[%s]\n", psCaReqBuffer);
	
	return iRet;
}

/*
 * 发送应答报文
 * 该函数假设通讯的方式为采用有4位包文长度的包头的报文
 */
int SendCa8802RespPacket(int sockfd, char *sCaRespBuffer, int iSendBufferLen)
{
	int iRet;
	char sBufLen[64];
	
	if ( strlen( sCaRespBuffer ) < 4 )
	{
		PrintErrorLog(DBG_HERE, "返回报文长度不正确[%s]", sCaRespBuffer);
		close(sockfd);		
		return -1;
	}
	//sleep(1);
	
	snprintf(sBufLen, sizeof(sBufLen), "%4d%s", iSendBufferLen, sCaRespBuffer);
	
	iRet = SendSocketNoSync(sockfd,sBufLen, 4+iSendBufferLen, 30);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "AsSendCa_8802发送渠道应答报文报文头错误 返回码为[%d]\n", iRet);
		close(sockfd);
		return -1;
	}
	PrintTransLog(DBG_HERE, "发送渠道应答报文[%s]\n", sCaRespBuffer);

	return 0;
}

/**
 *	接收请求报文
 *  该函数假设通讯的方式为采用有包头的报文
 */
int RecvCa8803ReqPacket(int sockfd, char *psCaReqBuffer, char *psTradeNo)
{
	int iRet;
	char sTempBuffer[MAX_BUFFER_LEN];
	
	memset(sTempBuffer, 0, sizeof(sTempBuffer));
	iRet = RecvSocketWithSync(sockfd, sTempBuffer, MAX_BUFFER_LEN, 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "接收渠道请求报文错误 返回码为[%d]\n", iRet);
		close(sockfd);
		return -1;
	}
	//PrintHexTransLog(DBG_HERE, "收到渠道请求报文为 [%s]\n",  sTempBuffer);
	memcpy(psTradeNo, sTempBuffer, 4);
	memcpy(psCaReqBuffer, sTempBuffer+4, iRet-4);
	return iRet-4;
}

/*
 * 返回应答报文
 */
int SendCa8803RespPacket(int sockfd, char *sCaRespBuffer, int iSendBufferLen)
{
	int iRet;	
	
	//PrintTransLog(DBG_HERE, "发送渠道应答报文为 [%s]\n",  sCaRespBuffer);	
	iRet = SendSocketWithSync(sockfd,sCaRespBuffer, iSendBufferLen, 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "AsSendCa_8801发送渠道应答报文错误 返回码为[%d]\n", iRet);
		return -1;
	}
	return 0;
}



/* 
 * 接收渠道请求报文 
 */
RESULT RecvCaReqPacket(INT nSock, PSTR pszCaReqBuffer, PSTR pszPackCd)
{
	INT nRet=0;
	
	struct sockaddr_in struListenSockAddr;
	UINT nListenSockLen = sizeof(struListenSockAddr);

	
	/* 
	 * 读取渠道请求访问端口
	 */
#ifdef SYSTEM_AIX
	if(getsockname(nSock, (struct sockaddr *)&struListenSockAddr, \
		(unsigned long *) &nListenSockLen) != 0)
#else
	if(getsockname(nSock, (struct sockaddr *)&struListenSockAddr, \
		&nListenSockLen) != 0)
#endif
	{
		PrintErrorLog(DBG_HERE,"获取Socket信息失败[%s]\n",strerror(errno));
		return EXCEPTION;
	}
	
	if (ntohs(struListenSockAddr.sin_port) == 8801)
	     nRet=RecvCa8801ReqPacket(nSock, pszCaReqBuffer, pszPackCd);
	else if (ntohs(struListenSockAddr.sin_port) == 8802)
	     nRet=RecvCa8802ReqPacket(nSock, pszCaReqBuffer, pszPackCd); 
	else if (ntohs(struListenSockAddr.sin_port) == 8803)
	     nRet=RecvCa8803ReqPacket(nSock, pszCaReqBuffer, pszPackCd);
	else
		 nRet=RecvCa8801ReqPacket(nSock, pszCaReqBuffer, pszPackCd);
	if(nRet == -1)
	{
		PrintErrorLog(DBG_HERE, "调用接收渠道请求报文错误\n");
		return EXCEPTION;
	}

	return nRet;
}

/* 
 * 发送渠道应答报文 
 */
INT SendCaRespPacket(INT nSock, PSTR pszCaRespBuffer, INT nCommBufferLen)
{
	INT nRet=0; 
	
	struct sockaddr_in struListenSockAddr;
	UINT nListenSockLen = sizeof(struListenSockAddr);

	/* 
	 * 读取渠道请求访问端口
	 */
#ifdef SYSTEM_AIX
	if(getsockname(nSock, (struct sockaddr *)&struListenSockAddr, \
		(unsigned long *)&nListenSockLen) != 0)
#else
	if(getsockname(nSock, (struct sockaddr *)&struListenSockAddr, \
		&nListenSockLen) != 0)
#endif
	{
		PrintErrorLog(DBG_HERE,"获取Socket信息失败[%s]\n",strerror(errno));
		return EXCEPTION;
	}
	
	if (ntohs(struListenSockAddr.sin_port) == 8801)
	     nRet=SendCa8801RespPacket(nSock, pszCaRespBuffer, nCommBufferLen);
	else if (ntohs(struListenSockAddr.sin_port) == 8802)
	     nRet=SendCa8802RespPacket(nSock, pszCaRespBuffer, nCommBufferLen); 
	else if (ntohs(struListenSockAddr.sin_port) == 8803)
	     nRet=SendCa8803RespPacket(nSock, pszCaRespBuffer, nCommBufferLen);
	else
		nRet=SendCa8801RespPacket(nSock, pszCaRespBuffer, nCommBufferLen);
	if(nRet == -1)
	{
		PrintErrorLog(DBG_HERE, "调用发送渠道应答报文错误\n");
		return EXCEPTION;
	}

	return NORMAL;
}

int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr)
{
    int  nStringLen;
    char szNewString[8192];
    
    char *FindPos = strstr(sSrc, sMatchStr);
    if( (!FindPos) || (!sMatchStr) )
        return -1;
    
    while( FindPos )
    {
        memset(szNewString, 0, sizeof(szNewString));
        nStringLen = FindPos - sSrc;
        strncpy(szNewString, sSrc, nStringLen);
        strcat(szNewString, sReplaceStr);
        strcat(szNewString, FindPos + strlen(sMatchStr));
        strcpy(sSrc, szNewString);
        
        FindPos = strstr(FindPos + strlen(sReplaceStr), sMatchStr);
    }
    return 0;
}

static RESULT InitPackageToXml(SENDPACKAGE *pstruSendPackage, PXMLSTRU pstruXml)
{
    STR szTemp[100];
	
	sprintf(szTemp, "%d", pstruSendPackage->struRepeater.nCommType);
    InsertInXmlExt(pstruXml,"<omc>/<通信方式>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
    
    sprintf(szTemp, "%u", pstruSendPackage->struRepeater.nRepeaterId);
	InsertInXmlExt(pstruXml,"<omc>/<站点编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->struRepeater.nDeviceId);
	InsertInXmlExt(pstruXml,"<omc>/<设备编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%s", pstruSendPackage->struRepeater.szTelephoneNum);
	InsertInXmlExt(pstruXml,"<omc>/<站点电话>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->struRepeater.nPort);
	InsertInXmlExt(pstruXml,"<omc>/<端口号>",   szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%s", pstruSendPackage->struRepeater.szIP);
	InsertInXmlExt(pstruXml,"<omc>/<站点IP>",   szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->struRepeater.nProtocolDeviceType);
	InsertInXmlExt(pstruXml,"<omc>/<设备类型>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%s", pstruSendPackage->struRepeater.szSpecialCode);
	InsertInXmlExt(pstruXml,"<omc>/<其它标识>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%s", pstruSendPackage->struRepeater.szReserve);
	InsertInXmlExt(pstruXml,"<omc>/<设备型号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%s", pstruSendPackage->struRepeater.szNetCenter);
	InsertInXmlExt(pstruXml,"<omc>/<服务号码>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->struHead.nProtocolType);
	InsertInXmlExt(pstruXml,"<omc>/<协议类型>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->struHead.nCommandCode);
	InsertInXmlExt(pstruXml,"<omc>/<命令号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->nNeId);
	InsertInXmlExt(pstruXml,"<omc>/<网元编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruSendPackage->struHead.QA, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	return NORMAL;
}

RESULT SendToApplQrySet(SENDPACKAGE *pstruNeInfo)
{
    STR szTransBuffer[1000]; /*	数据交换缓冲	*/
    STR szTemp[1000];		 /*	数据交换缓冲	*/
	INT nConnectFd;
	
	memset(szTemp, 0, sizeof(szTemp));
	snprintf(szTemp,  sizeof(szTemp), 
					"6000|%d|%u|%d|07C5,07C6,07E7|1,1,1|1|%s|127.0.0.1|2|2|49| | |9999|%s|12|%s|0|3|0|0",
			 pstruNeInfo->nNeId,
			 pstruNeInfo->struRepeater.nRepeaterId,  
			 pstruNeInfo->struRepeater.nDeviceId,
			 pstruNeInfo->struRepeater.szTelephoneNum,
			 
			 pstruNeInfo->struRepeater.szNetCenter,
			 GetSysDateTime()
		    );
			 
	/*
	 *	建立连接
	 */
	if((nConnectFd = CreateConnectSocket("127.0.0.1", 8802, 10)) < 0)
	{
		PrintErrorLog(DBG_HERE, "同应用服务程序[127.0.0.1][8802]建立连接错误,请确信applserv已经启动\n" );
		return EXCEPTION;
	}
	
	memset(szTransBuffer, 0, sizeof(szTransBuffer));
	snprintf(szTransBuffer, sizeof(szTransBuffer), "%4d%s", strlen(szTemp), szTemp);
	if(SendSocketNoSync(nConnectFd,szTransBuffer, 4+strlen(szTemp), 10) < 0)
	{
		PrintErrorLog(DBG_HERE, "发送到数据应用服务的请求报文错误\n");
		close(nConnectFd);
		return EXCEPTION;
	}
	
	/*
	 *	接收服务程序的应答
	 */
	memset(szTransBuffer, 0, sizeof(szTransBuffer));
	if(RecvSocketNoSync(nConnectFd, szTransBuffer, sizeof(szTransBuffer), 30) < 0)
	{
		PrintErrorLog(DBG_HERE, "接收数据应用服务的应答报文错误\n");
		close(nConnectFd);
		return EXCEPTION;
	}
	
	close(nConnectFd);

	if (memcmp(szTransBuffer+4, "0000", 4) !=0)
	{
	    PrintErrorLog(DBG_HERE,"接收到应用服务应答报文失败，返回码[%s]\n", szTransBuffer);
		return EXCEPTION;
	}

	return NORMAL;
}

RESULT Process2GSms(PSTR pszUndecode,PSTR pszTelephone,PSTR pszNetCenterNum, int nProctlType)
{
    XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	DECODE_OUT	Decodeout;
	SENDPACKAGE struSendPackage;
	OBJECTSTRU struObject[MAX_OBJECT_NUM];
	STR szTemp[200], szMapId[10];
	int i, j, nCommUpType = 0, k=0;
	int nNeId, nAlarmCount=0;
	STR szNeName[50], szAlarmObjList[1000], szAlarmTime[20];
	BOOL bIsNewNeId=BOOLFALSE;
	STR szMsgCont[150];
	BYTEARRAY struPack;
	
	char szMapDataSet[MAX_OBJCONTEXT_LEN];
	STR szReqBuffer[MAX_BUFFER_LEN];
	int nStart=-1;
	int nEnd=-1;
	int index = 0, nLen;
	nLen = strlen(pszUndecode);
	for(index=0; index < nLen; index++){
		if(pszUndecode[index] == '!'){
			nStart = index;
			break;
		}
	}
	if (nStart<0)
	{
		PrintErrorLog(DBG_HERE,"[%s]没开始分割字符,报文非法!!!!!!\n", pszUndecode);
		return EXCEPTION;
	}
	for(index=nStart+1; index<nLen;index++){
		if(pszUndecode[index] == '!'){
			nEnd = index;
			break;
		}
	}

	if (nEnd<0)
	{
		PrintErrorLog(DBG_HERE,"没结束分割字符,报文非法!!!!!!\n");
		return EXCEPTION;
	}
	nLen = nEnd - nStart +1;
   	bufclr(szReqBuffer);
   	memcpy(szReqBuffer, pszUndecode+nStart, nLen);
   	
	struPack.pPack = szReqBuffer;
	struPack.Len = nLen;
	//struPack.pPack = pszUndecode;
	//struPack.Len = strlen(pszUndecode);
	//解析2G协议

    if (Decode_2G(M2G_SMS, &struPack, &Decodeout, struObject) != NORMAL)
    {
    	PrintErrorLog(DBG_HERE, "解析设备内容错误\n");
		return EXCEPTION; 
    }

	
    PrintDebugLog(DBG_HERE, "解析2G协议成功,协议类型[%02d],命令标识[%d],错误代码[%d],站点编号[%u],设备编号[%X],网络标识[%d],对象数[%d]\n",
        Decodeout.NPLayer.APID,  Decodeout.MAPLayer.CommandFalg, Decodeout.APLayer.ErrorCode, 
        Decodeout.NPLayer.structRepeater.RepeaterId, Decodeout.NPLayer.structRepeater.DeviceId,
        Decodeout.NPLayer.NetFlag, Decodeout.MAPLayer.ObjCount);
	
	//根据QB+RepeaterId获取QA
	int QB=Decodeout.NPLayer.NetFlag;//2G协议通讯标示流水号
	UINT nRepeaterId=Decodeout.NPLayer.structRepeater.RepeaterId;
	int nDeviceId = Decodeout.NPLayer.structRepeater.DeviceId;
	
	//初始化分解对象
	memset(&struSendPackage, 0, sizeof(SENDPACKAGE));
	struSendPackage.struRepeater.nRepeaterId=Decodeout.NPLayer.structRepeater.RepeaterId;
    struSendPackage.struRepeater.nDeviceId=Decodeout.NPLayer.structRepeater.DeviceId;
	//服务号码
	memcpy((char*)struSendPackage.struRepeater.szNetCenter, pszNetCenterNum,strlen(pszNetCenterNum));
	//设备通讯方式 
	struSendPackage.struRepeater.nCommType=M2G_SMS_TYPE;
	//上报设备手机号
	memcpy((char*)struSendPackage.struRepeater.szTelephoneNum, pszTelephone, strlen(pszTelephone));
	//对象数
	struSendPackage.struHead.nObjectCount= Decodeout.MAPLayer.ObjCount;
	//上报方式
	struSendPackage.struHead.nCommandCode=COMMAND_UP;
	
	if (GetNeInfo(&struSendPackage) != NORMAL)
			struSendPackage.struHead.nProtocolType= PROTOCOL_2G;
	
	if (Decodeout.NPLayer.APID == 0x03)
		struSendPackage.struHead.nProtocolType= PROTOCOL_DAS;

	//QA流水号
	memcpy((char*)struSendPackage.struHead.QA,"Sms123456789", 12);
	strncpy(struSendPackage.szRecvMsg, pszUndecode, strlen(pszUndecode));
	
	//初始化不为监控对象参数命令
	BOOL bObject0009=BOOLFALSE;
	BOOL bObject0141=BOOLFALSE;
	BOOL bObject0001=BOOLFALSE;
	BOOL bObject00000009=BOOLFALSE;		
	for(i=0; i< Decodeout.MAPLayer.ObjCount; i++)
	{
	    bufclr(szMapId);
	    struSendPackage.struMapObjList[i].nMapLen = struObject[i].OL;
	    
	    if (struSendPackage.struHead.nProtocolType == PROTOCOL_DAS)
	    	sprintf(szMapId, "%08X", struObject[i].MapID);
	    else
	    	sprintf(szMapId, "%04X", struObject[i].MapID);
	    
	    struSendPackage.struMapObjList[i].cErrorId = szMapId[0];
	    ReplaceCharByPos(szMapId, '0', 1);
	    
	    //2015.08.11
	    if (strcmp(szMapId, "0000") == 0 || strcmp(szMapId, "00000000") == 0) 
	    {
	    	struSendPackage.struHead.nObjectCount = i;
	    	break;
	    }
	    
	    if (nProctlType == PROTOCOL_JINXIN_DAS)
	    {
	    	strcpy(szTemp, szMapId);
	    	GetMapIdFromParam(nRepeaterId, szTemp, szMapId);
	    }
	    
	    strcpy(struSendPackage.struMapObjList[i].szMapId, szMapId);
	    
	    if (DecodeMapDataFromMapId(szMapId, struObject[i].OC, struSendPackage.struMapObjList[i].szMapData, 
	    	struSendPackage.struMapObjList[i].szMapType) == EXCEPTION)
	    {
	    	
	    }

	    PrintDebugLog(DBG_HERE,"MAPID=[%s],OL=[%d],MAPDATA=[%s]\n",szMapId,  struObject[i].OL, struSendPackage.struMapObjList[i].szMapData);
	    //PrintHexDebugLog("OC=", struObject[i].OC, struObject[i].OL);
		
		//如果是查询监控列表0009
		if (strcmp(szMapId, "0009") == 0 && Decodeout.MAPLayer.CommandFalg == 2)
		    bObject0009 = BOOLTRUE;
		else if ( strcmp(szMapId, "00000009") == 0)
			bObject00000009 = BOOLTRUE;
		
		if (strcmp(szMapId, "0141") == 0 || strcmp(szMapId, "00000141") == 0)
		{
		    nCommUpType = struObject[i].OC[0];
		    if (struObject[i].OC[0] == 0xCA)//判断是否gprs请求上报
		        bObject0141 = BOOLTRUE;
		}
		if(strcmp(szMapId, "074B") == 0)//监控1.0短信定时上报
		{
			memcpy(szMapDataSet, struObject[i].OC, MAX_OBJCONTEXT_LEN);
		}
		if (strcmp(szMapId, "0001") == 0 )//扩展参量上报
			bObject0001 = BOOLTRUE;
		if (strcmp(szMapId, "0004") == 0 )
			ReplaceStr(struSendPackage.struMapObjList[i].szMapData, ",", "-");
	}

    
    //如果是主动上报,处理上报
	if(Decodeout.MAPLayer.CommandFalg == 1)
	{
	     PrintDebugLog(DBG_HERE,"上报类型%X SMS开始===========\n", nCommUpType);
	     if (nCommUpType == 0x20 )
			return NORMAL;
		 /*
		  * 回复主动上报开始===============================================
          * 打包设备协议,生成短信内容
          */
	     int nUpQB=Decodeout.NPLayer.NetFlag;
	     int nObjCount=0; //回复报文为0
	     
	     memset(szMsgCont, 0, sizeof(szMsgCont));
	     struPack.pPack = szMsgCont;
	     struPack.Len = 0;
	     if (bObject0141 == BOOLTRUE)
	     {
	          memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		      struObject[0].MapID = 0x0A02;
		      struObject[0].OC[0] = 0x03;
		      struObject[0].OL = 1;
	          nObjCount=1; 
	          if (Encode_2G(M2G_SMS, PRJPRMSET, nUpQB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	          {
	               PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		           return EXCEPTION; 
	          }
	     }
	     else
	     {
	     	  if (Decodeout.NPLayer.APID == 0x03)
	     	  {
	     	  	  if (Encode_Das(M2G_SMS, RELAPSECOMMAND, nUpQB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
		          {
		              PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
			          return EXCEPTION; 
		          }
	     	  }
	     	  else
	     	  {
		          if (Encode_2G(M2G_SMS, RELAPSECOMMAND, nUpQB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
		          {
		              PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
			          return EXCEPTION; 
		          }
		      }
	     }
	     /*
	      *   创建xml对象
	      */
	     memset(pstruXml,0,sizeof(XMLSTRU));
	     CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	     
	     //取唯一流水号
	     strcpy(struSendPackage.struHead.QA, GetTypeNumber("Sms"));
	     //初始化xml
	     InitPackageToXml(&struSendPackage, pstruXml);
	     
	     //自动回复类型
	     InsertInXmlExt(pstruXml,"<omc>/<类型>",  "00", MODE_AUTOGROW|MODE_UNIQUENAME);
	     
	     InsertInXmlExt(pstruXml,"<omc>/<站点电话>", pszTelephone, MODE_AUTOGROW|MODE_UNIQUENAME);
	     sprintf(szTemp, "%d", nUpQB);
	     InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
         
	     InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	     InsertInXmlExt(pstruXml,"<omc>/<服务号码>", pszNetCenterNum, MODE_AUTOGROW|MODE_UNIQUENAME);
         //站点等级为快:OMC_QUICK_MSGLEVEL
	     InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     
         SaveToMsgQueue(pstruXml);
         /*
          *回复主动上报结束===============================================
          */
		 /*
		 if (SaveToRecordDeliveCrc(Decodeout.NPLayer.structRepeater.RepeaterId, Decodeout.NPLayer.structRepeater.DeviceId,
        		Decodeout.NPLayer.NetFlag, 0) != NORMAL)
		 {
			return EXCEPTION;
		 }
		 */
	
		 BOOL bAlarmRestore = BOOLFALSE;
		 STR szFrequentAlarmList[1000]; //频繁告警列表
		 STR szAlarmRestoreList[1000]; //告警告警列表
		 INT nAlarmTimes=0;
		 
		 bufclr(szFrequentAlarmList);
		 bufclr(szAlarmRestoreList);
		 if (nCommUpType == DEVICE_ALARM) //设备告警
		 {
		     /*
		      *  取告警列表,并取网元NeId保留
		      */
		     if (GetAlarmObjList2(nRepeaterId, nDeviceId, pszTelephone, &nNeId, szNeName, szAlarmObjList)!= NORMAL)
		     {
		         DeleteXml(pstruXml);
		         PrintErrorLog(DBG_HERE, "设备号[%u][%d]不存在记录\n", nRepeaterId, nDeviceId);
		         return EXCEPTION;
		     }
		     InsertInXmlExt(pstruXml,"<omc>/<告警列表>",szAlarmObjList,MODE_AUTOGROW|MODE_UNIQUENAME);
		     sprintf(szTemp, "%d", nNeId);
	     	 InsertInXmlExt(pstruXml,"<omc>/<网元编号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
             struSendPackage.nNeId = nNeId;
             
             BOOL bNewAlarm = BOOLFALSE;
             bufclr(szAlarmTime);
	         for(i=0; i< Decodeout.MAPLayer.ObjCount; i++)
	         {
	             if (struSendPackage.struMapObjList[i].cErrorId == '1')
	             {
	                 bAlarmRestore = BOOLFALSE;
	                 continue;
	             }
	             if (strstr(szAlarmObjList, struSendPackage.struMapObjList[i].szMapId) == NULL)
	             {
	             	 if (strcmp(struSendPackage.struMapObjList[i].szMapId, "0801") == 0 
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "0802") == 0 
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "0803") == 0 
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "080F") == 0
	             	 	 
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "04F0") == 0
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "04F1") == 0
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "04F2") == 0
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "04F3") == 0
	             	 )
	             	 {
	             	 	 strcpy(szAlarmTime, struSendPackage.struMapObjList[i].szMapData);
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<告警时间>",szAlarmTime,MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	             	 if (strcmp(struSendPackage.struMapObjList[i].szMapId, "050B") == 0 ||
	             	 	 strcmp(struSendPackage.struMapObjList[i].szMapId, "08B3") == 0)   //TD
	             	 {
	             	 	 PrintDebugLog(DBG_HERE,"电平强度[%s]\n",struSendPackage.struMapObjList[i].szMapData);
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<电平强度>",struSendPackage.struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	             	 if (strcmp(struSendPackage.struMapObjList[i].szMapId, "050C") == 0)
	             	 {
	             	 	PrintDebugLog(DBG_HERE,"服务小区[%s]\n",struSendPackage.struMapObjList[i].szMapData);
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<服务小区>",struSendPackage.struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	                 continue;
	             }
	             
	             if (strcmp(struSendPackage.struMapObjList[i].szMapData, "1") == 0)//告警处理
	             {
	             	 if (nAlarmTimes++ >= 1)
	             	 {
	             	 	 if (DealNewAlarm(pstruXml) == NORMAL)
                      	 strcat(szFrequentAlarmList, struAlarmPara.szAlarmName);//用于频繁上报
	             	 	 //频繁告警
	             	 	 SaveToMaintainLog("新告警", "",  &struSendPackage);
	             	 	 InitAlarmPara(nNeId);
	             	 	 AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarm");//是否频繁告警
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
	                 		 strcat(szAlarmRestoreList, struAlarmPara.szAlarmName);//用于告警恢复前转

	             		 SaveToMaintainLog("告警恢复", "",  &struSendPackage);
	             		 InitAlarmPara(nNeId);
	             		 AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarmClear");//频繁告警清除
	             		 if (strlen(szAlarmRestoreList) > 0)
	                 		 TransferAlarm();
	                 }
	                 if (bNewAlarm == BOOLTRUE)
	             	 	InsertInXmlExt(pstruXml,"<omc>/<告警对象>",szAlarmObjResv,MODE_AUTOGROW|MODE_UNIQUENAME);
	                 
	             }            
	         }
	         
             if (bNewAlarm == BOOLTRUE)
	         {
	             if (DealNewAlarm(pstruXml) == NORMAL)
                      strcat(szFrequentAlarmList, struAlarmPara.szAlarmName);//用于频繁上报
	             //频繁告警
	             SaveToMaintainLog("新告警", "",  &struSendPackage);
	             InitAlarmPara(nNeId);
	             AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarm");//是否频繁告警
	         }

		 }
		 else if (nCommUpType == OPENSTATION) //开站上报
		 {
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     SaveToMaintainLog("开站上报", "",  &struSendPackage);
		     if (bIsNewNeId == BOOLFALSE)
		     {
		        InitAlarmPara(nNeId);
	            AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeBulid");//开站上报
	         }
	         else
	         {
	         	/* 2010.6.8 add */
	         	QueryMapList(M2G_SMS, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         	InitPackageToXml(&struSendPackage, pstruXml);
	         	 
	         	InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  "0009", MODE_AUTOGROW|MODE_UNIQUENAME); 
	         	InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//查询
	         	SaveToMsgQueue(pstruXml);
	            SaveEleQryLog(pstruXml);
	         	 
	         	/* 2010.6.8 去除，采用短信开站上报流程
	         	STR szMapId0009List[1000];
	         	STR szProvinceId[10];
	         	bufclr(szMapId0009List);
	         	GetSysParameter("par_SectionName = 'Province' and par_KeyName = 'ProvinceAreCode'", szProvinceId);
	         	if (GetSysParameter("par_SectionName = 'ApplServer' and par_KeyName = 'ObjectList'", szMapId0009List) != NORMAL)
				{
					strcpy(szMapId0009List, "0002,0003,0004,0005,000A,0010,0018,0020,0101,0102,0111,0112,0113,0114,0115,0120,0130,0131,0133,0136,0137,0150,0160,0161,0162,0163,0164,0165,0166,0201,0204,0301,0304,0507,0508,0509,050A,050B,050C,0701,0704,0707,0708,070A,070B,070C,070D,070E,070F,0710,0711,0712,0713,0714,0715,0716,0717,0718,0719,071A,071B,071C,071D,071E,071F,0720,0721,0722,0723,0724,0725,0726,0727,0728,0729,073A,073B,073C,073D,073E,073F,0779,07A0,07A1,07A2,07A4,07B0,07B2,07B3,07B4,07B5,07B6,07B7,07B8,07B9,07BA,07BB,07BC,07BD,07BE,07BF,07C0,07C1,07C2,07C3,07D0,07D1,07D2,07E1,07E3,07E4,07E5,07E6,07E9");
					PrintErrorLog(DBG_HERE,"获取开站上报的监控量列表失败\n");
				}
	         	if (UpdateEleObjList(&struSendPackage, szMapId0009List, szProvinceId, 1) == NORMAL)
	         	{
	         		sleep(1);
	      			QryEleFristTime(nNeId, M2G_SMS);
	      		}
	      		*/
	         }
		 }
		 else if (nCommUpType == DEVICECHANGED) //变更上报
		 {
		     nNeId = GetNeId(nRepeaterId, nDeviceId, pszTelephone, &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     
		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeRevamp");//变更上报
	         
	         //////////////////////////////////////////查询监控量
	         sleep(5);
	         QueryMapList(M2G_SMS, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //SaveToMaintainLog("变更上报", "",  &struSendPackage);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//查询
	         SaveToMsgQueue(pstruXml);
	         SaveEleQryLog(pstruXml);
		     
		 }
		 else if (nCommUpType == DEVICEREPAIR) //修复上报
		 {
		     nNeId = GetNeId(nRepeaterId, nDeviceId, pszTelephone, &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;

		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeRebuild");//变更上报
	         
	         //////////////////////////////////////////查询监控量
	         sleep(5);
	         if (bIsNewNeId == BOOLTRUE)
	         {
	             QueryMapList(M2G_SMS, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	             InitPackageToXml(&struSendPackage, pstruXml);
	             //SaveToMaintainLog("修复上报", "",  &struSendPackage);
	             InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//查询
	             SaveToMsgQueue(pstruXml);
	             SaveEleQryLog(pstruXml);
	         }
	         else
	         {
	             /*
	              *开站上报
	              *实现网元监控量查询，记录消息队列表
	              */
	             QryEleFristTime(nNeId, M2G_SMS);
	         }
	         
		     
		 }
		 else if (nCommUpType == PERSONPATROL) //巡检上报信息
		 {
		     nNeId = GetNeId(nRepeaterId, nDeviceId, pszTelephone, &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;

		     SaveToMaintainLog("巡检上报", "",  &struSendPackage);
		     
		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeCheck");//巡检上报
		 }
		 else if (nCommUpType == 0x06) //登录到监控中心上报
		 {
		   
		    SaveToMaintainLog("登录到监控中心", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x07) //心跳上报
		 {
		   
		    SaveToMaintainLog("心跳上报", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x08) //远程升级结果上报
		 {
		     
		 	 sprintf(szTemp, "%d", struSendPackage.nNeId);
		     InsertInXmlExt(pstruXml,"<omc>/<网元编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     sprintf(szTemp, "%u", struSendPackage.struRepeater.nRepeaterId);
		     InsertInXmlExt(pstruXml,"<omc>/<设备编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     sprintf(szTemp, "%d", struSendPackage.struRepeater.nDeviceId);
		     InsertInXmlExt(pstruXml,"<omc>/<次编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     for(i=0; i< Decodeout.MAPLayer.ObjCount; i++)
		     {
		         sprintf(szMapId, "%04X", struObject[i].MapID);
    	         if (strcmp(szMapId, "000A") == 0 || strcmp(szMapId, "000a") == 0)
    	         {
    	             InsertInXmlExt(pstruXml,"<omc>/<VerInfo>", struSendPackage.struMapObjList[i].szMapData, MODE_AUTOGROW|MODE_UNIQUENAME);
    	         }
    	         else if (strcmp(szMapId, "0018") == 0)
    	         {
    	             InsertInXmlExt(pstruXml,"<omc>/<UpdateResult>", struSendPackage.struMapObjList[i].szMapData, MODE_AUTOGROW|MODE_UNIQUENAME);
    	         }
		     }
		     RemortUpdateDbOperate3(pstruXml);
		     SaveToMaintainLog("远程升级结果上报", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x09) //GPRS登录失败上报
		 {
		     
		     SaveToMaintainLog("GPRS登录失败上报", "",  &struSendPackage);
		     //SendToApplQrySet(&struSendPackage);
		     //if (ExistAlarmLog(ALM_DLSB_ID, struSendPackage.nNeId, &nAlarmCount) == BOOLFALSE)
		     //	SaveToAlarmLog(ALM_DLSB_ID, struSendPackage.nNeId, nAlarmCount);
		     	
		 }
		 else if (nCommUpType == 0x0A) //批采结束上报
		 {
		    
		    SaveToMaintainLog("批采结束上报", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x0B) //软件异常复位上报
		 {
		    
		    SaveToMaintainLog("软件异常复位上报", "",  &struSendPackage);
		 }
 		 else if (nCommUpType == 0x20 ) //基本型效果监控1.0定时上报
 		 {
			char szOC[256];
			char szId[5];
			char szData[256];
			char szDataType[20];
			int nDataLen;
			
 			char *psz074BObjList[28]=
 			{
 				"050C","0509","050A","050B",
 				"0710","0711","0712","0713","0714","0715",
 				"0716","0717","0718","0719","071A","071B",
 				"071C","071D","071E","071F","0720","0721",
 				"0722","0723","0724","0725","0726","0727"
 			};
 			
 			for (i = 0, j = 0; i < 28; i ++)
 			{
 				strcpy( szId, psz074BObjList[i]);
 			    if (GetMapIdFromCache(szId, szDataType, &nDataLen) != NORMAL)
 			    {
 			        PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", szId);
 			        DeleteXml(pstruXml);
 					return EXCEPTION;
 			    }
 			    memset(szOC, 0, sizeof(szOC));
 			    memcpy(szOC, szMapDataSet+j, nDataLen);
 			    j += nDataLen;
 			    DecodeMapDataFromMapId(szId, szOC, szData, szTemp);

 			    struSendPackage.struMapObjList[i].cErrorId = '0';
 			    struSendPackage.struMapObjList[i].nMapLen = nDataLen;
 			    strcpy(struSendPackage.struMapObjList[i].szMapId, szId);
 			    strcpy(struSendPackage.struMapObjList[i].szMapData, szData);
 			}
			struSendPackage.struHead.nObjectCount = 28;
			
		     
			 DecodeQryOnTime(&struSendPackage, BOOLTRUE, BOOLTRUE);
		 }
		 else if (nCommUpType == 0xCB || nCommUpType == 0x2E) //GPRS数据上报失败
		 {
		    
		     SaveToMaintainLog("GPRS数据上报失败", "",  &struSendPackage);
		     //SendToApplQrySet(&struSendPackage);
		     if (ExistAlarmLog(ALM_SJSB_ID, struSendPackage.nNeId, &nAlarmCount) == BOOLFALSE)
		     	SaveToAlarmLog(ALM_SJSB_ID, struSendPackage.nNeId, nAlarmCount);
		 }
		 else if (nCommUpType == 0x2C) //网络模式选择失败上报
		 {
		     
		     SaveToMaintainLog("网络模式选择失败上报", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x2D) //网络模式选择恢复上报
		 {
		     
		     SaveToMaintainLog("网络模式选择恢复上报", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x22) //34:效果监控PESQ上行测试结束上报
	     {
	        
	         for(i=0; i< Decodeout.MAPLayer.ObjCount; i++)
	         {
	             sprintf(szMapId, "%04X", struObject[i].MapID);
                 if (strcmp(szMapId, "0771") == 0)
                 {
	     	         sprintf(szTemp, "%d", struSendPackage.nNeId);
	                 InsertInXmlExt(pstruXml,"<omc>/<网元编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	                 InsertInXmlExt(pstruXml,"<omc>/<呼叫方向>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
	                 
                     if (DecodeGprsPesq(struObject[i].OC, struObject[i].OL, pstruXml)==NORMAL)
                     	SaveGprsPesqLog(pstruXml);
                     //SaveToMaintainLog("PESQ上行测试结束上报", "",  &struSendPackage);
                     break;
                 }
	             
	         }
	     }
	     else if (nCommUpType == 0x23) //PESQ下行测试结束上报
		 {
		   
		    for(i=0; i< Decodeout.MAPLayer.ObjCount; i++)
		    {
		        sprintf(szMapId, "%04X", struObject[i].MapID);
    	        if (strcmp(szMapId, "0771") == 0)
    	        {
			        sprintf(szTemp, "%d", struSendPackage.nNeId);
		            InsertInXmlExt(pstruXml,"<omc>/<网元编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		            InsertInXmlExt(pstruXml,"<omc>/<呼叫方向>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
		            
    	            if (DecodeGprsPesq(struObject[i].OC, struObject[i].OL, pstruXml)==NORMAL)
    	            	SaveGprsPesqLog(pstruXml);
    	            //SaveToMaintainLog("PESQ下行测试结束上报", "",  &struSendPackage);
    	            break;
    	        }
		        
		    }
		 }
		 else if (nCommUpType == 0x29) //GPRS开站请求上报
		 {
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     SaveToMaintainLog("GPRS请求开站上报", "",  &struSendPackage);

	         {
    	
	         	InitPackageToXml(&struSendPackage, pstruXml);
 	         	InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//查询
 	         	InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  "0009", MODE_AUTOGROW|MODE_UNIQUENAME);
 	         	InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
 	         	QueryMapList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         	SaveToGprsQueue(pstruXml);
	         	SaveEleQryLog(pstruXml);
	         	
	         }
	         
		 }
		 else if (nCommUpType == 0x33) //车载设备开机上报
		 {
		    
		     SaveToMaintainLog("车载设备开机上报", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x34) //车载设备关机上报
		 {
		     
		     SaveToMaintainLog("车载设备关机上报", "",  &struSendPackage);
		 }
		 else if (nCommUpType == DEVICESTARTUP) //202:效果监控请求上报
		 {
	         nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     SaveToMaintainLog("GPRS请求开站上报", "",  &struSendPackage);
		 }
		 DeleteXml(pstruXml);
		 
		 return NORMAL;
		
	}
	/*
	 *根据QB+RepeaterId获取原来信息
	 *删除对应流水号
	 */
    if(GetSendPackageInfo(QB, nRepeaterId, &struSendPackage) != NORMAL)
    {
		PrintErrorLog(DBG_HERE, "根据QB+RepeaterId获取QA错误\n");
		return EXCEPTION; 
	}
	/*
	 *设备忙情况
	 */
	if (Decodeout.NPLayer.NPJHFlag == 0x1)
	{
	     switch(struSendPackage.struHead.nCommandCode)
	     {
	         case COMMAND_QUERY:              //查询
		     case COMMAND_FCTPRM_QRY:
		     case COMMAND_PRJPRM_QRY:
			      UpdateEleFromCommByPacket(struSendPackage.struHead.QA, "busy", "", "");
			      break;
			 case COMMAND_SET:                //设置
		     case COMMAND_FCTPRM_SET:
		     case COMMAND_PRJPRM_SET:
	         case COMMAND_FACTORY_MODE:
			       UpdateEleFromCommBySetPacket(struSendPackage.struHead.QA, "busy");
			       break;
			 default:
			       break;
	     }
	     UpdateEleLastTime(struSendPackage.nNeId);//add by wwj at 2010.08.03
	     
	     SaveSysErrorLog(struSendPackage.nNeId, "",  "7", struSendPackage.szRecvMsg);//设备忙错误
	     return NORMAL;
	}
	/*
	 *处理查询监控mapid
	 */
	if (bObject0009 == BOOLTRUE)
	{

	     int nMapIdCount=(struObject[0].OL - 2)/2;
		 int j, nMapIdValue;
		 STR szMapIdValue[5];
		
		 for(j=0;j<nMapIdCount;j++)
		 {
		 	 nMapIdValue=ReadWORD(struObject[0].OC+2+j*2);
		 	 sprintf(szMapIdValue, "%04X", nMapIdValue);
		 	 STR szMapListId[29];
		 	 sprintf(szMapListId, "Ne%u%d", nRepeaterId, nDeviceId);
	         SaveToMapList(szMapListId, szMapIdValue);
	     }
	     CommitTransaction();

		 int nTotalQuery = struObject[0].OC[0];
		 int nNowQuery = struObject[0].OC[1];
		 PrintDebugLog(DBG_HERE, "站点[%u]总监控参量[%d]当前返回监控参量[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//表示还需要继续取
		 {
		 	 memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		     struObject[0].MapID = 0x0009;
		     struObject[0].OC[0] = nTotalQuery;
		     struObject[0].OC[1] = nNowQuery + 1;
		     struObject[0].OL = 2;
    
	         int nObjCount=1; 
	         
	         memset(szMsgCont, 0, sizeof(szMsgCont));
	         struPack.pPack = szMsgCont;
	         struPack.Len = 0;
	         //获取当前的2G协议流水号
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_SMS, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		         return EXCEPTION; 
	         }
	         /*
	          *   创建xml对象
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //查询
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<站点电话>", pszTelephone, MODE_AUTOGROW|MODE_UNIQUENAME);
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<服务号码>", pszNetCenterNum, MODE_AUTOGROW|MODE_UNIQUENAME);
             //站点等级为快:OMC_QUICK_MSGLEVEL
	         InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	         
		 	 SaveToMsgQueue(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//全部取完做分析监控列表
		 {
		     if (struSendPackage.struHead.nProtocolType == 47)
		     {
			     //查询扩展参量0001
			     memset(pstruXml,0,sizeof(XMLSTRU));
	         	 CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         	 InitPackageToXml(&struSendPackage, pstruXml);
	         	 
	         	  
	         	 
			 	 QueryMap0001List(M2G_SMS, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
				 SaveToMsgQueue(pstruXml);
				 DeleteXml(pstruXml);
				 
			 }
			 struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
			 
			 //return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "站点[%u]总监控量[%d]小于当前返回监控参量[%d],异常包!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }

	}
	
	if (bObject00000009 == BOOLTRUE)
	{
	     int nMapIdCount=(struObject[k].OL - 2)/4;
		 int nTotalQuery = struObject[k].OC[0];
		 int nNowQuery = struObject[k].OC[1];
 		 
 		 int j;
 		 DWORD nMapIdValue;
		 STR szMapIdValue[5];
		 STR szMapListId[29];
		 
		 for(j=0;j<nMapIdCount;j++)
		 {
		 	 nMapIdValue=ReadDWORD(struObject[0].OC+2+j*4);
		 	 sprintf(szMapIdValue, "%08X", nMapIdValue);
		 	 sprintf(szMapListId, "Ne%u%d", nRepeaterId, nDeviceId);
	         SaveToMapList(szMapListId, szMapIdValue);
	     }
	     CommitTransaction();
	     
		 PrintDebugLog(DBG_HERE, "站点[%u]总监控参量[%d]当前返回监控参量[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//表示还需要继续取
		 {
		 	 memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		     struObject[0].MapID = 0x00000009;
		     struObject[0].OC[0] = nTotalQuery;
		     struObject[0].OC[1] = nNowQuery + 1;
		     struObject[0].OL = 2;
    
	         int nObjCount=1; 
	         
	         memset(szMsgCont, 0, sizeof(szMsgCont));
	         struPack.pPack = szMsgCont;
	         struPack.Len = 0;
	         //获取当前的2G协议流水号
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_Das(M2G_SMS, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		         return EXCEPTION; 
	         }
		 	 /*
	          *   创建xml对象
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //查询
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //站点等级为快:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToMsgQueue(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//全部取完做分析监控列表
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "站点[%u]总监控量[%d]小于当前返回监控参量[%d],异常包!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }

		 

	}
	
	if (bObject0001 == BOOLTRUE)
	{
		 //查询0001失败，执行分析0009交易流程
		 
		 if (struSendPackage.struMapObjList[0].cErrorId != '0')
		 {
		 	 //struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     //DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
		     QryEleFristTime(struSendPackage.nNeId, M2G_SMS);
		 }
	     int nMapIdCount=(struObject[0].OL - 2)/6;
		 int nTotalQuery = struObject[0].OC[0];
		 int nNowQuery = struObject[0].OC[1];
 		 
 		 int j, nMapIdValue;
		 STR szMapIdValue[9], szMap0001Value[5];
		 STR szMapListId[29];
		 
		 for(j=0;j<nMapIdCount;j++)
		 {
		 	 nMapIdValue=ReadWORD(struObject[0].OC+2+j*6);
		 	 sprintf(szMap0001Value, "%04X", nMapIdValue);
		 	 
		 	 nMapIdValue=ReadDWORD(struObject[0].OC+2+j*6+2);
		 	 sprintf(szMapIdValue, "%08X", nMapIdValue);
		 	 
		 	 sprintf(szMapListId, "Ne%u%d", nRepeaterId, nDeviceId);
	         SaveToMap0001List(szMapListId, szMapIdValue, szMap0001Value);
	     }
	     CommitTransaction();
	     
		 PrintDebugLog(DBG_HERE, "站点[%u]总监控参量[%d]当前返回监控参量[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//表示还需要继续取
		 {
		 	 memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		     struObject[0].MapID = 0x0001;
		     struObject[0].OC[0] = nTotalQuery;
		     struObject[0].OC[1] = nNowQuery + 1;
		     struObject[0].OL = 2;
    
	         int nObjCount=1; 
	         
	         memset(szMsgCont, 0, sizeof(szMsgCont));
	         struPack.pPack = szMsgCont;
	         struPack.Len = 0;
	         //获取当前的2G协议流水号
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_SMS, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		         return EXCEPTION; 
	         }
	 	 
		 	 /*
	          *   创建xml对象
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //查询
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<站点电话>", pszTelephone, MODE_AUTOGROW|MODE_UNIQUENAME);
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<服务号码>", pszNetCenterNum, MODE_AUTOGROW|MODE_UNIQUENAME);
             //站点等级为快:OMC_QUICK_MSGLEVEL
	         InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         SaveToMsgQueue(pstruXml);
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//全部取完做分析监控列表
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryMap0001List((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "站点[%u]总监控量[%d]小于当前返回监控参量[%d],异常包!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }

		 

	}
	//命令来处理
	switch(struSendPackage.struHead.nCommandCode)
	{
		case COMMAND_QUERY:              //解析查询
		case COMMAND_FCTPRM_QRY:
		case COMMAND_PRJPRM_QRY:
			DecodeQryElementParam((SENDPACKAGE *)&struSendPackage);
			break;
		case COMMAND_SET:                //解析设置
		case COMMAND_FCTPRM_SET:
		case COMMAND_PRJPRM_SET:
	    case COMMAND_FACTORY_MODE:
			DecodeSetElementParam((SENDPACKAGE *)&struSendPackage);
			break;

		case COMMAND_QUERY_MAPLIST:    //取到监控量列表
			DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
			break;
		default:
			break;
	}
    
    if (strncmp(struSendPackage.struHead.QA, "update", 6) == 0)
    {
        RemortUpdateDbOperate2(struSendPackage.struHead.QA);
    }
	return NORMAL;	
}


RESULT Process2GGprs(int nQryLogId, PSTR pszUndecode, INT nLen)
{
    XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	DECODE_OUT	Decodeout;
	SENDPACKAGE struSendPackage;
	OBJECTSTRU struObject[MAX_OBJECT_NUM];
	STR szTemp[200], szMapId[10];
	int i, k=0;
	UCHAR nCommUpType = 0;
	int nNeId, nEleQryLogId;
	STR szNeName[50], szAlarmObjList[1000], szAlarmTime[20];
	BOOL bIsNewNeId=BOOLFALSE;
	UINT nTstId;
	BYTEARRAY struPack;
	STR szMsgCont[500];
	STR szDeviceIp[30];
	int nDevicePort;
	
	//转义处理
	struPack.pPack = pszUndecode;
	struPack.Len = nLen;
	
    if(!AscUnEsc(&struPack))
	{
	    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
		return -1;
	}
   	
   	//PrintHexDebugLog("接收转义请求报文",  struPack.pPack, struPack.Len);	
	
	//解析2G协议
    if (Decode_2G(M2G_TCPIP, &struPack, &Decodeout, struObject) != NORMAL)
    {
    	PrintErrorLog(DBG_HERE, "解析设备内容错误\n");
		return EXCEPTION; 
    }

	
    PrintDebugLog(DBG_HERE, "解析GPRS协议成功,协议类型[%02d],命令标识[%d],错误代码[%d],站点编号[%u],设备编号[%d],网络标识[%d],对象数[%d]\n",
        Decodeout.NPLayer.APID,  Decodeout.MAPLayer.CommandFalg, Decodeout.APLayer.ErrorCode, 
        Decodeout.NPLayer.structRepeater.RepeaterId, Decodeout.NPLayer.structRepeater.DeviceId,
        Decodeout.NPLayer.NetFlag, Decodeout.MAPLayer.ObjCount);
	
	int QB=Decodeout.NPLayer.NetFlag;//2G协议通讯标示流水号
	UINT nRepeaterId=Decodeout.NPLayer.structRepeater.RepeaterId;
	int nDeviceId=Decodeout.NPLayer.structRepeater.DeviceId;
		
	//初始化分解对象
	memset(&struSendPackage, 0, sizeof(SENDPACKAGE));
	struSendPackage.struRepeater.nRepeaterId=Decodeout.NPLayer.structRepeater.RepeaterId;
    struSendPackage.struRepeater.nDeviceId=Decodeout.NPLayer.structRepeater.DeviceId;
	
	//设备通讯方式 
	struSendPackage.struRepeater.nCommType=M2G_UDP_TYPE;
	
	//对象数
	struSendPackage.struHead.nObjectCount= Decodeout.MAPLayer.ObjCount;
	//上报方式
	struSendPackage.struHead.nCommandCode=COMMAND_UP;
	
	//if (GetNeInfo(&struSendPackage) != NORMAL)
	struSendPackage.struHead.nProtocolType= PROTOCOL_2G;
	
	//协议方式 2013.10.9
	if (Decodeout.NPLayer.APID == 0x03)
	{
		struSendPackage.struHead.nProtocolType= PROTOCOL_DAS;
	}

	//QA流水号
	strcpy(struSendPackage.struHead.QA, GetTypeNumber("Gprs"));
	 
	
	//初始化不为监控对象参数命令
	BOOL bObject0009=BOOLFALSE;
	BOOL bObject0606=BOOLFALSE;//批采
	BOOL bObject0875=BOOLFALSE;//时隙
	BOOL bObject0AEC=BOOLFALSE;//das
	BOOL bObject0AED=BOOLFALSE;//rfid
	BOOL bObject0001=BOOLFALSE;
	BOOL bObject00000009=BOOLFALSE;
	BOOL bObject00000606=BOOLFALSE;//批采
	for(i=0; i< Decodeout.MAPLayer.ObjCount; i++)
	{
	    bufclr(szMapId);
	    struSendPackage.struMapObjList[i].nMapLen = struObject[i].OL;
	    if (struSendPackage.struHead.nProtocolType == PROTOCOL_DAS)
	    	sprintf(szMapId, "%08X", struObject[i].MapID);
	    else
	    	sprintf(szMapId, "%04X", struObject[i].MapID);
	    
	    struSendPackage.struMapObjList[i].cErrorId = szMapId[0];
	    ReplaceCharByPos(szMapId, '0', 1);
	    
	    if (struSendPackage.struHead.nProtocolType == PROTOCOL_JINXIN_DAS)
	    {
	    	strcpy(szTemp, szMapId);
	    	GetMapIdFromParam(nRepeaterId, szTemp, szMapId);

	    }
	    
	    strcpy(struSendPackage.struMapObjList[i].szMapId, szMapId);
	    if (DecodeMapDataFromMapId(szMapId, struObject[i].OC, struSendPackage.struMapObjList[i].szMapData, 
	    	struSendPackage.struMapObjList[i].szMapType) == EXCEPTION)
	    {

	    }
	    //PrintDebugLog(DBG_HERE,"MAPID=[%s],OL=[%d]\n",szMapId,  struObject[i].OL);
		//PrintHexDebugLog("MAPDATA", struObject[i].OC, struObject[i].OL);
				
		//取上报类型
		if (strcmp(szMapId, "0141") == 0 || strcmp(szMapId, "00000141") == 0)
		{
		    nCommUpType = struObject[i].OC[0];
		    PrintDebugLog(DBG_HERE, "上报类型[%X]GPRS开始\n", nCommUpType);
		    if (nCommUpType == 0xC8)         //gprs的开站上报
		    {
		        bObject0009 = BOOLTRUE;
		        k = i+1;
		        break;
		    }
		}
		if (strcmp(szMapId, "0009") == 0 )//监控量上报
			bObject0009 = BOOLTRUE;
		else if (strcmp(szMapId, "0606") == 0)//批采
		    bObject0606 = BOOLTRUE;
		else if (strcmp(szMapId, "00000606") == 0)//批采
		    bObject00000606 = BOOLTRUE;
		else if (strcmp(szMapId, "0875") == 0)//时隙
		    bObject0875 = BOOLTRUE;    	
		else if (strcmp(szMapId, "0AEC") == 0)//das
		    bObject0AEC = BOOLTRUE;
		else if (strcmp(szMapId, "0AED") == 0)//rfid
		    bObject0AED = BOOLTRUE; 
		else if ( strcmp(szMapId, "00000009") == 0)
			bObject00000009 = BOOLTRUE;
		if (strcmp(szMapId, "0001") == 0 )//扩展参量上报
			bObject0001 = BOOLTRUE;
		if (strcmp(szMapId, "0004") == 0 )
			ReplaceStr(struSendPackage.struMapObjList[i].szMapData, ",", "-");
		//判断时隙
		if (strcmp(struSendPackage.struMapObjList[i].szMapType, "ratedata") == 0)
			bObject0875 = BOOLTRUE; 
	}
	
	/*
 	 * 初始化xml报文
	*/
	memset(pstruXml,0,sizeof(XMLSTRU));
	CreateXml(pstruXml, FALSE, OMC_ROOT_PATH, NULL);
		
	if(Decodeout.MAPLayer.CommandFalg == 1)
	{
		 BOOL bAlarmRestore = BOOLFALSE;
		 STR szFrequentAlarmList[1000]; //频繁告警列表
		 STR szAlarmRestoreList[1000]; //告警告警列表
		 INT nAlarmTimes=0;
		 
		 bufclr(szFrequentAlarmList);
		 bufclr(szAlarmRestoreList);
		
		
		//根据上报类型处理
		if (nCommUpType == DEVICE_ALARM) //设备告警
		{
		     /*
		      *  取告警列表,并取网元NeId保留
		      */
		     if (GetAlarmObjList3(nRepeaterId, nDeviceId, &nNeId, szNeName, szAlarmObjList)!= NORMAL)
		     {
		         DeleteXml(pstruXml);
		         PrintErrorLog(DBG_HERE, "设备号[%u][%d]不存在记录\n", nRepeaterId, nDeviceId);
		         return EXCEPTION;
		     }
		     InsertInXmlExt(pstruXml,"<omc>/<告警列表>",szAlarmObjList,MODE_AUTOGROW|MODE_UNIQUENAME);
		     sprintf(szTemp, "%d", nNeId);
	     	 InsertInXmlExt(pstruXml,"<omc>/<网元编号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
             struSendPackage.nNeId = nNeId;
             
             BOOL bNewAlarm = BOOLFALSE;
             bufclr(szAlarmTime);
	         for(i=0; i< Decodeout.MAPLayer.ObjCount; i++)
	         {
	             if (struSendPackage.struMapObjList[i].cErrorId == '1')
	             {
	                 bAlarmRestore = BOOLFALSE;
	                 continue;
	             }
	             if (strstr(szAlarmObjList, struSendPackage.struMapObjList[i].szMapId) == NULL)
	             {
	             	 if (strcmp(struSendPackage.struMapObjList[i].szMapId, "0801") == 0 
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "0802") == 0 
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "0803") == 0 
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "080F") == 0
	             	 	 
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "04F0") == 0
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "04F1") == 0
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "04F2") == 0
	             	 	 || strcmp(struSendPackage.struMapObjList[i].szMapId, "04F3") == 0
	             	 )
	             	 {
	             	 	 strcpy(szAlarmTime, struSendPackage.struMapObjList[i].szMapData);
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<告警时间>",szAlarmTime,MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	             	 if (strcmp(struSendPackage.struMapObjList[i].szMapId, "050B") == 0 ||
	             	 	 strcmp(struSendPackage.struMapObjList[i].szMapId, "08B3") == 0)   //TD
	             	 {
	             	 	 PrintDebugLog(DBG_HERE,"电平强度[%s]\n",struSendPackage.struMapObjList[i].szMapData);
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<电平强度>",struSendPackage.struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	             	 if (strcmp(struSendPackage.struMapObjList[i].szMapId, "050C") == 0)
	             	 {
	             	 	PrintDebugLog(DBG_HERE,"服务小区[%s]\n",struSendPackage.struMapObjList[i].szMapData);
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<服务小区>",struSendPackage.struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	                 continue;
	             }
	             
	             if (strcmp(struSendPackage.struMapObjList[i].szMapData, "1") == 0)//告警处理
	             {
	             	 if (nAlarmTimes++ >= 1)
	             	 {
	             	 	 if (DealNewAlarm(pstruXml) == NORMAL)
                      	 strcat(szFrequentAlarmList, struAlarmPara.szAlarmName);//用于频繁上报
	             	 	 //频繁告警

	             	 	 SaveToMaintainLog("新告警", "",  &struSendPackage);
	             	 	 InitAlarmPara(nNeId);
	             	 	// AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarm");//是否频繁告警
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
	                 		 strcat(szAlarmRestoreList, struAlarmPara.szAlarmName);//用于告警恢复前转

	             		 SaveToMaintainLog("告警恢复", "",  &struSendPackage);
	             		 InitAlarmPara(nNeId);
	             		// AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarmClear");//频繁告警清除
	             		 if (strlen(szAlarmRestoreList) > 0)
	                 		 TransferAlarm();
	                 }
	                 
	                 if (bNewAlarm == BOOLTRUE)
	             	 	InsertInXmlExt(pstruXml,"<omc>/<告警对象>",szAlarmObjResv,MODE_AUTOGROW|MODE_UNIQUENAME);
	             }            
	         }
	         
             if (bNewAlarm == BOOLTRUE)
	         {
	             if (DealNewAlarm(pstruXml) == NORMAL)
                      strcat(szFrequentAlarmList, struAlarmPara.szAlarmName);//用于频繁上报
	             //频繁告警

	             SaveToMaintainLog("新告警", "",  &struSendPackage);
	             InitAlarmPara(nNeId);
	            // AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarm");//是否频繁告警
	         }

		}
		else if (nCommUpType == OPENSTATION) //UDP开站上报
		{
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     SaveToMaintainLog("开站上报", "",  &struSendPackage);
		     if (bIsNewNeId == BOOLFALSE)
		     {
		        InitAlarmPara(nNeId);
	            AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeBulid");//开站上报
	         }
	         else
	         {
	         	/*取设备IP和端口 */
	         	if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         	{
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "设备[%u][%d]不存在IP和PORT记录\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     	}

	         	strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
				struSendPackage.struRepeater.nPort = nDevicePort;
				
	         	/* 2010.6.8 add */
	         	QueryMapList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         	InitPackageToXml(&struSendPackage, pstruXml);
	         	 
	         	InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//查询
	         	InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         	InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	         					
	         	SaveToGprsQueue(pstruXml);
	            SaveEleQryLog(pstruXml);
	         	         	
	         }
		}
		else if (nCommUpType == DEVICECHANGED) //变更上报
		{
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     
		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeRevamp");//变更上报
	         /*取设备IP和端口 */
	         if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         {
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "设备[%u][%d]不存在IP和PORT记录\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     }
	         strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
			 struSendPackage.struRepeater.nPort = nDevicePort;
	         
	         //////////////////////////////////////////查询监控量
	         QueryMapList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         InitPackageToXml(&struSendPackage, pstruXml);

	         SaveToMaintainLog("变更上报", "",  &struSendPackage);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//查询
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);	
	         SaveToGprsQueue(pstruXml);
	         SaveEleQryLog(pstruXml);
		     
		}
		else if (nCommUpType == DEVICEREPAIR) //修复上报
		{
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;

		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeRebuild");
	         /*取设备IP和端口 */
	         if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         {
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "设备[%u][%d]不存在IP和PORT记录\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     }
	         strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
			 struSendPackage.struRepeater.nPort = nDevicePort;
	         
	         //////////////////////////////////////////查询监控量
	         if (bIsNewNeId == BOOLFALSE)
	         {
	             QueryMapList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	             InitPackageToXml(&struSendPackage, pstruXml);

	             SaveToMaintainLog("修复上报", "",  &struSendPackage);
	             
	             InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//查询
	         	 InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         	 InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	             SaveToGprsQueue(pstruXml);
	             SaveEleQryLog(pstruXml);
	         }
	         else
	         {
	             /*
	              *开站上报
	              *实现网元监控量查询，记录消息队列表
	              */
	             QryEleFristTime(nNeId, M2G_SMS);
	         }
	         
		     
		}
		else if (nCommUpType == PERSONPATROL) //巡检上报信息
		{
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;

		     SaveToMaintainLog("巡检上报", "",  &struSendPackage);
		     
		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeCheck");//巡检上报
		}
		else if (nCommUpType == 0xC8) //老的gprs开站上报
    	{
		     {
		         if (GetPackInfoFromMainLog(&struSendPackage) != NORMAL)
    	     	{
    	     	    PrintErrorLog(DBG_HERE, "GPRS开站上报没有发送请求上报类型0xCA,返回错误!\n");
    	     	    DeleteXml(pstruXml);
		     	    return EXCEPTION;
    	     	}
		     }
    	     
    	     
		 	sprintf(szTemp, "%d", struSendPackage.nNeId);
		     InsertInXmlExt(pstruXml,"<omc>/<网元编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
    	 
		 	//SaveToMaintainLog("GPRS监控量上报", "",  &struSendPackage); 
		 	//分析上报监控量
		 	DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
		}
		else if (nCommUpType == 0x06) //登录到监控中心上报
		{
		    
		    SaveToMaintainLog("登录到监控中心", "",  &struSendPackage);
		}
		else if (nCommUpType == 0x07) //心跳上报
		{
		   
		    SaveToMaintainLog("心跳上报", "",  &struSendPackage);
		}
		else if (nCommUpType == 0x08) //远程升级结果上报
		{
		    
			sprintf(szTemp, "%d", struSendPackage.nNeId);
		    InsertInXmlExt(pstruXml,"<omc>/<网元编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		    InsertInXmlExt(pstruXml,"<omc>/<网元编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
			sprintf(szTemp, "%u", struSendPackage.struRepeater.nRepeaterId);
			InsertInXmlExt(pstruXml,"<omc>/<设备编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
			sprintf(szTemp, "%d", struSendPackage.struRepeater.nDeviceId);
			InsertInXmlExt(pstruXml,"<omc>/<次编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		    for(i=0; i< Decodeout.MAPLayer.ObjCount; i++)
		    {
		        sprintf(szMapId, "%04X", struObject[i].MapID);
    	        if (strcmp(szMapId, "000A") == 0 || strcmp(szMapId, "000a") == 0)
    	        {
    	            InsertInXmlExt(pstruXml,"<omc>/<VerInfo>", struSendPackage.struMapObjList[i].szMapData, MODE_AUTOGROW|MODE_UNIQUENAME);
    	        }
    	        else if (strcmp(szMapId, "0018") == 0)
    	        {
    	            InsertInXmlExt(pstruXml,"<omc>/<UpdateResult>", struSendPackage.struMapObjList[i].szMapData, MODE_AUTOGROW|MODE_UNIQUENAME);
    	        }
		    }
		    RemortUpdateDbOperate3(pstruXml);
		    SaveToMaintainLog("远程升级结果上报", "",  &struSendPackage);
		}
		else if (nCommUpType == 0x09) //GPRS登录失败上报
		{
		   
		    SaveToMaintainLog("GPRS登录失败上报", "",  &struSendPackage);
		}
		else if (nCommUpType == 0x0A) //批采结束上报
		{
			char szMapObject[100];
		    
		    SaveToMaintainLog("批采结束上报", "",  &struSendPackage);
		    /*取设备IP和端口 */
	         	if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         	{
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "设备[%u][%d]不存在IP和PORT记录\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     	}
	         	strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
				struSendPackage.struRepeater.nPort = nDevicePort;
		    //2015.08.18
		    //初始化xml
	     	InitPackageToXml(&struSendPackage, pstruXml);
	     	if (struSendPackage.struHead.nProtocolType == PROTOCOL_DAS)
	     	{
		    	InsertInXmlExt(pstruXml,"<omc>/<监控对象>","00000606",MODE_AUTOGROW|MODE_UNIQUENAME);
		    	GetBatPickMap8(struSendPackage.nNeId, szMapObject);
		    }
		    else
		    {
		    	InsertInXmlExt(pstruXml,"<omc>/<监控对象>","0606",MODE_AUTOGROW|MODE_UNIQUENAME);
		    	GetBatPickMap4(struSendPackage.nNeId, szMapObject);
		    }
		    
		    PrintDebugLog(DBG_HERE, "szMapObject[%s]\n", szMapObject);
		    InsertInXmlExt(pstruXml,"<omc>/<监控对象内容>",szMapObject,MODE_AUTOGROW|MODE_UNIQUENAME);
		    
		    InsertInXmlExt(pstruXml,"<omc>/<类型>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);//查询
         	InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
         	
		    struSendPackage.struHead.nCommandCode=COMMAND_QUERY;
		    
		    QryElementParam(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
        	SaveToGprsQueue(pstruXml);
        	SaveEleQryLog(pstruXml);
        	
        	InitBatPick(pstruXml);
        	SaveBatPickLog(pstruXml);
		}
		else if (nCommUpType == 0x0B) //软件异常复位上报
		{
		   
		    SaveToMaintainLog("软件异常复位上报", "",  &struSendPackage);
		}
		else if (nCommUpType == 0x20 || nCommUpType == 0x21 || nCommUpType == 0xC9) //定时上报
		{
		    
			 //0x21 TD定时上报
			 if (nCommUpType == 0x21)
			 {
			 	DecodeQryOnTime(&struSendPackage, BOOLFALSE, BOOLFALSE);
			    //SaveToMaintainLog("TD-SCDMA定时上报", "",  &struSendPackage);
			 }
			 else if (nCommUpType == 0x20)
			 {
			 	DecodeQryOnTime(&struSendPackage, BOOLTRUE, BOOLFALSE);
			    //SaveToMaintainLog("GPRS定时上报", "",  &struSendPackage);
			 }
			 else 
			 {
			 	DecodeQryOnTime(&struSendPackage, BOOLTRUE, BOOLTRUE);
			 }	
		}
		
		else if (nCommUpType == 0x26) //远程升级请求上报
		{
		    
		    SaveToMaintainLog("远程升级请求上报", "",  &struSendPackage);
		}
		else if (nCommUpType == 0xF1 || nCommUpType == 0xF2) //das变更上报, das删除上报
		{
			 nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     if (nCommUpType == 0xF1) 
		     	SaveToMaintainLog("Topologic update", "",  &struSendPackage);
		     else if (nCommUpType == 0xF2)
		     	SaveToMaintainLog("Delete device", "",  &struSendPackage); 

	         //////////////////////////////////////////查询监控量
	         /*取设备IP和端口 */
	         if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         {
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "设备[%u][%d]不存在IP和PORT记录\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     }
	         strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
			 struSendPackage.struRepeater.nPort = nDevicePort;
			
	         QueryDasList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         InitPackageToXml(&struSendPackage, pstruXml);
	        	         
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);//查询
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);	
	         SaveToGprsQueue(pstruXml);
	         SaveEleQryLog(pstruXml);
		}
		else if (nCommUpType == 0xF3)
		{
			 nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     SaveToMaintainLog("RFID update", "",  &struSendPackage);

	         //////////////////////////////////////////查询监控量
	         /*取设备IP和端口 */
	         if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         {
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "设备[%u][%d]不存在IP和PORT记录\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     }
	         strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
			 struSendPackage.struRepeater.nPort = nDevicePort;
			 
	         QueryRfidList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         InitPackageToXml(&struSendPackage, pstruXml);
	        	         
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);//查询
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);	
	         SaveToGprsQueue(pstruXml);
	         SaveEleQryLog(pstruXml);
		}
		
		DeleteXml(pstruXml);
		return NORMAL;
	}
	
	/*
	 *根据QB+RepeaterId获取原来信息
	 *删除对应流水号
	 */
	if (getenv("WUXIAN")!=NULL)
	{
		if (GetRedisPackageInfo(QB, &struSendPackage, pstruXml) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "根据QB+RepeaterId获取redis错误 %u_%d_%d\n", nRepeaterId, nDeviceId, QB);
			return EXCEPTION;
		}
	}
	else if(GetGprsPackageInfo(QB, nRepeaterId, &struSendPackage) != NORMAL)
    {
		PrintErrorLog(DBG_HERE, "根据QB+RepeaterId获取QA错误\n");
		return EXCEPTION; 
	}
	
	/*
	 *处理开站上报查监控量
	 */
	if (bObject0009 == BOOLTRUE)
	{
	     int nMapIdCount=(struObject[k].OL - 2)/2;
		 int nTotalQuery = struObject[k].OC[0];
		 int nNowQuery = struObject[k].OC[1];
 		 
 		 int j, nMapIdValue;
		 STR szMapIdValue[5];
		 STR szMapListId[29];
		 
		 for(j=0;j<nMapIdCount;j++)
		 {
		 	 nMapIdValue=ReadWORD(struObject[0].OC+2+j*2);
		 	 sprintf(szMapIdValue, "%04X", nMapIdValue);
		 	 sprintf(szMapListId, "Ne%u%d", nRepeaterId, nDeviceId);
	         SaveToMapList(szMapListId, szMapIdValue);
	     }
	     CommitTransaction();
	     
		 PrintDebugLog(DBG_HERE, "站点[%u]总监控参量[%d]当前返回监控参量[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//表示还需要继续取
		 {
		 	 memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		     struObject[0].MapID = 0x0009;
		     struObject[0].OC[0] = nTotalQuery;
		     struObject[0].OC[1] = nNowQuery + 1;
		     struObject[0].OL = 2;
    
	         int nObjCount=1; 
	         
	         memset(szMsgCont, 0, sizeof(szMsgCont));
	         struPack.pPack = (BYTE*)szMsgCont;
	         struPack.Len = 0;
	         //获取当前的2G协议流水号
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
			 	   return -1;
			 }
	         /*
	          *   创建xml对象
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //查询
	         sprintf(szTemp, "%d", nEleQryLogId);
		     //InsertInXmlExt(pstruXml,"<omc>/<流水号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     InsertInXmlExt(pstruXml,"<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //站点等级为快:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);
		 	 //SaveEleQryLog(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//全部取完做分析监控列表
		 {
		 	 if (struSendPackage.struHead.nProtocolType == 47)
		 	 {
			 	 //查询扩展参量0001
			 	 memset(pstruXml,0,sizeof(XMLSTRU));
		         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
		         InitPackageToXml(&struSendPackage, pstruXml);
		         	 
			 	 QueryMap0001List(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
				 SaveToGprsQueue(pstruXml);
				 //SaveEleQryLog(pstruXml);
				 DeleteXml(pstruXml);
			 }
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		     		     
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "站点[%d]总监控量[%d]小于当前返回监控参量[%d],异常包!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }

		 

	}
	
	if (bObject00000009 == BOOLTRUE)
	{
	     int nMapIdCount=(struObject[k].OL - 2)/4;
		 int nTotalQuery = struObject[k].OC[0];
		 int nNowQuery = struObject[k].OC[1];
 		 
 		 int j;
 		 DWORD nMapIdValue;
		 STR szMapIdValue[5];
		 STR szMapListId[29];
		 
		 for(j=0;j<nMapIdCount;j++)
		 {
		 	 nMapIdValue=ReadDWORD(struObject[0].OC+2+j*4);
		 	 sprintf(szMapIdValue, "%08X", nMapIdValue);
		 	 sprintf(szMapListId, "Ne%u%d", nRepeaterId, nDeviceId);
	         SaveToMapList(szMapListId, szMapIdValue);
	     }
	     CommitTransaction();
	     
		 PrintDebugLog(DBG_HERE, "站点[%d]总监控参量[%d]当前返回监控参量[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//表示还需要继续取
		 {
		 	 memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		     struObject[0].MapID = 0x00000009;
		     struObject[0].OC[0] = nTotalQuery;
		     struObject[0].OC[1] = nNowQuery + 1;
		     struObject[0].OL = 2;
    
	         int nObjCount=1; 
	         
	         memset(szMsgCont, 0, sizeof(szMsgCont));
	         struPack.pPack = szMsgCont;
	         struPack.Len = 0;
	         //获取当前的2G协议流水号
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_Das(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
			 	   return -1;
			 }
	         /*
	          *   创建xml对象
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //查询
	         sprintf(szTemp, "%d", nEleQryLogId);
		     //InsertInXmlExt(pstruXml,"<omc>/<流水号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     InsertInXmlExt(pstruXml,"<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //站点等级为快:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);
		 	 //SaveEleQryLog(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//全部取完做分析监控列表
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "站点[%d]总监控量[%d]小于当前返回监控参量[%d],异常包!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }

		 

	}
	/*
	 *处理京信4位和8位共存, 执行DecodeQueryMap0001List
	 */
	if (bObject0001 == BOOLTRUE)
	{
		 //查询0001失败，执行分析0009交易流程
		 if (struSendPackage.struMapObjList[0].cErrorId != '0')
		 {
		 	 //struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     //DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
		     QryEleFristTime(struSendPackage.nNeId, M2G_TCPIP);
		 }
	     int nMapIdCount=(struObject[k].OL - 2)/6;
		 int nTotalQuery = struObject[k].OC[0];
		 int nNowQuery = struObject[k].OC[1];
 		 
 		 int j, nMapIdValue;
		 STR szMapIdValue[9], szMap0001Value[5];
		 STR szMapListId[29];
		 
		 for(j=0;j<nMapIdCount;j++)
		 {
		 	 nMapIdValue=ReadWORD(struObject[0].OC+2+j*6);
		 	 sprintf(szMap0001Value, "%04X", nMapIdValue);
		 	 
		 	 nMapIdValue=ReadDWORD(struObject[0].OC+2+j*6+2);
		 	 sprintf(szMapIdValue, "%08X", nMapIdValue);
		 	 
		 	 sprintf(szMapListId, "Ne%d%d", nRepeaterId, nDeviceId);
	         SaveToMap0001List(szMapListId, szMapIdValue, szMap0001Value);
	     }
	     CommitTransaction();
	     
		 PrintDebugLog(DBG_HERE, "站点[%d]总监控参量[%d]当前返回监控参量[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//表示还需要继续取
		 {
		 	 memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		     struObject[0].MapID = 0x0001;
		     struObject[0].OC[0] = nTotalQuery;
		     struObject[0].OC[1] = nNowQuery + 1;
		     struObject[0].OL = 2;
    
	         int nObjCount=1; 
	         
	         memset(szMsgCont, 0, sizeof(szMsgCont));
	         struPack.pPack = szMsgCont;
	         struPack.Len = 0;
	         //获取当前的2G协议流水号
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
			 	   return -1;
			 }
	         /*
	          *   创建xml对象
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //查询
	         sprintf(szTemp, "%d", nEleQryLogId);
		     InsertInXmlExt(pstruXml,"<omc>/<流水号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     InsertInXmlExt(pstruXml,"<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //站点等级为快:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);
		 	 //SaveEleQryLog(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//全部取完做分析监控列表
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryMap0001List((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "站点[%d]总监控量[%d]小于当前返回监控参量[%d],异常包!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }

		 

	}
	
	/*
	 *处理DAS查拓扑图, 执行DecodeQueryDasList
	 */
	if (bObject0AEC == BOOLTRUE)
	{
		 int nMapIdCount=(struObject[0].OL - 9)/31; //条数
		 
		 int nTotalQuery = struObject[0].OC[0];
		 int nNowQuery = struObject[0].OC[1];
 		 
 		 int j, nMapIdValue;
		 STR szMapIdValue[5];
		 STR szMapListId[29];
		 int nConnStat, nDeviceTypeId;
		 STR szRouter[20], szAddrInfo[50];
		 UCHAR cMapData1, cMapData2, cMapData3, cMapData4;
 		 for(j=0;j<nMapIdCount;j++)
		 {
		 	 nDeviceId = struObject[0].OC[9+j*31];
		 	 cMapData1 = struObject[0].OC[9+j*31+1];
		 	 cMapData2 = struObject[0].OC[9+j*31+2];
         	 cMapData3 = struObject[0].OC[9+j*31+3];
         	 cMapData4 = struObject[0].OC[9+j*31+4];
		 	 sprintf(szRouter, "%u.%u.%u.%u", cMapData1, cMapData2, cMapData3,cMapData4);
			 cMapData1 = struObject[0].OC[9+j*31+5];
		 	 cMapData2 = struObject[0].OC[9+j*31+6];
         	 cMapData3 = struObject[0].OC[9+j*31+7];
         	 cMapData4 = struObject[0].OC[9+j*31+8];
		 	 sprintf(szDeviceIp, "%u.%u.%u.%u", cMapData1, cMapData2, cMapData3,cMapData4);
		 	 nConnStat = struObject[0].OC[9+j*31+9];
		 	 nDeviceTypeId = struObject[0].OC[9+j*31+10];
		 	 
		 	 PrintDebugLog(DBG_HERE, "%s, %s,%d\n", szRouter, szDeviceIp, nDeviceTypeId);
		 	 
		 	 memset(szAddrInfo, 0, sizeof(szAddrInfo));
		 	 memcpy(szAddrInfo, struObject[0].OC+9+j*31+11, 20);
		 	 PrintDebugLog(DBG_HERE, "%s\n", szAddrInfo);
		 	 
	         SaveToDasList(nRepeaterId, nDeviceId, szRouter, szDeviceIp, nConnStat, nDeviceTypeId, szAddrInfo);
	     }
	     CommitTransaction();
	     
		 PrintDebugLog(DBG_HERE, "站点[%d]总监控参量[%d]当前返回监控参量[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//表示还需要继续取
		 {
		 	 memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		     struObject[0].MapID = 0x0AEC;
		     struObject[0].OC[0] = nTotalQuery;
		     struObject[0].OC[1] = nNowQuery + 1;
		     struObject[0].OC[2] = 0x06;
		     struObject[0].OC[3] = 0x01;
		     struObject[0].OC[4] = 0x02;
		     struObject[0].OC[5] = 0x03;
		     struObject[0].OC[6] = 0x04;
		     struObject[0].OC[7] = 0x06;
		     struObject[0].OC[8] = 0x07;
		     //struObject[0].OC[9] = 0x08;
		     struObject[0].OL = 9;
  		     /*
		     strcpy(szSpecialCode, struSendPackage.struRepeater.szSpecialCode);
		     nSeperateNum = SeperateString(szSpecialCode,  ',', pszSpecialCodeStr, MAX_SEPERATE_NUM);	
	
			if (nSeperateNum < 4) 
			{
				PrintErrorLog(DBG_HERE,"分割错误[%s]\n",szSpecialCode);
				return -1;
			}	
			struObjList[0].OC[2] = nSeperateNum;
			//将放入分割参数
			for(i=0; i< nSeperateNum; i++)
			{
				struObjList[0].OC[i+3] = atoi(pszSpecialCodeStr[i]);
			}
			struObjList[0].OL = i+3;
			*/
	         int nObjCount=1; 
	         
	         memset(szMsgCont, 0, sizeof(szMsgCont));
	         struPack.pPack = szMsgCont;
	         struPack.Len = 0;
	         //获取当前的2G协议流水号
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
			 	   return -1;
			 }
	         /*
	          *   创建xml对象
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //查询
	         sprintf(szTemp, "%d", nEleQryLogId);
		     InsertInXmlExt(pstruXml,"<omc>/<流水号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     InsertInXmlExt(pstruXml,"<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //站点等级为快:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);
		 	 //SaveEleQryLog(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery && nMapIdCount > 0)//全部取完做分析监控列表
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryDasList((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "站点[%d]总监控量[%d]小于当前返回监控参量[%d],异常包!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }
		 return NORMAL;
	}
	
	/*
	 *处理RFID查拓扑图
	 */
	if (bObject0AED == BOOLTRUE)
	{
		 int nMapIdCount=(struObject[0].OL - 6)/6; //条数
		 
		 int nTotalQuery = struObject[0].OC[0];
		 int nNowQuery = struObject[0].OC[1];
 		 
 		 int j, nMapIdValue;
		 STR szMapIdValue[5];
		 STR szMapListId[29];
		 int nConnStat, nDeviceTypeId;
		 STR szRouter[20];
		 UCHAR cMapData1, cMapData2, cMapData3, cMapData4;
 		 for(j=0;j<nMapIdCount;j++)
		 {
		 	 cMapData1 = struObject[0].OC[6+j*6];
		 	 cMapData2 = struObject[0].OC[6+j*6+1];
         	 cMapData3 = struObject[0].OC[6+j*6+2];
         	 cMapData4 = struObject[0].OC[6+j*6+3];
         	 //sprintf(szRouter, "%u.%u.%u.%u", cMapData1, cMapData2, cMapData3,cMapData4);
		 	 sprintf(szRouter, "%02x%02x%02x%02x", cMapData1, cMapData2, cMapData3,cMapData4);
		 	 nConnStat = struObject[0].OC[6+j*6+4];
		 	 nDeviceTypeId = struObject[0].OC[6+j*6+5];
		 	 
	         SaveToRfidList(nRepeaterId, nDeviceId, szRouter, nConnStat, nDeviceTypeId);
	     }
	     CommitTransaction();
	     
		 PrintDebugLog(DBG_HERE, "站点[%d]总监控参量[%d]当前返回监控参量[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//表示还需要继续取
		 {
		 	 memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		     struObject[0].MapID = 0x0AED;
		     struObject[0].OC[0] = nTotalQuery;
		     struObject[0].OC[1] = nNowQuery + 1;
		     struObject[0].OC[2] = 0x03;
		     struObject[0].OC[3] = 0x01;
		     struObject[0].OC[4] = 0x02;
		     struObject[0].OC[5] = 0x03;
		     struObject[0].OL = 6;
    
	         int nObjCount=1; 
	         
	         memset(szMsgCont, 0, sizeof(szMsgCont));
	         struPack.pPack = szMsgCont;
	         struPack.Len = 0;
	         //获取当前的2G协议流水号
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
			 	   return -1;
			 }
	         /*
	          *   创建xml对象
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //查询
	         sprintf(szTemp, "%d", nEleQryLogId);
		     InsertInXmlExt(pstruXml,"<omc>/<流水号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     InsertInXmlExt(pstruXml,"<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //站点等级为快:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);
		 	 //SaveEleQryLog(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery && nMapIdCount > 0)//全部取完做分析监控列表
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryRfidList((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "站点[%d]总监控量[%d]小于当前返回监控参量[%d],异常包!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }
		 return NORMAL;
	}
	/*
	 *处理批采
	 */
	if (bObject0606 == BOOLTRUE)
	{
		unsigned short nQryId;
		STR szQryId[5];
		int nMapId;
		STR szMapId[10];
		int nBatPickLen;
		STR szBatPickValue[100];
		

         /*
          *   创建xml对象
          */
         memset(pstruXml,0,sizeof(XMLSTRU));
         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
         InitPackageToXml(&struSendPackage, pstruXml);		
		
		sprintf(szTemp, "%d", struSendPackage.nNeId);
		InsertInXmlExt(pstruXml,"<omc>/<网元编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		sprintf(szTemp, "%d", QB);
		InsertInXmlExt(pstruXml,"<omc>/<通信包标识>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		
		nQryId=ReadWORD(struObject[0].OC);
		sprintf(szQryId, "%04X", nQryId);
		nMapId=ReadWORD(struObject[0].OC+2);
		sprintf(szMapId, "%04X", nMapId);
		nBatPickLen = struObject[0].OL-4;
		memset(szBatPickValue, 0, sizeof(szBatPickValue));
		memcpy(szBatPickValue, struObject[0].OC+4, nBatPickLen);
		
		sprintf(szTemp, "%d", nQryId);
		InsertInXmlExt(pstruXml,"<omc>/<查询编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<批采对象>", szMapId, MODE_AUTOGROW|MODE_UNIQUENAME);
		sprintf(szTemp, "%d", nBatPickLen);
		InsertInXmlExt(pstruXml,"<omc>/<批采长度>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
//		InsertInXmlExt(pstruXml,"<omc>/<批采数值>", szBatPickValue, MODE_AUTOGROW|MODE_UNIQUENAME);

		if (CheckBatPickTmp(pstruXml) == 0)
		{
			DeleteXml(pstruXml);
		 	return NORMAL;
		}	
			
		PrintHexDebugLog("批采数值", szBatPickValue, nBatPickLen);	
		
		if (strcmp(szQryId, "FFFF" ) != 0)
		{	
			/*
			 *保存批采数据
			 */
			 SaveBatPickDat(pstruXml, szBatPickValue, nBatPickLen);

			/*
			 *继续发送查询包
			 */
			 nQryId++;			
		 	 memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		     struObject[0].MapID = 0x0606;
		     struObject[0].OC[0] = LOBYTE(nQryId);
		     struObject[0].OC[1] = HIBYTE(nQryId);
		     struObject[0].OC[2] = 0;
		     struObject[0].OC[3] = 0;
		     struObject[0].OL = 4;
    
	         int nObjCount=1; 
	         
	         memset(szMsgCont, 0, sizeof(szMsgCont));
	         struPack.pPack = szMsgCont;
	         struPack.Len = 0;
	         //获取当前的2G协议流水号
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
			 	   return -1;
			 }

	         //查询
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //站点等级为快:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);

			/*
			 *更新批采临时表
			 */
			 UpdateBatPickTmp(pstruXml);
			 
			 DeleteXml(pstruXml);
		 	 return NORMAL;							
			
		}
		else
		{
			DeleteBatPickTmp(pstruXml);
			PrintDebugLog(DBG_HERE,"QA=%s", struSendPackage.struHead.QA);
			UpdateEleFromCommByPacket(struSendPackage.struHead.QA, "1", "", "");
			//2015.08.18
			if (GetBatPickCount(pstruXml) < 0)
				UpdateEleQryLogErrorFromBatPick(struSendPackage.struHead.QA, szMapId);
			DeleteXml(pstruXml);
		 	return NORMAL;			
		}		
	}
	
	if (bObject00000606 == BOOLTRUE)
	{
		unsigned short nQryId;
		STR szQryId[5];
		int nMapId;
		STR szMapId[10];
		int nBatPickLen;
		STR szBatPickValue[100];
		

         /*
          *   创建xml对象
          */
         memset(pstruXml,0,sizeof(XMLSTRU));
         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
         InitPackageToXml(&struSendPackage, pstruXml);		
		
		sprintf(szTemp, "%d", struSendPackage.nNeId);
		InsertInXmlExt(pstruXml,"<omc>/<网元编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		sprintf(szTemp, "%d", QB);
		InsertInXmlExt(pstruXml,"<omc>/<通信包标识>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		
		nQryId=ReadWORD(struObject[0].OC);
		sprintf(szQryId, "%04X", nQryId);
		
		nMapId=ReadDWORD(struObject[0].OC+2);
		sprintf(szMapId, "%08X", nMapId);
		
		nBatPickLen = struObject[0].OL-6;
		memset(szBatPickValue, 0, sizeof(szBatPickValue));
		memcpy(szBatPickValue, struObject[0].OC+6, nBatPickLen);
		
		sprintf(szTemp, "%d", nQryId);
		InsertInXmlExt(pstruXml,"<omc>/<查询编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<批采对象>", szMapId, MODE_AUTOGROW|MODE_UNIQUENAME);
		sprintf(szTemp, "%d", nBatPickLen);
		InsertInXmlExt(pstruXml,"<omc>/<批采长度>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
//		InsertInXmlExt(pstruXml,"<omc>/<批采数值>", szBatPickValue, MODE_AUTOGROW|MODE_UNIQUENAME);

		if (CheckBatPickTmp(pstruXml) == 0)
		{
			DeleteXml(pstruXml);
		 	return NORMAL;
		}	
			
		PrintHexDebugLog("批采数值", szBatPickValue, nBatPickLen);	
		
		if (strcmp(szQryId, "FFFF" ) != 0)
		{	
			/*
			 *保存批采数据
			 */
			 SaveBatPickDat(pstruXml, szBatPickValue, nBatPickLen);

			/*
			 *继续发送查询包
			 */
			 nQryId++;			
		 	 memset(&struObject[0], 0, sizeof(OBJECTSTRU));
		     struObject[0].MapID = 0x00000606;
		     struObject[0].OC[0] = LOBYTE(nQryId);
		     struObject[0].OC[1] = HIBYTE(nQryId);
		     struObject[0].OC[2] = LOBYTE(LOWORD(nMapId));
		     struObject[0].OC[3] = HIBYTE(LOWORD(nMapId));
		     struObject[0].OC[4] = LOBYTE(HIWORD(nMapId));
		     struObject[0].OC[5] = HIBYTE(HIWORD(nMapId));
		     struObject[0].OL = 6;
    
	         int nObjCount=1; 
	         
	         memset(szMsgCont, 0, sizeof(szMsgCont));
	         struPack.pPack = szMsgCont;
	         struPack.Len = 0;
	         //获取当前的2G协议流水号
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_Das(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "回复主动上报打包错误\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
			 	   return -1;
			 }

	         //查询
	         InsertInXmlExt(pstruXml,"<omc>/<类型>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<通信方式>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //站点等级为快:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);

			/*
			 *更新批采临时表
			 */
			 UpdateBatPickTmp(pstruXml);
			 
			 DeleteXml(pstruXml);
		 	 return NORMAL;							
			
		}
		else
		{
			DeleteBatPickTmp(pstruXml);
			PrintDebugLog(DBG_HERE,"QA=%s\n", struSendPackage.struHead.QA);
			UpdateEleFromCommByPacket(struSendPackage.struHead.QA, "1", "", "");
			//2015.08.18
			if (GetBatPickCount(pstruXml) < 0)
				UpdateEleQryLogErrorFromBatPick(struSendPackage.struHead.QA, szMapId);
			DeleteXml(pstruXml);
		 	return NORMAL;			
		}		
	}
	/*
	 *历史时隙
	 */
	if (bObject0875 == BOOLTRUE)
	{
		int nBatPickLen;
		STR szBatPickValue[100];
		

         /*
          *   创建xml对象
          */
         memset(pstruXml,0,sizeof(XMLSTRU));
         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
         InitPackageToXml(&struSendPackage, pstruXml);		
		
		sprintf(szTemp, "%d", struSendPackage.nNeId);
		InsertInXmlExt(pstruXml,"<omc>/<网元编号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		sprintf(szTemp, "%d", QB);
		InsertInXmlExt(pstruXml,"<omc>/<通信包标识>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<监控对象>", szMapId, MODE_AUTOGROW|MODE_UNIQUENAME);
				
		nBatPickLen = struObject[0].OL-4;
		memset(szBatPickValue, 0, sizeof(szBatPickValue));
		memcpy(szBatPickValue, struObject[0].OC+4, nBatPickLen);
		
		sprintf(szTemp, "%d", nBatPickLen);
		InsertInXmlExt(pstruXml,"<omc>/<时隙长度>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);

		if (CheckShiXiPack(pstruXml) == 0)
		{
			DeleteXml(pstruXml);
		 	return NORMAL;
		}	
			
		PrintHexDebugLog("历史时隙", szBatPickValue, nBatPickLen);	
		
		//保存时隙数据
		SaveShiXiDat(pstruXml, szBatPickValue, nBatPickLen);
		DeleteShiXiTmp(pstruXml);
		
		PrintDebugLog(DBG_HERE,"QA=%s", struSendPackage.struHead.QA);
		UpdateEleFromCommByPacket(struSendPackage.struHead.QA, "1", "", "");
		DeleteXml(pstruXml);
		return NORMAL;							
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


RESULT ProcessCGSms(int nProctlType, PSTR pszUndecode, PSTR pszTelephone, PSTR pszNetCenterNum)
{
	int i, nNeId;
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
		
	//获取命令号
	int nCmdId;
	unsigned char szCmdId[2];
	unsigned char *pszCmdId;
	int Hivalue,Lovalue,temp;
	
	char szTemp[200];
	
	pszCmdId = pszUndecode+1+4*2;
	memcpy(szCmdId, pszCmdId, 2);
	
	temp = szCmdId[0];
	Hivalue=(temp>=48&&temp<=57)?temp-48:temp-55;
	temp = szCmdId[1];
	Lovalue=(temp>=48&&temp<=57)?temp-48:temp-55;
	nCmdId = (Hivalue*16+Lovalue);//反向转义

	CURSORSTRU  struCursor;
	char szSql[MAX_BUFFER_LEN];
	char szCmdObjList[MAX_OBJ_LEN];
	TBINDVARSTRU struBindVar;
	{
		memset(&struBindVar, 0, sizeof(struBindVar));
		struBindVar.struBind[0].nVarType = SQLT_INT;
		struBindVar.struBind[0].VarValue.nValueInt = nProctlType;
		struBindVar.nVarCount++;
		
		struBindVar.struBind[1].nVarType = SQLT_INT;
		struBindVar.struBind[1].VarValue.nValueInt = nCmdId;
		struBindVar.nVarCount++;
		
		sprintf(szSql, "select CDO_OBJECTS from NE_COMMANDOBJECTS where CDO_PROTOCOLTYPE = :v_0 and  CDO_COMMANDCODE = :v_1 ");
	}
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%d][%d]\n",szSql, nProctlType, nCmdId);
	if(BindSelectTableRecord(szSql,&struCursor, &struBindVar)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息为[%s]\n",szSql,GetSQLErrorMessage());
		return -1;
	}
	if(FetchCursor(&struCursor)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s][%d][%d]无记录\n",szSql, nProctlType, nCmdId);
		FreeCursor(&struCursor);
		return -1;
	}
	strcpy(szCmdObjList, GetTableFieldValue(&struCursor,"CDO_OBJECTS"));
	FreeCursor(&struCursor);
	
	//反向转义
	BYTEARRAY struPack;
	struPack.pPack = pszUndecode;
	struPack.Len = strlen(pszUndecode);
	if (ByteCombine(nProctlType, &struPack, szCmdObjList) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "转义处理出错\n");
		return EXCEPTION;
	}
	PrintHexDebugLog("反向转义", struPack.pPack, struPack.Len);
	
	//解析协议
	int res;
	CMDHEAD struCmdHead;
	UBYTE ubCmdBody[200];
	
	OMCOBJECT struOmcObj[MAX_OBJ_NUM];
	int nOmcObjNum;
	
	res = Decode_Gsm(struPack.pPack, struPack.Len, &struCmdHead, ubCmdBody);
	if (res != 0)
	{
		PrintErrorLog(DBG_HERE, "解析协议出错,返回结果[%d]\n", res);
		return EXCEPTION;
	}
	
	PrintDebugLog(DBG_HERE, "ubProVer[%d]ubPacNum[%d]ubPacIndex[%d]ubDevType[%d]ubCmdId[%d]udwRepId[%u]ubDevId[%d]ubAnsFlag[%d]ubCmdBodyLen[%d]\n",
		struCmdHead.ubProVer,struCmdHead.ubPacNum,struCmdHead.ubPacIndex,struCmdHead.ubDevType,struCmdHead.ubCmdId,struCmdHead.udwRepId,struCmdHead.ubDevId,struCmdHead.ubAnsFlag,struCmdHead.ubCmdBodyLen
		);
	
	res = DecodeCmdBodyFromCmdId(nProctlType, szCmdObjList, &struCmdHead, ubCmdBody, struOmcObj, &nOmcObjNum);	
	if (res != 0)
	{
		PrintErrorLog(DBG_HERE, "解析协议出错,返回结果[%d]\n", res);
		return EXCEPTION;
	}
	
	//初始化分解对象
	SENDPACKAGE struSendPackage;
	memset(&struSendPackage, 0, sizeof(SENDPACKAGE));
	struSendPackage.struRepeater.nRepeaterId=struCmdHead.udwRepId;
    struSendPackage.struRepeater.nDeviceId=struCmdHead.ubDevId;
	memcpy((char*)struSendPackage.struRepeater.szNetCenter, pszNetCenterNum,strlen(pszNetCenterNum));//服务号码
	struSendPackage.struRepeater.nCommType=M2G_SMS_TYPE;//设备通讯方式 
	memcpy((char*)struSendPackage.struRepeater.szTelephoneNum, pszTelephone, strlen(pszTelephone));//上报设备手机号
//	struSendPackage.struHead.nObjectCount= 0;//对象数
	struSendPackage.struHead.nObjectCount= nOmcObjNum;//对象数
	struSendPackage.struHead.nProtocolType=PROTOCOL_GSM;
	memcpy((char*)struSendPackage.struHead.QA,"Sms123456789", 12);//QA流水号
	
	char szProperty[2000];
	char szContent[2000];
	memset(szProperty, 0, sizeof(szProperty));
	memset(szContent, 0, sizeof(szContent));
	for(i=0; i< nOmcObjNum; i++)
	{
		STR szObjId[10];
		STR szObjVal[MAX_VAL_LEN];
		
		strcpy(szObjId, struOmcObj[i].szObjId);
		strcpy(szObjVal, struOmcObj[i].szObjVal);
		
		//特殊obj		
		UWORD uwSoftVer;
		if (strcmp(szObjId, "000A") == 0)//版本号
		{
			uwSoftVer = atoi(szObjVal);
			sprintf(szObjVal, "%d.%d", LOWBYTE(uwSoftVer), HIGBYTE(uwSoftVer));
		}
		
		//放入SendPackage	
	    struSendPackage.struMapObjList[i].cErrorId = '0';
	    
	    strcpy(struSendPackage.struMapObjList[i].szMapId, szObjId);
	    strcpy(struSendPackage.struMapObjList[i].szMapData, szObjVal);
	    
	    sprintf(szProperty, "%s%s,", szProperty, struSendPackage.struMapObjList[i].szMapId);
	    sprintf(szContent, "%s%s,", szContent, struSendPackage.struMapObjList[i].szMapData);
	}
	PrintDebugLog(DBG_HERE, "Property[%s]\n", szProperty);
	PrintDebugLog(DBG_HERE, "Content[%s]\n", szContent);
	
	//主动上报处理
	if (nCmdId == 0x10 || //C,HeiBei,XianChuang,Sunwave,WiLiKe
		nCmdId == 0x40 || nCmdId == 0x41)//G
	{
		//回复报文	     
		CMDHEAD struRespCmdHead;
		UBYTE ubOutPack[200];
		int nOutLen;
		
		
		switch (nProctlType)
		{
			case PROTOCOL_GSM:
				struRespCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_CDMA:
				struRespCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_HEIBEI:
				struRespCmdHead.ubProVer = 0x03;
				break;	
			case PROTOCOL_HEIBEI2:
				struRespCmdHead.ubProVer = 0x03;
				break;	
			case PROTOCOL_XC_CP:
				struRespCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_SUNWAVE:
				struRespCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_WLK:
				struRespCmdHead.ubProVer = 0x01;
				break;	
			default:
				break;
		}
		
		struRespCmdHead.ubPacNum = 1;
		struRespCmdHead.ubPacIndex = 1;
		struRespCmdHead.ubDevType = struCmdHead.ubDevType;
		struRespCmdHead.ubCmdId = nCmdId;
		struRespCmdHead.udwRepId = struCmdHead.udwRepId;
		struRespCmdHead.ubDevId = struCmdHead.ubDevId;
		struRespCmdHead.ubAnsFlag = 0;
		struRespCmdHead.ubCmdBodyLen = 0;
		
		Encode_GSM(&struRespCmdHead, NULL, ubOutPack, &nOutLen);
		
		struPack.pPack = ubOutPack;
		struPack.Len = nOutLen;
		if (ByteSplit(nProctlType, &struPack, NULL) != NORMAL)//字节拆分
		{
			PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
			return -1;
		}	
		
	     memset(pstruXml,0,sizeof(XMLSTRU));
	     CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL); //创建xml对象
	     strcpy(struSendPackage.struHead.QA, GetTypeNumber("Sms"));//取唯一流水号
	     InitPackageToXml(&struSendPackage, pstruXml);//初始化xml
	     InsertInXmlExt(pstruXml,"<omc>/<类型>",  "00", MODE_AUTOGROW|MODE_UNIQUENAME);//自动回复类型
	     InsertInXmlExt(pstruXml,"<omc>/<序列号>","0",MODE_AUTOGROW|MODE_UNIQUENAME);//通信服务器删除消息无需通信包标识号
	     InsertInXmlExt(pstruXml,"<omc>/<消息内容>",(char *)ubOutPack,MODE_AUTOGROW|MODE_UNIQUENAME);
	     InsertInXmlExt(pstruXml,"<omc>/<站点电话>", pszTelephone, MODE_AUTOGROW|MODE_UNIQUENAME);
	     InsertInXmlExt(pstruXml,"<omc>/<服务号码>", pszNetCenterNum, MODE_AUTOGROW|MODE_UNIQUENAME);
	     InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     
         SaveToMsgQueue(pstruXml);		
		
		 //告警处理
		 BOOL bAlarmRestore = BOOLFALSE;
		 STR szFrequentAlarmList[1000]; //频繁告警列表
		 STR szAlarmRestoreList[1000]; //告警告警列表
		 
		 char szNeName[50], szAlarmObjList[1000], szAlarmTime[20];
		 
		 bufclr(szFrequentAlarmList);
		 bufclr(szAlarmRestoreList);

	     if (GetAlarmObjList2(struCmdHead.udwRepId, struCmdHead.ubDevId, pszTelephone, &nNeId, szNeName, szAlarmObjList)!= NORMAL)
	     {
	         DeleteXml(pstruXml);
	         PrintErrorLog(DBG_HERE, "设备号[%d][%d]不存在记录\n", struCmdHead.udwRepId,  struCmdHead.ubDevId);
	         return EXCEPTION;
	     }
	     InsertInXmlExt(pstruXml,"<omc>/<告警列表>",szAlarmObjList,MODE_AUTOGROW|MODE_UNIQUENAME);
	     sprintf(szTemp, "%d", nNeId);
     	 InsertInXmlExt(pstruXml,"<omc>/<网元编号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
         struSendPackage.nNeId = nNeId;
         
         bufclr(szAlarmTime);
		 for(i=0; i< nOmcObjNum; i++)
         {
             if (strstr(szAlarmObjList, struOmcObj[i].szObjId) == NULL)
             {
                 continue;
             }
             
             sprintf(szAlarmTime, "%s", GetSysDateTime());
			 InsertInXmlExt(pstruXml,"<omc>/<告警时间>",szAlarmTime,MODE_AUTOGROW|MODE_UNIQUENAME);
			 InsertInXmlExt(pstruXml,"<omc>/<告警对象>",struOmcObj[i].szObjId, MODE_AUTOGROW|MODE_UNIQUENAME);
             
             if (strcmp(struOmcObj[i].szObjVal, "1") == 0)//告警处理
             {
             	
				if (DealNewAlarm(pstruXml) == NORMAL)
					strcat(szFrequentAlarmList, struAlarmPara.szAlarmName);//用于频繁上报
				//频繁告警
				SaveToMaintainLog("新告警", "",  &struSendPackage);
				InitAlarmPara(nNeId);
				//AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarm");//是否频繁告警

             }
             else if (strcmp(struOmcObj[i].szObjVal, "0") == 0)//只有站点确实处于告警状态才恢复
             {
                 sprintf(szTemp, "%s:1",  struOmcObj[i].szObjId);
                 if (strstr(szAlarmObjList, szTemp) != NULL)
                 {
                 	 bAlarmRestore = BOOLTRUE;
                 	 
                 	 if (AlarmComeback(pstruXml) == NORMAL)
                 		 strcat(szAlarmRestoreList, struAlarmPara.szAlarmName);//用于告警恢复前转
             
             		 SaveToMaintainLog("告警恢复", "",  &struSendPackage);
             		 InitAlarmPara(nNeId);
             		 //AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarmClear");//频繁告警清除
             		 if (strlen(szAlarmRestoreList) > 0)
                 		 TransferAlarm();
                 }
                 
             }            
         }
         
         DeleteXml(pstruXml);
		 return NORMAL;
		
	}	

	
	////查询设置处理

	//获取通信标识号
	int nQB;

	sprintf(szTemp, "%lu%d%d", struCmdHead.udwRepId, struCmdHead.ubDevId, nCmdId);
	sprintf(szSql,"select comm_netflag from comm_packtoken where comm_repcmd = '%s' order by comm_netflag desc ", szTemp);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息为[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息为[%s]\n",szSql,GetSQLErrorMessage());
		FreeCursor(&struCursor);
		return -1;
	}
	nQB = atoi(GetTableFieldValue(&struCursor,"comm_netflag"));
	FreeCursor(&struCursor);
	
	//删除临时表通信标识号
	sprintf(szSql,"delete from comm_packtoken where comm_netflag= %d ",nQB);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();

	//根据QB+RepeaterId获取原来信息并删除对应流水号记录
	if(GetSendPackageInfo(nQB, struCmdHead.udwRepId, &struSendPackage) != NORMAL)
    {
		PrintErrorLog(DBG_HERE, "根据QB+RepeaterId获取QA错误\n");
		return EXCEPTION; 
	}

	
	//命令来处理
	switch(nCmdId)
	{
		//C,HeiBei,XianChuang,Sunwave,WiLiKe
		case 0x20:              
		case 0X21:
		//WiLiKe	
		case 0XE0:
				
		case 0XE1:
		case 0XE2:
		//HeiBei,XianChuang	
		case 0XE3:
		case 0XE4:
		//G	
		case 0x50:	
		case 0x51:	
		case 0x52:	
		case 0x53:	
		case 0x54:	
		case 0x55:										
			DecodeQryElementParam((SENDPACKAGE *)&struSendPackage);//解析查询
			break;
		//C,HeiBei,XianChuang,Sunwave,WiLiKe	
		case 0x30:              
		case 0X31:
		case 0X32:
		case 0X33:
		case 0X34:
		case 0X35:
		//C,HeiBei,XianChuang,Sunwave		
		case 0x36:
		//HeiBei	              
		case 0XF0:
		//HeiBei, XianChuang	
		case 0XF3:
			
		case 0XF4:	
		//G
		case 0X60:	
		case 0X61:
		case 0X62:	
		case 0X63:	
		case 0X64:	
		case 0X65:	
		case 0X66:	
		case 0X67:	
		case 0X68:
		//WiLiKe	
		case 0XDF:		
															
			DecodeSetElementParam((SENDPACKAGE *)&struSendPackage);//解析设置	
			break;
		default:
			break;
	}		
	
	return NORMAL;
}

/* 
 * 解析2G协议，并做相应处理
 * pszUndecode 短信内容
 * szTelephone 手机号码
 * szNetCenterNum 特服号
 */
RESULT DecodeAndProcessSms(PSTR pszUndecode,PSTR pszTelephone,PSTR pszNetCenterNum)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	int nProctlType;
	
	PrintDebugLog(DBG_HERE, "处理报文[%s][%s]\n", pszTelephone, pszUndecode);
	
	TBINDVARSTRU struBindVar;
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_STR; 
	strcpy(struBindVar.struBind[0].VarValue.szValueChar, pszTelephone);
	struBindVar.nVarCount++;
	
	sprintf(szSql,"select ne_ProtocoltypeId from ne_Element where ne_NeTelNum = :v_0");
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s]\n",szSql, pszTelephone);
	if(BindSelectTableRecord(szSql,&struCursor, &struBindVar) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if (FetchCursor(&struCursor) == NORMAL)
	{
		nProctlType = atoi(GetTableFieldValue(&struCursor, "ne_ProtocoltypeId"));
	}
	else
	{
		PrintDebugLog(DBG_HERE,"根据号码没找到主记录,默认按2G协议处理!\n");
		nProctlType = PROTOCOL_2G;
	}	
	FreeCursor(&struCursor);	

	if (nProctlType == PROTOCOL_2G ||
		nProctlType == PROTOCOL_DAS ||
		nProctlType == PROTOCOL_JINXIN_DAS)
	{	
		Process2GSms(pszUndecode, pszTelephone, pszNetCenterNum, nProctlType);
	}
	else if (nProctlType == PROTOCOL_GSM || 
		nProctlType == PROTOCOL_CDMA ||
		nProctlType == PROTOCOL_HEIBEI ||
		nProctlType == PROTOCOL_HEIBEI2 ||
		nProctlType == PROTOCOL_XC_CP ||
		nProctlType == PROTOCOL_SUNWAVE ||
		nProctlType == PROTOCOL_WLK)
	{	
		ProcessCGSms(nProctlType, pszUndecode, pszTelephone, pszNetCenterNum);
	}

	
	return NORMAL;
}



/* 
 * 解析TCP/IP协议，并做相应处理
 * pszUndecode 短信内容
 * szTelephone 手机号码
 * szNetCenterNum 特服号
 */
RESULT DecodeAndProcessGprs(PSTR pszUndecode, INT nLen)
{
	int nProctlType, nField;
	PSTR pszSepStr[3];
	char szReqBuffer[MAX_BUFFER_LEN];
	
	strcpy(szReqBuffer, pszUndecode);

	nProctlType = PROTOCOL_2G;//默认
	
	if (nProctlType == PROTOCOL_2G)
	{	//nField=SeperateString(pszUndecode, ',', pszSepStr, 2);
		//PrintDebugLog(DBG_HERE, "nField [%d]\n", nField);
		//if (nField==2)
		//	Process2GGprs(atoi(pszSepStr[0]), pszSepStr[1], strlen(pszSepStr[1]));
		//else
			Process2GGprs(0, szReqBuffer, nLen);
	}
	else if (nProctlType == PROTOCOL_GSM || 
		nProctlType == PROTOCOL_CDMA ||
		nProctlType == PROTOCOL_HEIBEI ||
		nProctlType == PROTOCOL_HEIBEI2 ||
		nProctlType == PROTOCOL_XC_CP ||
		nProctlType == PROTOCOL_SUNWAVE ||
		nProctlType == PROTOCOL_WLK)
	{
		//ProcessCGGprs(pszUndecode, nLen);
		PrintErrorLog(DBG_HERE, "CG网通信不支持GPRS方式\n");
	}
	
	return NORMAL;
}
	     
	     
	     
