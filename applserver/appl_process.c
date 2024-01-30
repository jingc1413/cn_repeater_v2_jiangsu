/*
 * ����: ����ϵͳӦ�÷�����(�������󲿷�)
 *
 * �޸ļ�¼:
 * ��־�� -		2008-8-28 ����
 */
 
#include <ebdgdl.h>
#include <omcpublic.h>
#include <applserver.h>
#include <mobile2g.h>

extern ALARMPARAMETER struAlarmPara;


/**
 *	����XML������
 *  �ú�������ͨѶ�ķ�ʽΪ�����а�ͷ��XML����
 *  �ұ��Ĵ����ŵ�·��Ϊ<omc>/<transcd>
 */
int RecvCa8801ReqPacket(int sockfd, char *psCaReqBuffer, char *psTradeNo)
{
	int iRet;
	XMLSTRU  CaReqXml;							/* �����������ܱ���XML */
	PXMLSTRU pCaReqXml = &CaReqXml;		
	char sTempstr[64];
	 
	//SetSyncString("\x03\x00",2);
	
	iRet = RecvSocketWithSync(sockfd,psCaReqBuffer, MAX_BUFFER_LEN, 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "�������������Ĵ��� ������Ϊ[%d]\n", iRet);
		close(sockfd);
		return -1;
	}
	
	memset(pCaReqXml,0,sizeof(XMLSTRU));
	if(ImportXml(pCaReqXml, FALSE, psCaReqBuffer) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"�����������ĵ�ΪXML�ṹ����.���ĸ�ʽ�Ƿ�\n");
		close(sockfd);
		return -1;
	}
	
	if(DemandInXmlExt(pCaReqXml, "<omc>/<packcd>", sTempstr, sizeof(sTempstr)) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "DemandInXmlExt ��ȡ�����������Ľ��״���ʧ��, ȡ����[%s]��ֵ\n",
			"<omc>/<packcd>");
		close(sockfd);
		return -1;
	}
	DeleteXml(pCaReqXml);
	PrintTransLog(DBG_HERE, "�յ�����������Ϊ [%s]\n",  psCaReqBuffer);
	strcpy(psTradeNo, sTempstr);
	return iRet;
}


/*
 * ����XMLӦ����
 */
int SendCa8801RespPacket(int sockfd, char *sCaRespBuffer, int iSendBufferLen)
{
	int iRet;	
	
	PrintTransLog(DBG_HERE, "��������Ӧ����Ϊ [%s]\n",  sCaRespBuffer);	
	iRet = SendSocketWithSync(sockfd,sCaRespBuffer, iSendBufferLen, 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "AsSendCa_8801��������Ӧ���Ĵ��� ������Ϊ[%d]\n", iRet);
		return -1;
	}
	return 0;
}

/*
 * ���ձ���
 * �ú�������ͨѶ�ķ�ʽΪ������4λ���ĳ��ȵİ�ͷ�ı���
 */
int RecvCa8802ReqPacket(int sockfd, char *psCaReqBuffer, char *psTradeNo)
{
	int iRet;
	char sBufLen[64];
	
	memset( sBufLen, 0, sizeof( sBufLen ));
	iRet = RecvSocketNoSync(sockfd, sBufLen, 4, 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "�������������Ĵ��� ������Ϊ[%d]\n", iRet);
		close(sockfd);
		return -1;
	}

	TrimAllSpace( sBufLen );
	PrintDebugLog(DBG_HERE, "��������������ǰ��λ����Ϊ[%s]\n", sBufLen);
	if ( atoi( sBufLen ) < 1 )
	{
		PrintErrorLog(DBG_HERE, "�������������ĳ��ȴ��� ����Ϊ[%s]\n", sBufLen);
		close(sockfd);
		return -1;		
	}
	
	iRet = RecvSocketNoSync(sockfd, psCaReqBuffer, atoi( sBufLen ), 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "�������������Ĵ��� ������Ϊ[%d]\n", iRet);
		close(sockfd);
		return -1;
	}
	memcpy(psTradeNo,psCaReqBuffer,4);
	PrintTransLog(DBG_HERE,"�յ�����������Ϊ[%s]\n", psCaReqBuffer);
	
	return iRet;
}

/*
 * ����Ӧ����
 * �ú�������ͨѶ�ķ�ʽΪ������4λ���ĳ��ȵİ�ͷ�ı���
 */
int SendCa8802RespPacket(int sockfd, char *sCaRespBuffer, int iSendBufferLen)
{
	int iRet;
	char sBufLen[64];
	
	if ( strlen( sCaRespBuffer ) < 4 )
	{
		PrintErrorLog(DBG_HERE, "���ر��ĳ��Ȳ���ȷ[%s]", sCaRespBuffer);
		close(sockfd);		
		return -1;
	}
	//sleep(1);
	
	snprintf(sBufLen, sizeof(sBufLen), "%4d%s", iSendBufferLen, sCaRespBuffer);
	
	iRet = SendSocketNoSync(sockfd,sBufLen, 4+iSendBufferLen, 30);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "AsSendCa_8802��������Ӧ���ı���ͷ���� ������Ϊ[%d]\n", iRet);
		close(sockfd);
		return -1;
	}
	PrintTransLog(DBG_HERE, "��������Ӧ����[%s]\n", sCaRespBuffer);

	return 0;
}

/**
 *	����������
 *  �ú�������ͨѶ�ķ�ʽΪ�����а�ͷ�ı���
 */
int RecvCa8803ReqPacket(int sockfd, char *psCaReqBuffer, char *psTradeNo)
{
	int iRet;
	char sTempBuffer[MAX_BUFFER_LEN];
	
	memset(sTempBuffer, 0, sizeof(sTempBuffer));
	iRet = RecvSocketWithSync(sockfd, sTempBuffer, MAX_BUFFER_LEN, 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "�������������Ĵ��� ������Ϊ[%d]\n", iRet);
		close(sockfd);
		return -1;
	}
	//PrintHexTransLog(DBG_HERE, "�յ�����������Ϊ [%s]\n",  sTempBuffer);
	memcpy(psTradeNo, sTempBuffer, 4);
	memcpy(psCaReqBuffer, sTempBuffer+4, iRet-4);
	return iRet-4;
}

/*
 * ����Ӧ����
 */
int SendCa8803RespPacket(int sockfd, char *sCaRespBuffer, int iSendBufferLen)
{
	int iRet;	
	
	//PrintTransLog(DBG_HERE, "��������Ӧ����Ϊ [%s]\n",  sCaRespBuffer);	
	iRet = SendSocketWithSync(sockfd,sCaRespBuffer, iSendBufferLen, 60);
	if(iRet < 0)
	{
		PrintErrorLog(DBG_HERE, "AsSendCa_8801��������Ӧ���Ĵ��� ������Ϊ[%d]\n", iRet);
		return -1;
	}
	return 0;
}



/* 
 * �������������� 
 */
RESULT RecvCaReqPacket(INT nSock, PSTR pszCaReqBuffer, PSTR pszPackCd)
{
	INT nRet=0;
	
	struct sockaddr_in struListenSockAddr;
	UINT nListenSockLen = sizeof(struListenSockAddr);

	
	/* 
	 * ��ȡ����������ʶ˿�
	 */
#ifdef SYSTEM_AIX
	if(getsockname(nSock, (struct sockaddr *)&struListenSockAddr, \
		(unsigned long *) &nListenSockLen) != 0)
#else
	if(getsockname(nSock, (struct sockaddr *)&struListenSockAddr, \
		&nListenSockLen) != 0)
#endif
	{
		PrintErrorLog(DBG_HERE,"��ȡSocket��Ϣʧ��[%s]\n",strerror(errno));
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
		PrintErrorLog(DBG_HERE, "���ý������������Ĵ���\n");
		return EXCEPTION;
	}

	return nRet;
}

/* 
 * ��������Ӧ���� 
 */
INT SendCaRespPacket(INT nSock, PSTR pszCaRespBuffer, INT nCommBufferLen)
{
	INT nRet=0; 
	
	struct sockaddr_in struListenSockAddr;
	UINT nListenSockLen = sizeof(struListenSockAddr);

	/* 
	 * ��ȡ����������ʶ˿�
	 */
#ifdef SYSTEM_AIX
	if(getsockname(nSock, (struct sockaddr *)&struListenSockAddr, \
		(unsigned long *)&nListenSockLen) != 0)
#else
	if(getsockname(nSock, (struct sockaddr *)&struListenSockAddr, \
		&nListenSockLen) != 0)
#endif
	{
		PrintErrorLog(DBG_HERE,"��ȡSocket��Ϣʧ��[%s]\n",strerror(errno));
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
		PrintErrorLog(DBG_HERE, "���÷�������Ӧ���Ĵ���\n");
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
    InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
    
    sprintf(szTemp, "%u", pstruSendPackage->struRepeater.nRepeaterId);
	InsertInXmlExt(pstruXml,"<omc>/<վ����>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->struRepeater.nDeviceId);
	InsertInXmlExt(pstruXml,"<omc>/<�豸���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%s", pstruSendPackage->struRepeater.szTelephoneNum);
	InsertInXmlExt(pstruXml,"<omc>/<վ��绰>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->struRepeater.nPort);
	InsertInXmlExt(pstruXml,"<omc>/<�˿ں�>",   szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%s", pstruSendPackage->struRepeater.szIP);
	InsertInXmlExt(pstruXml,"<omc>/<վ��IP>",   szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->struRepeater.nProtocolDeviceType);
	InsertInXmlExt(pstruXml,"<omc>/<�豸����>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%s", pstruSendPackage->struRepeater.szSpecialCode);
	InsertInXmlExt(pstruXml,"<omc>/<������ʶ>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%s", pstruSendPackage->struRepeater.szReserve);
	InsertInXmlExt(pstruXml,"<omc>/<�豸�ͺ�>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%s", pstruSendPackage->struRepeater.szNetCenter);
	InsertInXmlExt(pstruXml,"<omc>/<�������>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->struHead.nProtocolType);
	InsertInXmlExt(pstruXml,"<omc>/<Э������>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->struHead.nCommandCode);
	InsertInXmlExt(pstruXml,"<omc>/<�����>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	sprintf(szTemp, "%d", pstruSendPackage->nNeId);
	InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	InsertInXmlExt(pstruXml,"<omc>/<��ˮ��>", pstruSendPackage->struHead.QA, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	return NORMAL;
}

RESULT SendToApplQrySet(SENDPACKAGE *pstruNeInfo)
{
    STR szTransBuffer[1000]; /*	���ݽ�������	*/
    STR szTemp[1000];		 /*	���ݽ�������	*/
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
	 *	��������
	 */
	if((nConnectFd = CreateConnectSocket("127.0.0.1", 8802, 10)) < 0)
	{
		PrintErrorLog(DBG_HERE, "ͬӦ�÷������[127.0.0.1][8802]�������Ӵ���,��ȷ��applserv�Ѿ�����\n" );
		return EXCEPTION;
	}
	
	memset(szTransBuffer, 0, sizeof(szTransBuffer));
	snprintf(szTransBuffer, sizeof(szTransBuffer), "%4d%s", strlen(szTemp), szTemp);
	if(SendSocketNoSync(nConnectFd,szTransBuffer, 4+strlen(szTemp), 10) < 0)
	{
		PrintErrorLog(DBG_HERE, "���͵�����Ӧ�÷���������Ĵ���\n");
		close(nConnectFd);
		return EXCEPTION;
	}
	
	/*
	 *	���շ�������Ӧ��
	 */
	memset(szTransBuffer, 0, sizeof(szTransBuffer));
	if(RecvSocketNoSync(nConnectFd, szTransBuffer, sizeof(szTransBuffer), 30) < 0)
	{
		PrintErrorLog(DBG_HERE, "��������Ӧ�÷����Ӧ���Ĵ���\n");
		close(nConnectFd);
		return EXCEPTION;
	}
	
	close(nConnectFd);

	if (memcmp(szTransBuffer+4, "0000", 4) !=0)
	{
	    PrintErrorLog(DBG_HERE,"���յ�Ӧ�÷���Ӧ����ʧ�ܣ�������[%s]\n", szTransBuffer);
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
		PrintErrorLog(DBG_HERE,"[%s]û��ʼ�ָ��ַ�,���ķǷ�!!!!!!\n", pszUndecode);
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
		PrintErrorLog(DBG_HERE,"û�����ָ��ַ�,���ķǷ�!!!!!!\n");
		return EXCEPTION;
	}
	nLen = nEnd - nStart +1;
   	bufclr(szReqBuffer);
   	memcpy(szReqBuffer, pszUndecode+nStart, nLen);
   	
	struPack.pPack = szReqBuffer;
	struPack.Len = nLen;
	//struPack.pPack = pszUndecode;
	//struPack.Len = strlen(pszUndecode);
	//����2GЭ��

    if (Decode_2G(M2G_SMS, &struPack, &Decodeout, struObject) != NORMAL)
    {
    	PrintErrorLog(DBG_HERE, "�����豸���ݴ���\n");
		return EXCEPTION; 
    }

	
    PrintDebugLog(DBG_HERE, "����2GЭ��ɹ�,Э������[%02d],�����ʶ[%d],�������[%d],վ����[%u],�豸���[%X],�����ʶ[%d],������[%d]\n",
        Decodeout.NPLayer.APID,  Decodeout.MAPLayer.CommandFalg, Decodeout.APLayer.ErrorCode, 
        Decodeout.NPLayer.structRepeater.RepeaterId, Decodeout.NPLayer.structRepeater.DeviceId,
        Decodeout.NPLayer.NetFlag, Decodeout.MAPLayer.ObjCount);
	
	//����QB+RepeaterId��ȡQA
	int QB=Decodeout.NPLayer.NetFlag;//2GЭ��ͨѶ��ʾ��ˮ��
	UINT nRepeaterId=Decodeout.NPLayer.structRepeater.RepeaterId;
	int nDeviceId = Decodeout.NPLayer.structRepeater.DeviceId;
	
	//��ʼ���ֽ����
	memset(&struSendPackage, 0, sizeof(SENDPACKAGE));
	struSendPackage.struRepeater.nRepeaterId=Decodeout.NPLayer.structRepeater.RepeaterId;
    struSendPackage.struRepeater.nDeviceId=Decodeout.NPLayer.structRepeater.DeviceId;
	//�������
	memcpy((char*)struSendPackage.struRepeater.szNetCenter, pszNetCenterNum,strlen(pszNetCenterNum));
	//�豸ͨѶ��ʽ 
	struSendPackage.struRepeater.nCommType=M2G_SMS_TYPE;
	//�ϱ��豸�ֻ���
	memcpy((char*)struSendPackage.struRepeater.szTelephoneNum, pszTelephone, strlen(pszTelephone));
	//������
	struSendPackage.struHead.nObjectCount= Decodeout.MAPLayer.ObjCount;
	//�ϱ���ʽ
	struSendPackage.struHead.nCommandCode=COMMAND_UP;
	
	if (GetNeInfo(&struSendPackage) != NORMAL)
			struSendPackage.struHead.nProtocolType= PROTOCOL_2G;
	
	if (Decodeout.NPLayer.APID == 0x03)
		struSendPackage.struHead.nProtocolType= PROTOCOL_DAS;

	//QA��ˮ��
	memcpy((char*)struSendPackage.struHead.QA,"Sms123456789", 12);
	strncpy(struSendPackage.szRecvMsg, pszUndecode, strlen(pszUndecode));
	
	//��ʼ����Ϊ��ض����������
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
		
		//����ǲ�ѯ����б�0009
		if (strcmp(szMapId, "0009") == 0 && Decodeout.MAPLayer.CommandFalg == 2)
		    bObject0009 = BOOLTRUE;
		else if ( strcmp(szMapId, "00000009") == 0)
			bObject00000009 = BOOLTRUE;
		
		if (strcmp(szMapId, "0141") == 0 || strcmp(szMapId, "00000141") == 0)
		{
		    nCommUpType = struObject[i].OC[0];
		    if (struObject[i].OC[0] == 0xCA)//�ж��Ƿ�gprs�����ϱ�
		        bObject0141 = BOOLTRUE;
		}
		if(strcmp(szMapId, "074B") == 0)//���1.0���Ŷ�ʱ�ϱ�
		{
			memcpy(szMapDataSet, struObject[i].OC, MAX_OBJCONTEXT_LEN);
		}
		if (strcmp(szMapId, "0001") == 0 )//��չ�����ϱ�
			bObject0001 = BOOLTRUE;
		if (strcmp(szMapId, "0004") == 0 )
			ReplaceStr(struSendPackage.struMapObjList[i].szMapData, ",", "-");
	}

    
    //����������ϱ�,�����ϱ�
	if(Decodeout.MAPLayer.CommandFalg == 1)
	{
	     PrintDebugLog(DBG_HERE,"�ϱ�����%X SMS��ʼ===========\n", nCommUpType);
	     if (nCommUpType == 0x20 )
			return NORMAL;
		 /*
		  * �ظ������ϱ���ʼ===============================================
          * ����豸Э��,���ɶ�������
          */
	     int nUpQB=Decodeout.NPLayer.NetFlag;
	     int nObjCount=0; //�ظ�����Ϊ0
	     
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
	               PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		           return EXCEPTION; 
	          }
	     }
	     else
	     {
	     	  if (Decodeout.NPLayer.APID == 0x03)
	     	  {
	     	  	  if (Encode_Das(M2G_SMS, RELAPSECOMMAND, nUpQB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
		          {
		              PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
			          return EXCEPTION; 
		          }
	     	  }
	     	  else
	     	  {
		          if (Encode_2G(M2G_SMS, RELAPSECOMMAND, nUpQB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
		          {
		              PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
			          return EXCEPTION; 
		          }
		      }
	     }
	     /*
	      *   ����xml����
	      */
	     memset(pstruXml,0,sizeof(XMLSTRU));
	     CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	     
	     //ȡΨһ��ˮ��
	     strcpy(struSendPackage.struHead.QA, GetTypeNumber("Sms"));
	     //��ʼ��xml
	     InitPackageToXml(&struSendPackage, pstruXml);
	     
	     //�Զ��ظ�����
	     InsertInXmlExt(pstruXml,"<omc>/<����>",  "00", MODE_AUTOGROW|MODE_UNIQUENAME);
	     
	     InsertInXmlExt(pstruXml,"<omc>/<վ��绰>", pszTelephone, MODE_AUTOGROW|MODE_UNIQUENAME);
	     sprintf(szTemp, "%d", nUpQB);
	     InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
         
	     InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	     InsertInXmlExt(pstruXml,"<omc>/<�������>", pszNetCenterNum, MODE_AUTOGROW|MODE_UNIQUENAME);
         //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	     InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     
         SaveToMsgQueue(pstruXml);
         /*
          *�ظ������ϱ�����===============================================
          */
		 /*
		 if (SaveToRecordDeliveCrc(Decodeout.NPLayer.structRepeater.RepeaterId, Decodeout.NPLayer.structRepeater.DeviceId,
        		Decodeout.NPLayer.NetFlag, 0) != NORMAL)
		 {
			return EXCEPTION;
		 }
		 */
	
		 BOOL bAlarmRestore = BOOLFALSE;
		 STR szFrequentAlarmList[1000]; //Ƶ���澯�б�
		 STR szAlarmRestoreList[1000]; //�澯�澯�б�
		 INT nAlarmTimes=0;
		 
		 bufclr(szFrequentAlarmList);
		 bufclr(szAlarmRestoreList);
		 if (nCommUpType == DEVICE_ALARM) //�豸�澯
		 {
		     /*
		      *  ȡ�澯�б�,��ȡ��ԪNeId����
		      */
		     if (GetAlarmObjList2(nRepeaterId, nDeviceId, pszTelephone, &nNeId, szNeName, szAlarmObjList)!= NORMAL)
		     {
		         DeleteXml(pstruXml);
		         PrintErrorLog(DBG_HERE, "�豸��[%u][%d]�����ڼ�¼\n", nRepeaterId, nDeviceId);
		         return EXCEPTION;
		     }
		     InsertInXmlExt(pstruXml,"<omc>/<�澯�б�>",szAlarmObjList,MODE_AUTOGROW|MODE_UNIQUENAME);
		     sprintf(szTemp, "%d", nNeId);
	     	 InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
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
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<�澯ʱ��>",szAlarmTime,MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	             	 if (strcmp(struSendPackage.struMapObjList[i].szMapId, "050B") == 0 ||
	             	 	 strcmp(struSendPackage.struMapObjList[i].szMapId, "08B3") == 0)   //TD
	             	 {
	             	 	 PrintDebugLog(DBG_HERE,"��ƽǿ��[%s]\n",struSendPackage.struMapObjList[i].szMapData);
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<��ƽǿ��>",struSendPackage.struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	             	 if (strcmp(struSendPackage.struMapObjList[i].szMapId, "050C") == 0)
	             	 {
	             	 	PrintDebugLog(DBG_HERE,"����С��[%s]\n",struSendPackage.struMapObjList[i].szMapData);
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<����С��>",struSendPackage.struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	                 continue;
	             }
	             
	             if (strcmp(struSendPackage.struMapObjList[i].szMapData, "1") == 0)//�澯����
	             {
	             	 if (nAlarmTimes++ >= 1)
	             	 {
	             	 	 if (DealNewAlarm(pstruXml) == NORMAL)
                      	 strcat(szFrequentAlarmList, struAlarmPara.szAlarmName);//����Ƶ���ϱ�
	             	 	 //Ƶ���澯
	             	 	 SaveToMaintainLog("�¸澯", "",  &struSendPackage);
	             	 	 InitAlarmPara(nNeId);
	             	 	 AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarm");//�Ƿ�Ƶ���澯
	             	 }
	             	 InsertInXmlExt(pstruXml,"<omc>/<�澯����>",struSendPackage.struMapObjList[i].szMapId,
	             	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 bNewAlarm = BOOLTRUE;

	             }
	             else if (strcmp(struSendPackage.struMapObjList[i].szMapData, "0") == 0)//ֻ��վ��ȷʵ���ڸ澯״̬�Żָ�
	             {
	             	 STR szAlarmObjResv[5];
	             	 if (bNewAlarm == BOOLTRUE)
	             	 	strcpy(szAlarmObjResv, DemandStrInXmlExt(pstruXml, "<omc>/<�澯����>"));
						
	             	 InsertInXmlExt(pstruXml,"<omc>/<�澯����>",struSendPackage.struMapObjList[i].szMapId,
	             	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	                 sprintf(szTemp, "%s:1",  struSendPackage.struMapObjList[i].szMapId);
	                 if (strstr(szAlarmObjList, szTemp) != NULL)
	                 {
	                 	 bAlarmRestore = BOOLTRUE;
	                 	 
	                 	 if (AlarmComeback(pstruXml) == NORMAL)
	                 		 strcat(szAlarmRestoreList, struAlarmPara.szAlarmName);//���ڸ澯�ָ�ǰת

	             		 SaveToMaintainLog("�澯�ָ�", "",  &struSendPackage);
	             		 InitAlarmPara(nNeId);
	             		 AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarmClear");//Ƶ���澯���
	             		 if (strlen(szAlarmRestoreList) > 0)
	                 		 TransferAlarm();
	                 }
	                 if (bNewAlarm == BOOLTRUE)
	             	 	InsertInXmlExt(pstruXml,"<omc>/<�澯����>",szAlarmObjResv,MODE_AUTOGROW|MODE_UNIQUENAME);
	                 
	             }            
	         }
	         
             if (bNewAlarm == BOOLTRUE)
	         {
	             if (DealNewAlarm(pstruXml) == NORMAL)
                      strcat(szFrequentAlarmList, struAlarmPara.szAlarmName);//����Ƶ���ϱ�
	             //Ƶ���澯
	             SaveToMaintainLog("�¸澯", "",  &struSendPackage);
	             InitAlarmPara(nNeId);
	             AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarm");//�Ƿ�Ƶ���澯
	         }

		 }
		 else if (nCommUpType == OPENSTATION) //��վ�ϱ�
		 {
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     SaveToMaintainLog("��վ�ϱ�", "",  &struSendPackage);
		     if (bIsNewNeId == BOOLFALSE)
		     {
		        InitAlarmPara(nNeId);
	            AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeBulid");//��վ�ϱ�
	         }
	         else
	         {
	         	/* 2010.6.8 add */
	         	QueryMapList(M2G_SMS, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         	InitPackageToXml(&struSendPackage, pstruXml);
	         	 
	         	InsertInXmlExt(pstruXml,"<omc>/<��ض���>",  "0009", MODE_AUTOGROW|MODE_UNIQUENAME); 
	         	InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//��ѯ
	         	SaveToMsgQueue(pstruXml);
	            SaveEleQryLog(pstruXml);
	         	 
	         	/* 2010.6.8 ȥ�������ö��ſ�վ�ϱ�����
	         	STR szMapId0009List[1000];
	         	STR szProvinceId[10];
	         	bufclr(szMapId0009List);
	         	GetSysParameter("par_SectionName = 'Province' and par_KeyName = 'ProvinceAreCode'", szProvinceId);
	         	if (GetSysParameter("par_SectionName = 'ApplServer' and par_KeyName = 'ObjectList'", szMapId0009List) != NORMAL)
				{
					strcpy(szMapId0009List, "0002,0003,0004,0005,000A,0010,0018,0020,0101,0102,0111,0112,0113,0114,0115,0120,0130,0131,0133,0136,0137,0150,0160,0161,0162,0163,0164,0165,0166,0201,0204,0301,0304,0507,0508,0509,050A,050B,050C,0701,0704,0707,0708,070A,070B,070C,070D,070E,070F,0710,0711,0712,0713,0714,0715,0716,0717,0718,0719,071A,071B,071C,071D,071E,071F,0720,0721,0722,0723,0724,0725,0726,0727,0728,0729,073A,073B,073C,073D,073E,073F,0779,07A0,07A1,07A2,07A4,07B0,07B2,07B3,07B4,07B5,07B6,07B7,07B8,07B9,07BA,07BB,07BC,07BD,07BE,07BF,07C0,07C1,07C2,07C3,07D0,07D1,07D2,07E1,07E3,07E4,07E5,07E6,07E9");
					PrintErrorLog(DBG_HERE,"��ȡ��վ�ϱ��ļ�����б�ʧ��\n");
				}
	         	if (UpdateEleObjList(&struSendPackage, szMapId0009List, szProvinceId, 1) == NORMAL)
	         	{
	         		sleep(1);
	      			QryEleFristTime(nNeId, M2G_SMS);
	      		}
	      		*/
	         }
		 }
		 else if (nCommUpType == DEVICECHANGED) //����ϱ�
		 {
		     nNeId = GetNeId(nRepeaterId, nDeviceId, pszTelephone, &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     
		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeRevamp");//����ϱ�
	         
	         //////////////////////////////////////////��ѯ�����
	         sleep(5);
	         QueryMapList(M2G_SMS, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //SaveToMaintainLog("����ϱ�", "",  &struSendPackage);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//��ѯ
	         SaveToMsgQueue(pstruXml);
	         SaveEleQryLog(pstruXml);
		     
		 }
		 else if (nCommUpType == DEVICEREPAIR) //�޸��ϱ�
		 {
		     nNeId = GetNeId(nRepeaterId, nDeviceId, pszTelephone, &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;

		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeRebuild");//����ϱ�
	         
	         //////////////////////////////////////////��ѯ�����
	         sleep(5);
	         if (bIsNewNeId == BOOLTRUE)
	         {
	             QueryMapList(M2G_SMS, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	             InitPackageToXml(&struSendPackage, pstruXml);
	             //SaveToMaintainLog("�޸��ϱ�", "",  &struSendPackage);
	             InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//��ѯ
	             SaveToMsgQueue(pstruXml);
	             SaveEleQryLog(pstruXml);
	         }
	         else
	         {
	             /*
	              *��վ�ϱ�
	              *ʵ����Ԫ�������ѯ����¼��Ϣ���б�
	              */
	             QryEleFristTime(nNeId, M2G_SMS);
	         }
	         
		     
		 }
		 else if (nCommUpType == PERSONPATROL) //Ѳ���ϱ���Ϣ
		 {
		     nNeId = GetNeId(nRepeaterId, nDeviceId, pszTelephone, &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;

		     SaveToMaintainLog("Ѳ���ϱ�", "",  &struSendPackage);
		     
		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeCheck");//Ѳ���ϱ�
		 }
		 else if (nCommUpType == 0x06) //��¼����������ϱ�
		 {
		    nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
			struSendPackage.nNeId = nNeId;
		    SaveToMaintainLog("��¼���������", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x07) //�����ϱ�
		 {
		    //SaveToMaintainLog("�����ϱ�", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x08) //Զ����������ϱ�
		 {
		     
		 	 sprintf(szTemp, "%d", struSendPackage.nNeId);
		     InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     sprintf(szTemp, "%u", struSendPackage.struRepeater.nRepeaterId);
		     InsertInXmlExt(pstruXml,"<omc>/<�豸���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     sprintf(szTemp, "%d", struSendPackage.struRepeater.nDeviceId);
		     InsertInXmlExt(pstruXml,"<omc>/<�α��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
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
		     SaveToMaintainLog("Զ����������ϱ�", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x09) //GPRS��¼ʧ���ϱ�
		 {
		     
		     SaveToMaintainLog("GPRS��¼ʧ���ϱ�", "",  &struSendPackage);
		     //SendToApplQrySet(&struSendPackage);
		     //if (ExistAlarmLog(ALM_DLSB_ID, struSendPackage.nNeId, &nAlarmCount) == BOOLFALSE)
		     //	SaveToAlarmLog(ALM_DLSB_ID, struSendPackage.nNeId, nAlarmCount);
		     	
		 }
		 else if (nCommUpType == 0x0A) //���ɽ����ϱ�
		 {
		    
		    SaveToMaintainLog("���ɽ����ϱ�", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x0B) //����쳣��λ�ϱ�
		 {
		    
		    SaveToMaintainLog("����쳣��λ�ϱ�", "",  &struSendPackage);
		 }
 		 else if (nCommUpType == 0x20 ) //������Ч�����1.0��ʱ�ϱ�
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
 			        PrintErrorLog(DBG_HERE, "ϵͳ�����ڸ�[%s]�����\n", szId);
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
		 else if (nCommUpType == 0xCB || nCommUpType == 0x2E) //GPRS�����ϱ�ʧ��
		 {
		    
		     SaveToMaintainLog("GPRS�����ϱ�ʧ��", "",  &struSendPackage);
		     //SendToApplQrySet(&struSendPackage);
		     if (ExistAlarmLog(ALM_SJSB_ID, struSendPackage.nNeId, &nAlarmCount) == BOOLFALSE)
		     	SaveToAlarmLog(ALM_SJSB_ID, struSendPackage.nNeId, nAlarmCount);
		 }
		 else if (nCommUpType == 0x2C) //����ģʽѡ��ʧ���ϱ�
		 {
		     
		     SaveToMaintainLog("����ģʽѡ��ʧ���ϱ�", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x2D) //����ģʽѡ��ָ��ϱ�
		 {
		     
		     SaveToMaintainLog("����ģʽѡ��ָ��ϱ�", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x22) //34:Ч�����PESQ���в��Խ����ϱ�
	     {
	        
	         for(i=0; i< Decodeout.MAPLayer.ObjCount; i++)
	         {
	             sprintf(szMapId, "%04X", struObject[i].MapID);
                 if (strcmp(szMapId, "0771") == 0)
                 {
	     	         sprintf(szTemp, "%d", struSendPackage.nNeId);
	                 InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	                 InsertInXmlExt(pstruXml,"<omc>/<���з���>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
	                 
                     if (DecodeGprsPesq(struObject[i].OC, struObject[i].OL, pstruXml)==NORMAL)
                     	SaveGprsPesqLog(pstruXml);
                     //SaveToMaintainLog("PESQ���в��Խ����ϱ�", "",  &struSendPackage);
                     break;
                 }
	             
	         }
	     }
	     else if (nCommUpType == 0x23) //PESQ���в��Խ����ϱ�
		 {
		   
		    for(i=0; i< Decodeout.MAPLayer.ObjCount; i++)
		    {
		        sprintf(szMapId, "%04X", struObject[i].MapID);
    	        if (strcmp(szMapId, "0771") == 0)
    	        {
			        sprintf(szTemp, "%d", struSendPackage.nNeId);
		            InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		            InsertInXmlExt(pstruXml,"<omc>/<���з���>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
		            
    	            if (DecodeGprsPesq(struObject[i].OC, struObject[i].OL, pstruXml)==NORMAL)
    	            	SaveGprsPesqLog(pstruXml);
    	            //SaveToMaintainLog("PESQ���в��Խ����ϱ�", "",  &struSendPackage);
    	            break;
    	        }
		        
		    }
		 }
		 else if (nCommUpType == 0x29) //GPRS��վ�����ϱ�
		 {
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     SaveToMaintainLog("GPRS����վ�ϱ�", "",  &struSendPackage);

	         {
    	
	         	InitPackageToXml(&struSendPackage, pstruXml);
 	         	InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//��ѯ
 	         	InsertInXmlExt(pstruXml,"<omc>/<��ض���>",  "0009", MODE_AUTOGROW|MODE_UNIQUENAME);
 	         	InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
 	         	QueryMapList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         	SaveToGprsQueue(pstruXml);
	         	SaveEleQryLog(pstruXml);
	         	
	         }
	         
		 }
		 else if (nCommUpType == 0x33) //�����豸�����ϱ�
		 {
		    
		     SaveToMaintainLog("�����豸�����ϱ�", "",  &struSendPackage);
		 }
		 else if (nCommUpType == 0x34) //�����豸�ػ��ϱ�
		 {
		     
		     SaveToMaintainLog("�����豸�ػ��ϱ�", "",  &struSendPackage);
		 }
		 else if (nCommUpType == DEVICESTARTUP) //202:Ч����������ϱ�
		 {
	         nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     SaveToMaintainLog("GPRS����վ�ϱ�", "",  &struSendPackage);
		 }
		 DeleteXml(pstruXml);
		 
		 return NORMAL;
		
	}
	/*
	 *����QB+RepeaterId��ȡԭ����Ϣ
	 *ɾ����Ӧ��ˮ��
	 */
    if(GetSendPackageInfo(QB, nRepeaterId, &struSendPackage) != NORMAL)
    {
		PrintErrorLog(DBG_HERE, "����QB+RepeaterId��ȡQA����\n");
		return EXCEPTION; 
	}
	/*
	 *�豸æ���
	 */
	if (Decodeout.NPLayer.NPJHFlag == 0x1)
	{
	     switch(struSendPackage.struHead.nCommandCode)
	     {
	         case COMMAND_QUERY:              //��ѯ
		     case COMMAND_FCTPRM_QRY:
		     case COMMAND_PRJPRM_QRY:
			      UpdateEleFromCommByPacket(struSendPackage.struHead.QA, "busy", "", "");
			      break;
			 case COMMAND_SET:                //����
		     case COMMAND_FCTPRM_SET:
		     case COMMAND_PRJPRM_SET:
	         case COMMAND_FACTORY_MODE:
			       UpdateEleFromCommBySetPacket(struSendPackage.struHead.QA, "busy");
			       break;
			 default:
			       break;
	     }
	     UpdateEleLastTime(struSendPackage.nNeId);//add by wwj at 2010.08.03
	     
	     SaveSysErrorLog(struSendPackage.nNeId, "",  "7", struSendPackage.szRecvMsg);//�豸æ����
	     return NORMAL;
	}
	/*
	 *�����ѯ���mapid
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
		 PrintDebugLog(DBG_HERE, "վ��[%u]�ܼ�ز���[%d]��ǰ���ؼ�ز���[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//��ʾ����Ҫ����ȡ
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
	         //��ȡ��ǰ��2GЭ����ˮ��
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_SMS, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		         return EXCEPTION; 
	         }
	         /*
	          *   ����xml����
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //��ѯ
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<վ��绰>", pszTelephone, MODE_AUTOGROW|MODE_UNIQUENAME);
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<�������>", pszNetCenterNum, MODE_AUTOGROW|MODE_UNIQUENAME);
             //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	         InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	         
		 	 SaveToMsgQueue(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//ȫ��ȡ������������б�
		 {
		     if (struSendPackage.struHead.nProtocolType == 47)
		     {
			     //��ѯ��չ����0001
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
		 	 PrintErrorLog(DBG_HERE, "վ��[%u]�ܼ����[%d]С�ڵ�ǰ���ؼ�ز���[%d],�쳣��!\n",  nRepeaterId, nTotalQuery, nNowQuery);
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
	     
		 PrintDebugLog(DBG_HERE, "վ��[%u]�ܼ�ز���[%d]��ǰ���ؼ�ز���[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//��ʾ����Ҫ����ȡ
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
	         //��ȡ��ǰ��2GЭ����ˮ��
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_Das(M2G_SMS, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		         return EXCEPTION; 
	         }
		 	 /*
	          *   ����xml����
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //��ѯ
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToMsgQueue(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//ȫ��ȡ������������б�
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "վ��[%u]�ܼ����[%d]С�ڵ�ǰ���ؼ�ز���[%d],�쳣��!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }

		 

	}
	
	if (bObject0001 == BOOLTRUE)
	{
		 //��ѯ0001ʧ�ܣ�ִ�з���0009��������
		 
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
	     
		 PrintDebugLog(DBG_HERE, "վ��[%u]�ܼ�ز���[%d]��ǰ���ؼ�ز���[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//��ʾ����Ҫ����ȡ
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
	         //��ȡ��ǰ��2GЭ����ˮ��
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_SMS, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		         return EXCEPTION; 
	         }
	 	 
		 	 /*
	          *   ����xml����
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //��ѯ
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<վ��绰>", pszTelephone, MODE_AUTOGROW|MODE_UNIQUENAME);
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<�������>", pszNetCenterNum, MODE_AUTOGROW|MODE_UNIQUENAME);
             //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	         InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         SaveToMsgQueue(pstruXml);
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//ȫ��ȡ������������б�
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryMap0001List((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "վ��[%u]�ܼ����[%d]С�ڵ�ǰ���ؼ�ز���[%d],�쳣��!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }

		 

	}
	//����������
	switch(struSendPackage.struHead.nCommandCode)
	{
		case COMMAND_QUERY:              //������ѯ
		case COMMAND_FCTPRM_QRY:
		case COMMAND_PRJPRM_QRY:
			DecodeQryElementParam((SENDPACKAGE *)&struSendPackage);
			break;
		case COMMAND_SET:                //��������
		case COMMAND_FCTPRM_SET:
		case COMMAND_PRJPRM_SET:
	    case COMMAND_FACTORY_MODE:
			DecodeSetElementParam((SENDPACKAGE *)&struSendPackage);
			break;

		case COMMAND_QUERY_MAPLIST:    //ȡ��������б�
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
	
	//ת�崦��
	struPack.pPack = pszUndecode;
	struPack.Len = nLen;
	
    if(!AscUnEsc(&struPack))
	{
	    PrintErrorLog(DBG_HERE,"ת�����[%s]\n",struPack.pPack);
		return -1;
	}
   	
   	//PrintHexDebugLog("����ת��������",  struPack.pPack, struPack.Len);	
	
	//����2GЭ��
    if (Decode_2G(M2G_TCPIP, &struPack, &Decodeout, struObject) != NORMAL)
    {
    	PrintErrorLog(DBG_HERE, "�����豸���ݴ���\n");
		return EXCEPTION; 
    }

    PrintDebugLog(DBG_HERE, "����GPRSЭ��ɹ�,Э������[%02d],�����ʶ[%d],�������[%d],վ����[%u],�豸���[%d],�����ʶ[%d],������[%d]\n",
        Decodeout.NPLayer.APID,  Decodeout.MAPLayer.CommandFalg, Decodeout.APLayer.ErrorCode, 
        Decodeout.NPLayer.structRepeater.RepeaterId, Decodeout.NPLayer.structRepeater.DeviceId,
        Decodeout.NPLayer.NetFlag, Decodeout.MAPLayer.ObjCount);
	
	int QB=Decodeout.NPLayer.NetFlag;//2GЭ��ͨѶ��ʾ��ˮ��
	UINT nRepeaterId=Decodeout.NPLayer.structRepeater.RepeaterId;
	int nDeviceId=Decodeout.NPLayer.structRepeater.DeviceId;
		
	//��ʼ���ֽ����
	memset(&struSendPackage, 0, sizeof(SENDPACKAGE));
	struSendPackage.struRepeater.nRepeaterId=Decodeout.NPLayer.structRepeater.RepeaterId;
    struSendPackage.struRepeater.nDeviceId=Decodeout.NPLayer.structRepeater.DeviceId;
	
	//�豸ͨѶ��ʽ 
	struSendPackage.struRepeater.nCommType=M2G_UDP_TYPE;
	
	//������
	struSendPackage.struHead.nObjectCount= Decodeout.MAPLayer.ObjCount;
	//�ϱ���ʽ
	struSendPackage.struHead.nCommandCode=COMMAND_UP;
	
	//if (GetNeInfo(&struSendPackage) != NORMAL)
	struSendPackage.struHead.nProtocolType= PROTOCOL_2G;
	
	//Э�鷽ʽ 2013.10.9
	if (Decodeout.NPLayer.APID == 0x03)
	{
		struSendPackage.struHead.nProtocolType= PROTOCOL_DAS;
	}

	//QA��ˮ��
	strcpy(struSendPackage.struHead.QA, GetTypeNumber("Gprs"));
	 
	
	//��ʼ����Ϊ��ض����������
	BOOL bObject0009=BOOLFALSE;
	BOOL bObject0606=BOOLFALSE;//����
	BOOL bObject0875=BOOLFALSE;//ʱ϶
	BOOL bObject0AEC=BOOLFALSE;//das
	BOOL bObject0AED=BOOLFALSE;//rfid
	BOOL bObject0001=BOOLFALSE;
	BOOL bObject00000009=BOOLFALSE;
	BOOL bObject00000606=BOOLFALSE;//����
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
				
		//ȡ�ϱ�����
		if (strcmp(szMapId, "0141") == 0 || strcmp(szMapId, "00000141") == 0)
		{
		    nCommUpType = struObject[i].OC[0];
		    PrintDebugLog(DBG_HERE, "�ϱ�����[%X]GPRS��ʼ\n", nCommUpType);
		    if (nCommUpType == 0xC8)         //gprs�Ŀ�վ�ϱ�
		    {
		        bObject0009 = BOOLTRUE;
		        k = i+1;
		        break;
		    }
		}
		if (strcmp(szMapId, "0009") == 0 )//������ϱ�
			bObject0009 = BOOLTRUE;
		else if (strcmp(szMapId, "0606") == 0)//����
		    bObject0606 = BOOLTRUE;
		else if (strcmp(szMapId, "00000606") == 0)//����
		    bObject00000606 = BOOLTRUE;
		else if (strcmp(szMapId, "0875") == 0)//ʱ϶
		    bObject0875 = BOOLTRUE;    	
		else if (strcmp(szMapId, "0AEC") == 0)//das
		    bObject0AEC = BOOLTRUE;
		else if (strcmp(szMapId, "0AED") == 0)//rfid
		    bObject0AED = BOOLTRUE; 
		else if ( strcmp(szMapId, "00000009") == 0)
			bObject00000009 = BOOLTRUE;
		if (strcmp(szMapId, "0001") == 0 )//��չ�����ϱ�
			bObject0001 = BOOLTRUE;
		if (strcmp(szMapId, "0004") == 0 )
			ReplaceStr(struSendPackage.struMapObjList[i].szMapData, ",", "-");
		//�ж�ʱ϶
		if (strcmp(struSendPackage.struMapObjList[i].szMapType, "ratedata") == 0)
			bObject0875 = BOOLTRUE; 
	}
	
	/*
 	 * ��ʼ��xml����
	*/
	memset(pstruXml,0,sizeof(XMLSTRU));
	CreateXml(pstruXml, FALSE, OMC_ROOT_PATH, NULL);
		
	if(Decodeout.MAPLayer.CommandFalg == 1)
	{
		 BOOL bAlarmRestore = BOOLFALSE;
		 STR szFrequentAlarmList[1000]; //Ƶ���澯�б�
		 STR szAlarmRestoreList[1000]; //�澯�澯�б�
		 INT nAlarmTimes=0;
		 
		 bufclr(szFrequentAlarmList);
		 bufclr(szAlarmRestoreList);
		
		
		//�����ϱ����ʹ���
		if (nCommUpType == DEVICE_ALARM) //�豸�澯
		{
		     /*
		      *  ȡ�澯�б�,��ȡ��ԪNeId����
		      */
		     if (GetAlarmObjList3(nRepeaterId, nDeviceId, &nNeId, szNeName, szAlarmObjList)!= NORMAL)
		     {
		         DeleteXml(pstruXml);
		         PrintErrorLog(DBG_HERE, "�豸��[%u][%d]�����ڼ�¼\n", nRepeaterId, nDeviceId);
		         return EXCEPTION;
		     }
		     InsertInXmlExt(pstruXml,"<omc>/<�澯�б�>",szAlarmObjList,MODE_AUTOGROW|MODE_UNIQUENAME);
		     sprintf(szTemp, "%d", nNeId);
	     	 InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
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
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<�澯ʱ��>",szAlarmTime,MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	             	 if (strcmp(struSendPackage.struMapObjList[i].szMapId, "050B") == 0 ||
	             	 	 strcmp(struSendPackage.struMapObjList[i].szMapId, "08B3") == 0)   //TD
	             	 {
	             	 	 PrintDebugLog(DBG_HERE,"��ƽǿ��[%s]\n",struSendPackage.struMapObjList[i].szMapData);
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<��ƽǿ��>",struSendPackage.struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	             	 if (strcmp(struSendPackage.struMapObjList[i].szMapId, "050C") == 0)
	             	 {
	             	 	PrintDebugLog(DBG_HERE,"����С��[%s]\n",struSendPackage.struMapObjList[i].szMapData);
	             	 	 InsertInXmlExt(pstruXml,"<omc>/<����С��>",struSendPackage.struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 }
	                 continue;
	             }
	             
	             if (strcmp(struSendPackage.struMapObjList[i].szMapData, "1") == 0)//�澯����
	             {
	             	 if (nAlarmTimes++ >= 1)
	             	 {
	             	 	 if (DealNewAlarm(pstruXml) == NORMAL)
                      	 strcat(szFrequentAlarmList, struAlarmPara.szAlarmName);//����Ƶ���ϱ�
	             	 	 //Ƶ���澯

	             	 	 SaveToMaintainLog("�¸澯", "",  &struSendPackage);
	             	 	 InitAlarmPara(nNeId);
	             	 	// AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarm");//�Ƿ�Ƶ���澯
	             	 }
	             	 InsertInXmlExt(pstruXml,"<omc>/<�澯����>",struSendPackage.struMapObjList[i].szMapId,
	             	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 bNewAlarm = BOOLTRUE;

	             }
	             else if (strcmp(struSendPackage.struMapObjList[i].szMapData, "0") == 0)//ֻ��վ��ȷʵ���ڸ澯״̬�Żָ�
	             {
	             	 STR szAlarmObjResv[5];
	             	 if (bNewAlarm == BOOLTRUE)
						strcpy(szAlarmObjResv, DemandStrInXmlExt(pstruXml, "<omc>/<�澯����>"));
					 	
	             	 InsertInXmlExt(pstruXml,"<omc>/<�澯����>",struSendPackage.struMapObjList[i].szMapId,
	             	 		MODE_AUTOGROW|MODE_UNIQUENAME);
	             	 		
	                 sprintf(szTemp, "%s:1",  struSendPackage.struMapObjList[i].szMapId);
	                 if (strstr(szAlarmObjList, szTemp) != NULL)
	                 {
	                 	 bAlarmRestore = BOOLTRUE;
	                 	 
	                 	 if (AlarmComeback(pstruXml) == NORMAL)
	                 		 strcat(szAlarmRestoreList, struAlarmPara.szAlarmName);//���ڸ澯�ָ�ǰת

	             		 SaveToMaintainLog("�澯�ָ�", "",  &struSendPackage);
	             		 InitAlarmPara(nNeId);
	             		// AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarmClear");//Ƶ���澯���
	             		 if (strlen(szAlarmRestoreList) > 0)
	                 		 TransferAlarm();
	                 }
	                 
	                 if (bNewAlarm == BOOLTRUE)
	             	 	InsertInXmlExt(pstruXml,"<omc>/<�澯����>",szAlarmObjResv,MODE_AUTOGROW|MODE_UNIQUENAME);
	             }            
	         }
	         
             if (bNewAlarm == BOOLTRUE)
	         {
	             if (DealNewAlarm(pstruXml) == NORMAL)
                      strcat(szFrequentAlarmList, struAlarmPara.szAlarmName);//����Ƶ���ϱ�
	             //Ƶ���澯

	             SaveToMaintainLog("�¸澯", "",  &struSendPackage);
	             InitAlarmPara(nNeId);
	            // AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarm");//�Ƿ�Ƶ���澯
	         }

		}
		else if (nCommUpType == OPENSTATION) //UDP��վ�ϱ�
		{
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     SaveToMaintainLog("��վ�ϱ�", "",  &struSendPackage);
		     if (bIsNewNeId == BOOLFALSE)
		     {
		        InitAlarmPara(nNeId);
	            AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeBulid");//��վ�ϱ�
	         }
	         else
	         {
	         	/*ȡ�豸IP�Ͷ˿� */
	         	if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         	{
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "�豸[%u][%d]������IP��PORT��¼\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     	}

	         	strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
				struSendPackage.struRepeater.nPort = nDevicePort;
				
	         	/* 2010.6.8 add */
	         	QueryMapList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         	InitPackageToXml(&struSendPackage, pstruXml);
	         	 
	         	InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//��ѯ
	         	InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         	InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	         					
	         	SaveToGprsQueue(pstruXml);
	            SaveEleQryLog(pstruXml);
	         	         	
	         }
		}
		else if (nCommUpType == DEVICECHANGED) //����ϱ�
		{
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     
		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeRevamp");//����ϱ�
	         /*ȡ�豸IP�Ͷ˿� */
	         if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         {
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "�豸[%u][%d]������IP��PORT��¼\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     }
	         strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
			 struSendPackage.struRepeater.nPort = nDevicePort;
	         
	         //////////////////////////////////////////��ѯ�����
	         QueryMapList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         InitPackageToXml(&struSendPackage, pstruXml);

	         SaveToMaintainLog("����ϱ�", "",  &struSendPackage);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//��ѯ
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);	
	         SaveToGprsQueue(pstruXml);
	         SaveEleQryLog(pstruXml);
		     
		}
		else if (nCommUpType == DEVICEREPAIR) //�޸��ϱ�
		{
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;

		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeRebuild");
	         /*ȡ�豸IP�Ͷ˿� */
	         if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         {
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "�豸[%u][%d]������IP��PORT��¼\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     }
	         strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
			 struSendPackage.struRepeater.nPort = nDevicePort;
	         
	         //////////////////////////////////////////��ѯ�����
	         if (bIsNewNeId == BOOLFALSE)
	         {
	             QueryMapList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	             InitPackageToXml(&struSendPackage, pstruXml);

	             SaveToMaintainLog("�޸��ϱ�", "",  &struSendPackage);
	             
	             InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);//��ѯ
	         	 InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         	 InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	             SaveToGprsQueue(pstruXml);
	             SaveEleQryLog(pstruXml);
	         }
	         else
	         {
	             /*
	              *��վ�ϱ�
	              *ʵ����Ԫ�������ѯ����¼��Ϣ���б�
	              */
	             QryEleFristTime(nNeId, M2G_SMS);
	         }
	         
		     
		}
		else if (nCommUpType == PERSONPATROL) //Ѳ���ϱ���Ϣ
		{
		     nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;

		     SaveToMaintainLog("Ѳ���ϱ�", "",  &struSendPackage);
		     
		     InitAlarmPara(nNeId);
	         AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqNeCheck");//Ѳ���ϱ�
		}
		else if (nCommUpType == 0xC8) //�ϵ�gprs��վ�ϱ�
    	{
		     {
		         if (GetPackInfoFromMainLog(&struSendPackage) != NORMAL)
    	     	{
    	     	    PrintErrorLog(DBG_HERE, "GPRS��վ�ϱ�û�з��������ϱ�����0xCA,���ش���!\n");
    	     	    DeleteXml(pstruXml);
		     	    return EXCEPTION;
    	     	}
		     }
    	     
    	     
		 	sprintf(szTemp, "%d", struSendPackage.nNeId);
		     InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
    	 
		 	//SaveToMaintainLog("GPRS������ϱ�", "",  &struSendPackage); 
		 	//�����ϱ������
		 	DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
		}
		else if (nCommUpType == 0x06) //��¼����������ϱ�
		{
			nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
			struSendPackage.nNeId = nNeId;
		    SaveToMaintainLog("��¼���������", "",  &struSendPackage);
			DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
		}
		else if (nCommUpType == 0x07) //�����ϱ�
		{
		    //SaveToMaintainLog("�����ϱ�", "",  &struSendPackage);
		}
		else if (nCommUpType == 0x08) //Զ����������ϱ�
		{
		    
			sprintf(szTemp, "%d", struSendPackage.nNeId);
		    InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		    InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
			sprintf(szTemp, "%u", struSendPackage.struRepeater.nRepeaterId);
			InsertInXmlExt(pstruXml,"<omc>/<�豸���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
			sprintf(szTemp, "%d", struSendPackage.struRepeater.nDeviceId);
			InsertInXmlExt(pstruXml,"<omc>/<�α��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
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
		    SaveToMaintainLog("Զ����������ϱ�", "",  &struSendPackage);
		}
		else if (nCommUpType == 0x09) //GPRS��¼ʧ���ϱ�
		{
		   
		    SaveToMaintainLog("GPRS��¼ʧ���ϱ�", "",  &struSendPackage);
		}
		else if (nCommUpType == 0x0A) //���ɽ����ϱ�
		{
			char szMapObject[100];
		    
		    SaveToMaintainLog("���ɽ����ϱ�", "",  &struSendPackage);
		    /*ȡ�豸IP�Ͷ˿� */
	         	if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         	{
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "�豸[%u][%d]������IP��PORT��¼\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     	}
	         	strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
				struSendPackage.struRepeater.nPort = nDevicePort;
		    //2015.08.18
		    //��ʼ��xml
	     	InitPackageToXml(&struSendPackage, pstruXml);
	     	if (struSendPackage.struHead.nProtocolType == PROTOCOL_DAS)
	     	{
		    	InsertInXmlExt(pstruXml,"<omc>/<��ض���>","00000606",MODE_AUTOGROW|MODE_UNIQUENAME);
		    	GetBatPickMap8(struSendPackage.nNeId, szMapObject);
		    }
		    else
		    {
		    	InsertInXmlExt(pstruXml,"<omc>/<��ض���>","0606",MODE_AUTOGROW|MODE_UNIQUENAME);
		    	GetBatPickMap4(struSendPackage.nNeId, szMapObject);
		    }
		    
		    PrintDebugLog(DBG_HERE, "szMapObject[%s]\n", szMapObject);
		    InsertInXmlExt(pstruXml,"<omc>/<��ض�������>",szMapObject,MODE_AUTOGROW|MODE_UNIQUENAME);
		    
		    InsertInXmlExt(pstruXml,"<omc>/<����>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);//��ѯ
         	InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
         	
		    struSendPackage.struHead.nCommandCode=COMMAND_QUERY;
		    
		    QryElementParam(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
        	SaveToGprsQueue(pstruXml);
        	SaveEleQryLog(pstruXml);
        	
        	InitBatPick(pstruXml);
        	SaveBatPickLog(pstruXml);
		}
		else if (nCommUpType == 0x0B) //����쳣��λ�ϱ�
		{
		   
		    SaveToMaintainLog("����쳣��λ�ϱ�", "",  &struSendPackage);
		}
		else if (nCommUpType == 0x20 || nCommUpType == 0x21 || nCommUpType == 0xC9) //��ʱ�ϱ�
		{
		    
			 //0x21 TD��ʱ�ϱ�
			 if (nCommUpType == 0x21)
			 {
			 	DecodeQryOnTime(&struSendPackage, BOOLFALSE, BOOLFALSE);
			    //SaveToMaintainLog("TD-SCDMA��ʱ�ϱ�", "",  &struSendPackage);
			 }
			 else if (nCommUpType == 0x20)
			 {
			 	DecodeQryOnTime(&struSendPackage, BOOLTRUE, BOOLFALSE);
			    //SaveToMaintainLog("GPRS��ʱ�ϱ�", "",  &struSendPackage);
			 }
			 else 
			 {
			 	DecodeQryOnTime(&struSendPackage, BOOLTRUE, BOOLTRUE);
			 }	
		}
		
		else if (nCommUpType == 0x26) //Զ�����������ϱ�
		{
		    
		    SaveToMaintainLog("Զ�����������ϱ�", "",  &struSendPackage);
		}
		else if (nCommUpType == 0xF1 || nCommUpType == 0xF2) //das����ϱ�, dasɾ���ϱ�
		{
			 nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     if (nCommUpType == 0xF1) 
		     	SaveToMaintainLog("Topologic update", "",  &struSendPackage);
		     else if (nCommUpType == 0xF2)
		     	SaveToMaintainLog("Delete device", "",  &struSendPackage); 

	         //////////////////////////////////////////��ѯ�����
	         /*ȡ�豸IP�Ͷ˿� */
	         if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         {
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "�豸[%u][%d]������IP��PORT��¼\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     }
	         strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
			 struSendPackage.struRepeater.nPort = nDevicePort;
			
	         QueryDasList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         InitPackageToXml(&struSendPackage, pstruXml);
	        	         
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);//��ѯ
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);	
	         SaveToGprsQueue(pstruXml);
	         SaveEleQryLog(pstruXml);
		}
		else if (nCommUpType == 0xF3)
		{
			 nNeId = GetNeId(nRepeaterId, nDeviceId, "", &bIsNewNeId);
		     struSendPackage.nNeId = nNeId;
		     SaveToMaintainLog("RFID update", "",  &struSendPackage);

	         //////////////////////////////////////////��ѯ�����
	         /*ȡ�豸IP�Ͷ˿� */
	         if (GetDeviceIp(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort) != NORMAL)
	         {
		         	DeleteXml(pstruXml);
		         	PrintErrorLog(DBG_HERE, "�豸[%u][%d]������IP��PORT��¼\n", nRepeaterId, nDeviceId);
		         	return EXCEPTION;
		     }
	         strcpy(struSendPackage.struRepeater.szIP, szDeviceIp);
			 struSendPackage.struRepeater.nPort = nDevicePort;
			 
	         QueryRfidList(M2G_TCPIP, &struSendPackage.struHead, &struSendPackage.struRepeater, pstruXml);
	         InitPackageToXml(&struSendPackage, pstruXml);
	        	         
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);//��ѯ
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);	
	         SaveToGprsQueue(pstruXml);
	         SaveEleQryLog(pstruXml);
		}
		
		DeleteXml(pstruXml);
		return NORMAL;
	}
	
	/*
	 *����QB+RepeaterId��ȡԭ����Ϣ
	 *ɾ����Ӧ��ˮ��
	 */
	if (getenv("WUXIAN")!=NULL)
	{
		if (GetRedisPackageInfo(QB, &struSendPackage, pstruXml) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "����QB+RepeaterId��ȡredis���� %u_%d_%d\n", nRepeaterId, nDeviceId, QB);
			return EXCEPTION;
		}
	}
	else if(GetGprsPackageInfo(QB, nRepeaterId, &struSendPackage) != NORMAL)
    {
		PrintErrorLog(DBG_HERE, "����QB+RepeaterId��ȡQA����\n");
		return EXCEPTION; 
	}
	
	/*
	 *����վ�ϱ�������
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
	     
		 PrintDebugLog(DBG_HERE, "վ��[%u]�ܼ�ز���[%d]��ǰ���ؼ�ز���[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//��ʾ����Ҫ����ȡ
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
	         //��ȡ��ǰ��2GЭ����ˮ��
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"ת�����[%s]\n",struPack.pPack);
			 	   return -1;
			 }
	         /*
	          *   ����xml����
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //��ѯ
	         sprintf(szTemp, "%d", nEleQryLogId);
		     //InsertInXmlExt(pstruXml,"<omc>/<��ˮ��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     InsertInXmlExt(pstruXml,"<omc>/<��־��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);
		 	 //SaveEleQryLog(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//ȫ��ȡ������������б�
		 {
		 	 if (struSendPackage.struHead.nProtocolType == 47)
		 	 {
			 	 //��ѯ��չ����0001
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
		 	 PrintErrorLog(DBG_HERE, "վ��[%d]�ܼ����[%d]С�ڵ�ǰ���ؼ�ز���[%d],�쳣��!\n",  nRepeaterId, nTotalQuery, nNowQuery);
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
	     
		 PrintDebugLog(DBG_HERE, "վ��[%d]�ܼ�ز���[%d]��ǰ���ؼ�ز���[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//��ʾ����Ҫ����ȡ
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
	         //��ȡ��ǰ��2GЭ����ˮ��
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_Das(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"ת�����[%s]\n",struPack.pPack);
			 	   return -1;
			 }
	         /*
	          *   ����xml����
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //��ѯ
	         sprintf(szTemp, "%d", nEleQryLogId);
		     //InsertInXmlExt(pstruXml,"<omc>/<��ˮ��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     InsertInXmlExt(pstruXml,"<omc>/<��־��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);
		 	 //SaveEleQryLog(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//ȫ��ȡ������������б�
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryMapList((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "վ��[%d]�ܼ����[%d]С�ڵ�ǰ���ؼ�ز���[%d],�쳣��!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }

		 

	}
	/*
	 *������4λ��8λ����, ִ��DecodeQueryMap0001List
	 */
	if (bObject0001 == BOOLTRUE)
	{
		 //��ѯ0001ʧ�ܣ�ִ�з���0009��������
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
	     
		 PrintDebugLog(DBG_HERE, "վ��[%d]�ܼ�ز���[%d]��ǰ���ؼ�ز���[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//��ʾ����Ҫ����ȡ
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
	         //��ȡ��ǰ��2GЭ����ˮ��
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"ת�����[%s]\n",struPack.pPack);
			 	   return -1;
			 }
	         /*
	          *   ����xml����
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //��ѯ
	         sprintf(szTemp, "%d", nEleQryLogId);
		     InsertInXmlExt(pstruXml,"<omc>/<��ˮ��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     InsertInXmlExt(pstruXml,"<omc>/<��־��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);
		 	 //SaveEleQryLog(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery)//ȫ��ȡ������������б�
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryMap0001List((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "վ��[%d]�ܼ����[%d]С�ڵ�ǰ���ؼ�ز���[%d],�쳣��!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }

		 

	}
	
	/*
	 *����DAS������ͼ, ִ��DecodeQueryDasList
	 */
	if (bObject0AEC == BOOLTRUE)
	{
		 int nMapIdCount=(struObject[0].OL - 9)/31; //����
		 
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
	     
		 PrintDebugLog(DBG_HERE, "վ��[%d]�ܼ�ز���[%d]��ǰ���ؼ�ز���[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//��ʾ����Ҫ����ȡ
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
				PrintErrorLog(DBG_HERE,"�ָ����[%s]\n",szSpecialCode);
				return -1;
			}	
			struObjList[0].OC[2] = nSeperateNum;
			//������ָ����
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
	         //��ȡ��ǰ��2GЭ����ˮ��
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"ת�����[%s]\n",struPack.pPack);
			 	   return -1;
			 }
	         /*
	          *   ����xml����
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //��ѯ
	         sprintf(szTemp, "%d", nEleQryLogId);
		     InsertInXmlExt(pstruXml,"<omc>/<��ˮ��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     InsertInXmlExt(pstruXml,"<omc>/<��־��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);
		 	 //SaveEleQryLog(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery && nMapIdCount > 0)//ȫ��ȡ������������б�
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryDasList((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "վ��[%d]�ܼ����[%d]С�ڵ�ǰ���ؼ�ز���[%d],�쳣��!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }
		 return NORMAL;
	}
	
	/*
	 *����RFID������ͼ
	 */
	if (bObject0AED == BOOLTRUE)
	{
		 int nMapIdCount=(struObject[0].OL - 6)/6; //����
		 
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
	     
		 PrintDebugLog(DBG_HERE, "վ��[%d]�ܼ�ز���[%d]��ǰ���ؼ�ز���[%d]\n", nRepeaterId, nTotalQuery, nNowQuery);
		 if( nTotalQuery > nNowQuery)//��ʾ����Ҫ����ȡ
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
	         //��ȡ��ǰ��2GЭ����ˮ��
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"ת�����[%s]\n",struPack.pPack);
			 	   return -1;
			 }
	         /*
	          *   ����xml����
	          */
	         memset(pstruXml,0,sizeof(XMLSTRU));
	         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
	         InitPackageToXml(&struSendPackage, pstruXml);
	         //��ѯ
	         sprintf(szTemp, "%d", nEleQryLogId);
		     InsertInXmlExt(pstruXml,"<omc>/<��ˮ��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		     InsertInXmlExt(pstruXml,"<omc>/<��־��>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);
		 	 //SaveEleQryLog(pstruXml);
		 	 
		 	 DeleteXml(pstruXml);
		 	 return NORMAL;
		 }
		 else if( nTotalQuery == nNowQuery && nMapIdCount > 0)//ȫ��ȡ������������б�
		 {
		     struSendPackage.struHead.nCommandCode = COMMAND_QUERY_MAPLIST;
		     DecodeQueryRfidList((SENDPACKAGE *)&struSendPackage);
		     return NORMAL;
		 }
		 else
		 {
		 	 PrintErrorLog(DBG_HERE, "վ��[%d]�ܼ����[%d]С�ڵ�ǰ���ؼ�ز���[%d],�쳣��!\n",  nRepeaterId, nTotalQuery, nNowQuery);
			 return EXCEPTION; 
		 }
		 return NORMAL;
	}
	/*
	 *��������
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
          *   ����xml����
          */
         memset(pstruXml,0,sizeof(XMLSTRU));
         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
         InitPackageToXml(&struSendPackage, pstruXml);		
		
		sprintf(szTemp, "%d", struSendPackage.nNeId);
		InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		sprintf(szTemp, "%d", QB);
		InsertInXmlExt(pstruXml,"<omc>/<ͨ�Ű���ʶ>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		
		nQryId=ReadWORD(struObject[0].OC);
		sprintf(szQryId, "%04X", nQryId);
		nMapId=ReadWORD(struObject[0].OC+2);
		sprintf(szMapId, "%04X", nMapId);
		nBatPickLen = struObject[0].OL-4;
		memset(szBatPickValue, 0, sizeof(szBatPickValue));
		memcpy(szBatPickValue, struObject[0].OC+4, nBatPickLen);
		
		sprintf(szTemp, "%d", nQryId);
		InsertInXmlExt(pstruXml,"<omc>/<��ѯ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<���ɶ���>", szMapId, MODE_AUTOGROW|MODE_UNIQUENAME);
		sprintf(szTemp, "%d", nBatPickLen);
		InsertInXmlExt(pstruXml,"<omc>/<���ɳ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
//		InsertInXmlExt(pstruXml,"<omc>/<������ֵ>", szBatPickValue, MODE_AUTOGROW|MODE_UNIQUENAME);

		if (CheckBatPickTmp(pstruXml) == 0)
		{
			DeleteXml(pstruXml);
		 	return NORMAL;
		}	
			
		PrintHexDebugLog("������ֵ", szBatPickValue, nBatPickLen);	
		
		if (strcmp(szQryId, "FFFF" ) != 0)
		{	
			/*
			 *������������
			 */
			 SaveBatPickDat(pstruXml, szBatPickValue, nBatPickLen);

			/*
			 *�������Ͳ�ѯ��
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
	         //��ȡ��ǰ��2GЭ����ˮ��
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_2G(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"ת�����[%s]\n",struPack.pPack);
			 	   return -1;
			 }

	         //��ѯ
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);

			/*
			 *����������ʱ��
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
          *   ����xml����
          */
         memset(pstruXml,0,sizeof(XMLSTRU));
         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
         InitPackageToXml(&struSendPackage, pstruXml);		
		
		sprintf(szTemp, "%d", struSendPackage.nNeId);
		InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		sprintf(szTemp, "%d", QB);
		InsertInXmlExt(pstruXml,"<omc>/<ͨ�Ű���ʶ>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		
		nQryId=ReadWORD(struObject[0].OC);
		sprintf(szQryId, "%04X", nQryId);
		
		nMapId=ReadDWORD(struObject[0].OC+2);
		sprintf(szMapId, "%08X", nMapId);
		
		nBatPickLen = struObject[0].OL-6;
		memset(szBatPickValue, 0, sizeof(szBatPickValue));
		memcpy(szBatPickValue, struObject[0].OC+6, nBatPickLen);
		
		sprintf(szTemp, "%d", nQryId);
		InsertInXmlExt(pstruXml,"<omc>/<��ѯ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<���ɶ���>", szMapId, MODE_AUTOGROW|MODE_UNIQUENAME);
		sprintf(szTemp, "%d", nBatPickLen);
		InsertInXmlExt(pstruXml,"<omc>/<���ɳ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
//		InsertInXmlExt(pstruXml,"<omc>/<������ֵ>", szBatPickValue, MODE_AUTOGROW|MODE_UNIQUENAME);

		if (CheckBatPickTmp(pstruXml) == 0)
		{
			DeleteXml(pstruXml);
		 	return NORMAL;
		}	
			
		PrintHexDebugLog("������ֵ", szBatPickValue, nBatPickLen);	
		
		if (strcmp(szQryId, "FFFF" ) != 0)
		{	
			/*
			 *������������
			 */
			 SaveBatPickDat(pstruXml, szBatPickValue, nBatPickLen);

			/*
			 *�������Ͳ�ѯ��
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
	         //��ȡ��ǰ��2GЭ����ˮ��
	         int n2G_QB;
	         if (strcmp(getenv("DATABASE"), "mysql") == 0)
				n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   			 else
   		     	n2G_QB = GetCurrent2GSquenue();
   		
	         if (Encode_Das(M2G_TCPIP, QUERYCOMMAND, n2G_QB, &Decodeout.NPLayer.structRepeater, struObject, nObjCount, &struPack) != NORMAL)
	         {
	             PrintErrorLog(DBG_HERE, "�ظ������ϱ��������\n");
		         return EXCEPTION; 
	         }
		 	 if(!AscEsc(&struPack))
			 {
			 	   PrintErrorLog(DBG_HERE,"ת�����[%s]\n",struPack.pPack);
			 	   return -1;
			 }

	         //��ѯ
	         InsertInXmlExt(pstruXml,"<omc>/<����>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);
	         InsertInXmlExt(pstruXml,"<omc>/<ͨ�ŷ�ʽ>",  "6", MODE_AUTOGROW|MODE_UNIQUENAME); //UDP
	         sprintf(szTemp, "%d", n2G_QB);
	         InsertInXmlExt(pstruXml,"<omc>/<���к�>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	         
	         InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
			 
			 //վ��ȼ�Ϊ��:OMC_QUICK_MSGLEVEL
	     	 InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     	 
		 	 SaveToGprsQueue(pstruXml);

			/*
			 *����������ʱ��
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
	 *��ʷʱ϶
	 */
	if (bObject0875 == BOOLTRUE)
	{
		int nBatPickLen;
		STR szBatPickValue[100];
		

         /*
          *   ����xml����
          */
         memset(pstruXml,0,sizeof(XMLSTRU));
         CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL);
         InitPackageToXml(&struSendPackage, pstruXml);		
		
		sprintf(szTemp, "%d", struSendPackage.nNeId);
		InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		sprintf(szTemp, "%d", QB);
		InsertInXmlExt(pstruXml,"<omc>/<ͨ�Ű���ʶ>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<��ض���>", szMapId, MODE_AUTOGROW|MODE_UNIQUENAME);
				
		nBatPickLen = struObject[0].OL-4;
		memset(szBatPickValue, 0, sizeof(szBatPickValue));
		memcpy(szBatPickValue, struObject[0].OC+4, nBatPickLen);
		
		sprintf(szTemp, "%d", nBatPickLen);
		InsertInXmlExt(pstruXml,"<omc>/<ʱ϶����>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);

		if (CheckShiXiPack(pstruXml) == 0)
		{
			DeleteXml(pstruXml);
		 	return NORMAL;
		}	
			
		PrintHexDebugLog("��ʷʱ϶", szBatPickValue, nBatPickLen);	
		
		//����ʱ϶����
		SaveShiXiDat(pstruXml, szBatPickValue, nBatPickLen);
		DeleteShiXiTmp(pstruXml);
		
		PrintDebugLog(DBG_HERE,"QA=%s", struSendPackage.struHead.QA);
		UpdateEleFromCommByPacket(struSendPackage.struHead.QA, "1", "", "");
		DeleteXml(pstruXml);
		return NORMAL;							
	}	 
	
	//����������
	switch(struSendPackage.struHead.nCommandCode)
	{
		case COMMAND_QUERY:              //������ѯ
			DecodeQryElementParam((SENDPACKAGE *)&struSendPackage);
			break;
		case COMMAND_SET:                //��������
			DecodeSetElementParam((SENDPACKAGE *)&struSendPackage);
			break;
		case COMMAND_QUERY_MAPLIST:    //ȡ��������б�
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
		
	//��ȡ�����
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
	nCmdId = (Hivalue*16+Lovalue);//����ת��

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
	PrintDebugLog(DBG_HERE,"ִ��SQL���[%s][%d][%d]\n",szSql, nProctlType, nCmdId);
	if(BindSelectTableRecord(szSql,&struCursor, &struBindVar)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"ִ��SQL���[%s]ʧ��,������ϢΪ[%s]\n",szSql,GetSQLErrorMessage());
		return -1;
	}
	if(FetchCursor(&struCursor)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"ִ��SQL���[%s][%d][%d]�޼�¼\n",szSql, nProctlType, nCmdId);
		FreeCursor(&struCursor);
		return -1;
	}
	strcpy(szCmdObjList, GetTableFieldValue(&struCursor,"CDO_OBJECTS"));
	FreeCursor(&struCursor);
	
	//����ת��
	BYTEARRAY struPack;
	struPack.pPack = pszUndecode;
	struPack.Len = strlen(pszUndecode);
	if (ByteCombine(nProctlType, &struPack, szCmdObjList) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "ת�崦�����\n");
		return EXCEPTION;
	}
	PrintHexDebugLog("����ת��", struPack.pPack, struPack.Len);
	
	//����Э��
	int res;
	CMDHEAD struCmdHead;
	UBYTE ubCmdBody[200];
	
	OMCOBJECT struOmcObj[MAX_OBJ_NUM];
	int nOmcObjNum;
	
	res = Decode_Gsm(struPack.pPack, struPack.Len, &struCmdHead, ubCmdBody);
	if (res != 0)
	{
		PrintErrorLog(DBG_HERE, "����Э�����,���ؽ��[%d]\n", res);
		return EXCEPTION;
	}
	
	PrintDebugLog(DBG_HERE, "ubProVer[%d]ubPacNum[%d]ubPacIndex[%d]ubDevType[%d]ubCmdId[%d]udwRepId[%u]ubDevId[%d]ubAnsFlag[%d]ubCmdBodyLen[%d]\n",
		struCmdHead.ubProVer,struCmdHead.ubPacNum,struCmdHead.ubPacIndex,struCmdHead.ubDevType,struCmdHead.ubCmdId,struCmdHead.udwRepId,struCmdHead.ubDevId,struCmdHead.ubAnsFlag,struCmdHead.ubCmdBodyLen
		);
	
	res = DecodeCmdBodyFromCmdId(nProctlType, szCmdObjList, &struCmdHead, ubCmdBody, struOmcObj, &nOmcObjNum);	
	if (res != 0)
	{
		PrintErrorLog(DBG_HERE, "����Э�����,���ؽ��[%d]\n", res);
		return EXCEPTION;
	}
	
	//��ʼ���ֽ����
	SENDPACKAGE struSendPackage;
	memset(&struSendPackage, 0, sizeof(SENDPACKAGE));
	struSendPackage.struRepeater.nRepeaterId=struCmdHead.udwRepId;
    struSendPackage.struRepeater.nDeviceId=struCmdHead.ubDevId;
	memcpy((char*)struSendPackage.struRepeater.szNetCenter, pszNetCenterNum,strlen(pszNetCenterNum));//�������
	struSendPackage.struRepeater.nCommType=M2G_SMS_TYPE;//�豸ͨѶ��ʽ 
	memcpy((char*)struSendPackage.struRepeater.szTelephoneNum, pszTelephone, strlen(pszTelephone));//�ϱ��豸�ֻ���
//	struSendPackage.struHead.nObjectCount= 0;//������
	struSendPackage.struHead.nObjectCount= nOmcObjNum;//������
	struSendPackage.struHead.nProtocolType=PROTOCOL_GSM;
	memcpy((char*)struSendPackage.struHead.QA,"Sms123456789", 12);//QA��ˮ��
	
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
		
		//����obj		
		UWORD uwSoftVer;
		if (strcmp(szObjId, "000A") == 0)//�汾��
		{
			uwSoftVer = atoi(szObjVal);
			sprintf(szObjVal, "%d.%d", LOWBYTE(uwSoftVer), HIGBYTE(uwSoftVer));
		}
		
		//����SendPackage	
	    struSendPackage.struMapObjList[i].cErrorId = '0';
	    
	    strcpy(struSendPackage.struMapObjList[i].szMapId, szObjId);
	    strcpy(struSendPackage.struMapObjList[i].szMapData, szObjVal);
	    
	    sprintf(szProperty, "%s%s,", szProperty, struSendPackage.struMapObjList[i].szMapId);
	    sprintf(szContent, "%s%s,", szContent, struSendPackage.struMapObjList[i].szMapData);
	}
	PrintDebugLog(DBG_HERE, "Property[%s]\n", szProperty);
	PrintDebugLog(DBG_HERE, "Content[%s]\n", szContent);
	
	//�����ϱ�����
	if (nCmdId == 0x10 || //C,HeiBei,XianChuang,Sunwave,WiLiKe
		nCmdId == 0x40 || nCmdId == 0x41)//G
	{
		//�ظ�����	     
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
		if (ByteSplit(nProctlType, &struPack, NULL) != NORMAL)//�ֽڲ��
		{
			PrintErrorLog(DBG_HERE,"ת�����[%s]\n",struPack.pPack);
			return -1;
		}	
		
	     memset(pstruXml,0,sizeof(XMLSTRU));
	     CreateXml(pstruXml,FALSE, OMC_ROOT_PATH,NULL); //����xml����
	     strcpy(struSendPackage.struHead.QA, GetTypeNumber("Sms"));//ȡΨһ��ˮ��
	     InitPackageToXml(&struSendPackage, pstruXml);//��ʼ��xml
	     InsertInXmlExt(pstruXml,"<omc>/<����>",  "00", MODE_AUTOGROW|MODE_UNIQUENAME);//�Զ��ظ�����
	     InsertInXmlExt(pstruXml,"<omc>/<���к�>","0",MODE_AUTOGROW|MODE_UNIQUENAME);//ͨ�ŷ�����ɾ����Ϣ����ͨ�Ű���ʶ��
	     InsertInXmlExt(pstruXml,"<omc>/<��Ϣ����>",(char *)ubOutPack,MODE_AUTOGROW|MODE_UNIQUENAME);
	     InsertInXmlExt(pstruXml,"<omc>/<վ��绰>", pszTelephone, MODE_AUTOGROW|MODE_UNIQUENAME);
	     InsertInXmlExt(pstruXml,"<omc>/<�������>", pszNetCenterNum, MODE_AUTOGROW|MODE_UNIQUENAME);
	     InsertInXmlExt(pstruXml,"<omc>/<վ��ȼ�>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	     
         SaveToMsgQueue(pstruXml);		
		
		 //�澯����
		 BOOL bAlarmRestore = BOOLFALSE;
		 STR szFrequentAlarmList[1000]; //Ƶ���澯�б�
		 STR szAlarmRestoreList[1000]; //�澯�澯�б�
		 
		 char szNeName[50], szAlarmObjList[1000], szAlarmTime[20];
		 
		 bufclr(szFrequentAlarmList);
		 bufclr(szAlarmRestoreList);

	     if (GetAlarmObjList2(struCmdHead.udwRepId, struCmdHead.ubDevId, pszTelephone, &nNeId, szNeName, szAlarmObjList)!= NORMAL)
	     {
	         DeleteXml(pstruXml);
	         PrintErrorLog(DBG_HERE, "�豸��[%d][%d]�����ڼ�¼\n", struCmdHead.udwRepId,  struCmdHead.ubDevId);
	         return EXCEPTION;
	     }
	     InsertInXmlExt(pstruXml,"<omc>/<�澯�б�>",szAlarmObjList,MODE_AUTOGROW|MODE_UNIQUENAME);
	     sprintf(szTemp, "%d", nNeId);
     	 InsertInXmlExt(pstruXml,"<omc>/<��Ԫ���>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
         struSendPackage.nNeId = nNeId;
         
         bufclr(szAlarmTime);
		 for(i=0; i< nOmcObjNum; i++)
         {
             if (strstr(szAlarmObjList, struOmcObj[i].szObjId) == NULL)
             {
                 continue;
             }
             
             sprintf(szAlarmTime, "%s", GetSysDateTime());
			 InsertInXmlExt(pstruXml,"<omc>/<�澯ʱ��>",szAlarmTime,MODE_AUTOGROW|MODE_UNIQUENAME);
			 InsertInXmlExt(pstruXml,"<omc>/<�澯����>",struOmcObj[i].szObjId, MODE_AUTOGROW|MODE_UNIQUENAME);
             
             if (strcmp(struOmcObj[i].szObjVal, "1") == 0)//�澯����
             {
             	
				if (DealNewAlarm(pstruXml) == NORMAL)
					strcat(szFrequentAlarmList, struAlarmPara.szAlarmName);//����Ƶ���ϱ�
				//Ƶ���澯
				SaveToMaintainLog("�¸澯", "",  &struSendPackage);
				InitAlarmPara(nNeId);
				//AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarm");//�Ƿ�Ƶ���澯

             }
             else if (strcmp(struOmcObj[i].szObjVal, "0") == 0)//ֻ��վ��ȷʵ���ڸ澯״̬�Żָ�
             {
                 sprintf(szTemp, "%s:1",  struOmcObj[i].szObjId);
                 if (strstr(szAlarmObjList, szTemp) != NULL)
                 {
                 	 bAlarmRestore = BOOLTRUE;
                 	 
                 	 if (AlarmComeback(pstruXml) == NORMAL)
                 		 strcat(szAlarmRestoreList, struAlarmPara.szAlarmName);//���ڸ澯�ָ�ǰת
             
             		 SaveToMaintainLog("�澯�ָ�", "",  &struSendPackage);
             		 InitAlarmPara(nNeId);
             		 //AlarmFrequent(struSendPackage.nMaintainLogId, "chkFrqAlarmClear");//Ƶ���澯���
             		 if (strlen(szAlarmRestoreList) > 0)
                 		 TransferAlarm();
                 }
                 
             }            
         }
         
         DeleteXml(pstruXml);
		 return NORMAL;
		
	}	

	
	////��ѯ���ô���

	//��ȡͨ�ű�ʶ��
	int nQB;

	sprintf(szTemp, "%lu%d%d", struCmdHead.udwRepId, struCmdHead.ubDevId, nCmdId);
	sprintf(szSql,"select comm_netflag from comm_packtoken where comm_repcmd = '%s' order by comm_netflag desc ", szTemp);
	PrintDebugLog(DBG_HERE,"ִ��SQL���[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"ִ��SQL���[%s]ʧ��,������ϢΪ[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"ִ��SQL���[%s]ʧ��,������ϢΪ[%s]\n",szSql,GetSQLErrorMessage());
		FreeCursor(&struCursor);
		return -1;
	}
	nQB = atoi(GetTableFieldValue(&struCursor,"comm_netflag"));
	FreeCursor(&struCursor);
	
	//ɾ����ʱ��ͨ�ű�ʶ��
	sprintf(szSql,"delete from comm_packtoken where comm_netflag= %d ",nQB);
	PrintDebugLog(DBG_HERE,"ִ��SQL���[%s]\n",szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"ִ��SQL���[%s]ʧ��,������Ϣ[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();

	//����QB+RepeaterId��ȡԭ����Ϣ��ɾ����Ӧ��ˮ�ż�¼
	if(GetSendPackageInfo(nQB, struCmdHead.udwRepId, &struSendPackage) != NORMAL)
    {
		PrintErrorLog(DBG_HERE, "����QB+RepeaterId��ȡQA����\n");
		return EXCEPTION; 
	}

	
	//����������
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
			DecodeQryElementParam((SENDPACKAGE *)&struSendPackage);//������ѯ
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
															
			DecodeSetElementParam((SENDPACKAGE *)&struSendPackage);//��������	
			break;
		default:
			break;
	}		
	
	return NORMAL;
}

/* 
 * ����2GЭ�飬������Ӧ����
 * pszUndecode ��������
 * szTelephone �ֻ�����
 * szNetCenterNum �ط���
 */
RESULT DecodeAndProcessSms(PSTR pszUndecode,PSTR pszTelephone,PSTR pszNetCenterNum)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	int nProctlType;
	
	PrintDebugLog(DBG_HERE, "������[%s][%s]\n", pszTelephone, pszUndecode);
	
	TBINDVARSTRU struBindVar;
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_STR; 
	strcpy(struBindVar.struBind[0].VarValue.szValueChar, pszTelephone);
	struBindVar.nVarCount++;
	
	sprintf(szSql,"select ne_ProtocoltypeId from ne_Element where ne_NeTelNum = :v_0");
	PrintDebugLog(DBG_HERE,"ִ��SQL���[%s][%s]\n",szSql, pszTelephone);
	if(BindSelectTableRecord(szSql,&struCursor, &struBindVar) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"ִ��SQL���[%s]ʧ��[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if (FetchCursor(&struCursor) == NORMAL)
	{
		nProctlType = atoi(GetTableFieldValue(&struCursor, "ne_ProtocoltypeId"));
	}
	else
	{
		PrintDebugLog(DBG_HERE,"���ݺ���û�ҵ�����¼,Ĭ�ϰ�2GЭ�鴦��!\n");
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
 * ����TCP/IPЭ�飬������Ӧ����
 * pszUndecode ��������
 * szTelephone �ֻ�����
 * szNetCenterNum �ط���
 */
RESULT DecodeAndProcessGprs(PSTR pszUndecode, INT nLen)
{
	int nProctlType, nField;
	PSTR pszSepStr[3];
	char szReqBuffer[MAX_BUFFER_LEN];
	
	strcpy(szReqBuffer, pszUndecode);

	nProctlType = PROTOCOL_2G;//Ĭ��
	
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
		PrintErrorLog(DBG_HERE, "CG��ͨ�Ų�֧��GPRS��ʽ\n");
	}
	
	return NORMAL;
}
	     
	     
	     
