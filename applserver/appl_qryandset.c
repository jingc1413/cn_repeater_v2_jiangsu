/*
 * 名称: 网管系统应用服务器(查询设置处理和解析)
 *
 * 修改记录:
 * 付志刚 -		2008-8-28 创建
 */
 
#include <ebdgdl.h>
#include <omcpublic.h>
#include <applserver.h>
#include <mobile2g.h>

static int n_gNeId;
static int n_gTaskId;
static int n_gTaskLogId;

/* 
 * 查询网元参数
 */
RESULT QryElementParam(INT nCommType, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater, PXMLSTRU pstruXml)
{
	PSTR pszSeperateStr[MAX_OBJECT_NUM];  /* 分割字符数组*/
	INT nObjCount=0, i, nSepCount;

	OBJECTSTRU struObjList[MAX_OBJECT_NUM];
	STR szMsgCont[500+1];
	STR szParam[1800];
	STR szTemp[100], szMapObject[1000];
	STR szDataType[20], szMapType[20], szObjOid[50], szMcpId[10];
    INT nDataLen, nTemp;
	
	//打包结果
	int n2GPack_Ret = 0;
	//int nMapIdCount = 0;
	REPEATERINFO stuRepeInfo;
	int n2G_QB, nEleQryLogId;
	BYTEARRAY struPack;

	//获取当前的2G协议流水号
	if (strcmp(getenv("DATABASE"), "mysql") == 0)
		n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   	else
   		n2G_QB = GetCurrent2GSquenue();
   	
   	sprintf(pstruHead->QA, "%d", nEleQryLogId);//strcpy(pstruHead->QA, Get2GNumber("Qry", n2G_QB));
   	
	if(pstruHead->nProtocolType==PROTOCOL_2G)
	{
		bufclr(szMapObject);
		strcpy(szParam, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象>"));
		nSepCount = SeperateString(szParam, ',',pszSeperateStr, MAX_SEPERATE_NUM);
	
		//将放入分割参数
		for(i=0; i< nSepCount; i++)
		{
			if (strlen(pszSeperateStr[i]) != 4) continue;
			memset(&struObjList[nObjCount], 0, sizeof(OBJECTSTRU));
			struObjList[nObjCount].MapID = strHexToInt(pszSeperateStr[i]);
			if (struObjList[nObjCount].MapID == 0x0606) //批采数据
			{
				struObjList[nObjCount].OC[0] = 0;
				struObjList[nObjCount].OC[1] = 0;
				nTemp = strHexToInt(DemandStrInXmlExt(pstruXml, "<omc>/<监控对象内容>"));
				struObjList[nObjCount].OC[2] = LOBYTE(nTemp);
				struObjList[nObjCount].OC[3] = HIBYTE(nTemp);
				struObjList[nObjCount].OL = 4;
			}
			else if (struObjList[nObjCount].MapID == 0x0875) //历史时隙
			{
				struObjList[nObjCount].OC[0] = 0;
				struObjList[nObjCount].OC[1] = 0;
				struObjList[nObjCount].OC[2] = atoi(DemandStrInXmlExt(pstruXml, "<omc>/<监控对象内容>"));
				struObjList[nObjCount].OC[3] = 0;
		    	//struObjList[nObjCount].OC[4] = 0;
				struObjList[nObjCount].OL = 4;
			}
			else
			{
				if (GetMapIdFromCache(pszSeperateStr[i], szDataType, &nDataLen) != NORMAL)
			    {
			        PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", pszSeperateStr[i]);
				    continue;
			    }
			    struObjList[nObjCount].OL = nDataLen;
			}
			sprintf(szMapObject, "%s%s,", szMapObject, pszSeperateStr[i]);
			nObjCount++;
			//PrintDebugLog(DBG_HERE, "监控编号[%04X]\n", struObjList[i].MapID);
		}
		TrimRightChar(szMapObject, ',');
		InsertInXmlExt(pstruXml,"<omc>/<监控对象>",szMapObject,MODE_AUTOGROW|MODE_UNIQUENAME);
		
		pstruHead->nObjectCount = nObjCount;
		
		memset(szMsgCont, 0, sizeof(szMsgCont));
		struPack.pPack = szMsgCont;
		struPack.Len = 0;
		
		memset(&stuRepeInfo, 0, sizeof(stuRepeInfo));				
		stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
		stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
		
		    	
	    PrintDebugLog(DBG_HERE, "CommType=[%d]DeviceId=[%d]RepeaterId=[%u]n2G_QB=[%d]nObjCount=[%d]\n", 
	        nCommType, stuRepeInfo.DeviceId, stuRepeInfo.RepeaterId,  n2G_QB, nObjCount);
        
        switch(pstruHead->nCommandCode)
		{
		    case COMMAND_QUERY: 
	             n2GPack_Ret= Encode_2G(nCommType, QUERYCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	             break;
	        case COMMAND_FCTPRM_QRY: 
	             n2GPack_Ret= Encode_2G(nCommType, FCTPRMQRY, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	             break;
	        case COMMAND_PRJPRM_QRY: 
	             n2GPack_Ret= Encode_2G(nCommType, PRJPRMQRY, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	             break;
	        default:
			     break;
	    }
	    //记录ne_qryandnet表，交给通信服务处理
	    sprintf(szTemp, "%d", nEleQryLogId);
	    InsertInXmlExt(pstruXml,"<omc>/<流水号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml, "<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    if (nCommType == M2G_TCPIP)
	    {
			if(!AscEsc(&struPack))
			{
			    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
				return -1;
			}
		}
	    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);

	}
	else if (pstruHead->nProtocolType == PROTOCOL_GSM || 
		pstruHead->nProtocolType == PROTOCOL_CDMA ||
		pstruHead->nProtocolType == PROTOCOL_HEIBEI ||
		pstruHead->nProtocolType == PROTOCOL_HEIBEI2 ||
		pstruHead->nProtocolType == PROTOCOL_XC_CP ||
		pstruHead->nProtocolType == PROTOCOL_SUNWAVE ||
		pstruHead->nProtocolType == PROTOCOL_WLK)
	{
		CMDHEAD struCmdHead;
		UBYTE ubOutPack[200];
		int nOutLen;
		
		switch(pstruHead->nProtocolType)
		{
			case PROTOCOL_GSM:
				struCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_CDMA:
				struCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_HEIBEI:
				struCmdHead.ubProVer = 0x03;
				break;	
			case PROTOCOL_HEIBEI2:
				struCmdHead.ubProVer = 0x03;
				break;	
			case PROTOCOL_XC_CP:
				struCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_SUNWAVE:
				struCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_WLK:
				struCmdHead.ubProVer = 0x01;
				break;	
			default:
				break;						
		}
		
		struCmdHead.ubPacNum = 1;
		struCmdHead.ubPacIndex = 1;
		struCmdHead.ubDevType = (UBYTE)(pstruRepeater->nProtocolDeviceType);
		struCmdHead.ubCmdId = (UBYTE)(pstruHead->nCommandCode);
		struCmdHead.udwRepId = (UDWORD)(pstruRepeater->nRepeaterId);
		struCmdHead.ubDevId = (UBYTE)(pstruRepeater->nDeviceId);
		struCmdHead.ubAnsFlag = 0;
		struCmdHead.ubCmdBodyLen = 0;
		
		Encode_GSM(&struCmdHead, NULL, ubOutPack, &nOutLen);
		
		PrintDebugLog(DBG_HERE, "协议[%d]查询命令[0x%X]\n", pstruHead->nProtocolType, struCmdHead.ubCmdId);
		PrintHexDebugLog("查询编码",  ubOutPack, nOutLen);
		
		//字节拆分
		struPack.pPack = ubOutPack;
		struPack.Len = nOutLen;
		if (ByteSplit(pstruHead->nProtocolType, &struPack, NULL) != NORMAL)
		{
			PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
			return -1;
		}
		PrintDebugLog(DBG_HERE, "转义编码[%s]\n", (char *)ubOutPack);	
		
		InsertInXmlExt(pstruXml,"<omc>/<消息内容>",(char *)ubOutPack,MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);

		//应用内部添加通信包标识号
		char szSql[MAX_BUFFER_LEN];
		
		sprintf(szTemp, "%lu%d%d", struCmdHead.udwRepId, struCmdHead.ubDevId, struCmdHead.ubCmdId);
		sprintf(szSql, "insert into comm_packtoken(comm_netflag, comm_repcmd, comm_eventtime) values(%d,  '%s', to_date( '%s','yyyy-mm-dd hh24:mi:ss'))",
			n2G_QB, szTemp, GetSysDateTime());
			
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
	        return EXCEPTION;
		}
		CommitTransaction();
		
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	else if (pstruHead->nProtocolType == PROTOCOL_SNMP)
	{
		bufclr(szMapObject);
		bufclr(szMsgCont);
		strcpy(szParam, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象>"));
		nSepCount = SeperateString(szParam, ',',pszSeperateStr, MAX_SEPERATE_NUM);
	
		//将放入分割参数
		for(i=0; i< nSepCount; i++)
		{
			//增加设备类型
			if (GetObjOidFromCache_Snmp(pszSeperateStr[i], pstruRepeater->nProtocolDeviceType, szDataType, szObjOid) != NORMAL)
		    {
		        PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", pszSeperateStr[i]);
			    continue;
		    }
			sprintf(szMapObject, "%s%s,", szMapObject, pszSeperateStr[i]);
			sprintf(szMsgCont, "%s%s;", szMsgCont, szObjOid);
			
		}
		TrimRightChar(szMapObject, ','); //逗号
		TrimRightChar(szMsgCont, ';'); //分号
		
		InsertInXmlExt(pstruXml,"<omc>/<监控对象>",szMapObject,MODE_AUTOGROW|MODE_UNIQUENAME);
		
		InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	else if (pstruHead->nProtocolType == PROTOCOL_DAS)
	{
		bufclr(szMapObject);
		strcpy(szParam, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象>"));
		nSepCount = SeperateString(szParam, ',',pszSeperateStr, MAX_SEPERATE_NUM);
	
		//将放入分割参数
		for(i=0; i< nSepCount; i++)
		{
			if (strlen(pszSeperateStr[i]) != 8) continue;
			memset(&struObjList[nObjCount], 0, sizeof(OBJECTSTRU));	
		    struObjList[nObjCount].MapID = strHexToInt(pszSeperateStr[i]);
			//判断时隙处理
			GetMapIdFromCache2(pszSeperateStr[i], szDataType, szMapType, &nDataLen);
			if (strcmp(szMapType, "ratedata") == 0)
			{
				struObjList[nObjCount].OC[0] = 0;
				struObjList[nObjCount].OC[1] = 0;
				struObjList[nObjCount].OC[2] = atoi(DemandStrInXmlExt(pstruXml, "<omc>/<监控对象内容>"));
				struObjList[nObjCount].OC[3] = 0;
		    	//struObjList[nObjCount].OC[4] = 0;
				struObjList[nObjCount].OL = 4;
			}
			else if (struObjList[nObjCount].MapID == 0x00000606) //批采数据
			{
				struObjList[nObjCount].OC[0] = 0;
				struObjList[nObjCount].OC[1] = 0;
				nTemp = strHexToInt(DemandStrInXmlExt(pstruXml, "<omc>/<监控对象内容>"));
				struObjList[nObjCount].OC[2] = LOBYTE(LOWORD(nTemp));
				struObjList[nObjCount].OC[3] = HIBYTE(LOWORD(nTemp));
				struObjList[nObjCount].OC[4] = LOBYTE(HIWORD(nTemp));
				struObjList[nObjCount].OC[5] = HIBYTE(HIWORD(nTemp));
				struObjList[nObjCount].OL = 6;
			}
		    else
		    {
				struObjList[nObjCount].OL = nDataLen;
				
			}
			sprintf(szMapObject, "%s%s,", szMapObject, pszSeperateStr[i]);
			nObjCount++;
			//PrintDebugLog(DBG_HERE, "监控编号[%04X]\n", struObjList[i].MapID);
		}
		TrimRightChar(szMapObject, ',');
		InsertInXmlExt(pstruXml,"<omc>/<监控对象>",szMapObject,MODE_AUTOGROW|MODE_UNIQUENAME);
		
		pstruHead->nObjectCount = nObjCount;
		
		memset(szMsgCont, 0, sizeof(szMsgCont));
		struPack.pPack = szMsgCont;
		struPack.Len = 0;
		
		memset(&stuRepeInfo, 0, sizeof(stuRepeInfo));				
		stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
		stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
		
		    	
	    PrintDebugLog(DBG_HERE, "CommType=[%d]DeviceId=[%d]RepeaterId=[%d]command=[%d]nObjCount=[%d]\n", 
	        nCommType, stuRepeInfo.DeviceId, stuRepeInfo.RepeaterId,  pstruHead->nCommandCode, nObjCount);
        
        switch(pstruHead->nCommandCode)
		{
		    case COMMAND_QUERY: 
	             n2GPack_Ret= Encode_Das(nCommType, QUERYCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	             break;
	        case COMMAND_FCTPRM_QRY: 
	             n2GPack_Ret= Encode_Das(nCommType, FCTPRMQRY, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	             break;
	        case COMMAND_PRJPRM_QRY: 
	             n2GPack_Ret= Encode_Das(nCommType, PRJPRMQRY, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	             break;
	        default:
			     break;
	    }
	    //记录ne_qryandnet表，交给通信服务处理
	    InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);
	    if (nCommType == M2G_TCPIP)
	    {
			if(!AscEsc(&struPack))
			{
			    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
				return -1;
			}
		}
	    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	else if (pstruHead->nProtocolType == PROTOCOL_JINXIN_DAS)
	{
		bufclr(szMapObject);
		strcpy(szParam, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象>"));
		nSepCount = SeperateString(szParam, ',',pszSeperateStr, MAX_SEPERATE_NUM);
	
		//将放入分割参数
		for(i=0; i< nSepCount; i++)
		{
		    memset(&struObjList[nObjCount], 0, sizeof(OBJECTSTRU));
			if (GetMapIdFromCache(pszSeperateStr[i], szDataType, &nDataLen) != NORMAL)
		    {
		        PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", pszSeperateStr[i]);
			    continue;
		    }

			struObjList[nObjCount].OL = nDataLen;
			if (strlen(pszSeperateStr[i]) == 8)
			{
				if (GetMcpIdFromParam(atoi(DemandStrInXmlExt(pstruXml,"<omc>/<网元编号>")), pszSeperateStr[i], szMcpId)==NORMAL)
				struObjList[nObjCount].MapID = strHexToInt(szMcpId);
				sprintf(szMapObject, "%s%s,", szMapObject, szMcpId);
				nObjCount++;
			}
			else
			{
				struObjList[nObjCount].MapID = strHexToInt(pszSeperateStr[i]);
				sprintf(szMapObject, "%s%s,", szMapObject, pszSeperateStr[i]);
				nObjCount++;
			}
			
			//PrintDebugLog(DBG_HERE, "监控编号[%04X]\n", struObjList[i].MapID);
		}
		TrimRightChar(szMapObject, ',');
		InsertInXmlExt(pstruXml,"<omc>/<监控对象>",szMapObject,MODE_AUTOGROW|MODE_UNIQUENAME);
		
		pstruHead->nObjectCount = nObjCount;
		
		memset(szMsgCont, 0, sizeof(szMsgCont));
		struPack.pPack = szMsgCont;
		struPack.Len = 0;
		
		memset(&stuRepeInfo, 0, sizeof(stuRepeInfo));				
		stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
		stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
		
		    	
	    PrintDebugLog(DBG_HERE, "CommType=[%d]DeviceId=[%d]RepeaterId=[%d]command=[%d]nObjCount=[%d]\n", 
	        nCommType, stuRepeInfo.DeviceId, stuRepeInfo.RepeaterId,  pstruHead->nCommandCode, nObjCount);
        
        switch(pstruHead->nCommandCode)
		{
		    case COMMAND_QUERY: 
	             n2GPack_Ret= Encode_2G(nCommType, QUERYCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	             break;
	        case COMMAND_FCTPRM_QRY: 
	             n2GPack_Ret= Encode_2G(nCommType, FCTPRMQRY, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	             break;
	        case COMMAND_PRJPRM_QRY: 
	             n2GPack_Ret= Encode_2G(nCommType, PRJPRMQRY, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	             break;
	        default:
			     break;
	    }
	    //记录ne_qryandnet表，交给通信服务处理
	    InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);
	    if (nCommType == M2G_TCPIP)
	    {
			if(!AscEsc(&struPack))
			{
			    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
				return -1;
			}
		}
	    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	}	
	else
	{
	    PrintErrorLog(DBG_HERE, "系统不支持该协议[%d]\n", pstruHead->nProtocolType);
		return EXCEPTION;
	}

    return NORMAL;
	
}




/* 
 * 设置网元参数
 */
RESULT SetElementParam(INT nCommType, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater, PXMLSTRU pstruXml)
{
    PSTR pszSepMapIdStr[MAX_OBJECT_NUM];  /* 分割监控对象数组*/
    PSTR pszSepMapDataStr[MAX_OBJECT_NUM];  /* 分割监控对象内容数组*/
	INT nObjCount=0,nDataCount, i, j, nSepCount;
	
	//SENDPACKAGE struSendPackage;		   /* 打包之前发送结构 */
	OBJECTSTRU struObjList[MAX_OBJECT_NUM];
	STR szMsgCont[MAX_BUFFER_LEN];
	STR szMapId[1800];
	STR szMapData[1800];
	STR szTemp[256+1], szSpecial[10];
	int nDataLen;
	STR szDataType[20], szMapType[20], szObjOid[50];
	
	//打包结果
	int n2GPack_Ret = 0;
	REPEATERINFO stuRepeInfo;
	int n2G_QB,nEleQryLogId;
	BYTEARRAY struPack;
	
	strcpy(szMapId, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象>"));
	strcpy(szMapData, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象内容>"));
	nSepCount = SeperateString(szMapId, ',', pszSepMapIdStr, MAX_SEPERATE_NUM);
	nDataCount = SeperateString(szMapData, ',', pszSepMapDataStr, MAX_SEPERATE_NUM);
	//特殊指令处理流水号
	strcpy(szSpecial, DemandStrInXmlExt(pstruXml, "<omc>/<流水号>"));
	//获取当前的2G协议流水号
	if (strcmp(getenv("DATABASE"), "mysql") == 0)
		n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   	else
		n2G_QB = GetCurrent2GSquenue();
	
   	sprintf(pstruHead->QA, "%d", nEleQryLogId);//strcpy(pstruHead->QA, Get2GNumber("Set", n2G_QB));
   	
   	if(pstruHead->nProtocolType == PROTOCOL_2G)
   	{	
		//将放入分割参数
		for(i=0; i< nSepCount; i++)
		{
			if (strlen(pszSepMapIdStr[i]) != 4) continue;
		    memset(&struObjList[nObjCount], 0, sizeof(OBJECTSTRU));
	
			struObjList[nObjCount].MapID = strHexToInt(pszSepMapIdStr[i]);
			nDataLen = EncodeMapDataFromMapId(pszSepMapIdStr[i], pszSepMapDataStr[i], struObjList[nObjCount].OC);
			struObjList[nObjCount].OL = nDataLen;
			
			if (struObjList[nObjCount].MapID == 0x0AFF)//进入工厂模式
		    {
		    	memset(szMapData, 0, sizeof(szMapData));
		    	memcpy(szMapData, struObjList[nObjCount].OC, nDataLen);
		    	struObjList[nObjCount].OC[0] = 0x5A;
		    	for(j=1; j< nDataLen; j++)
		    	{
		    		struObjList[nObjCount].OC[j]= szMapData[j-1]^0xA5;
		    	}
		    	struObjList[i].OL = 9;
		    }
		    if (struObjList[nObjCount].MapID == 0x0A01) //取日志实际长度为5
		    {
		    	memset(szMapData, 0, sizeof(szMapData));
		    	memcpy(szMapData, struObjList[nObjCount].OC, nDataLen);
		    	if (strcmp(szMapData, "FF") == 0)
		        {
		    		struObjList[nObjCount].OC[0] = strHexToInt("FF");
		    		struObjList[nObjCount].OL = 1;
		    	}
		    	else
		    	{
		    		bufclr(szTemp);
		    		strncpy(szTemp, szMapData, 1);
	         	    struObjList[nObjCount].OC[0] = strHexToInt(szTemp);
	         		strncpy(szTemp, szMapData+1, 2);
	         		struObjList[nObjCount].OC[1] = strHexToInt(szTemp);
	         		strncpy(szTemp, szMapData+3, 2);
	         		struObjList[nObjCount].OC[2] = strHexToInt(szTemp);
	         		strncpy(szTemp, szMapData+5, 2);
	         		struObjList[nObjCount].OC[3] = strHexToInt(szTemp);
	         		strncpy(szTemp, szMapData+7, 2);
	         		struObjList[nObjCount].OC[4] = strHexToInt(szTemp);
	         		struObjList[nObjCount].OL = 5;
		    	}
		    	
		    }
		    if (struObjList[nObjCount].MapID == 0x0606) //批采数据
			{
				struObjList[nObjCount].OC[0] = 0;
				struObjList[nObjCount].OC[1] = 0;
				struObjList[nObjCount].OC[2] = 0;
				struObjList[nObjCount].OC[3] = 0;
				struObjList[nObjCount].OL = 4;
			}
			if (struObjList[nObjCount].MapID == 0x0875) //历史时隙
			{
				struObjList[nObjCount].OC[0] = 0;
				struObjList[nObjCount].OC[1] = 0;
				struObjList[nObjCount].OC[2] = atoi(pszSepMapDataStr[i]);
				struObjList[nObjCount].OC[3] = 0;
		    	struObjList[nObjCount].OC[4] = 0;
				struObjList[nObjCount].OL = 5;
			}
			nObjCount++;	
		}
		memset(szMsgCont, 0, sizeof(szMsgCont));
		struPack.pPack = szMsgCont;
		struPack.Len = 0;
		memset(&stuRepeInfo, 0, sizeof(stuRepeInfo));

		{
			pstruHead->nObjectCount = nObjCount;
			stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
			stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
			
			    	
		    PrintDebugLog(DBG_HERE, "DeviceId[%d]\nRepeaterId[%d]\nn2G_QB[%d]\nnObjCount[%d]\n", stuRepeInfo.DeviceId,
		               stuRepeInfo.RepeaterId,  n2G_QB, nObjCount);
	        switch(pstruHead->nCommandCode)
			{
			    case COMMAND_SET: 
		             n2GPack_Ret= Encode_2G(nCommType, SETCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        case COMMAND_FCTPRM_SET: 
		             n2GPack_Ret= Encode_2G(nCommType, FCTPRMSET, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        case COMMAND_PRJPRM_SET: 
		             n2GPack_Ret= Encode_2G(nCommType, PRJPRMSET, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        case COMMAND_FACTORY_MODE:
		             n2GPack_Ret= Encode_2G(nCommType, FACTORYMODE, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        default:
				     break;
		    }
		    
		    //记录ne_qryandnet表，交给通信服务处理
		    sprintf(szTemp, "%d", nEleQryLogId);
	    	InsertInXmlExt(pstruXml,"<omc>/<流水号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    	InsertInXmlExt(pstruXml, "<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		    if (nCommType == M2G_TCPIP)
		    {
				if(!AscEsc(&struPack))
				{
				    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
					return -1;
				}
			}
		    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
		    sprintf(szTemp, "%d", n2G_QB);
		    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	
		}
	}
	else if (pstruHead->nProtocolType == PROTOCOL_GSM || 
		pstruHead->nProtocolType == PROTOCOL_CDMA ||
		pstruHead->nProtocolType == PROTOCOL_HEIBEI ||
		pstruHead->nProtocolType == PROTOCOL_HEIBEI2 ||
		pstruHead->nProtocolType == PROTOCOL_XC_CP ||
		pstruHead->nProtocolType == PROTOCOL_SUNWAVE ||
		pstruHead->nProtocolType == PROTOCOL_WLK)
	{
		char szObjList[MAX_OBJ_LEN];
		OMCOBJECT struOmcObject[MAX_OBJ_NUM];
		UBYTE ubCmdBody[200];
		int nCmdBodyMaxLen;
		UBYTE ubCmdBodyLen;
		int res;
		
		CMDHEAD struCmdHead;
		UBYTE ubOutPack[200];
		int nOutLen;
		
		for(i=0; i< nSepCount; i++)
		{
			strcpy(struOmcObject[i].szObjId, pszSepMapIdStr[i]);
			strcpy(struOmcObject[i].szObjVal, pszSepMapDataStr[i]);
			nObjCount++;
		}
		strcpy(szObjList, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象>"));
		nCmdBodyMaxLen = sizeof(ubCmdBody);
	
		res = EncodeCmdBodyFromCmdId(pstruHead->nProtocolType, szObjList, struOmcObject, nObjCount,ubCmdBody, nCmdBodyMaxLen, &ubCmdBodyLen);
		if (res == -1)
		{
			PrintErrorLog(DBG_HERE, "协议命令体编码出错!\n");
			return EXCEPTION;
		}
				
		switch(pstruHead->nProtocolType)
		{
			case PROTOCOL_GSM:
				struCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_CDMA:
				struCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_HEIBEI:
				struCmdHead.ubProVer = 0x03;
				break;	
			case PROTOCOL_HEIBEI2:
				struCmdHead.ubProVer = 0x03;
				break;	
			case PROTOCOL_XC_CP:
				struCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_SUNWAVE:
				struCmdHead.ubProVer = 0x02;
				break;	
			case PROTOCOL_WLK:
				struCmdHead.ubProVer = 0x01;
				break;	
			default:
				break;						
		}
			
		struCmdHead.ubPacNum = 1;
		struCmdHead.ubPacIndex = 1;
		struCmdHead.ubDevType = (UBYTE)(pstruRepeater->nProtocolDeviceType);
		struCmdHead.ubCmdId = (UBYTE)(pstruHead->nCommandCode);
		struCmdHead.udwRepId = (UDWORD)(pstruRepeater->nRepeaterId);
		struCmdHead.ubDevId = (UBYTE)(pstruRepeater->nDeviceId);
		struCmdHead.ubAnsFlag = 0;
		struCmdHead.ubCmdBodyLen = ubCmdBodyLen;
		
		Encode_GSM(&struCmdHead, ubCmdBody, ubOutPack, &nOutLen);
		
		PrintDebugLog(DBG_HERE, "协议[%d]设置命令[0x%X]\n", pstruHead->nProtocolType, struCmdHead.ubCmdId);
		//PrintHexDebugLog("设置编码",  ubOutPack, nOutLen);
		
		//字节拆分
		struPack.pPack = ubOutPack;
		struPack.Len = nOutLen;
		if (ByteSplit(pstruHead->nProtocolType, &struPack, szObjList) != NORMAL)
		{
			PrintErrorLog(DBG_HERE,"转义错误\n");
			return -1;
		}
		PrintDebugLog(DBG_HERE, "转义编码[%s]\n", (char *)ubOutPack);		
		
		InsertInXmlExt(pstruXml,"<omc>/<消息内容>",(char *)ubOutPack,MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);

		//应用内部添加通信包标识号
		char szSql[MAX_BUFFER_LEN];
		
		sprintf(szTemp, "%lu%d%d", struCmdHead.udwRepId, struCmdHead.ubDevId, struCmdHead.ubCmdId);
		sprintf(szSql, "insert into comm_packtoken(comm_netflag, comm_repcmd, comm_eventtime) values(%d,  '%s', to_date( '%s','yyyy-mm-dd hh24:mi:ss'))",
			n2G_QB, szTemp, GetSysDateTime());
			
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
	        return EXCEPTION;
		}
		CommitTransaction();
		
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	else if (pstruHead->nProtocolType == PROTOCOL_SNMP)
	{
		memset(szMsgCont, 0, sizeof(szMsgCont));
		//将放入分割参数
		for(i=0; i< nSepCount; i++)
		{
			PrintDebugLog(DBG_HERE, "系统[%s]监控量\n", pszSepMapIdStr[i]);
			
			if (GetObjOidFromCache_Snmp(pszSepMapIdStr[i], pstruRepeater->nProtocolDeviceType, szDataType, szObjOid) != NORMAL)
		    {
		        PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", pszSepMapIdStr[i]);
			    continue;
		    }
			if (strncmp(szDataType, "sint", 4) == 0 || strncmp(szDataType, "uint", 4) == 0)
				sprintf(szMsgCont, "%s%s i %s;", szMsgCont, szObjOid, pszSepMapDataStr[i]);
			//else if (strncmp(szDataType, "uint", 4) == 0)
			//	sprintf(szMsgCont, "%s%s u %s;", szMsgCont, szObjOid, pszSepMapDataStr[i]);
			else if (strncmp(szDataType, "IP", 2) == 0)
				sprintf(szMsgCont, "%s%s a %s;", szMsgCont, szObjOid, pszSepMapDataStr[i]);
			else
				sprintf(szMsgCont, "%s%s s %s;", szMsgCont, szObjOid, pszSepMapDataStr[i]);
			
		}
		TrimRightChar(szMsgCont, ';');
		
		InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	else if(pstruHead->nProtocolType == PROTOCOL_DAS)
   	{	
		//将放入分割参数
		
		for(i=0; i< nSepCount; i++)
		{
			if (strlen(pszSepMapIdStr[i]) != 8) continue;
		    
			memset(&struObjList[nObjCount], 0, sizeof(OBJECTSTRU));
			struObjList[nObjCount].MapID = strHexToInt(pszSepMapIdStr[i]);
			nDataLen = EncodeMapDataFromMapId(pszSepMapIdStr[i], pszSepMapDataStr[i], struObjList[nObjCount].OC);
			struObjList[nObjCount].OL = nDataLen;
			
			//判断时隙处理
			GetMapIdFromCache2(pszSepMapIdStr[i], szDataType, szMapType, &nDataLen);
			if (strcmp(szMapType, "ratedata") == 0)
			{
				struObjList[nObjCount].OC[0] = 0;
				struObjList[nObjCount].OC[1] = 0;
				struObjList[nObjCount].OC[2] = atoi(pszSepMapDataStr[i]);
				struObjList[nObjCount].OC[3] = 0;
		    	struObjList[nObjCount].OC[4] = 0;
				struObjList[nObjCount].OL = 5;
			}
			if (struObjList[nObjCount].MapID == 0x00000606) //批采数据
			{
				struObjList[nObjCount].OC[0] = 0;
				struObjList[nObjCount].OC[1] = 0;
				struObjList[nObjCount].OC[2] = 0;
				struObjList[nObjCount].OC[3] = 0;
				struObjList[nObjCount].OC[4] = 0;
				struObjList[nObjCount].OC[5] = 0;
				struObjList[nObjCount].OL = 6;
			}
			nObjCount++;
		}
		memset(szMsgCont, 0, sizeof(szMsgCont));
		struPack.pPack = szMsgCont;
		struPack.Len = 0;
		memset(&stuRepeInfo, 0, sizeof(stuRepeInfo));

		{
			pstruHead->nObjectCount = nObjCount;
			stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
			stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
			
			    	
		    PrintDebugLog(DBG_HERE, "DeviceId[%d]\nRepeaterId[%d]\nn2G_QB[%d]\nnObjCount[%d]\n", stuRepeInfo.DeviceId,
		               stuRepeInfo.RepeaterId,  n2G_QB, nObjCount);
	        switch(pstruHead->nCommandCode)
			{
			    case COMMAND_SET: 
		             n2GPack_Ret= Encode_Das(nCommType, SETCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        case COMMAND_FCTPRM_SET: 
		             n2GPack_Ret= Encode_Das(nCommType, FCTPRMSET, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        case COMMAND_PRJPRM_SET: 
		             n2GPack_Ret= Encode_Das(nCommType, PRJPRMSET, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        case COMMAND_FACTORY_MODE:
		             n2GPack_Ret= Encode_Das(nCommType, FACTORYMODE, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        default:
				     break;
		    }
		    
		    //记录ne_qryandnet表，交给通信服务处理
		    InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);
		    if (nCommType == M2G_TCPIP)
		    {
				if(!AscEsc(&struPack))
				{
				    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
					return -1;
				}
			}
		    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
		    sprintf(szTemp, "%d", n2G_QB);
		    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	
		}
	}
	else if(pstruHead->nProtocolType == PROTOCOL_JINXIN_DAS)
   	{	
		//将放入分割参数
		for(i=0; i< nSepCount; i++)
		{
		    memset(&struObjList[nObjCount], 0, sizeof(OBJECTSTRU));
		    
			if (strlen(pszSepMapIdStr[i]) == 8)
			{
				memset(szTemp,0,sizeof(szTemp));
				if (GetMcpIdFromParam(atoi(DemandStrInXmlExt(pstruXml,"<omc>/<网元编号>")), pszSepMapIdStr[i], szTemp)!= NORMAL) 
					continue;
				struObjList[nObjCount].MapID = strHexToInt(szTemp);
			}
			else
			{
				struObjList[nObjCount].MapID = strHexToInt(pszSepMapIdStr[i]);
			}
			//struObjList[i].MapID = strHexToInt(pszSepMapIdStr[i]);
			nDataLen = EncodeMapDataFromMapId(pszSepMapIdStr[i], pszSepMapDataStr[i], struObjList[nObjCount].OC);
			struObjList[nObjCount].OL = nDataLen;
			nObjCount++;
		}
		memset(szMsgCont, 0, sizeof(szMsgCont));
		struPack.pPack = szMsgCont;
		struPack.Len = 0;
		memset(&stuRepeInfo, 0, sizeof(stuRepeInfo));

		{
			
			pstruHead->nObjectCount = nObjCount;
			stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
			stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
			
			    	
		    PrintDebugLog(DBG_HERE, "DeviceId[%d]\nRepeaterId[%d]\nn2G_QB[%d]\nnObjCount[%d]\n", stuRepeInfo.DeviceId,
		               stuRepeInfo.RepeaterId,  n2G_QB, nObjCount);
	        switch(pstruHead->nCommandCode)
			{
			    case COMMAND_SET: 
		             n2GPack_Ret= Encode_2G(nCommType, SETCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        case COMMAND_FCTPRM_SET: 
		             n2GPack_Ret= Encode_2G(nCommType, FCTPRMSET, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        case COMMAND_PRJPRM_SET: 
		             n2GPack_Ret= Encode_2G(nCommType, PRJPRMSET, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        case COMMAND_FACTORY_MODE:
		             n2GPack_Ret= Encode_2G(nCommType, FACTORYMODE, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
		             break;
		        default:
				     break;
		    }
		    
		    //记录ne_qryandnet表，交给通信服务处理
		    InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);
		    if (nCommType == M2G_TCPIP)
		    {
				if(!AscEsc(&struPack))
				{
				    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
					return -1;
				}
			}
		    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
		    sprintf(szTemp, "%d", n2G_QB);
		    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	
		}
	}	
	else	
	{
	    PrintErrorLog(DBG_HERE, "系统不支持该协议[%d]\n", pstruHead->nProtocolType);
		return EXCEPTION;
	}
	
	return NORMAL;
}

/* 
 * 查询监控量参数
 */
RESULT QueryMapList(INT nCommType, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater, PXMLSTRU pstruXml)
{
	INT i;

	OBJECTSTRU struObjList[MAX_OBJECT_NUM];
	STR szMsgCont[500+1];
	
	//打包结果
	int n2GPack_Ret = 0;
	REPEATERINFO stuRepeInfo;
	int n2G_QB,nEleQryLogId;
	BYTEARRAY struPack;
	INT nObjCount =1;
	STR szTemp[10];
			
	pstruHead->nObjectCount = 1;
    pstruHead->nCommandCode = COMMAND_QUERY;
	//获取当前的2G协议流水号
	if (strcmp(getenv("DATABASE"), "mysql") == 0)
		n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   	else
   		n2G_QB = GetCurrent2GSquenue();
   		
   	sprintf(pstruHead->QA, "%d", nEleQryLogId); //strcpy(pstruHead->QA, Get2GNumber("Gprs", n2G_QB));

   		
	//将放入分割参数
	for(i=0; i< pstruHead->nObjectCount; i++)
	{
	    memset(&struObjList[i], 0, sizeof(OBJECTSTRU));
		struObjList[i].MapID = 0x0009;
		struObjList[i].OC[0] = 0x01;
		struObjList[i].OC[1] = 0x01;
		struObjList[i].OL = 2;
	}
	
	memset(szMsgCont, 0, sizeof(szMsgCont));
	struPack.pPack = szMsgCont;
	struPack.Len = 0;
	memset(&stuRepeInfo, 0, sizeof(stuRepeInfo));
	if(pstruHead->nProtocolType==PROTOCOL_2G ||
	   pstruHead->nProtocolType==PROTOCOL_JINXIN_DAS)
	{
		stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
		stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
		
		    	
	    PrintDebugLog(DBG_HERE, "DeviceId[%d]\nRepeaterId[%d]\nn2G_QB[%d]\nnObjCount[%d]\n", stuRepeInfo.DeviceId,
	               stuRepeInfo.RepeaterId,  n2G_QB, nObjCount);

	    n2GPack_Ret= Encode_2G(nCommType, QUERYCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	    if (nCommType == M2G_TCPIP)
	    {
			if(!AscEsc(&struPack))
			{
			    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
				return -1;
			}
		}
	    //记录ne_qryandnet表，交给通信服务处理
	    sprintf(szTemp, "%d", nEleQryLogId);
	    InsertInXmlExt(pstruXml,"<omc>/<流水号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml, "<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<监控对象>","0009",MODE_AUTOGROW|MODE_UNIQUENAME);

	}
	else if(pstruHead->nProtocolType==PROTOCOL_DAS)
	{
		memset(&struObjList[0], 0, sizeof(OBJECTSTRU));
		struObjList[0].MapID = 0x00000009;
		struObjList[0].OC[0] = 0x01;
		struObjList[0].OC[1] = 0x01;
		struObjList[0].OL = 2;
		
		stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
		stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
		
		    	
	    PrintDebugLog(DBG_HERE, "DeviceId[%d]\nRepeaterId[%d]\nn2G_QB[%d]\nnObjCount[%d]\n", stuRepeInfo.DeviceId,
	               stuRepeInfo.RepeaterId,  n2G_QB, nObjCount);

	    n2GPack_Ret= Encode_Das(nCommType, QUERYCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	    if (nCommType == M2G_TCPIP)
	    {
			if(!AscEsc(&struPack))
			{
			    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
				return -1;
			}
		}
	    //记录ne_qryandnet表，交给通信服务处理
	    InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<监控对象>","00000009",MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	else
	{
	    PrintErrorLog(DBG_HERE, "系统不支持该协议[%d]\n", pstruHead->nProtocolType);
		return EXCEPTION;
	}
	return NORMAL;
}

/* 
 * 查询监控量参数
 */
RESULT QueryMap0001List(INT nCommType, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater, PXMLSTRU pstruXml)
{
	INT i;

	OBJECTSTRU struObjList[MAX_OBJECT_NUM];
	STR szMsgCont[500+1];
	
	//打包结果
	int n2GPack_Ret = 0;
	REPEATERINFO stuRepeInfo;
	int n2G_QB,nEleQryLogId;
	BYTEARRAY struPack;
	INT nObjCount =1;
	STR szTemp[10];
			
	pstruHead->nObjectCount = 1;
    pstruHead->nCommandCode = COMMAND_QUERY;
	//获取当前的2G协议流水号
	if (strcmp(getenv("DATABASE"), "mysql") == 0)
		n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   	else
   		n2G_QB = GetCurrent2GSquenue();

   	sprintf(pstruHead->QA, "%d", nEleQryLogId);//strcpy(pstruHead->QA, Get2GNumber("Gprs", n2G_QB));

   		
	//将放入分割参数
	for(i=0; i< pstruHead->nObjectCount; i++)
	{
	    memset(&struObjList[i], 0, sizeof(OBJECTSTRU));
		struObjList[i].MapID = 0x0001;
		struObjList[i].OC[0] = 0x01;
		struObjList[i].OC[1] = 0x01;
		struObjList[i].OL = 2;
	}
	
	memset(szMsgCont, 0, sizeof(szMsgCont));
	struPack.pPack = szMsgCont;
	struPack.Len = 0;
	memset(&stuRepeInfo, 0, sizeof(stuRepeInfo));
	if(pstruHead->nProtocolType==PROTOCOL_2G ||
	   pstruHead->nProtocolType==PROTOCOL_JINXIN_DAS)
	{
		stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
		stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
		
		    	
	    PrintDebugLog(DBG_HERE, "DeviceId[%d]\nRepeaterId[%d]\nn2G_QB[%d]\nnObjCount[%d]\n", stuRepeInfo.DeviceId,
	               stuRepeInfo.RepeaterId,  n2G_QB, nObjCount);

	    n2GPack_Ret= Encode_2G(nCommType, QUERYCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	    if (nCommType == M2G_TCPIP)
	    {
			if(!AscEsc(&struPack))
			{
			    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
				return -1;
			}
		}
	    //记录ne_qryandnet表，交给通信服务处理
	    sprintf(szTemp, "%d", nEleQryLogId);
	    InsertInXmlExt(pstruXml,"<omc>/<流水号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml, "<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<监控对象>","0001",MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);

	}
	
	else
	{
	    PrintErrorLog(DBG_HERE, "系统不支持该协议[%d]\n", pstruHead->nProtocolType);
		return EXCEPTION;
	}
	return NORMAL;
}
/* 
 * 查询das拓扑结构
 */
RESULT QueryDasList(INT nCommType, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater, PXMLSTRU pstruXml)
{

	OBJECTSTRU struObjList[MAX_OBJECT_NUM];
	STR szMsgCont[500+1];
	
	//打包结果
	int n2GPack_Ret = 0;
	REPEATERINFO stuRepeInfo;
	int n2G_QB,nEleQryLogId;
	BYTEARRAY struPack;
	INT nObjCount =1;
	STR szTemp[10];

			
	pstruHead->nObjectCount = 1;
    pstruHead->nCommandCode = COMMAND_QUERY;
	//获取当前的2G协议流水号
	if (strcmp(getenv("DATABASE"), "mysql") == 0)
		n2G_QB = Get2GSerial("Mobile2G", &nEleQryLogId);//GetDbSerial(&n2G_QB, "Mobile2G");
   	else
   		n2G_QB = GetCurrent2GSquenue();

   	sprintf(pstruHead->QA, "%d", nEleQryLogId);//strcpy(pstruHead->QA, Get2GNumber("Gprs", n2G_QB));

   	
   	//strcpy(szSpecialCode, pstruRepeater->szSpecialCode);
   	memset(&struObjList[0], 0, sizeof(OBJECTSTRU));
	struObjList[0].MapID = 0x0AEC;
	struObjList[0].OC[0] = 0x01;
	struObjList[0].OC[1] = 0x01;
	struObjList[0].OC[2] = 0x06;
	struObjList[0].OC[3] = 0x01;
	struObjList[0].OC[4] = 0x02;
	struObjList[0].OC[5] = 0x03;
	struObjList[0].OC[6] = 0x04;
	struObjList[0].OC[7] = 0x06;
	struObjList[0].OC[8] = 0x07;
	//struObjList[0].OC[9] = 0x08;
	struObjList[0].OL = 9;
    /*
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
	
	memset(szMsgCont, 0, sizeof(szMsgCont));
	struPack.pPack = szMsgCont;
	struPack.Len = 0;
	memset(&stuRepeInfo, 0, sizeof(stuRepeInfo));
	if(pstruHead->nProtocolType==PROTOCOL_2G)
	{
		stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
		stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
		
		    	
	    PrintDebugLog(DBG_HERE, "DeviceId[%d]\nRepeaterId[%d]\nn2G_QB[%d]\nnObjCount[%d]\n", stuRepeInfo.DeviceId,
	               stuRepeInfo.RepeaterId,  n2G_QB, nObjCount);

	    n2GPack_Ret= Encode_2G(nCommType, QUERYCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	    if (nCommType == M2G_TCPIP)
	    {
			if(!AscEsc(&struPack))
			{
			    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
				return -1;
			}
		}
	    //记录ne_qryandnet表，交给通信服务处理
	    sprintf(szTemp, "%d", nEleQryLogId);
	    InsertInXmlExt(pstruXml,"<omc>/<流水号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml, "<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
		
		DeleDasList(stuRepeInfo.RepeaterId);
	}
	else
	{
		strcpy(szMsgCont, "1.3.6.1.4.1.31731.6.1.1");
	    InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
		
		DeleDasList(stuRepeInfo.RepeaterId);
	}
	return NORMAL;
}

/* 
 * 查询rfid拓扑结构
 */
RESULT QueryRfidList(INT nCommType, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater, PXMLSTRU pstruXml)
{
	INT i;

	OBJECTSTRU struObjList[MAX_OBJECT_NUM];
	STR szMsgCont[500+1];
	
	//打包结果
	int n2GPack_Ret = 0;
	REPEATERINFO stuRepeInfo;
	int n2G_QB,nEleQryLogId;
	BYTEARRAY struPack;
	INT nObjCount =1;
	STR szTemp[10];
			
	pstruHead->nObjectCount = 1;
	pstruHead->nProtocolType = PROTOCOL_2G;
    pstruHead->nCommandCode = COMMAND_QUERY;
	//获取当前的2G协议流水号
	if (strcmp(getenv("DATABASE"), "mysql") == 0)
		GetDbSerial(&n2G_QB, "Mobile2G");
   	else
   		n2G_QB = GetCurrent2GSquenue();

   	sprintf(pstruHead->QA, "%d", nEleQryLogId);//	strcpy(pstruHead->QA, Get2GNumber("Gprs", n2G_QB));

   		
	//将放入分割参数
	for(i=0; i< pstruHead->nObjectCount; i++)
	{
	    memset(&struObjList[i], 0, sizeof(OBJECTSTRU));
		struObjList[i].MapID = 0x0AED;
		struObjList[0].OC[0] = 0x01;
		struObjList[0].OC[1] = 0x01;
		struObjList[0].OC[2] = 0x03;
		struObjList[0].OC[3] = 0x01;
		struObjList[0].OC[4] = 0x02;
		struObjList[0].OC[5] = 0x03;
		struObjList[0].OL = 6;
	}
	
	memset(szMsgCont, 0, sizeof(szMsgCont));
	struPack.pPack = szMsgCont;
	struPack.Len = 0;
	memset(&stuRepeInfo, 0, sizeof(stuRepeInfo));
	if(pstruHead->nProtocolType==PROTOCOL_2G)
	{
		stuRepeInfo.DeviceId= pstruRepeater->nDeviceId;
		stuRepeInfo.RepeaterId= pstruRepeater->nRepeaterId;
		
		    	
	    PrintDebugLog(DBG_HERE, "DeviceId[%d]\nRepeaterId[%d]\nn2G_QB[%d]\nnObjCount[%d]\n", stuRepeInfo.DeviceId,
	               stuRepeInfo.RepeaterId,  n2G_QB, nObjCount);

	    n2GPack_Ret= Encode_2G(nCommType, QUERYCOMMAND, n2G_QB, &stuRepeInfo, struObjList, nObjCount, &struPack);
	    if (nCommType == M2G_TCPIP)
	    {
			if(!AscEsc(&struPack))
			{
			    PrintErrorLog(DBG_HERE,"转义错误[%s]\n",struPack.pPack);
				return -1;
			}
		}
	    //记录ne_qryandnet表，交给通信服务处理
	    InsertInXmlExt(pstruXml,"<omc>/<流水号>", pstruHead->QA, MODE_AUTOGROW|MODE_UNIQUENAME);
	    InsertInXmlExt(pstruXml,"<omc>/<消息内容>",szMsgCont,MODE_AUTOGROW|MODE_UNIQUENAME);
	    sprintf(szTemp, "%d", n2G_QB);
	    InsertInXmlExt(pstruXml,"<omc>/<序列号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
		
		DeleRfidList(stuRepeInfo.RepeaterId, stuRepeInfo.DeviceId);
	}
	else
	{
	    PrintErrorLog(DBG_HERE, "系统不支持该协议[%d]\n", pstruHead->nProtocolType);
		return EXCEPTION;
	}
	return NORMAL;
}
/*
 *用于查询所有监控量方法
 *
 */
RESULT QryEleFristTime(int nNeId, int nCommType)
{
    STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	REPEATER_INFO struRepeater;
	COMMANDHEAD struHead;
	STR szNeObjectList[MAX_BUFFER_LEN];
	//STR szObjList[2000];
	PSTR pszSepParamStr[MAX_OBJECT_NUM];  /* 分割字符数组*/
	INT nObjCount, i;

	sprintf(szSql,"select ne_neid, ne_CommTypeId,ne_RepeaterId, ne_DeviceId, ne_NeTelNum,ne_DeviceIp, ne_DevicePort, ne_ProtocoltypeId,ne_ProtocolDeviceTypeId, ne_DeviceModelId, ne_ServerTelNum,ne_ActiveCol from ne_Element where ne_neid = %d", nNeId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if (FetchCursor(&struCursor) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE,"执行SQL语句[%s] 失败[%s]\n",szSql,GetSQLErrorMessage());
		FreeCursor(&struCursor);
		return EXCEPTION;
	}
	memset(pstruXml,0,sizeof(XMLSTRU));
	CreateXml(pstruXml,FALSE,OMC_ROOT_PATH,NULL);
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
	               
	InsertInXmlExt(pstruXml,"<omc>/<站点IP>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_DeviceIp")),
	               MODE_AUTOGROW|MODE_UNIQUENAME);
	InsertInXmlExt(pstruXml,"<omc>/<端口号>", GetTableFieldValue(&struCursor, "ne_DevicePort"),
	               MODE_AUTOGROW|MODE_UNIQUENAME);
	               
	InsertInXmlExt(pstruXml,"<omc>/<协议类型>", GetTableFieldValue(&struCursor, "ne_ProtocoltypeId"),
	               MODE_AUTOGROW|MODE_UNIQUENAME);
	InsertInXmlExt(pstruXml,"<omc>/<设备类型>", GetTableFieldValue(&struCursor, "ne_ProtocolDeviceTypeId"),
	               MODE_AUTOGROW|MODE_UNIQUENAME);
	//InsertInXmlExt(pstruXml,"<omc>/<其它标识>", GetTableFieldValue(&struCursor, "ne_OtherDeviceId"),
	//               MODE_AUTOGROW|MODE_UNIQUENAME);
	
	InsertInXmlExt(pstruXml,"<omc>/<设备型号>", GetTableFieldValue(&struCursor, "ne_DeviceModelId"),
	               MODE_AUTOGROW|MODE_UNIQUENAME);
	//服务号码
	InsertInXmlExt(pstruXml,"<omc>/<服务号码>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_ServerTelNum")),
	               MODE_AUTOGROW|MODE_UNIQUENAME);
	//命令暂时为查询:COMMAND_QUERY
    InsertInXmlExt(pstruXml,"<omc>/<命令号>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
    InsertInXmlExt(pstruXml,"<omc>/<类型>",  "11", MODE_AUTOGROW|MODE_UNIQUENAME);
	/*
	 * 站点等级为快:OMC_QUICK_MSGLEVEL
	 */
	InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	strcpy(szNeObjectList, TrimAllSpace(GetTableFieldValue(&struCursor, "ne_ActiveCol")));
	PrintDebugLog(DBG_HERE,"监控对象[%s]\n", szNeObjectList);
	FreeCursor(&struCursor);
	
	//取对象列表
	//GetNeObjectList(szNeObjectList);
			
	memset(&struHead, 0, sizeof(COMMANDHEAD));
    memset(&struRepeater, 0, sizeof(REPEATER_INFO));
	struRepeater.nCommType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<通信方式>"));  
	struRepeater.nRepeaterId = atol(DemandStrInXmlExt(pstruXml,"<omc>/<站点编号>")); 
	struRepeater.nDeviceId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<设备编号>")); 
	strcpy(struRepeater.szTelephoneNum, DemandStrInXmlExt(pstruXml,"<omc>/<站点电话>"));
	
	strcpy(struRepeater.szIP, DemandStrInXmlExt(pstruXml,"<omc>/<站点IP>"));
	struRepeater.nPort = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<端口号>"));
	
			
	struRepeater.nProtocolDeviceType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<设备类型>"));
	strcpy(struRepeater.szSpecialCode, DemandStrInXmlExt(pstruXml,"<omc>/<其它标识>"));
	strcpy(struRepeater.szReserve, DemandStrInXmlExt(pstruXml,"<omc>/<设备型号>"));
	strcpy(struRepeater.szNetCenter, DemandStrInXmlExt(pstruXml,"<omc>/<服务号码>"));
	
	struHead.nProtocolType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<协议类型>"));
	struHead.nCommandCode = COMMAND_QUERY;
	//分解对象
	if (nCommType == M2G_SMS)
		ResolveQryParamArray(szNeObjectList);
	else
		ResolveQryParamArrayGprs(szNeObjectList);
	
	nObjCount = SeperateString(szNeObjectList, '|', pszSepParamStr, MAX_SEPERATE_NUM);
    for(i=0; i< nObjCount; i++)
	{   
	    if (strlen(pszSepParamStr[i]) >= 4)
	    {
	    	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime((INT)time(NULL)+i*10), MODE_AUTOGROW|MODE_UNIQUENAME);
	    	InsertInXmlExt(pstruXml,"<omc>/<监控对象>", pszSepParamStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
	    	if (nCommType == M2G_SMS)
	    	{
        		QryElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
        		SaveToMsgQueue(pstruXml);
        	}
        	else if (nCommType == M2G_SNMP_TYPE)//通信方式：snmp协议
			{
				struHead.nProtocolType = PROTOCOL_SNMP;
				QryElementParam(M2G_SNMP_TYPE, &struHead, &struRepeater, pstruXml);
	        	//生成设置命令保存
	        	SaveToSnmpQueue(pstruXml);
			}
        	else
        	{
        		QryElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
        		SaveToGprsQueue(pstruXml);
        	}
	    	SaveEleQryLog(pstruXml);
	    }
	}
			
	DeleteXml(pstruXml);
	
	return NORMAL;
}

/*
 *  分解定时上报 用于效果监控
 */
RESULT DecodeQryOnTime(SENDPACKAGE *pstruSendPackage, BOOL bGprsTime, BOOL bBatchQuery)
{
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
    STR szProperty[2000];
	STR szContent[2000];
	STR szFailMapId[2000];
	STR szErrorId[2000];
	STR szAlarmName[2000];
	STR szAlarmValue[2000];
	STR szNewAlarmName[2000];
	STR szTemp[100], szAlarmTime[20];
	STR szQrySerial[29];
	STR szAlarmObjList[1000];
	PSTR pszAlarmNameStr[MAX_SEPERATE_NUM];
	PSTR pszAlarmValueStr[MAX_SEPERATE_NUM];
	INT nSeperateNum, i;
	STR szAlarmObjResotre[1000];
	
	
	//初始化变量
	memset(szProperty, 0, sizeof(szProperty));
	memset(szContent, 0, sizeof(szContent));
	memset(szAlarmName, 0, sizeof(szAlarmName));
	memset(szAlarmValue, 0, sizeof(szAlarmValue));
	memset(szNewAlarmName, 0, sizeof(szNewAlarmName));
	memset(szFailMapId, 0, sizeof(szFailMapId));
	memset(szErrorId, 0, sizeof(szErrorId));
	memset(szAlarmTime, 0, sizeof(szAlarmTime));
	
	strcpy(szQrySerial, pstruSendPackage->struHead.QA);
	n_gNeId = pstruSendPackage->nNeId;
	
	memset(pstruXml,0,sizeof(XMLSTRU));
	CreateXml(pstruXml,FALSE,OMC_ROOT_PATH,NULL);
	for (i=0; i< pstruSendPackage->struHead.nObjectCount; i++)
	{
        if (pstruSendPackage->struMapObjList[i].cErrorId == '0')//监控对象标号正确
		{
			//if (strlen(pstruSendPackage->struMapObjList[i].szMapData) > 0)//判断是否为空值,为空不处理
			{
		    sprintf(szProperty, "%s%s,", szProperty, pstruSendPackage->struMapObjList[i].szMapId);
		    sprintf(szContent, "%s%s,", szContent, pstruSendPackage->struMapObjList[i].szMapData);
			sprintf(szTemp, "<omc>/<%s>", pstruSendPackage->struMapObjList[i].szMapId);
			InsertInXmlExt(pstruXml, szTemp, pstruSendPackage->struMapObjList[i].szMapData,
				MODE_AUTOGROW|MODE_UNIQUENAME);
			}
			 //处理告警
			 if (strcmp(pstruSendPackage->struMapObjList[i].szMapType, "alarm") == 0)
			 {
			     //
			     sprintf(szAlarmName, "%s%s,", szAlarmName, pstruSendPackage->struMapObjList[i].szMapId);
		         sprintf(szAlarmValue, "%s%s,", szAlarmValue, pstruSendPackage->struMapObjList[i].szMapData);
			 	 if (strcmp(pstruSendPackage->struMapObjList[i].szMapData, "1") == 0)
			 	 {
			 	     //根据MapId取告警名AlarmName
			 	     sprintf(szNewAlarmName, "%s%s,", szNewAlarmName, GetAlarmName(pstruSendPackage->struMapObjList[i].szMapId));
		    
			 	 }
			 }
			 //取告警时间			
			 if ((strcmp(pstruSendPackage->struMapObjList[i].szMapId, "0801") == 0)
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "0802") == 0)
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "0803") == 0)
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "080F") == 0)
			    
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "04F0") == 0)
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "04F1") == 0)
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "04F2") == 0)
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "04F3") == 0)
			 )
			 {
			 	 strcpy(szAlarmTime, pstruSendPackage->struMapObjList[i].szMapData);
			 	 InsertInXmlExt(pstruXml,"<omc>/<告警时间>",pstruSendPackage->struMapObjList[i].szMapData,
			 	 	MODE_AUTOGROW|MODE_UNIQUENAME);
			 }
			 //取服务小区号 2010.3.26 update
			 if (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "050C") == 0)
			 {
			 	 InsertInXmlExt(pstruXml,"<omc>/<服务小区>",pstruSendPackage->struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
			 }
			 //取电平强度 2010.3.26 update
			 if (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "050B") == 0 ||
			 	 strcmp(pstruSendPackage->struMapObjList[i].szMapId, "08B3") == 0)
			 {
			 	 InsertInXmlExt(pstruXml,"<omc>/<电平强度>",pstruSendPackage->struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
			 } 
		}
		else//监控对象标号有错误
		{
		    //用空格分割
		    sprintf(szFailMapId, "%s%s,", szFailMapId, pstruSendPackage->struMapObjList[i].szMapId);
		    sprintf(szErrorId, "%s%c,", szErrorId, pstruSendPackage->struMapObjList[i].cErrorId);
            sprintf(szProperty, "%s%s,", szProperty, pstruSendPackage->struMapObjList[i].szMapId);
		    sprintf(szContent, "%s ,", szContent);		

		}
	}
	
	TrimRightChar(szProperty, ',');
	TrimRightOneChar(szContent, ',');
	TrimRightChar(szAlarmName, ',');
	TrimRightChar(szAlarmValue, ',');
	TrimRightChar(szNewAlarmName, ',');
	TrimRightChar(szFailMapId, ',');
	TrimRightChar(szErrorId, ',');
	
	PrintDebugLog(DBG_HERE, "QrySerial=[%s],Property[%s],Content[%s],AlarmName[%s],AlarmValue[%s],NewAlarmName[%s],FailMapId[%s]\n",
	     szQrySerial, szProperty, szContent, szAlarmName, szAlarmValue, szNewAlarmName,szFailMapId);
	
	if(bBatchQuery == BOOLFALSE)
	{	
		//保持定时上报信息
		if (bGprsTime == BOOLTRUE)
		{	if (pstruSendPackage->struRepeater.nProtocolDeviceType == 35)//基本型2.0
				SaveTimerUploadLog_jb2(n_gNeId, pstruXml);
			else
				SaveTimerUploadLog(n_gNeId, pstruXml);
		}
		else
			SaveTdTimerUploadLog(n_gNeId, pstruXml);
	}

	//更新网元查询信息(对站点查询后返回结果的更新包括网元表和网元辅助表)成功时
	//UpdateEleQryLogOnTime(szQrySerial, szProperty, szContent);
	UpdateEleQryLogFromComm(pstruSendPackage, szProperty, szContent);
	
		
	//定时上报不处理告警 2009.2.27
	if (strlen(szAlarmName) > 0 )
	{
	    BOOL bAlarmRestore = BOOLTRUE ;//默认为告警恢复前转
	    //取网元名NeName
	    strcpy(szAlarmObjList, GetAlarmObjList(n_gNeId));
	    sprintf(szTemp, "%d", pstruSendPackage->nNeId);
		InsertInXmlExt(pstruXml,"<omc>/<网元编号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
		InsertInXmlExt(pstruXml,"<omc>/<告警列表>",szAlarmObjList,MODE_AUTOGROW|MODE_UNIQUENAME);
		
	    //分解告警名
	    nSeperateNum = SeperateString(szAlarmName,  ',', pszAlarmNameStr, MAX_SEPERATE_NUM);
	    //分解告警mapid
	    nSeperateNum = SeperateString(szAlarmValue,  ',', pszAlarmValueStr, MAX_SEPERATE_NUM);
	    for(i=0; i< nSeperateNum; i++)
	    {
	        if (strstr(szAlarmObjList, pszAlarmNameStr[i]) == NULL)
	            continue;
	        InsertInXmlExt(pstruXml,"<omc>/<告警对象>", pszAlarmNameStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
	        if (strcmp(pszAlarmValueStr[i], "1") == 0)//告警处理
	        {
	            bAlarmRestore = BOOLFALSE ;  //取消告警前转
	            sprintf(szTemp, "%s:0",  pszAlarmNameStr[i]);
	            if (strstr(szAlarmObjList, szTemp) != NULL)//只有站点确实处于非告警状态才告警
	            {
	                DealNewAlarm(pstruXml);
	             }
	         }
	         else if (strcmp(pszAlarmValueStr[i], "0") == 0)//只有站点确实处于告警状态才恢复
	         {
	            sprintf(szTemp, "%s:1",  pszAlarmNameStr[i]);
	            if (strstr(szAlarmObjList, szTemp) != NULL)//只有站点确实处于非告警状态才告警
	            {
	                AlarmComeback(pstruXml);
	                sprintf(szAlarmObjResotre, "%s%s", szAlarmObjResotre, pszAlarmNameStr[i]);
	             }
	         }
	    }
	    

	    
	}
    
    DeleteXml(pstruXml);
    return NORMAL;
}
/*
 *  保存网元性能参数
 */
RESULT SaveEffectControl(INT nNeId, PSTR pszProperty, PSTR pszContent)
{
	char szSql[MAX_BUFFER_LEN];
	INT nMacId;
	
	if (strcmp(getenv("DATABASE"), "mysql") == 0)
	{
		if(GetMySqlSequence(&nMacId, "man_effectcontrol")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取查询设置流水号错误\n");
			return EXCEPTION;
		}
	}
	else
	{
		if(GetDbSequence(&nMacId, "SEQ_EFFECTCONTROL")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取性能参数流水号错误\n");
			return EXCEPTION;
		}
	}
	
	
	
	snprintf(szSql, sizeof(szSql),
		"insert into  man_effectControl(mac_id,mac_neid,mac_property,mac_content, mac_eventTime)"
 		"values  (%d, %d, '%s',  '%s', sysdate)",
 		nMacId, nNeId, pszProperty, pszContent);
	
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
/*
 *  保存网元查询日志
 */
RESULT SaveEleQryLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	//int nEleQryLogId=atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<日志号>"));	
	//int qry_EleId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	//if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<服务状态>")) == 0)
	char szDateTime[100];
	strcpy(szDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<定时发送时间>"));
	if (strncmp(szDateTime, "0", 1) == 0 || strlen(szDateTime) <= 1) 
		sprintf(szDateTime, "%s%s", GetSystemDate(), GetSystemTime());
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into man_EleQryLog(qry_EleQryLogId,qry_ListId,qry_EleId,qry_Property,qry_Style,"
		    " qry_Commtype,qry_TaskId,qry_CustomerID,qry_User,qry_BeginTime, qry_Number,"
		    "qry_TxPackCount,qry_TaskLogId,qry_logid, qry_windowlogid,qry_packstatus) values("
		    "%s,  %d,  %s,   '%s', '%s',"
			" %d, %d, '%s', '%s', to_date('%s','yyyymmddhh24miss'), '%s', "
			" 1, %d,  %d, '%s',   'packing') ",
			DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<明细号>")),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<监控对象>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<命令号>"),  //类型该为命令号2009.1.7
			
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<通信方式>")),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<任务号>")),
			"", "0", szDateTime,
			DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"),

    	    atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<任务日志号>")),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<登录流水号>")),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<窗体流水号>")
		);
	}	
	/*else
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into man_EleQryLog(qry_EleQryLogId,qry_ListId,qry_EleId,qry_Property,qry_Style,"
		    " qry_Commtype,qry_TaskId,qry_CustomerID,qry_User,qry_BeginTime, qry_endtime, qry_Number,"
		    "qry_TxPackCount,qry_TaskLogId,qry_logid, qry_windowlogid,qry_packstatus, qry_issuccess, qry_failcontent) values("
		    "%s,  %d,  %s,   '%s', '%s',"
			" %d, %d, '%s', '%s', to_date( '%s','yyyy-mm-dd hh24:mi:ss'), to_date( '%s','yyyy-mm-dd hh24:mi:ss'), '%s', "
			" 1, %d,  %d, '%s',   'packing', 0, '网络状态不正常,稍后再试') ",
			DemandStrInXmlExt(pstruReqXml, "<omc>/<日志号>"),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<明细号>")),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<监控对象>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<命令号>"),  //类型该为命令号2009.1.7
			
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<通信方式>")),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<任务号>")),
			"", "0", GetSysDateTime(), GetSysDateTime(),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"),
			
    	    atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<任务日志号>")),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<登录流水号>")),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<窗体流水号>")
		);
	}*/
	
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	//调用存储过程
	/*
	if (CallInserTeleLog(nEleQryLogId, qry_EleId, 0) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE, "执行存储过程[%s]失败\n", "insertelelog");
        return EXCEPTION;
	}
	*/

	return NORMAL;  
    
}


/*
 *  保存网元设置日志
 */
RESULT SaveEleSetLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	
	//int qry_EleSetLogId=atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<日志号>"));	
	//int qry_EleId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	//if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<服务状态>")) == 0)
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into man_EleSetLog(set_EleSetLogId,set_ListId,set_EleId,set_Property,set_Content,set_Style,"
		    " set_Commtype,set_TaskId,set_Customers,set_User,set_BeginTime, set_Number,set_TxPackCount,"
		    " set_TaskLogId,set_logid,set_windowlogid,set_packstatus) values("
		    "%s,  %d,  %s,  '%s', '%s', '%s',"
			" %s, %d, '%s', '%s', to_date( '%s','yyyy-mm-dd hh24:mi:ss'), '%s',  1,"
			" %d,  %d, '%s',   'packing') ",
			DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<明细号>")),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<监控对象>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<监控对象内容>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<命令号>"), //类型该为命令号2009.1.7
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<通信方式>"),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<任务号>")),
			"", "", GetSysDateTime(),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"),
			
    	    atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<任务日志号>")),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<登录流水号>")),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<窗体流水号>")
		);
	}
	/*else
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into man_EleSetLog(set_EleSetLogId,set_ListId,set_EleId,set_Property,set_Content,set_Style,"
		    " set_Commtype,set_TaskId,set_Customers,set_User,set_BeginTime, set_endtime, set_Number,set_TxPackCount,"
		    " set_TaskLogId,set_logid,set_windowlogid,set_packstatus, set_issuccess, set_failcontent) values("
		    "%s,  %d,  %s,  '%s', '%s', '%s',"
			" %s, %d, '%s', '%s', to_date( '%s','yyyy-mm-dd hh24:mi:ss'), to_date( '%s','yyyy-mm-dd hh24:mi:ss'), '%s',  1,"
			" %d,  %d, '%s',   'packing', 0, '网络状态不正常，稍后再试') ",
			DemandStrInXmlExt(pstruReqXml, "<omc>/<日志号>"),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<明细号>")),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<监控对象>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<监控对象内容>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<命令号>"), //类型该为命令号2009.1.7
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<通信方式>"),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<任务号>")),
			"", "", GetSysDateTime(),GetSysDateTime(),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"),
			
    	    atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<任务日志号>")),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<登录流水号>")),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<窗体流水号>")
		);
	}*/
	
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	//调用存储过程
	/*
	if (CallInserTeleLog(qry_EleSetLogId, qry_EleId, 1) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE, "执行存储过程[%s]失败\n", "insertelelog");
        return EXCEPTION;
	}
	*/
	return NORMAL;  
    
}


/* 
 * 根据属性和参数内容更新man_eleqrylog等表
 */
RESULT SetEleQryLogErrorFromComm(PSTR pszQryNumber, PSTR pszErrMapId, PSTR pszErrorId, PSTR pszErrContent)
{
    STR szSql[MAX_SQL_LEN];
    CURSORSTRU struCursor;
	STR szErrorMemo[200], szFailContent[MAX_BUFFER_LEN];
	PSTR pszErrMapIdStr[MAX_SEPERATE_NUM];
	PSTR pszErrorIdStr[MAX_SEPERATE_NUM];
	PSTR pszErrContentStr[MAX_SEPERATE_NUM];
	INT nSeperateNum, i, nErrorId;
	TBINDVARSTRU struBindVar;
    
    memset(szFailContent, 0, sizeof(szFailContent));
    //分解错误监控量
	nSeperateNum = SeperateString(pszErrMapId,  ',', pszErrMapIdStr, MAX_SEPERATE_NUM);
	nSeperateNum = SeperateString(pszErrContent,  ',', pszErrContentStr, MAX_SEPERATE_NUM);
	nSeperateNum = SeperateString(pszErrorId,  ',', pszErrorIdStr, MAX_SEPERATE_NUM);
	for(i=0; i< nSeperateNum; i++)
	{
		memset(&struBindVar, 0, sizeof(struBindVar));
		struBindVar.struBind[0].nVarType = SQLT_INT;
		struBindVar.struBind[0].VarValue.nValueInt = strHexToInt(pszErrorIdStr[i]);
		struBindVar.nVarCount++;
		
		memset(szErrorMemo, 0, sizeof(szErrorMemo));
	    memset(szSql, 0, sizeof(szSql));
	    sprintf(szSql, " select  ma_MapErrorName from ne_2GMapError where ma_MapErrorId= :v_0");
	    PrintDebugLog(DBG_HERE, "执行SQL语句[%s][%s]\n", szSql, pszErrorIdStr[i]);
	    if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    	return EXCEPTION;
	    }
	    if(FetchCursor(&struCursor) == NORMAL)
	    {
	    	strcpy(szErrorMemo,  TrimAllSpace(GetTableFieldValue(&struCursor, "ma_MapErrorName")));
	    }
	    FreeCursor(&struCursor);
	    
	    nErrorId = strHexToInt(pszErrorIdStr[i]);
	    if (nErrorId == 2 || nErrorId == 5 || nErrorId == 6)
	    	sprintf(szFailContent, "%s%s:%s[%s],", szFailContent, pszErrMapIdStr[i], szErrorMemo, pszErrContentStr[i]);
	    else
	    	sprintf(szFailContent, "%s%s:%s,", szFailContent, pszErrMapIdStr[i], szErrorMemo);
	}
	
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[0].VarValue.szValueChar, szFailContent);
	struBindVar.nVarCount++;
	
	struBindVar.struBind[1].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[1].VarValue.szValueChar, pszQryNumber);
	struBindVar.nVarCount++;
		
	memset(szSql, 0, sizeof(szSql));
    sprintf(szSql," update man_eleqrylog set qry_FailContent=:v_0 where qry_Number= :v_1");
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s][%s][%s]\n", szSql, szFailContent,pszQryNumber);
    if(BindExecuteSQL(szSql, &struBindVar) !=NORMAL ) 
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();
	

	return NORMAL;
   
}


/* 
 * 根据属性和参数内容更新ne_Element等表
 */
RESULT SetEleSetLogErrorFromComm(PSTR pszQryNumber, PSTR pszErrMapId,PSTR pszErrorId, PSTR pszErrContent)
{
    STR szSql[MAX_SQL_LEN];
    CURSORSTRU struCursor;
	STR szErrorMemo[200], szFailContent[MAX_BUFFER_LEN];
	PSTR pszErrMapIdStr[MAX_SEPERATE_NUM];
	PSTR pszErrContentStr[MAX_SEPERATE_NUM];
	PSTR pszErrorIdStr[MAX_SEPERATE_NUM];
	INT nSeperateNum, i, nErrorId;
    TBINDVARSTRU struBindVar;
    
    memset(szFailContent, 0, sizeof(szFailContent));
    //分解错误监控量
	nSeperateNum = SeperateString(pszErrMapId,  ',', pszErrMapIdStr, MAX_SEPERATE_NUM);
	nSeperateNum = SeperateString(pszErrContent,  ',', pszErrContentStr, MAX_SEPERATE_NUM);
	nSeperateNum = SeperateString(pszErrorId,  ',', pszErrorIdStr, MAX_SEPERATE_NUM);
	for(i=0; i< nSeperateNum; i++)
	{
		memset(&struBindVar, 0, sizeof(struBindVar));
		struBindVar.struBind[0].nVarType = SQLT_INT;
		struBindVar.struBind[0].VarValue.nValueInt = strHexToInt(pszErrorIdStr[i]);
		struBindVar.nVarCount++;
		
		memset(szErrorMemo, 0, sizeof(szErrorMemo));
	    memset(szSql, 0, sizeof(szSql));
	    sprintf(szSql, " select   ma_MapErrorName from ne_2GMapError where ma_MapErrorId= :v_0");
	    PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	    if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    	return EXCEPTION;
	    }
	    if(FetchCursor(&struCursor) == NORMAL)
	    {
	    	strcpy(szErrorMemo,  TrimAllSpace(GetTableFieldValue(&struCursor, "ma_MapErrorName")));
	    }
	    FreeCursor(&struCursor);
	    
	    nErrorId = strHexToInt(pszErrorIdStr[i]);
	    if (nErrorId == 2 || nErrorId == 5 || nErrorId == 6)
	    	sprintf(szFailContent, "%s%s:%s[%s],", szFailContent, pszErrMapIdStr[i], szErrorMemo, pszErrContentStr[i]);
	    else
	    	sprintf(szFailContent, "%s%s:%s,", szFailContent, pszErrMapIdStr[i], szErrorMemo);
	}
	TrimRightChar(szFailContent, ',');

	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[0].VarValue.szValueChar, szFailContent);
	struBindVar.nVarCount++;
	
	struBindVar.struBind[1].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[1].VarValue.szValueChar, pszQryNumber);
	struBindVar.nVarCount++;
	memset(szSql, 0, sizeof(szSql));
    sprintf(szSql," update man_elesetlog set set_FailContent=:v_0 where set_Number= :v_1");
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s][%s][%s]\n", szSql, szFailContent,pszQryNumber);
    if(BindExecuteSQL(szSql, &struBindVar) !=NORMAL ) 
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();

	return NORMAL;
   
}



/* 
 * 根据属性和参数内容更新ne_Element等表
 */
RESULT UpdateEleSetLogFromComm(SENDPACKAGE *pstruSendPackage, PSTR pszQryNumber, PSTR pszProperty, PSTR pszContent)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	STR szElementSql[MAX_SQL_LEN];
	STR szMonitorSql[MAX_SQL_LEN];
	PSTR pszPropertyStr[MAX_SEPERATE_NUM];
	PSTR pszContentStr[MAX_SEPERATE_NUM];
	INT nSeperateNum, i, n_protocoltypeid;
	STR szAlarmEnabledObjList[1000];
	BOOL bAlarmEnabled = BOOLFALSE, bCqtMatche=BOOLFALSE;
	int nCount;
	STR szCallNum[20], szBeCallNum[20], szTelNum[20];
	TBINDVARSTRU struBindVar;
	
	bufclr(szCallNum);
	bufclr(szBeCallNum);
	n_protocoltypeid = pstruSendPackage->struHead.nProtocolType;
	strcpy(szTelNum, pstruSendPackage->struRepeater.szTelephoneNum);
	
	
	//分解监控量内容
	nSeperateNum = SeperateString(pszContent,  ',', pszContentStr, MAX_SEPERATE_NUM);
	//分解监控mapid
	nSeperateNum = SeperateString(pszProperty,  ',', pszPropertyStr, MAX_SEPERATE_NUM);
	
	memset(szElementSql, 0, sizeof(szElementSql));
	sprintf(szElementSql, "update ne_Element set");
	
	memset(szMonitorSql, 0, sizeof(szMonitorSql));
	sprintf(szMonitorSql, "update ne_SignerMonitorGprs set");
	for(i=0; i< nSeperateNum; i++)
	{
	    if (strcmp(pszPropertyStr[i], "0002") == 0 || strcmp(pszPropertyStr[i], "00000002") == 0) //公司
	        sprintf(szElementSql, "%s ne_CompanyId= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0003") == 0 || strcmp(pszPropertyStr[i], "00000003") == 0) //设备类别
            sprintf(szElementSql, "%s ne_ProtocolDeviceTypeId= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0004") == 0 || strcmp(pszPropertyStr[i], "00000004") == 0) //设备型号
	        sprintf(szElementSql, "%s ne_DeviceModelId= '%s',", szElementSql, pszContentStr[i]);
        else if (strcmp(pszPropertyStr[i], "0005") == 0 || strcmp(pszPropertyStr[i], "00000005") == 0) //
	        sprintf(szElementSql, "%s ne_SnNumber= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0006") == 0 || strcmp(pszPropertyStr[i], "00000006") == 0) //
	        sprintf(szElementSql, "%s ne_ChannelCount= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0007") == 0 || strcmp(pszPropertyStr[i], "00000007") == 0) //经度
	    {
	    	if (strncmp(pszContentStr[i], "E", 1) == 0 || strncmp(pszContentStr[i], "e", 1) == 0)
	    		pszContentStr[i] = pszContentStr[i]+1;
	    	sprintf(szElementSql, "%s ne_Lon= %s,", szElementSql, pszContentStr[i]);	
	    }
	    else if (strcmp(pszPropertyStr[i], "0008") == 0 || strcmp(pszPropertyStr[i], "00000008") == 0) //纬度
	    {
	    	if (strncmp(pszContentStr[i], "N", 1) == 0 || strncmp(pszContentStr[i], "n", 1) == 0)
	    		pszContentStr[i] = pszContentStr[i]+1;
	    	sprintf(szElementSql, "%s ne_Lat= %s,", szElementSql, pszContentStr[i]);		    	
	    }	
	    else if (strcmp(pszPropertyStr[i], "0009") == 0) //
	        sprintf(szElementSql, "%s ne_ObjList= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "000A") == 0 || strcmp(pszPropertyStr[i], "0000000A") == 0) //
	        sprintf(szElementSql, "%s ne_ver= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0101") == 0 || strcmp(pszPropertyStr[i], "00000101") == 0) //
	        sprintf(szElementSql, "%s ne_RepeaterId= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0102") == 0 || strcmp(pszPropertyStr[i], "00000102") == 0) //
	        sprintf(szElementSql, "%s ne_DeviceId= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0110") == 0 || strcmp(pszPropertyStr[i], "00000110") == 0) //
	        sprintf(szElementSql, "%s ne_SmscNum= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0111") == 0 || strcmp(pszPropertyStr[i], "00000111") == 0) //
	        sprintf(szElementSql, "%s ne_TelNum1= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0112") == 0 || strcmp(pszPropertyStr[i], "00000112") == 0) //
	        sprintf(szElementSql, "%s ne_TelNum2= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0113") == 0 || strcmp(pszPropertyStr[i], "00000113") == 0) //
	        sprintf(szElementSql, "%s ne_TelNum3= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0114") == 0 || strcmp(pszPropertyStr[i], "00000114") == 0) //
	        sprintf(szElementSql, "%s ne_TelNum4= '%s',", szElementSql, pszContentStr[i]); 
	    else if (strcmp(pszPropertyStr[i], "0115") == 0 || strcmp(pszPropertyStr[i], "00000115") == 0) //
	        sprintf(szElementSql, "%s ne_TelNum5= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0120") == 0 || strcmp(pszPropertyStr[i], "00000120") == 0) //
	        sprintf(szElementSql, "%s ne_OmcTelNum= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0130") == 0 || strcmp(pszPropertyStr[i], "00000130") == 0) //
	        sprintf(szElementSql, "%s ne_OmcIp= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0131") == 0 || strcmp(pszPropertyStr[i], "00000131") == 0) //
	        sprintf(szElementSql, "%s ne_OmcPort= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0151") == 0 || strcmp(pszPropertyStr[i], "00000151") == 0) //UDP Device IP added by wwj at 20100625
	        sprintf(szElementSql, "%s ne_DeviceIp= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0139") == 0 || strcmp(pszPropertyStr[i], "00000139") == 0) //UDP Device Port added by wwj at 20100625
	        sprintf(szElementSql, "%s ne_DevicePort= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0140") == 0) //
	        sprintf(szElementSql, "%s ne_UploadCommTypeId= %s,", szElementSql, pszContentStr[i]);
	    
        else if (strcmp(pszPropertyStr[i], "0150") == 0) //日期、时间
	    {
	    	memset(&struBindVar, 0, sizeof(struBindVar));
        	struBindVar.struBind[0].nVarType = SQLT_STR; 
			strcpy(struBindVar.struBind[0].VarValue.szValueChar, pszContentStr[i]);
			struBindVar.nVarCount++;
			
			struBindVar.struBind[1].nVarType = SQLT_INT; //SQLT_INT
			struBindVar.struBind[1].VarValue.nValueInt = n_gNeId;
			struBindVar.nVarCount++;
			if (strcmp(getenv("DATABASE"), "mysql") == 0)			
	        	sprintf(szSql,"update  ne_Elementparam  set epm_curvalue=to_date(:v_0, 'yyyy-mm-dd hh24:mi:ss') where epm_neid= :v_1 and epm_objid='0150'");
	        else
	        	sprintf(szSql,"update  ne_Elementparam  set epm_curvalue=sysdate - to_date(:v_0, 'yyyy-mm-dd hh24:mi:ss') where epm_neid= :v_1 and epm_objid='0150'");
	        PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%d]\n",szSql, pszContentStr[i], n_gNeId);
	        if(BindExecuteSQL(szSql, &struBindVar)!=NORMAL)
	        {
		        PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		        continue;
	        }
	            	
	    }
	    else if (strcmp(pstruSendPackage->struMapObjList[i].szMapType, "alarmYN") == 0) //告警使能 
	    {
	    	if (bAlarmEnabled == BOOLFALSE)
	    	{
	    		memset(szAlarmEnabledObjList,0, sizeof(szAlarmEnabledObjList));
	            strcpy(szAlarmEnabledObjList, GetAlarmEnabledObjList(n_gNeId));
	        }
	        ReplaceAlarmObjStr(szAlarmEnabledObjList, pszPropertyStr[i], pszContentStr[i]);
	        bAlarmEnabled = BOOLTRUE;
	    }
	    else
	    {
	    	memset(&struBindVar, 0, sizeof(struBindVar));
			struBindVar.struBind[0].nVarType = SQLT_INT; //SQLT_INT
			struBindVar.struBind[0].VarValue.nValueInt = n_gNeId;
			struBindVar.nVarCount++;
		
			struBindVar.struBind[1].nVarType = SQLT_STR; 
			strcpy(struBindVar.struBind[1].VarValue.szValueChar, pszPropertyStr[i]);
			struBindVar.nVarCount++;
			if (n_protocoltypeid == PROTOCOL_JINXIN_DAS)
				sprintf(szSql,"select count(*) as empcount from  ne_elementparam  where epm_NeId= :v_0 and  epm_McpId= :v_1");
			else
	        	sprintf(szSql,"select count(*) as empcount from  ne_elementparam  where epm_NeId= :v_0 and  epm_ObjId= :v_1");
	        PrintDebugLog(DBG_HERE, "执行SQL语句[%s][%d][%s]\n", szSql, n_gNeId, pszPropertyStr[i]);
	        if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
	        {
	        	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	        				  szSql, GetSQLErrorMessage());
	        	return EXCEPTION;
	        }
	        if(FetchCursor(&struCursor) != NORMAL)
	        {
	            FreeCursor(&struCursor);
	        	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", \
	        				  szSql, GetSQLErrorMessage());
	        	return EXCEPTION;
	        }
	        nCount = atoi(GetTableFieldValue(&struCursor, "empcount"));
	        FreeCursor(&struCursor);
	        if (nCount > 0)
	        {
	        	memset(&struBindVar, 0, sizeof(struBindVar));
	        	
	        	struBindVar.struBind[0].nVarType = SQLT_STR; 
				strcpy(struBindVar.struBind[0].VarValue.szValueChar, pszContentStr[i]);
				struBindVar.nVarCount++;
				
				struBindVar.struBind[1].nVarType = SQLT_INT; //SQLT_INT
				struBindVar.struBind[1].VarValue.nValueInt = n_gNeId;
				struBindVar.nVarCount++;
				
				struBindVar.struBind[2].nVarType = SQLT_STR; 
				strcpy(struBindVar.struBind[2].VarValue.szValueChar, pszPropertyStr[i]);
				struBindVar.nVarCount++;
				
				if (n_protocoltypeid == PROTOCOL_JINXIN_DAS)//更新映射ID
					sprintf(szSql,"update ne_elementparam set epm_CurValue= :v_0, epm_UpdateTime=sysdate where epm_NeId= :v_1 and  epm_McpId= :v_2");
				else
	            	sprintf(szSql,"update ne_elementparam set epm_CurValue= :v_0, epm_UpdateTime=sysdate where epm_NeId= :v_1 and  epm_ObjId= :v_2");
	            PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%d][%s]\n",szSql, pszContentStr[i], n_gNeId, pszPropertyStr[i]);
	            if(BindExecuteSQL(szSql, &struBindVar)!=NORMAL)
	            {
		            PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		            return EXCEPTION;
	            }
	            
	        }
	        else
	        {
	        	if (n_protocoltypeid == PROTOCOL_JINXIN_DAS)
	        	{
	        		memset(&struBindVar, 0, sizeof(struBindVar));
	        	
		        	struBindVar.struBind[0].nVarType = SQLT_STR; 
					strcpy(struBindVar.struBind[0].VarValue.szValueChar, pszContentStr[i]);
					struBindVar.nVarCount++;
					
					struBindVar.struBind[1].nVarType = SQLT_INT; //SQLT_INT
					struBindVar.struBind[1].VarValue.nValueInt = n_gNeId;
					struBindVar.nVarCount++;
					
					struBindVar.struBind[2].nVarType = SQLT_STR; 
					strcpy(struBindVar.struBind[2].VarValue.szValueChar, pszPropertyStr[i]);
					struBindVar.nVarCount++;
					
	        		sprintf(szSql,"update ne_elementparam set epm_CurValue= :v_0, epm_UpdateTime=sysdate where epm_NeId= :v_1 and  epm_ObjId= :v_2");
		            PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%d][%s]\n",szSql, pszContentStr[i], n_gNeId, pszPropertyStr[i]);
		            if(BindExecuteSQL(szSql, &struBindVar)!=NORMAL)
		            {
			            PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
			            return EXCEPTION;
		            }
	        	}
	        }
	    }
	    
	    if (strcmp(pszPropertyStr[i], "0873") == 0 || strcmp(pszPropertyStr[i], "00000873") == 0) //
	        sprintf(szElementSql, "%s ne_route= '%s',", szElementSql, pszContentStr[i]);    
	}
	//网元信息表有条件
	//if (strlen(szElementSql) > 21 || bAlarmEnabled == BOOLTRUE)
	{
	    if (bAlarmEnabled == BOOLTRUE)
	        sprintf(szElementSql, "%s ne_AlarmEnabledObjList = '%s', ne_lastupdatetime = to_date( '%s','yyyy-mm-dd hh24:mi:ss')  where ne_NeId = %d", 
	            szElementSql, szAlarmEnabledObjList, GetSysDateTime(), n_gNeId);
	    else
	        sprintf(szElementSql, "%s ne_lastupdatetime = to_date( '%s','yyyy-mm-dd hh24:mi:ss')  where ne_NeId = %d", 
	            szElementSql, GetSysDateTime(), n_gNeId);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szElementSql);
	    if(ExecuteSQL(szElementSql)!=NORMAL)
	    {
		    PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szElementSql,GetSQLErrorMessage());
		    return EXCEPTION;
	    }
	}
	//效果监控列表有条件
	if (strlen(szMonitorSql) > 31)
	{
	    sprintf(szMonitorSql, "%s where smg_neid = %d", TrimRightChar(szMonitorSql,','), n_gNeId);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szMonitorSql);
	    if(ExecuteSQL(szMonitorSql)!=NORMAL)
	    {
		    PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szMonitorSql,GetSQLErrorMessage());
		    return EXCEPTION;
	    }
	}
	CommitTransaction();
	
	if (bCqtMatche==BOOLTRUE)
	{
		CqtMathchJob2(n_gNeId, szTelNum, szBeCallNum);
	}
	return NORMAL;    
	
	
}


/* 
 * 根据属性和参数内容更新man_EleQryLog等表
 */
RESULT UpdateEleFromCommByPacket(PSTR pszQryNumber, PSTR pszContent, PSTR pszAlarmName, PSTR pszAlarmValue)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	STR szTklEndTime[100];
	BOOL bTimeOut = BOOLFALSE;
	int nCount=-1;
	TBINDVARSTRU struBindVar;
	
	if (n_gTaskLogId > 0)
	{
		/* 2021.12.23 delete
		memset(&struBindVar, 0, sizeof(struBindVar));
		struBindVar.struBind[0].nVarType = SQLT_INT;
		struBindVar.struBind[0].VarValue.nValueInt = n_gTaskLogId;
		struBindVar.nVarCount++;
	    //取man_tasklog
	    memset(szSql, 0, sizeof(szSql));
	    if (strcmp(getenv("DATABASE"), "mysql") == 0)
	    	sprintf(szSql, "select tkl_endtime as tkl_endtime from man_tasklog where tkl_tasklogid= :v_0");
	    else
	    	sprintf(szSql, "select to_char(tkl_endtime, 'yyyy-mm-dd hh24:mi:ss') as tkl_endtime from man_tasklog where tkl_tasklogid= :v_0");
	    PrintDebugLog(DBG_HERE, "执行SQL语句[%s][%d]\n", szSql, n_gTaskLogId);
	    if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
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
	    strcpy(szTklEndTime, (GetTableFieldValue(&struCursor, "tkl_endtime")));
	    FreeCursor(&struCursor);
	    //未超时
	    if (strlen(szTklEndTime) > 0)
	    {
	        bTimeOut = BOOLTRUE;
	    }
	    
	    memset(&struBindVar, 0, sizeof(struBindVar));
		struBindVar.struBind[0].nVarType = SQLT_INT;
		struBindVar.struBind[0].VarValue.nValueInt = n_gTaskLogId;
		struBindVar.nVarCount++;
		
		struBindVar.struBind[1].nVarType = SQLT_INT;
		struBindVar.struBind[1].VarValue.nValueInt = n_gNeId;
		struBindVar.nVarCount++;
		
	    memset(szSql, 0, sizeof(szSql));
	    sprintf(szSql, "select count(*) as v_count from man_EleQryLog where qry_TaskLogid= :v_0 and qry_EleId= :v_1 and qry_IsSuccess=1");
	    PrintDebugLog(DBG_HERE, "执行SQL语句[%s][%d][%d]\n", szSql, n_gTaskLogId, n_gNeId);
	    if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    	return EXCEPTION;
	    }
	    if(FetchCursor(&struCursor) == NORMAL)
	    {
	    	nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
	    }
	    FreeCursor(&struCursor);
	    */
	    
	    /* 2022.1.14 delete
	    memset(&struBindVar, 0, sizeof(struBindVar));
		struBindVar.struBind[0].nVarType = SQLT_INT;
		struBindVar.struBind[0].VarValue.nValueInt = n_gTaskLogId;
		struBindVar.nVarCount++;
		
	    memset(szSql, 0, sizeof(szSql));
	    if (bTimeOut == BOOLFALSE) //&& nCount==0 2021.12.23
	    {
	    	sprintf(szSql, "update man_TaskLog set tkl_RxPackCount=tkl_RxPackCount+1,tkl_EleSuccessCount=tkl_EleSuccessCount+1 where tkl_tasklogid= :v_0 ");
	    }
	    else
	    {
            sprintf(szSql, "update man_TaskLog set tkl_RxPackCount=tkl_RxPackCount+1 where tkl_tasklogid= :v_0 ");
	    }
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%d]\n", szSql, n_gTaskLogId);
	    if(BindExecuteSQL(szSql, &struBindVar)!=NORMAL)
	    {
	         PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
	         return EXCEPTION;
	    }
	    CommitTransaction();
	    */
    }
    memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[0].VarValue.szValueChar, pszContent);
	struBindVar.nVarCount++;
	
	struBindVar.struBind[1].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[1].VarValue.szValueChar,  GetSysDateTime());
	struBindVar.nVarCount++;
			
	struBindVar.struBind[2].nVarType = SQLT_INT;
	struBindVar.struBind[2].VarValue.nValueInt = bTimeOut;
	struBindVar.nVarCount++;
	
	struBindVar.struBind[3].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[3].VarValue.szValueChar, pszAlarmName);
	struBindVar.nVarCount++;
	
	struBindVar.struBind[4].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[4].VarValue.szValueChar, pszQryNumber);
	struBindVar.nVarCount++;
		
    //更新man_EleQryLog表
    if (strlen(pszAlarmName) > 0)
    {
        sprintf(szSql, "update man_EleQryLog set qry_Content= :v_0,qry_EndTime=to_date(:v_1,'yyyy-mm-dd hh24:mi:ss'),qry_IsSuccess=1,qry_IsAlarm=1, qry_IsOutTime=:v_2,qry_RxPackCount=1,qry_AlarmName=:v_3 where qry_Number= :v_4 ");
        PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%s][%s][%s]\n", szSql, pszContent, GetSysDateTime(), pszAlarmName, pszQryNumber);
    }
    else
    {
    	if (strcmp(pszContent, "busy") == 0)
    	{
    		memset(&struBindVar, 0, sizeof(struBindVar));
			struBindVar.struBind[0].nVarType = SQLT_STR;
			strcpy(struBindVar.struBind[0].VarValue.szValueChar, GetSystemDate());
			struBindVar.nVarCount++;
			
			struBindVar.struBind[1].nVarType = SQLT_STR;
			strcpy(struBindVar.struBind[1].VarValue.szValueChar,  pszQryNumber);
			struBindVar.nVarCount++;
    		sprintf(szSql," update man_eleqrylog set qry_FailContent='busy',qry_EndTime=to_date(:v_0,'yyyy-mm-dd hh24:mi:ss'),qry_IsSuccess=0, qry_RxPackCount=1 where qry_Number= :v_1");
    		PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%s]\n", szSql,  GetSysDateTime(), pszQryNumber);
    	}
		else
		{
			sprintf(szSql, "update man_EleQryLog set qry_Content= :v_0,qry_EndTime=to_date(:v_1,'yyyy-mm-dd hh24:mi:ss'),qry_IsSuccess=1,qry_IsAlarm=0, qry_IsOutTime=:v_2,qry_RxPackCount=1,qry_AlarmName=:v_3 where qry_Number=:v_4");
			PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%s][%s][%s]\n", szSql, pszContent, GetSysDateTime(), pszAlarmName, pszQryNumber);
        }
    }
	
	if(BindExecuteSQL(szSql, &struBindVar)!=NORMAL)
	{
		 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息[%s]\n",szSql, GetSQLErrorMessage());
		 return EXCEPTION;
	}
	CommitTransaction();
	//有告警信息
	if ((strlen(pszAlarmName) > 0) && (strstr(pszAlarmValue, "1") != NULL) && (n_gTaskLogId > 0))
	{
	    memset(szSql, 0, sizeof(szSql));
	    sprintf(szSql, "select count(*) as v_count from man_TaskAlarmLog where tal_tasklogid = %d and tal_neid = %d ", n_gTaskLogId, n_gNeId);
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
	    nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
	    FreeCursor(&struCursor);
	    
	    memset(szSql, 0, sizeof(szSql));
	    if (nCount > 0)
	        sprintf(szSql, "update man_TaskAlarmLog set tal_alarmname = tal_alarmname||','||'%s' where tal_tasklogid= %d ", pszAlarmName, n_gTaskLogId);
	    else
	        sprintf(szSql, "insert into  man_TaskAlarmLog(tal_tasklogid,tal_neid,tal_alarmname) values (%d, %d,'%s') ", n_gTaskLogId, n_gNeId, pszAlarmName);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
	    if(ExecuteSQL(szSql)!=NORMAL)
	    {
	    	 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
	    	 return EXCEPTION;
	    }
	    CommitTransaction();
	}
	
	return NORMAL;
}

/* 
 * 保存查询错误日志
 */
RESULT UpdateAlarmElementParam(INT nNeId, PSTR pszMapId)
{
	STR szSql[MAX_SQL_LEN];
	
	//更新网元信息表
	    sprintf(szSql,"update ne_elementparam set epm_CurValue= '1', epm_UpdateTime=sysdate where epm_NeId= %d and  epm_ObjId= '%s'", 
	                    nNeId, pszMapId);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	    if(ExecuteSQL(szSql)!=NORMAL)
	    {
		    PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		    return EXCEPTION;
	    }
	    CommitTransaction();
	    if(GetAffectedRows() <= 0)
	    {
            memset(szSql, 0, sizeof(szSql));
            sprintf(szSql,"insert into ne_ElementParam(epm_NeId,epm_ObjId, epm_CurValue, epm_UpdateTime) values(%d, '%s', '1', sysdate)", nNeId, pszMapId);
	     	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	    	if(ExecuteSQL(szSql)!=NORMAL)
	     	{ 
		    	PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		    	return EXCEPTION;
	     	}
	    }
	     
	    
	    return NORMAL;
}

/* 
 * 保存查询错误日志
 */
RESULT SaveSysErrorLog(INT nNeId, PSTR pszErrMapId,PSTR pszErrorId, PSTR pszRecvMsg)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	//int n_taskid, n_tasklogid;
	STR szErrorMemo[200];
	PSTR pszErrMapIdStr[MAX_SEPERATE_NUM];
	PSTR pszErrorIdStr[MAX_SEPERATE_NUM];
	INT nSeperateNum, i;
	
	//分解错误监控量
	nSeperateNum = SeperateString(pszErrMapId,  ',', pszErrMapIdStr, MAX_SEPERATE_NUM);
	nSeperateNum = SeperateString(pszErrorId,  ',', pszErrorIdStr, MAX_SEPERATE_NUM);
	for(i=0; i< nSeperateNum; i++)
	{
	    //取man_EleQryLog
	    memset(szSql, 0, sizeof(szSql));
	    sprintf(szSql, " select   ma_MapErrorName from ne_2GMapError where ma_MapErrorId= %d", atoi(pszErrorIdStr[i]));
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
	    strcpy(szErrorMemo,  GetTableFieldValue(&struCursor, "ma_MapErrorName"));
	    FreeCursor(&struCursor);
        
	    memset(szSql, 0, sizeof(szSql));
	    sprintf(szSql, "insert into sys_errorlog(sys_errorid,sys_errortyle,sys_neid,sys_errormemo, sys_recvmsg) values(%s, '%s通信错误提示', %d, '%s', '%s')", 
	            pszErrorIdStr[i], pszErrMapIdStr[i], nNeId, szErrorMemo, pszRecvMsg);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
	    if(ExecuteSQL(szSql)!=NORMAL)
	    {
	    	 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
	    	 return EXCEPTION;
	    }
    }
    
    return NORMAL;
	   
}



/* 
 * 分解查询网元参数
 */
RESULT DecodeQryElementParam(SENDPACKAGE *pstruSendPackage)
{
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	STR szProperty[2000];
	STR szContent[2000];
	STR szFailContent[2000];
	STR szFailMapId[1000];
	STR szErrorId[1000];
	STR szAlarmName[1000];
	STR szAlarmValue[1000];
	STR szNewAlarmName[1000];
	STR szTemp[100];
	STR szQrySerial[29];
	STR szAlarmObjList[1000];
	PSTR pszAlarmNameStr[MAX_SEPERATE_NUM];
	PSTR pszAlarmValueStr[MAX_SEPERATE_NUM];
	INT nSeperateNum, i;
	STR szAlarmObjResotre[1000];
	
	//初始化变量
	memset(szProperty, 0, sizeof(szProperty));
	memset(szContent, 0, sizeof(szContent));
	memset(szFailContent, 0, sizeof(szFailContent));
	memset(szAlarmName, 0, sizeof(szAlarmName));
	memset(szAlarmValue, 0, sizeof(szAlarmValue));
	memset(szNewAlarmName, 0, sizeof(szNewAlarmName));
	memset(szFailMapId, 0, sizeof(szFailMapId));
	memset(szErrorId, 0, sizeof(szErrorId));
	memset(szErrorId, 0, sizeof(szErrorId));
	
	strcpy(szQrySerial, pstruSendPackage->struHead.QA);
	n_gNeId = pstruSendPackage->nNeId;
	n_gTaskId = pstruSendPackage->nTaskId;
	n_gTaskLogId = pstruSendPackage->nTaskLogId;
	
	memset(pstruXml,0,sizeof(XMLSTRU));
	CreateXml(pstruXml,FALSE,OMC_ROOT_PATH,NULL);
	sprintf(szTemp, "%d", pstruSendPackage->nNeId);
	InsertInXmlExt(pstruXml,"<omc>/<网元编号>",szTemp,MODE_AUTOGROW|MODE_UNIQUENAME);
	for (i=0; i< pstruSendPackage->struHead.nObjectCount; i++)
	{
		if (pstruSendPackage->struMapObjList[i].cErrorId == '0')//监控对象标号正确
		{
			if (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "0007") == 0)//经度
			{
				strcpy(szTemp,pstruSendPackage->struMapObjList[i].szMapData);
				if (strncmp(szTemp, "E", 1) == 0)
					strcpy(pstruSendPackage->struMapObjList[i].szMapData,szTemp+1);
			}
			if (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "0008") == 0)//经度
			{
				strcpy(szTemp,pstruSendPackage->struMapObjList[i].szMapData);
				if (strncmp(szTemp, "N", 1) == 0)
					strcpy(pstruSendPackage->struMapObjList[i].szMapData,szTemp+1);
			}
			//if (strlen(pstruSendPackage->struMapObjList[i].szMapData) > 0)//判断是否为空值,为空不处理
			{
		    sprintf(szProperty, "%s%s,", szProperty, pstruSendPackage->struMapObjList[i].szMapId);
		    sprintf(szContent, "%s%s,", szContent, pstruSendPackage->struMapObjList[i].szMapData);
			}
			 //处理告警
			 if (strcmp(pstruSendPackage->struMapObjList[i].szMapType, "alarm") == 0)
			 {
			     //
			     sprintf(szAlarmName, "%s%s,", szAlarmName, pstruSendPackage->struMapObjList[i].szMapId);
		         sprintf(szAlarmValue, "%s%s,", szAlarmValue, pstruSendPackage->struMapObjList[i].szMapData);
			 	 if (strcmp(pstruSendPackage->struMapObjList[i].szMapData, "1") == 0)
			 	 {
			 	     //根据MapId取告警名AlarmName
			 	     sprintf(szNewAlarmName, "%s%s,", szNewAlarmName, GetAlarmName(pstruSendPackage->struMapObjList[i].szMapId));
		    
			 	 }
			 }
			 //性能监控
			 if (strcmp(pstruSendPackage->struMapObjList[i].szMapType, "realtime") == 0) 
			 {
			 	 SaveEffectControl(pstruSendPackage->nNeId,
			 	 	pstruSendPackage->struMapObjList[i].szMapId, pstruSendPackage->struMapObjList[i].szMapData);
			 }			
			 if ((strcmp(pstruSendPackage->struMapObjList[i].szMapId, "0801") == 0)    //电源掉电告警/告警恢复时间
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "0802") == 0)  //接收电平强度告警/告警恢复时间
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "0803") == 0)
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "080F") == 0) //电池低电压告警/告警恢复时间
			    
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "04F0") == 0)
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "04F1") == 0)
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "04F2") == 0)
			    || (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "04F3") == 0)
			 )
			 {
			 	 InsertInXmlExt(pstruXml,"<omc>/<告警时间>",pstruSendPackage->struMapObjList[i].szMapData,
			 	 	MODE_AUTOGROW|MODE_UNIQUENAME);
			 }
			 
			 //取服务小区号 2010.3.26 update
			 if (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "050C") == 0)
			 {
			 	 InsertInXmlExt(pstruXml,"<omc>/<服务小区>",pstruSendPackage->struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
			 }
			 //取电平强度 2010.3.26 update
			 if (strcmp(pstruSendPackage->struMapObjList[i].szMapId, "050B") == 0 ||
			 	 strcmp(pstruSendPackage->struMapObjList[i].szMapId, "08B3") == 0)
			 {
			 	 InsertInXmlExt(pstruXml,"<omc>/<电平强度>",pstruSendPackage->struMapObjList[i].szMapData,
	             	 	 		MODE_AUTOGROW|MODE_UNIQUENAME);
			 }
			 
			 
		}
		else//监控对象标号有错误
		{
		    //用空格分割
		    sprintf(szFailMapId, "%s%s,", szFailMapId, pstruSendPackage->struMapObjList[i].szMapId);
		    sprintf(szErrorId, "%s%c,", szErrorId, pstruSendPackage->struMapObjList[i].cErrorId);
            sprintf(szProperty, "%s%s,", szProperty, pstruSendPackage->struMapObjList[i].szMapId);
			
            if (pstruSendPackage->struMapObjList[i].cErrorId == '2' ||
            	pstruSendPackage->struMapObjList[i].cErrorId == '5' ||
            	pstruSendPackage->struMapObjList[i].cErrorId == '6' )
			    sprintf(szContent, "%s%s,", szContent, pstruSendPackage->struMapObjList[i].szMapData);
			else  
				sprintf(szContent, "%s ,", szContent);
			
		    sprintf(szFailContent, "%s%s,", szFailContent, pstruSendPackage->struMapObjList[i].szMapData);
		}
	}
	TrimRightChar(szProperty, ',');
	TrimRightOneChar(szContent, ',');
	TrimRightChar(szAlarmName, ',');
	TrimRightChar(szAlarmValue, ',');
	TrimRightChar(szNewAlarmName, ',');

	
	PrintDebugLog(DBG_HERE, "QrySerial=[%s],Property[%s],Content[%s],AlarmName[%s],AlarmValue[%s],NewAlarmName[%s],FailMapId[%s]\n",
	     szQrySerial, szProperty, szContent, szAlarmName, szAlarmValue, szNewAlarmName,szFailMapId);
	
	//更新数据库失败内容
	if (strlen(TrimAllSpace(szFailMapId)) > 0)
	{
	    TrimRightChar(szFailMapId, ',');
	    TrimRightChar(szErrorId, ',');
	    SetEleQryLogErrorFromComm(szQrySerial, szFailMapId,  szErrorId, szFailContent);
	    SaveSysErrorLog(n_gNeId, szFailMapId,  szErrorId, pstruSendPackage->szRecvMsg);
	}
	
	
	//更新网元查询记录表,并处理新告警szNewAlarmName
	UpdateEleFromCommByPacket(szQrySerial,szContent, szAlarmName, szAlarmValue);
	
	//更新网元查询信息(对站点查询后返回结果的更新包括网元表和网元辅助表)成功时
	UpdateEleQryLogFromComm(pstruSendPackage, szProperty, szContent);
	
		
	//处理告警
	if (strlen(szAlarmName) > 0)
	{
	    BOOL bAlarmRestore = BOOLTRUE ;//默认为告警恢复前转
	    //取网元名NeName
	    strcpy(szAlarmObjList, GetAlarmObjList(n_gNeId));
	    InsertInXmlExt(pstruXml,"<omc>/<告警列表>",szAlarmObjList,MODE_AUTOGROW|MODE_UNIQUENAME);
	    //分解告警名
	    nSeperateNum = SeperateString(szAlarmName,  ',', pszAlarmNameStr, MAX_SEPERATE_NUM);
	    //分解告警mapid
	    nSeperateNum = SeperateString(szAlarmValue,  ',', pszAlarmValueStr, MAX_SEPERATE_NUM);
	    for(i=0; i< nSeperateNum; i++)
	    {
	        if (strstr(szAlarmObjList, pszAlarmNameStr[i]) == NULL)
	            continue;
			InsertInXmlExt(pstruXml,"<omc>/<告警对象>", pszAlarmNameStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
	        if (strcmp(pszAlarmValueStr[i], "1") == 0)//告警处理
	        {
	            bAlarmRestore = BOOLFALSE ;  //取消告警前转
	            sprintf(szTemp, "%s:0",  pszAlarmNameStr[i]);
	            if (strstr(szAlarmObjList, szTemp) != NULL)//只有站点确实处于非告警状态才告警
	            {
	                DealNewAlarm(pstruXml);
	             }
	        }
	        else if (strcmp(pszAlarmValueStr[i], "0") == 0)//只有站点确实处于告警状态才恢复
	        {
	            sprintf(szTemp, "%s:1",  pszAlarmNameStr[i]);
	            if (strstr(szAlarmObjList, szTemp) != NULL || n_gTaskId == 0)
	            {
	                AlarmComeback(pstruXml);
	                sprintf(szAlarmObjResotre, "%s%s", szAlarmObjResotre, pszAlarmNameStr[i]);
	            }
	        }
	        //UpdateAlarmElementParam(n_gNeId, pszAlarmNameStr[i]);
	    }
	    

	    
	}
	
	return NORMAL;
	
}

/* 
 * 分解查询监控列表
 */
RESULT DecodeQueryMapList(SENDPACKAGE *pstruSendPackage)
{
    STR szMapId0009List[MAX_BUFFER_LEN*2];
	STR szQrySerial[29], szMapListId[29];
	STR szProvinceId[10];
	BOOL bResult;
    
    memset(szMapId0009List, 0, sizeof(szMapId0009List));
	strcpy(szQrySerial, pstruSendPackage->struHead.QA);
	n_gNeId = pstruSendPackage->nNeId;
	
	bufclr(szMapId0009List);
	if (strstr(szQrySerial, "Gprs") == NULL)//如果不是GPRS
	{
		sprintf(szMapListId, "Ne%u%d", pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId);
		if (pstruSendPackage->struHead.nProtocolType != 47)
		{
			if (GetMapId0009List(szMapListId, szMapId0009List) != NORMAL)
			{
				 PrintErrorLog(DBG_HERE,"执行开站上报的监控量列表失败\n");
		     	 return EXCEPTION;
			}
		}
		else
		{
			if (GetJinXinMapId0009List(szMapListId, szMapId0009List) != NORMAL)
			{
				 PrintErrorLog(DBG_HERE,"执行开站上报的监控量列表失败\n");
		     	 return EXCEPTION;
			}
		}
	}
	else
	{         		
		sprintf(szMapListId, "Ne%u%d", pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId);
		if (pstruSendPackage->struHead.nProtocolType != 47)
		{
			GetMapId0009List(szMapListId, szMapId0009List);
		}
		else
		{
			GetJinXinMapId0009List(szMapListId, szMapId0009List);
		}
	}
	//2014.6.17新加
	char szSql[MAX_BUFFER_LEN];
	memset(szSql, 0, sizeof(szSql));
    sprintf(szSql,"delete from ne_maplist where qs_qrynumber = '%s'", szMapListId);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
    CommitTransaction();
	
	GetSysParameter("par_SectionName = 'Province' and par_KeyName = 'ProvinceAreCode'", szProvinceId);
	if (ExistNeId(pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId) == BOOLTRUE)
	{
	    //更新上报的监控量列表
	    bResult = UpdateEleObjList(pstruSendPackage, szMapId0009List, szProvinceId, 0);
	}
	else
	{
	    //是开站上报的监控量列表
	    bResult = UpdateEleObjList(pstruSendPackage, szMapId0009List, szProvinceId, 1);
	}
	
	if (bResult != NORMAL)
	{
	     PrintErrorLog(DBG_HERE,"执行开站上报的监控量列表失败\n");
	     return EXCEPTION;
	}
	UpdateEleFromCommByPacket(szQrySerial, "1", "", "");
	
	if (pstruSendPackage->struHead.nProtocolType != 47)
	{
		{
			 QryEleFristTime(n_gNeId, M2G_TCPIP);
		}
	}

    return NORMAL;
}


/* 
 * 分解查询监控列表
 */
RESULT DecodeQueryMap0001List(SENDPACKAGE *pstruSendPackage)
{
    STR szMapId0001List[MAX_BUFFER_LEN], szMapIdList[MAX_BUFFER_LEN];
	STR szQrySerial[29], szMapListId[29];
	STR szProvinceId[10];
	BOOL bResult;
    
    memset(szMapId0001List, 0, sizeof(szMapId0001List));
    memset(szMapIdList, 0, sizeof(szMapIdList));
	strcpy(szQrySerial, pstruSendPackage->struHead.QA);
	n_gNeId = pstruSendPackage->nNeId;
	
	
	if (strstr(szQrySerial, "Gprs") == NULL)//如果不是GPRS
	{
		sprintf(szMapListId, "Ne%u%d", pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId);
		if (GetMapId0001List(szMapListId, szMapId0001List, szMapIdList) != NORMAL)
		{
			 PrintErrorLog(DBG_HERE,"执行开站上报的监控量列表失败\n");
	     	 return EXCEPTION;
		}
	}
	else
	{         		
		sprintf(szMapListId, "Ne%u%d", pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId);
		GetMapId0001List(szMapListId, szMapId0001List, szMapIdList);
	}
	
	GetSysParameter("par_SectionName = 'Province' and par_KeyName = 'ProvinceAreCode'", szProvinceId);
	if (ExistNeId(pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId) == BOOLTRUE)
	{
	    //更新上报的监控量列表
	    bResult = UpdateEleObj0001List(pstruSendPackage, szMapId0001List, szMapIdList, szProvinceId, 0);
	}
	else
	{
	    //是开站上报的监控量列表
	    bResult = UpdateEleObj0001List(pstruSendPackage, szMapId0001List, szMapIdList, szProvinceId, 1);
	}
	
	if (bResult != NORMAL)
	{
	     PrintErrorLog(DBG_HERE,"执行开站上报的监控量列表失败\n");
	     return EXCEPTION;
	}
	UpdateEleFromCommByPacket(szQrySerial, "1", "", "");
	

	{
		 QryEleFristTime(n_gNeId, M2G_TCPIP);
	}

    return NORMAL;
}

RESULT DeleDasNeElement(UINT nRepeaterId)
{
	STR szSql[MAX_SQL_LEN];
	
	memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "INSERT INTO ne_elementdelete SELECT * FROM ne_element  WHERE NE_REPEATERID = %u AND NE_ROUTE NOT IN (SELECT route_addr FROM ne_daslist WHERE repead_id = %u)",
         nRepeaterId, nRepeaterId);
     
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        //return EXCEPTION;
	}
	
	CommitTransaction();
	//if(GetAffectedRows() > 0)
	{
		memset(szSql, 0, sizeof(szSql));
	    snprintf(szSql, sizeof(szSql), "DELETE FROM ne_element WHERE NE_REPEATERID = %u AND NE_ROUTE NOT IN (SELECT route_addr FROM ne_daslist WHERE repead_id = %u)",
	         nRepeaterId, nRepeaterId);
	     
	    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
	        return EXCEPTION;
		}
	}
    
    
    return NORMAL;
}

/* 
 * 分解查询DAS监控列表
 */
RESULT DecodeQueryDasList(SENDPACKAGE *pstruSendPackage)
{
    STR szMapId0009List[MAX_BUFFER_LEN];
	STR szQrySerial[29];
	STR szProvinceId[10];
	STR szRouteAddr[20];
	UINT nRepeaterId, nDeviceTypeId;
	BOOL bResult;
	char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
    UINT nNeId;
    
    memset(szMapId0009List, 0, sizeof(szMapId0009List));
	strcpy(szQrySerial, pstruSendPackage->struHead.QA);
	
	n_gNeId = pstruSendPackage->nNeId;
	nRepeaterId = pstruSendPackage->struRepeater.nRepeaterId;
	
	//删除不存在的网元表
	DeleDasNeElement(nRepeaterId);
	
	GetSysParameter("par_SectionName = 'Province' and par_KeyName = 'ProvinceAreCode'", szProvinceId);
	
	sprintf(szSql,"SELECT device_id, route_addr, ip_addr, device_typeid, conn_stat, addr_info FROM ne_daslist WHERE repead_id = %u", nRepeaterId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while(FetchCursor(&struCursor) == NORMAL)
	{
		nDeviceTypeId = atoi(GetTableFieldValue(&struCursor,"device_typeid"));
		strcpy(szRouteAddr, TrimAllSpace(GetTableFieldValue(&struCursor,"route_addr")));
		
		pstruSendPackage->struRepeater.nDeviceId = atoi(GetTableFieldValue(&struCursor,"device_id"));
		pstruSendPackage->struRepeater.nDeviceTypeId = nDeviceTypeId;
		strcpy(pstruSendPackage->struRepeater.szRouteAddr, szRouteAddr);
		//strcpy(pstruSendPackage->struRepeater.szIP, TrimAllSpace(GetTableFieldValue(&struCursor,"ip_addr")));
		pstruSendPackage->struRepeater.nConnStatus = atoi(GetTableFieldValue(&struCursor,"conn_stat"));
		
		strcpy(pstruSendPackage->struRepeater.szAddrInfo, GetTableFieldValue(&struCursor,"addr_info"));
		
		GetDasMapIdList(nDeviceTypeId, szMapId0009List);
		PrintDebugLog(DBG_HERE,"DasList[%s]\n",szMapId0009List);
		if (ExistDasNeId(nRepeaterId, pstruSendPackage->struRepeater.nDeviceId, szRouteAddr) == BOOLTRUE)
		{
		    //更新上报的监控量列表
		    bResult = UpdateDasEleObjList(pstruSendPackage, szMapId0009List, szProvinceId, 0);
		}
		else
		{
		    //是开站上报的监控量列表, 新增
		    GetDbSerial(&nNeId, "ne_Element");
		    pstruSendPackage->nNeId = nNeId;
		    n_gNeId = pstruSendPackage->nNeId;
		    bResult = UpdateDasEleObjList(pstruSendPackage, szMapId0009List, szProvinceId, 1);
		}
		if (bResult != NORMAL)
		{
		     PrintErrorLog(DBG_HERE,"执行开站上报的监控量列表失败\n");
		     return EXCEPTION;
		}
		QryEleFristTime(n_gNeId, pstruSendPackage->struRepeater.nCommType);
	}
	FreeCursor(&struCursor);
	
	UpdateEleFromCommByPacket(szQrySerial, "1", "", "");
	

    return NORMAL;
}


RESULT DeleRfidEle(UINT nRepeaterId, int nDeviceId)
{
	STR szSql[MAX_SQL_LEN];
	/*
	memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "INSERT INTO ne_rfiddelete SELECT * FROM ne_rfid  WHERE RFI_REPEATERID = %d AND RFI_DEVICEID = %d",
         nRepeaterId, nDeviceId);
     
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	if(GetAffectedRows() > 0)
	*/
	{
		memset(szSql, 0, sizeof(szSql));
	    snprintf(szSql, sizeof(szSql), "DELETE FROM ne_rfid WHERE RFI_REPEATERID = %u AND RFI_DEVICEID = %d",
	         nRepeaterId, nDeviceId);
	     
	    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
	        return EXCEPTION;
		}
	}
    
    
    return NORMAL;
}

/* 
 * 分解查询Rfid监控列表
 */
RESULT DecodeQueryRfidList(SENDPACKAGE *pstruSendPackage)
{
	STR szQrySerial[29];
	STR szRouteAddr[20];
	UINT nRepeaterId, nDeviceId, nDeviceTypeId;
	BOOL bResult;
	char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
    
	strcpy(szQrySerial, pstruSendPackage->struHead.QA);
	n_gNeId = pstruSendPackage->nNeId;
	nRepeaterId = pstruSendPackage->struRepeater.nRepeaterId;
	nDeviceId = pstruSendPackage->struRepeater.nDeviceId;
	//删除不存在的RFID
	DeleRfidEle(nRepeaterId, nDeviceId);
	
	sprintf(szSql,"SELECT device_id, route_addr, device_typeid, conn_stat FROM ne_rfidlist WHERE repead_id = %u and device_id = %d", 
		nRepeaterId, nDeviceId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while(FetchCursor(&struCursor) == NORMAL)
	{
		nDeviceTypeId = atoi(GetTableFieldValue(&struCursor,"device_typeid"));
		strcpy(szRouteAddr, TrimAllSpace(GetTableFieldValue(&struCursor,"route_addr")));
		
		pstruSendPackage->struRepeater.nDeviceId = atoi(GetTableFieldValue(&struCursor,"device_id"));
		pstruSendPackage->struRepeater.nConnStatus = atoi(GetTableFieldValue(&struCursor,"conn_stat"));
		pstruSendPackage->struRepeater.nDeviceTypeId = nDeviceTypeId;
		strcpy(pstruSendPackage->struRepeater.szRouteAddr, szRouteAddr);
	
		/*
		if (ExistRfidEle(nRepeaterId, szRouteAddr) == BOOLTRUE)
		{
		    //更新上报的监控量列表
		    bResult = UpdateRfidEleObjList(pstruSendPackage,  0);
		}
		else*/
		{
		    //是开站上报的监控量列表, 新增
		    bResult = UpdateRfidEleObjList(pstruSendPackage,  1);
		}
		if (bResult != NORMAL)
		{
		     PrintErrorLog(DBG_HERE,"执行开站上报的监控量列表失败\n");
		     return EXCEPTION;
		}
	}
	FreeCursor(&struCursor);
	
	UpdateEleFromCommByPacket(szQrySerial, "1", "", "");
	

    return NORMAL;
}

/*
 * 判断是否存在非法的监控量
 */
RESULT IsExistMapId(PSTR pszMapId)
{
	if (strcmp(pszMapId, "07A3") == 0) return -1;
	else if (strcmp(pszMapId, "07B1") == 0) return -1;
	else if (strcmp(pszMapId, "07C4") == 0) return -1;
	else if (strcmp(pszMapId, "07C5") == 0) return -1;
	else if (strcmp(pszMapId, "07C6") == 0) return -1;
	else if (strcmp(pszMapId, "07E2") == 0) return -1;
	else if (strcmp(pszMapId, "07E7") == 0) return -1;
	else if (strcmp(pszMapId, "07E8") == 0) return -1;
	else if (strcmp(pszMapId, "080E") == 0) return -1;
	else if (strcmp(pszMapId, "0804") == 0) return -1;
	else if (strcmp(pszMapId, "08F0") == 0) return -1;
	else if (strcmp(pszMapId, "0771") == 0) return -1;
	else if (strcmp(pszMapId, "0773") == 0) return -1;
	else if (strcmp(pszMapId, "0774") == 0) return -1;
	else if (strcmp(pszMapId, "0775") == 0) return -1;
	else if (strcmp(pszMapId, "0776") == 0) return -1;
	else if (strcmp(pszMapId, "0777") == 0) return -1;
	else if (strcmp(pszMapId, "0778") == 0) return -1;
	else if (strcmp(pszMapId, "077A") == 0) return -1;
	else if (strcmp(pszMapId, "08AF") == 0) return -1;
	else if (strcmp(pszMapId, "077B") == 0) return -1;
	else if (strcmp(pszMapId, "077C") == 0) return -1;
	else if (strcmp(pszMapId, "0606") == 0) return -1;
	else if (strcmp(pszMapId, "00000606") == 0) return -1;
	return NORMAL;
}

/* 
 * 查询0AEC
 * nUpdateWay: 1表示新增，0 表示更新
 */
RESULT UpdateDasEleObjList(SENDPACKAGE *pstruSendPackage,  PSTR pszObjectList, PSTR pszProvinceId, INT nUpdateWay)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	STR szObjectList[MAX_BUFFER_LEN];
	STR szNeAlarmObjListOld[2000];
	STR szNeActiveCol[4000];
	STR szNeActiveRow[4000];
	STR szNeAlarmEnabled[4000];
	STR szNeAlarmObjList[4000];
	PSTR pszObjectIdStr[MAX_SEPERATE_NUM];
	INT nSeperateNum, i;
	UINT nNeId;
	STR szQryNumber[28];
	STR szActiveType[20], szDataType[20];
	STR szTemp[10];
	int nAreaId = 0;
	
	//if (nUpdateWay == 1)//新增
	//	GetDbSerial(&nNeId, "ne_Element");
	//else
	nNeId = pstruSendPackage->nNeId;
		
	strcpy(szQryNumber, pstruSendPackage->struHead.QA);
	
	bufclr(szNeAlarmObjListOld);
	if (nUpdateWay == 0)//更新
	{
        strcpy(szNeAlarmObjListOld, GetAlarmObjList(nNeId));
	}
	
	//if (nUpdateWay == 1)//新增
	{
		sprintf(szSql,"delete from ne_ElementParam where epm_NeId = %d", nNeId);
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
		if(ExecuteSQL(szSql)!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
			return EXCEPTION;
		}
		CommitTransaction();
		
		bufclr(szNeActiveCol);
	    bufclr(szNeActiveRow);
	    bufclr(szNeAlarmEnabled);
	    bufclr(szNeAlarmObjList);
	    bufclr(szObjectList);
	    
	    //strcpy(szObjectList, pszObjectList);
		nSeperateNum = SeperateString(pszObjectList,  ',', pszObjectIdStr, MAX_SEPERATE_NUM);
	    for(i=0; i< nSeperateNum; i++)
		{
	        if (IsExistMapId(pszObjectIdStr[i]) == NORMAL)
	        {
	             sprintf(szSql,"select obj_DataType, obj_ActiveType from ne_ObjectsList where obj_ObjId = '%s'", pszObjectIdStr[i]);
		         //PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
		         if(SelectTableRecord(szSql,&struCursor) != NORMAL)
		         {
		         	PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		         	return EXCEPTION;
		         }
		         if (FetchCursor(&struCursor) != NORMAL)
		         {
	                  PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", \
						  szSql, GetSQLErrorMessage());
				      FreeCursor(&struCursor);
				      continue;
		         }
		         strcpy(szDataType, TrimAllSpace(GetTableFieldValue(&struCursor,"obj_DataType")));
		         strcpy(szActiveType, TrimAllSpace(GetTableFieldValue(&struCursor,"obj_ActiveType")));
		         FreeCursor(&struCursor);
		         //拨测结果不记录
		         if (strcmp(szDataType, "HEX") == 0)
		            continue;
		         //存在所有对象的列表
		         sprintf(szObjectList, "%s%s,", szObjectList, pszObjectIdStr[i]);
		         
		         if (strcmp(szActiveType, "omc") == 0 || strcmp(szActiveType, "base") == 0 || 
		             strcmp(szActiveType, "no__omc") == 0 || strcmp(szActiveType, "no__base") == 0)
		         {
		             sprintf(szNeActiveCol, "%s%s,", szNeActiveCol, pszObjectIdStr[i]);
		             
		             if (strcmp(pszObjectIdStr[i], "0150") == 0 || strcmp(pszObjectIdStr[i], "07D1") == 0 ||
	                 strcmp(pszObjectIdStr[i], "0010") == 0 || strcmp(pszObjectIdStr[i], "0018") == 0 ||
	                 strcmp(pszObjectIdStr[i], "0020") == 0 || strcmp(pszObjectIdStr[i], "0011") == 0 ||
			 		 strcmp(pszObjectIdStr[i], "07D0") == 0)
		             {
		                 sprintf(szSql,"insert into ne_ElementParam(epm_NeId,epm_ObjId) values(%d,'%s')", nNeId, pszObjectIdStr[i]);
		                 PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
		                 if(ExecuteSQL(szSql)!=NORMAL)
		                 { 
			                 PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
			                 return EXCEPTION;
		                 }
		                 CommitTransaction();
		             }
		         }
		         else if (strcmp(szActiveType, "alarmYN") == 0)
		         {
		             sprintf(szNeAlarmEnabled, "%s%s:0,", szNeAlarmEnabled, pszObjectIdStr[i]);
		         }
		         else if (strcmp(szActiveType, "alarm") == 0)
		         {
		             sprintf(szNeAlarmObjList, "%s%s:0,", szNeAlarmObjList, pszObjectIdStr[i]);
		             //PrintDebugLog(DBG_HERE,"[%s]\n",szNeAlarmObjList);
		         }
		         else//mod by wwj at 2010.07.27
		         {
		             sprintf(szNeActiveRow, "%s%s,", szNeActiveRow, pszObjectIdStr[i]);
		             sprintf(szSql,"insert into ne_ElementParam(epm_NeId,epm_ObjId) values(%d,'%s')", nNeId, pszObjectIdStr[i]);
		             PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
		             if(ExecuteSQL(szSql)!=NORMAL)
		             { 
			             PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
			             return EXCEPTION;
		             }
		             CommitTransaction();	         	
		         }	
	  
	        }
	    }
	    //CommitTransaction();
	    
	    TrimRightChar(szNeActiveCol, ',');
	    TrimRightChar(szNeActiveRow, ',');
	    TrimRightChar(szNeAlarmEnabled, ',');
	    TrimRightChar(szNeAlarmObjList, ',');
	    TrimRightChar(szObjectList, ',');
	    
	    STR szRepeaterId[20];

	    sprintf(szRepeaterId, "%08x", pstruSendPackage->struRepeater.nRepeaterId);
	    bufclr(szTemp);
	    memcpy(szTemp, szRepeaterId, 2);
	    int nProvi = strHexToInt(szTemp);
	    bufclr(szTemp);
	    memcpy(szTemp, szRepeaterId+2, 2);
	    int nCity = nProvi * 100 + strHexToInt(szTemp);
	 	sprintf(szSql,"SELECT  are_AreaId from pre_Area where are_code = %d ", nCity);
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
		if(SelectTableRecord(szSql,&struCursor) != NORMAL)
		{
		    PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		    return EXCEPTION;
		}
		if (FetchCursor(&struCursor) == NORMAL)
		{
		    nAreaId = atoi( GetTableFieldValue(&struCursor,"are_AreaId"));
	    
		    //PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		}
		FreeCursor(&struCursor);
    }
    
    memset(szSql, 0, sizeof(szSql));
    if (nUpdateWay == 1)//新增
    {
         STR szNeName[100];
         
         sprintf(szNeName, "开站上报%s", GetSysDateTime());

         snprintf(szSql, sizeof(szSql), 
            " insert into ne_Element( ne_NeId,ne_Name,ne_ActiveCol,ne_ActiveRow,ne_AlarmEnabledObjList,"
	        " ne_AlarmObjList,ne_RepeaterId,ne_DeviceId,ne_InstallationDate,ne_NeTelNum,"
	        " ne_CommTypeId,ne_ServerTelNum,ne_ProtocoltypeId, ne_AreaId, ne_LastUpdateTime,"
	        " ne_OmcTelNum, ne_DeviceStatusId, ne_Deviceip, ne_devicePort, ne_DeviceTypeId, ne_Route) values("
	        " %d,  '%s',  '%s', '%s', '%s',"
	        " '%s', %u, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), '%s',"
	        " %d,  '%s', %d, %d,  to_date( '%s','yyyy-mm-dd hh24:mi:ss'),"
	        " '%s', %d, '%s', %d, %d, '%s')",
	        nNeId, szNeName,  szNeActiveCol, szNeActiveRow, szNeAlarmEnabled,
	        szNeAlarmObjList,  pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId,  GetSysDateTime(), pstruSendPackage->struRepeater.szTelephoneNum,
	        pstruSendPackage->struRepeater.nCommType, pstruSendPackage->struRepeater.szNetCenter, pstruSendPackage->struHead.nProtocolType, nAreaId, GetSysDateTime(),
	        pstruSendPackage->struRepeater.szNetCenter, 17, pstruSendPackage->struRepeater.szIP, pstruSendPackage->struRepeater.nPort,
	        pstruSendPackage->struRepeater.nDeviceTypeId, pstruSendPackage->struRepeater.szRouteAddr);

    }
    else
    {
        snprintf(szSql, sizeof(szSql), 
             "update ne_Element set ne_ActiveCol = '%s',ne_ActiveRow = '%s', ne_AlarmEnabledObjList = '%s', ne_AlarmObjList = '%s', "
             " ne_DeviceId = %d, ne_devicetypeid = %d, ne_Deviceip = '%s', ne_ChangedDate = to_date('%s','yyyy-mm-dd hh24:mi:ss'), ne_LastUpdateTime = to_date('%s','yyyy-mm-dd hh24:mi:ss')"
             " where ne_RepeaterId = %u and ne_Route = '%s'",
             szNeActiveCol, szNeActiveRow, szNeAlarmEnabled, szNeAlarmObjList,
             pstruSendPackage->struRepeater.nDeviceId, pstruSendPackage->struRepeater.nDeviceTypeId, pstruSendPackage->struRepeater.szIP,
             GetSysDateTime(), GetSysDateTime(),
             pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.szRouteAddr);
    }
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	// 自动清除现在没有告警监控量而原先有该告警监控量的告警记录
	if (nUpdateWay == 0)//更新
	{
	    STR szNeAlarmId[10];
	    nSeperateNum = SeperateString(szNeAlarmObjListOld,  ',', pszObjectIdStr, 200);
        for(i=0; i< nSeperateNum; i++)
        {
            if (strstr(pszObjectIdStr[i], ":1") == NULL)//没有告警
	            continue;
	        bufclr(szNeAlarmId);
	        memcpy(szNeAlarmId, pszObjectIdStr[i], 4);
	        memset(szSql, 0, sizeof(szSql));
            snprintf(szSql, sizeof(szSql), 
                "UPDATE alm_AlarmLog SET alg_ClearTime = to_date('%s','yyyy-mm-dd hh24:mi:ss'), alg_AlarmStatusId = 4"
                " WHERE alg_NeId = %d and alg_AlarmStatusId < 4 and alg_alarmId = (select alm_alarmId from  alm_alarm where alm_objid= '%s')",
                GetSysDateTime(), nNeId, szNeAlarmId);
     
            PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	        if(ExecuteSQL(szSql) != NORMAL)
	        {
	        	PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
	        		szSql, GetSQLErrorMessage());
                return EXCEPTION;
	        }
        }
        CommitTransaction();
	}
    
    return NORMAL;
	
}

RESULT UpdateRfidEleObjList(SENDPACKAGE *pstruSendPackage, INT nUpdateWay)
{
	STR szSql[MAX_SQL_LEN];

	INT nNeId;

	nNeId = pstruSendPackage->nNeId;
	
    memset(szSql, 0, sizeof(szSql));
    if (nUpdateWay == 1)//新增
    {
         STR szNeName[100];
         sprintf(szNeName, "%s", GetSysDateTime());
         snprintf(szSql, sizeof(szSql), 
            " insert into ne_rfid( rfi_name, rfi_RepeaterId,rfi_DeviceId, rfi_ConnStatus,rfi_DeviceTypeId,rfi_Route, rfi_updatetime) values("
	        " '%s', %u,  %d, %d, %d, '%s', to_date('%s','yyyy-mm-dd hh24:mi:ss'))",
	        szNeName, pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId,  
	        pstruSendPackage->struRepeater.nConnStatus, pstruSendPackage->struRepeater.nDeviceTypeId,
	         pstruSendPackage->struRepeater.szRouteAddr, GetSysDateTime());
    }
    else
    {
        snprintf(szSql, sizeof(szSql), 
             "update ne_rfid set rfi_connstatus = %d, rfi_devicetypeid = %d, rfi_UpdateTime = to_date('%s','yyyy-mm-dd hh24:mi:ss')"
             " where ne_RepeaterId = %u and ne_Route = '%s'",
             pstruSendPackage->struRepeater.nConnStatus, pstruSendPackage->struRepeater.nDeviceTypeId, GetSysDateTime(),
             pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.szRouteAddr);
    }
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

/* 
 * 开站上报，更新监控参量，更新ne_Element等表
 * nUpdateWay: 1表示新增，0 表示更新
 */
RESULT UpdateEleObjList(SENDPACKAGE *pstruSendPackage,  PSTR pszObjectList, PSTR pszProvinceId, INT nUpdateWay)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	STR szObjectList[MAX_BUFFER_LEN];
	STR szNeAlarmObjListOld[4000];
	STR szNeActiveCol[4000];
	STR szNeActiveRow[4000];
	STR szNeAlarmEnabled[4000];
	STR szNeAlarmObjList[4000];
	PSTR pszObjectIdStr[MAX_SEPERATE_NUM];
	INT nSeperateNum, i;
	INT nNeId;
	STR szQryNumber[28];
	STR szActiveType[20], szDataType[20];
	STR szTemp[10];
	
	
	nNeId = pstruSendPackage->nNeId;
	strcpy(szQryNumber, pstruSendPackage->struHead.QA);
	
	bufclr(szNeAlarmObjListOld);
	if (nUpdateWay == 0)//更新
	{
        strcpy(szNeAlarmObjListOld, GetAlarmObjList(nNeId));
	}

	sprintf(szSql,"delete from ne_ElementParam where epm_NeId = %d", nNeId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	
	bufclr(szNeActiveCol);
    bufclr(szNeActiveRow);
    bufclr(szNeAlarmEnabled);
    bufclr(szNeAlarmObjList);
    bufclr(szObjectList);
    
    //strcpy(szObjectList, pszObjectList);
	nSeperateNum = SeperateString(pszObjectList,  ',', pszObjectIdStr, MAX_SEPERATE_NUM);
    for(i=0; i< nSeperateNum; i++)
	{
        if (IsExistMapId(pszObjectIdStr[i]) == NORMAL)
        {
             sprintf(szSql,"select obj_DataType, obj_ActiveType from ne_ObjectsList where obj_ObjId = '%s'", pszObjectIdStr[i]);
	         //PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	         if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	         {
	         	PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
	         	return EXCEPTION;
	         }
	         if (FetchCursor(&struCursor) != NORMAL)
	         {
                  PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", \
					  szSql, GetSQLErrorMessage());
			      FreeCursor(&struCursor);
			      continue;
	         }
	         strcpy(szDataType, TrimAllSpace(GetTableFieldValue(&struCursor,"obj_DataType")));
	         strcpy(szActiveType, TrimAllSpace(GetTableFieldValue(&struCursor,"obj_ActiveType")));
	         FreeCursor(&struCursor);
	         
	         //存在所有对象的列表
	         sprintf(szObjectList, "%s%s,", szObjectList, pszObjectIdStr[i]);
	         
	         if (strcmp(szActiveType, "omc") == 0 || strcmp(szActiveType, "base") == 0 || 
	             strcmp(szActiveType, "no__omc") == 0 || strcmp(szActiveType, "no__base") == 0)
	         {
	             sprintf(szNeActiveCol, "%s%s,", szNeActiveCol, pszObjectIdStr[i]);
	             
	             if (strcmp(pszObjectIdStr[i], "0150") == 0 || strcmp(pszObjectIdStr[i], "07D1") == 0 ||
	                 strcmp(pszObjectIdStr[i], "0010") == 0 || strcmp(pszObjectIdStr[i], "0018") == 0 ||
	                 strcmp(pszObjectIdStr[i], "0020") == 0 || strcmp(pszObjectIdStr[i], "0011") == 0 ||
			 		 strcmp(pszObjectIdStr[i], "07D0") == 0)
	             {
	                 sprintf(szSql,"insert into ne_ElementParam(epm_NeId,epm_ObjId) values(%d,'%s')", nNeId, pszObjectIdStr[i]);
	                 PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	                 if(ExecuteSQL(szSql)!=NORMAL)
	                 { 
		                 PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		                 return EXCEPTION;
	                 }
	             }
	         }
	         else if (strcmp(szActiveType, "alarmYN") == 0)
	         {
	             sprintf(szNeAlarmEnabled, "%s%s:0,", szNeAlarmEnabled, pszObjectIdStr[i]);
	         }
	         else if (strcmp(szActiveType, "alarm") == 0)
	         {
	             sprintf(szNeAlarmObjList, "%s%s:0,", szNeAlarmObjList, pszObjectIdStr[i]);
	             //PrintDebugLog(DBG_HERE,"[%s]\n",szNeAlarmObjList);
	         }
	         else//mod by wwj at 2010.07.27
	         {
	             sprintf(szNeActiveRow, "%s%s,", szNeActiveRow, pszObjectIdStr[i]);
	             sprintf(szSql,"insert into ne_ElementParam(epm_NeId,epm_ObjId) values(%d,'%s')", nNeId, pszObjectIdStr[i]);
	             PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	             if(ExecuteSQL(szSql)!=NORMAL)
	             { 
		             PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		             return EXCEPTION;
	             }	         	
	         }	
  
        }
    }
    CommitTransaction();
    
    TrimRightChar(szNeActiveCol, ',');
    TrimRightChar(szNeActiveRow, ',');
    TrimRightChar(szNeAlarmEnabled, ',');
    TrimRightChar(szNeAlarmObjList, ',');
    TrimRightChar(szObjectList, ',');
    
    STR szRepeaterId[20];
    int nAreaId = 0;
    sprintf(szRepeaterId, "%08x", pstruSendPackage->struRepeater.nRepeaterId);
    bufclr(szTemp);
    memcpy(szTemp, szRepeaterId, 2);
    int nProvi = strHexToInt(szTemp);
    bufclr(szTemp);
    memcpy(szTemp, szRepeaterId+2, 2);
    int nCity = nProvi * 100 + strHexToInt(szTemp);
    /*
    sprintf(szSql,"SELECT are_AreaId from pre_Area where are_code = %d and are_parentid =  %d", nCity, atoi(pszProvinceId));
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if (FetchCursor(&struCursor) != NORMAL)
	{
	    FreeCursor(&struCursor);
	*/
	    sprintf(szSql,"SELECT are_AreaId from pre_Area where are_code = %d ", nCity);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	    if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	    {
		    PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		    return EXCEPTION;
	    }
	    if (FetchCursor(&struCursor) == NORMAL)
	    {
	        nAreaId = atoi( GetTableFieldValue(&struCursor,"are_AreaId"));
	    }
	//}
	
    FreeCursor(&struCursor);
        
    memset(szSql, 0, sizeof(szSql));
    if (nUpdateWay == 1)//新增
    {
         STR szNeName[100];
         sprintf(szNeName, "开站上报%s", GetSysDateTime());

         snprintf(szSql, sizeof(szSql), 
            " insert into ne_Element( ne_NeId,ne_Name,ne_ActiveCol,"
	        " ne_ActiveRow,ne_RepeaterId,ne_DeviceId,ne_InstallationDate,ne_NeTelNum,"
	        " ne_CommTypeId,ne_ServerTelNum,ne_ProtocoltypeId, ne_AreaId, ne_LastUpdateTime,"
	        " ne_OmcTelNum, ne_DeviceStatusId, ne_Deviceip, ne_devicePort) values("
	        " %d,  '%s', '%s',"
	        " '%s', %u, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), '%s',"
	        " %d,  '%s', %d, %d,  to_date( '%s','yyyy-mm-dd hh24:mi:ss'),"
	        " '%s', %d, '%s', %d)",
	        nNeId, szNeName,  szNeActiveCol,
	        szNeActiveRow,  pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId,  GetSysDateTime(), pstruSendPackage->struRepeater.szTelephoneNum,
	        pstruSendPackage->struRepeater.nCommType, pstruSendPackage->struRepeater.szNetCenter, pstruSendPackage->struHead.nProtocolType, nAreaId, GetSysDateTime(),
	        pstruSendPackage->struRepeater.szNetCenter, 17, pstruSendPackage->struRepeater.szIP, pstruSendPackage->struRepeater.nPort);
    }
    else
    {
        snprintf(szSql, sizeof(szSql), 
             "update ne_Element set ne_ActiveCol = '%s',ne_ActiveRow = '%s',  "
             " ne_RepeaterId = '%u', ne_DeviceId = %d, ne_ChangedDate = to_date('%s','yyyy-mm-dd hh24:mi:ss'), ne_LastUpdateTime = to_date('%s','yyyy-mm-dd hh24:mi:ss')"
             " where ne_NeId = %d",
             szNeActiveCol, szNeActiveRow, 
             pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId, GetSysDateTime(), GetSysDateTime(),
             nNeId);
    }
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	
	snprintf(szSql, sizeof(szSql), 
             "update ne_Element set ne_AlarmEnabledObjList = '%s', ne_AlarmObjList = '%s' where ne_NeId = %d",
             szNeAlarmEnabled, szNeAlarmObjList, nNeId);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}         
	CommitTransaction();
	// 自动清除现在没有告警监控量而原先有该告警监控量的告警记录
	if (nUpdateWay == 0)//更新
	{
	    STR szNeAlarmId[10];
	    nSeperateNum = SeperateString(szNeAlarmObjListOld,  ',', pszObjectIdStr, 200);
        for(i=0; i< nSeperateNum; i++)
        {
            if (strstr(pszObjectIdStr[i], ":1") == NULL)//没有告警
	            continue;
	        bufclr(szNeAlarmId);
	        memcpy(szNeAlarmId, pszObjectIdStr[i], 4);
	        memset(szSql, 0, sizeof(szSql));
            snprintf(szSql, sizeof(szSql), 
                "UPDATE alm_AlarmLog SET alg_ClearTime = to_date('%s','yyyy-mm-dd hh24:mi:ss'), alg_AlarmStatusId = 4"
                " WHERE alg_NeId = %d and alg_AlarmStatusId < 4 and alg_alarmId = (select alm_alarmId from  alm_alarm where alm_objid= '%s')",
                GetSysDateTime(), nNeId, szNeAlarmId);
     
            PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	        if(ExecuteSQL(szSql) != NORMAL)
	        {
	        	PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
	        		szSql, GetSQLErrorMessage());
                return EXCEPTION;
	        }
        }
        CommitTransaction();
	}
    
    return NORMAL;
	
}

RESULT UpdateEleObj0001List(SENDPACKAGE *pstruSendPackage,  PSTR pszObjectList, PSTR pszObj0001List, 
	PSTR pszProvinceId, INT nUpdateWay)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	STR szObjectList[MAX_BUFFER_LEN];
	STR szNeAlarmObjListOld[2000];
	STR szNeActiveCol[4000];
	STR szNeActiveRow[4000];
	STR szNeAlarmEnabled[4000];
	STR szNeAlarmObjList[4000];
	PSTR pszObjectIdStr[MAX_SEPERATE_NUM], pszObj0001Str[MAX_SEPERATE_NUM];
	INT nSeperateNum, i;
	INT nNeId;
	STR szQryNumber[28];
	STR szActiveType[20], szDataType[20];
	STR szTemp[10];
	
	
	nNeId = pstruSendPackage->nNeId;
	strcpy(szQryNumber, pstruSendPackage->struHead.QA);
	
	bufclr(szNeAlarmObjListOld);
	if (nUpdateWay == 0)//更新
	{
        strcpy(szNeAlarmObjListOld, GetAlarmObjList(nNeId));
	}

		
	strcpy(szNeActiveCol, ",");
    strcpy(szNeActiveRow, ",");
    strcpy(szNeAlarmEnabled, ",");
    strcpy(szNeAlarmObjList, ",");
    strcpy(szObjectList, ",");
    
    
    nSeperateNum = SeperateString(pszObj0001List,  ',', pszObj0001Str, MAX_SEPERATE_NUM);
	nSeperateNum = SeperateString(pszObjectList,  ',', pszObjectIdStr, MAX_SEPERATE_NUM);
    for(i=0; i< nSeperateNum; i++)
	{
        //if (IsExistMapId(pszObjectIdStr[i]) == NORMAL)
        {
             sprintf(szSql,"select obj_DataType, obj_ActiveType from ne_ObjectsList where obj_ObjId = '%s'", pszObjectIdStr[i]);
	         //PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	         if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	         {
	         	PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
	         	return EXCEPTION;
	         }
	         if (FetchCursor(&struCursor) != NORMAL)
	         {
                  PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", \
					  szSql, GetSQLErrorMessage());
			      FreeCursor(&struCursor);
			      continue;
	         }
	         strcpy(szDataType, TrimAllSpace(GetTableFieldValue(&struCursor,"obj_DataType")));
	         strcpy(szActiveType, TrimAllSpace(GetTableFieldValue(&struCursor,"obj_ActiveType")));
	         FreeCursor(&struCursor);
	         
	         //存在所有对象的列表
	         sprintf(szObjectList, "%s%s,", szObjectList, pszObjectIdStr[i]);
	         
	         if (strcmp(szActiveType, "omc") == 0 || strcmp(szActiveType, "base") == 0 || 
	             strcmp(szActiveType, "no__omc") == 0 || strcmp(szActiveType, "no__base") == 0)
	         {
	             sprintf(szNeActiveCol, "%s%s,", szNeActiveCol, pszObjectIdStr[i]);
	             
	             if (strcmp(pszObjectIdStr[i], "0150") == 0 || strcmp(pszObjectIdStr[i], "07D1") == 0 ||
	                 strcmp(pszObjectIdStr[i], "0010") == 0 || strcmp(pszObjectIdStr[i], "0018") == 0 ||
	                 strcmp(pszObjectIdStr[i], "0020") == 0 || strcmp(pszObjectIdStr[i], "0011") == 0 ||
			 		 strcmp(pszObjectIdStr[i], "07D0") == 0)
	             {
	                 sprintf(szSql,"insert into ne_ElementParam(epm_NeId,epm_ObjId) values(%d,'%s')", nNeId, pszObjectIdStr[i]);
	                 PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	                 if(ExecuteSQL(szSql)!=NORMAL)
	                 { 
		                 PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		                 return EXCEPTION;
	                 }
	             }
	         }
	         else if (strcmp(szActiveType, "alarmYN") == 0)
	         {
	             sprintf(szNeAlarmEnabled, "%s%s:0,", szNeAlarmEnabled, pszObjectIdStr[i]);
	             sprintf(szSql,"insert into ne_ElementParam(epm_NeId,epm_ObjId, epm_mcpid) values(%d,'%s', '%s')", nNeId, pszObjectIdStr[i], pszObj0001Str[i]);
	             PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	             if(ExecuteSQL(szSql)!=NORMAL)
	             { 
		             PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		             return EXCEPTION;
	             }	       
	         }
	         else if (strcmp(szActiveType, "alarm") == 0)
	         {
	             sprintf(szNeAlarmObjList, "%s%s:0,", szNeAlarmObjList, pszObjectIdStr[i]);
	             //PrintDebugLog(DBG_HERE,"[%s]\n",szNeAlarmObjList);
	             sprintf(szSql,"insert into ne_ElementParam(epm_NeId,epm_ObjId, epm_mcpid) values(%d,'%s', '%s')", nNeId, pszObjectIdStr[i], pszObj0001Str[i]);
	             PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	             if(ExecuteSQL(szSql)!=NORMAL)
	             { 
		             PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		             return EXCEPTION;
	             }	       
	         }
	         else
	         {
	             sprintf(szNeActiveRow, "%s%s,", szNeActiveRow, pszObjectIdStr[i]);
	             sprintf(szSql,"insert into ne_ElementParam(epm_NeId,epm_ObjId, epm_mcpid) values(%d,'%s', '%s')", nNeId, pszObjectIdStr[i], pszObj0001Str[i]);
	             PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	             if(ExecuteSQL(szSql)!=NORMAL)
	             { 
		             PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		             return EXCEPTION;
	             }	         	
	         }
	           
        }
    }
    CommitTransaction();
    
    TrimRightChar(szNeActiveCol, ',');
    TrimRightChar(szNeActiveRow, ',');
    TrimRightChar(szNeAlarmEnabled, ',');
    TrimRightChar(szNeAlarmObjList, ',');
    TrimRightChar(szObjectList, ',');

    
    STR szRepeaterId[20];
    int nAreaId = 0;
    sprintf(szRepeaterId, "%08x", pstruSendPackage->struRepeater.nRepeaterId);
    bufclr(szTemp);
    memcpy(szTemp, szRepeaterId, 2);
    int nProvi = strHexToInt(szTemp);
    bufclr(szTemp);
    memcpy(szTemp, szRepeaterId+2, 2);
    int nCity = nProvi * 100 + strHexToInt(szTemp);
    
	    sprintf(szSql,"SELECT are_AreaId from pre_Area where are_code = %d ", nCity);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	    if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	    {
		    PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		    return EXCEPTION;
	    }
	    if (FetchCursor(&struCursor) == NORMAL)
	    {
	        nAreaId = atoi( GetTableFieldValue(&struCursor,"are_AreaId"));
	    }
	
	
    FreeCursor(&struCursor);
        
    memset(szSql, 0, sizeof(szSql));
    if (nUpdateWay == 1)//新增
    {
         STR szNeName[100];
         sprintf(szNeName, "开站上报%s", GetSysDateTime());
         
         snprintf(szSql, sizeof(szSql), 
            " insert into ne_Element( ne_NeId,ne_Name,ne_ActiveCol,ne_ActiveRow,ne_AlarmEnabledObjList,"
	        " ne_AlarmObjList,ne_RepeaterId,ne_DeviceId,ne_InstallationDate,ne_NeTelNum,"
	        " ne_CommTypeId,ne_ServerTelNum,ne_ProtocoltypeId, ne_AreaId, ne_LastUpdateTime,"
	        " ne_OmcTelNum, ne_DeviceStatusId, ne_Deviceip, ne_devicePort) values("
	        " %d,  '%s', '%s',  '%s', '%s',"
	        " '%s', %u, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), '%s',"
	        " %d,  '%s', %d, %d,  to_date( '%s','yyyy-mm-dd hh24:mi:ss'),"
	        " '%s', %d, '%s', %d)",
	        nNeId, szNeName,  szNeActiveCol, szNeActiveRow, szNeAlarmEnabled,
	        szNeAlarmObjList,  pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId,  GetSysDateTime(), pstruSendPackage->struRepeater.szTelephoneNum,
	        pstruSendPackage->struRepeater.nCommType, pstruSendPackage->struRepeater.szNetCenter, pstruSendPackage->struHead.nProtocolType, nAreaId, GetSysDateTime(),
	        pstruSendPackage->struRepeater.szNetCenter, 17, pstruSendPackage->struRepeater.szIP, pstruSendPackage->struRepeater.nPort);
    }
    else
    {
    	if (strcmp(getenv("DATABASE"), "mysql") == 0)
    	{
        	snprintf(szSql, sizeof(szSql), 
             "update ne_Element set ne_ActiveCol = CONCAT(ne_ActiveCol, '%s'),ne_ActiveRow = CONCAT(ne_ActiveRow, '%s'), ne_AlarmEnabledObjList = CONCAT(ne_AlarmEnabledObjList, '%s'), ne_AlarmObjList = CONCAT(ne_AlarmObjList, '%s'), "
             " ne_RepeaterId = '%u', ne_DeviceId = %d, ne_ChangedDate = to_date('%s','yyyy-mm-dd hh24:mi:ss'), ne_LastUpdateTime = to_date('%s','yyyy-mm-dd hh24:mi:ss')"
             " where ne_NeId = %d",
             szNeActiveCol, szNeActiveRow, szNeAlarmEnabled, szNeAlarmObjList,
             pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId, GetSysDateTime(), GetSysDateTime(),
             nNeId);
        }
    	else
    	{
        	snprintf(szSql, sizeof(szSql), 
             "update ne_Element set ne_ActiveCol = ne_ActiveCol || '%s',ne_ActiveRow = ne_ActiveRow || '%s', ne_AlarmEnabledObjList = ne_AlarmEnabledObjList || '%s', ne_AlarmObjList = ne_AlarmObjList || '%s', "
             " ne_RepeaterId = '%u', ne_DeviceId = %d, ne_ChangedDate = to_date('%s','yyyy-mm-dd hh24:mi:ss'), ne_LastUpdateTime = to_date('%s','yyyy-mm-dd hh24:mi:ss')"
             " where ne_NeId = %d",
             szNeActiveCol, szNeActiveRow, szNeAlarmEnabled, szNeAlarmObjList,
             pstruSendPackage->struRepeater.nRepeaterId, pstruSendPackage->struRepeater.nDeviceId, GetSysDateTime(), GetSysDateTime(),
             nNeId);
        }
    }
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	// 自动清除现在没有告警监控量而原先有该告警监控量的告警记录
	if (nUpdateWay == 0)//更新
	{
	    STR szNeAlarmId[10];
	    nSeperateNum = SeperateString(szNeAlarmObjListOld,  ',', pszObjectIdStr, 200);
        for(i=0; i< nSeperateNum; i++)
        {
            if (strstr(pszObjectIdStr[i], ":1") == NULL)//没有告警
	            continue;
	        bufclr(szNeAlarmId);
	        memcpy(szNeAlarmId, pszObjectIdStr[i], 4);
	        memset(szSql, 0, sizeof(szSql));
            snprintf(szSql, sizeof(szSql), 
                "UPDATE alm_AlarmLog SET alg_ClearTime = to_date('%s','yyyy-mm-dd hh24:mi:ss'), alg_AlarmStatusId = 4"
                " WHERE alg_NeId = %d and alg_AlarmStatusId < 4 and alg_alarmId = (select alm_alarmId from  alm_alarm where alm_objid= '%s')",
                GetSysDateTime(), nNeId, szNeAlarmId);
     
            PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	        if(ExecuteSQL(szSql) != NORMAL)
	        {
	        	PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
	        		szSql, GetSQLErrorMessage());
                return EXCEPTION;
	        }
        }
        CommitTransaction();
	}
    
    return NORMAL;
	
}


/* 
 * 根据属性和参数内容更新ne_Element等表
 */
RESULT UpdateEleQryLogFromComm(SENDPACKAGE *pstruSendPackage, PSTR pszProperty, PSTR pszContent)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	STR szElementSql[MAX_SQL_LEN];
	STR szMonitorSql[MAX_SQL_LEN];
	PSTR pszPropertyStr[MAX_SEPERATE_NUM];
	PSTR pszContentStr[MAX_SEPERATE_NUM];
	INT nSeperateNum, i;
	STR szAlarmEnabledObjList[1000];
	BOOL bAlarmEnabled = BOOLFALSE, bCqtMatche=BOOLFALSE;
	int nCount;
	STR szCallNum[20], szBeCallNum[20], szTelNum[20];
	INT n_protocoltypeid, n_protocoldevicetypeid, n_neid;
	TBINDVARSTRU struBindVar;
	
	bufclr(szCallNum);
	bufclr(szBeCallNum);
	n_neid = pstruSendPackage->nNeId;

	n_protocoltypeid = pstruSendPackage->struHead.nProtocolType;
	n_protocoldevicetypeid = pstruSendPackage->struRepeater.nProtocolDeviceType;
	strcpy(szTelNum, pstruSendPackage->struRepeater.szTelephoneNum);
		
	
	//分解监控量内容
	nSeperateNum = SeperateString(pszContent,  ',', pszContentStr, MAX_SEPERATE_NUM);
	//分解监控mapid
	nSeperateNum = SeperateString(pszProperty,  ',', pszPropertyStr, MAX_SEPERATE_NUM);
	
	memset(szElementSql, 0, sizeof(szElementSql));
	sprintf(szElementSql, "update ne_Element set");
	
	memset(szMonitorSql, 0, sizeof(szMonitorSql));
	sprintf(szMonitorSql, "update ne_SignerMonitorGprs set");
	for(i=0; i< nSeperateNum; i++)
	{
	    if (strcmp(pszPropertyStr[i], "0002") == 0 || strcmp(pszPropertyStr[i], "00000002") == 0) //公司
	    {
	        //if (n_protocoltypeid == 1)
	        sprintf(szElementSql, "%s ne_CompanyId= %s,", szElementSql, pszContentStr[i]);
	    }
	    else if (strcmp(pszPropertyStr[i], "0003") == 0 || strcmp(pszPropertyStr[i], "00000003") == 0) //设备类别
	    {
	        if (n_protocoldevicetypeid != atoi(pszContentStr[i]))
	        {
	            //if(n_protocoltypeid == 1)
	            {
	                sprintf(szSql, "select dtp_devicetypeid from ne_devicetype where dtp_ProtocolDeviceTypeId= %s and dtp_protocoltypeid= %d",
	                    pszContentStr[i], n_protocoltypeid);
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
	                	continue;
	                }
	                int n_devicetypeid = atoi(GetTableFieldValue(&struCursor, "dtp_devicetypeid"));
	                FreeCursor(&struCursor);
	                //连接szElementSql
	                sprintf(szElementSql, "%s ne_ProtocolDeviceTypeId= %s, ne_devicetypeid=%d,", szElementSql, pszContentStr[i], n_devicetypeid);
	            }
	        }
	    }
	    else if (strcmp(pszPropertyStr[i], "0004") == 0 || strcmp(pszPropertyStr[i], "00000004") == 0) //设备型号
	        sprintf(szElementSql, "%s ne_DeviceModelId= '%s',", szElementSql, pszContentStr[i]);
        else if (strcmp(pszPropertyStr[i], "0005") == 0 || strcmp(pszPropertyStr[i], "00000005") == 0) //
	        sprintf(szElementSql, "%s ne_SnNumber= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0006") == 0 || strcmp(pszPropertyStr[i], "00000006") == 0) //
	        sprintf(szElementSql, "%s ne_ChannelCount= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0007") == 0 || strcmp(pszPropertyStr[i], "00000007") == 0) //经度
	    {
	    	if (strncmp(pszContentStr[i], "E", 1) == 0 || strncmp(pszContentStr[i], "e", 1) == 0)
	    		pszContentStr[i] = pszContentStr[i]+1;
	    	sprintf(szElementSql, "%s ne_Lon= %s,", szElementSql, pszContentStr[i]);
	    }
	    else if (strcmp(pszPropertyStr[i], "0008") == 0 || strcmp(pszPropertyStr[i], "00000008") == 0) //纬度
	    {
	    	if (strncmp(pszContentStr[i], "N", 1) == 0 || strncmp(pszContentStr[i], "n", 1) == 0)
	    		pszContentStr[i] = pszContentStr[i]+1;
	    	sprintf(szElementSql, "%s ne_Lat= %s,", szElementSql, pszContentStr[i]);
	    }	    	
	    else if (strcmp(pszPropertyStr[i], "0009") == 0 || strcmp(pszPropertyStr[i], "00000009") == 0) //
	        sprintf(szElementSql, "%s ne_ObjList= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "000A") == 0 || strcmp(pszPropertyStr[i], "0000000A") == 0) //
	        sprintf(szElementSql, "%s ne_ver= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0101") == 0 || strcmp(pszPropertyStr[i], "00000101") == 0) //
	        sprintf(szElementSql, "%s ne_RepeaterId= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0102") == 0 || strcmp(pszPropertyStr[i], "00000102") == 0) //
	        sprintf(szElementSql, "%s ne_DeviceId= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0110") == 0 || strcmp(pszPropertyStr[i], "00000110") == 0) //
	        sprintf(szElementSql, "%s ne_SmscNum= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0111") == 0 || strcmp(pszPropertyStr[i], "00000111") == 0) //
	        sprintf(szElementSql, "%s ne_TelNum1= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0112") == 0 || strcmp(pszPropertyStr[i], "00000112") == 0) //
	        sprintf(szElementSql, "%s ne_TelNum2= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0113") == 0 || strcmp(pszPropertyStr[i], "00000113") == 0) //
	        sprintf(szElementSql, "%s ne_TelNum3= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0114") == 0 || strcmp(pszPropertyStr[i], "00000114") == 0) //
	        sprintf(szElementSql, "%s ne_TelNum4= '%s',", szElementSql, pszContentStr[i]); 
	    else if (strcmp(pszPropertyStr[i], "0115") == 0 || strcmp(pszPropertyStr[i], "00000115") == 0) //
	        sprintf(szElementSql, "%s ne_TelNum5= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0120") == 0 || strcmp(pszPropertyStr[i], "00000120") == 0) //
	        sprintf(szElementSql, "%s ne_OmcTelNum= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0130") == 0 || strcmp(pszPropertyStr[i], "00000130") == 0) //
	        sprintf(szElementSql, "%s ne_OmcIp= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0131") == 0 || strcmp(pszPropertyStr[i], "00000131") == 0) //
	        sprintf(szElementSql, "%s ne_OmcPort= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0151") == 0 || strcmp(pszPropertyStr[i], "00000151") == 0) //UDP Device IP added by wwj at 20100625
	        sprintf(szElementSql, "%s ne_DeviceIp= '%s',", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "0139") == 0 || strcmp(pszPropertyStr[i], "00000139") == 0) //UDP Device Port added by wwj at 20100625
	        sprintf(szElementSql, "%s ne_DevicePort= %s,", szElementSql, pszContentStr[i]); 
	    else if (strcmp(pszPropertyStr[i], "0140") == 0) //
	        sprintf(szElementSql, "%s ne_UploadCommTypeId= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "01B0") == 0) //
	        sprintf(szElementSql, "%s ne_dependdevicecount= %s,", szElementSql, pszContentStr[i]);
	    else if (strcmp(pszPropertyStr[i], "01B6") == 0) //
	        sprintf(szElementSql, "%s ne_otherdeviceid= '%s',", szElementSql, pszContentStr[i]);

	    else if (strcmp(pszPropertyStr[i], "0150") == 0) //日期、时间
	    {
	    	memset(&struBindVar, 0, sizeof(struBindVar));
        	struBindVar.struBind[0].nVarType = SQLT_STR; 
			strcpy(struBindVar.struBind[0].VarValue.szValueChar, pszContentStr[i]);
			struBindVar.nVarCount++;
			
			struBindVar.struBind[1].nVarType = SQLT_INT; //SQLT_INT
			struBindVar.struBind[1].VarValue.nValueInt = n_neid;
			struBindVar.nVarCount++;
			if (strcmp(getenv("DATABASE"), "mysql") == 0)			
	        	sprintf(szSql,"update  ne_Elementparam  set epm_curvalue=to_date(:v_0, 'yyyy-mm-dd hh24:mi:ss') where epm_neid= :v_1 and epm_objid='0150'");
	        else
	        	sprintf(szSql,"update  ne_Elementparam  set epm_curvalue=sysdate - to_date(:v_0, 'yyyy-mm-dd hh24:mi:ss') where epm_neid= :v_1 and epm_objid='0150'");
	        PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%d]\n",szSql, pszContentStr[i], n_neid);
	        if(BindExecuteSQL(szSql, &struBindVar)!=NORMAL)
	        {
		        PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		        continue;
	        }
	    }
  
	    else if (strcmp(pstruSendPackage->struMapObjList[i].szMapType, "alarmYN") == 0) //告警使能
	    {
	    	if (bAlarmEnabled == BOOLFALSE)
	    	{
	    		memset(szAlarmEnabledObjList,0, sizeof(szAlarmEnabledObjList));
	            strcpy(szAlarmEnabledObjList, GetAlarmEnabledObjList(n_gNeId));
	        }
	        ReplaceAlarmObjStr(szAlarmEnabledObjList, pszPropertyStr[i], pszContentStr[i]);
	        bAlarmEnabled = BOOLTRUE;
	    }
	    
	    else
	    {
	    	memset(&struBindVar, 0, sizeof(struBindVar));
			struBindVar.struBind[0].nVarType = SQLT_INT; //SQLT_INT
			struBindVar.struBind[0].VarValue.nValueInt = n_neid;
			struBindVar.nVarCount++;
		
			struBindVar.struBind[1].nVarType = SQLT_STR; 
			strcpy(struBindVar.struBind[1].VarValue.szValueChar, pszPropertyStr[i]);
			struBindVar.nVarCount++;
			//if (n_protocoltypeid == PROTOCOL_JINXIN_DAS)
			//	sprintf(szSql,"select count(*) as empcount from  ne_elementparam  where epm_NeId= :v_0 and  epm_McpId= :v_1");
			//else
	        	sprintf(szSql,"select count(*) as empcount from  ne_elementparam  where epm_NeId= :v_0 and  epm_ObjId= :v_1");
	        PrintDebugLog(DBG_HERE, "执行SQL语句[%s][%d][%s]\n", szSql, n_neid, pszPropertyStr[i]);
	        if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
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
	            continue;
	        }
	        nCount = atoi(GetTableFieldValue(&struCursor, "empcount"));
	        FreeCursor(&struCursor);
	        
	        if (nCount > 0)
	        {   
	        	memset(&struBindVar, 0, sizeof(struBindVar));
	        	
	        	struBindVar.struBind[0].nVarType = SQLT_STR; 
				strcpy(struBindVar.struBind[0].VarValue.szValueChar, pszContentStr[i]);
				struBindVar.nVarCount++;
				
				struBindVar.struBind[1].nVarType = SQLT_INT; //SQLT_INT
				struBindVar.struBind[1].VarValue.nValueInt = n_neid;
				struBindVar.nVarCount++;
				
				struBindVar.struBind[2].nVarType = SQLT_STR; 
				strcpy(struBindVar.struBind[2].VarValue.szValueChar, pszPropertyStr[i]);
				struBindVar.nVarCount++;
				//if (n_protocoltypeid == PROTOCOL_JINXIN_DAS)//更新映射ID
				//	sprintf(szSql,"update ne_elementparam set epm_CurValue= :v_0, epm_UpdateTime=sysdate where epm_NeId= :v_1 and  epm_McpId= :v_2");
				//else
            		sprintf(szSql,"update ne_elementparam set epm_CurValue= :v_0, epm_UpdateTime=sysdate where epm_NeId= :v_1 and  epm_ObjId= :v_2");
	            PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%d][%s]\n",szSql, pszContentStr[i], n_neid, pszPropertyStr[i]);
	            if(BindExecuteSQL(szSql, &struBindVar)!=NORMAL)
	            {
		            PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		            return EXCEPTION;
	            }
	        }
	        /*
	        else
	        {
	        	if (n_protocoltypeid == PROTOCOL_JINXIN_DAS)
	        	{
	        		memset(&struBindVar, 0, sizeof(struBindVar));
	        	
		        	struBindVar.struBind[0].nVarType = SQLT_STR; 
					strcpy(struBindVar.struBind[0].VarValue.szValueChar, pszContentStr[i]);
					struBindVar.nVarCount++;
					
					struBindVar.struBind[1].nVarType = SQLT_INT; //SQLT_INT
					struBindVar.struBind[1].VarValue.nValueInt = n_neid;
					struBindVar.nVarCount++;
					
					struBindVar.struBind[2].nVarType = SQLT_STR; 
					strcpy(struBindVar.struBind[2].VarValue.szValueChar, pszPropertyStr[i]);
					struBindVar.nVarCount++;
					
	        		sprintf(szSql,"update ne_elementparam set epm_CurValue= :v_0, epm_UpdateTime=sysdate where epm_NeId= :v_1 and  epm_ObjId= :v_2");
		            PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%d][%s]\n",szSql, pszContentStr[i], n_neid, pszPropertyStr[i]);
		            if(BindExecuteSQL(szSql, &struBindVar)!=NORMAL)
		            {
			            PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
			            return EXCEPTION;
		            }
	        	}
	        }*/
	    }
	    
	    if (strcmp(pszPropertyStr[i], "0873") == 0 || strcmp(pszPropertyStr[i], "00000873") == 0) //
	        sprintf(szElementSql, "%s ne_route= '%s',", szElementSql, pszContentStr[i]);    
	}
	//网元信息表有条件
	if (strlen(szElementSql) > 22 || bAlarmEnabled == BOOLTRUE)
	{
	    if (bAlarmEnabled == BOOLTRUE)
	        sprintf(szElementSql, "%s ne_AlarmEnabledObjList = '%s', ne_lastupdatetime = to_date( '%s','yyyy-mm-dd hh24:mi:ss') where ne_NeId = %d", 
	            szElementSql, szAlarmEnabledObjList, GetSysDateTime(), n_neid);
	    else
	        sprintf(szElementSql, "%s ne_lastupdatetime = to_date( '%s','yyyy-mm-dd hh24:mi:ss') where ne_NeId = %d", 
	            szElementSql, GetSysDateTime(), n_neid);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szElementSql);
	    if(ExecuteSQL(szElementSql)!=NORMAL)
	    {
		    PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szElementSql,GetSQLErrorMessage());
		    return EXCEPTION;
	    }
	}
	else
	{
		memset(&struBindVar, 0, sizeof(struBindVar));
			
		struBindVar.struBind[0].nVarType = SQLT_STR; 
		strcpy(struBindVar.struBind[0].VarValue.szValueChar, GetSysDateTime());
		struBindVar.nVarCount++;
		
		struBindVar.struBind[1].nVarType = SQLT_INT;
		struBindVar.struBind[1].VarValue.nValueInt = n_neid;
		struBindVar.nVarCount++;
		
		memset(szElementSql, 0, sizeof(szElementSql));
		sprintf(szElementSql, "update ne_Element set ne_lastupdatetime = to_date( :v_0,'yyyy-mm-dd hh24:mi:ss') where ne_NeId = :v_1");
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%s][%d]\n",szElementSql, GetSysDateTime(), n_neid);
	    if(BindExecuteSQL(szElementSql, &struBindVar)!=NORMAL)
	    {
		    PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szElementSql,GetSQLErrorMessage());
		    return EXCEPTION;
	    }
	}
	//效果监控列表有条件
	if (strlen(szMonitorSql) > 31)
	{
	    sprintf(szMonitorSql, "%s where smg_neid = %d", TrimRightChar(szMonitorSql,','), n_neid);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szMonitorSql);
	    if(ExecuteSQL(szMonitorSql)!=NORMAL)
	    {
		    PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szMonitorSql,GetSQLErrorMessage());
		    return EXCEPTION;
	    }
	}
	CommitTransaction();
	
	if (bCqtMatche==BOOLTRUE)
	{
		CqtMathchJob2(n_neid, szTelNum, szBeCallNum);
	}
	return NORMAL;    
	
	
}

/* 
 * 根据属性和参数内容更新man_EleQryLog等表
 */
RESULT UpdateEleFromCommBySetPacket(PSTR pszQryNumber, PSTR pszProperty)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	STR szTklEndTime[20];
	BOOL bTimeOut = BOOLFALSE;
	int nCount=-1;
	
	if (n_gTaskLogId > 0)
	{
	    //取man_tasklog
	    /* 2022.1.14 delete
	    memset(szSql, 0, sizeof(szSql));
	    if (strcmp(getenv("DATABASE"), "mysql") == 0)
	    	sprintf(szSql, "select tkl_endtime as tkl_endtime from man_tasklog where tkl_tasklogid= %d", n_gTaskLogId);
	    else
	    	sprintf(szSql, "select to_char(tkl_endtime, 'yyyy-mm-dd hh24:mi:ss') as tkl_endtime from man_tasklog where tkl_tasklogid= %d", n_gTaskLogId);
	    PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	    if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    	return EXCEPTION;
	    }
	    if(FetchCursor(&struCursor) != NORMAL)
	    {
	        FreeCursor(&struCursor);
	    	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", \
	    				  szSql, GetSQLErrorMessage());
	    	return EXCEPTION;
	    }
	    strcpy(szTklEndTime, GetTableFieldValue(&struCursor, "tkl_endtime"));
	    FreeCursor(&struCursor);
	    
	    memset(szSql, 0, sizeof(szSql));
	    sprintf(szSql, "select count(*) as v_count from man_EleSetLog where set_TaskLogid=%d and set_EleId=%d and set_IsSuccess=1", n_gTaskLogId, n_gNeId);
	    PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	    if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    	return EXCEPTION;
	    }
	    if(FetchCursor(&struCursor) == NORMAL)
	    {
	    	nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
	    }
	    FreeCursor(&struCursor);
	    //未超时
	    if (strlen(szTklEndTime) > 0)
	    {
	        bTimeOut = BOOLTRUE;
	    }
	    memset(szSql, 0, sizeof(szSql));
	    if (bTimeOut == BOOLFALSE && nCount == 0)
	    {
            sprintf(szSql, "update man_TaskLog set tkl_RxPackCount=tkl_RxPackCount+1,tkl_EleSuccessCount=tkl_EleSuccessCount+1 where tkl_tasklogid=%d ", n_gTaskLogId);
	    }
	    else
	    {
            sprintf(szSql, "update man_TaskLog set tkl_RxPackCount=tkl_RxPackCount+1 where tkl_tasklogid= %d ", n_gTaskLogId);
	    }
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
	    if(ExecuteSQL(szSql)!=NORMAL)
	    {
	         PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
	         return EXCEPTION;
	    }
	    */
    }
    
    if (strcmp(pszProperty, "busy") == 0)
    {
    	sprintf(szSql, "update man_EleSetLog set set_EndTime=to_date('%s','yyyy-mm-dd hh24:mi:ss'),set_IsSuccess=0, set_IsOutTime=%d,set_RxPackCount=1, set_FailContent='busy'  where set_Number='%s'",
               GetSysDateTime(), bTimeOut, pszQryNumber);
    }
    else if (strlen(pszProperty) > 1)
    {    
        sprintf(szSql, "update man_EleSetLog set set_EndTime=to_date('%s','yyyy-mm-dd hh24:mi:ss'),set_IsSuccess=0, set_IsOutTime=%d,set_RxPackCount=1, set_Content=set_Content||' %s'  where set_Number='%s'",
               GetSysDateTime(), bTimeOut, pszProperty, pszQryNumber);
    }
    else
   	{
        sprintf(szSql, "update man_EleSetLog set set_EndTime=to_date('%s','yyyy-mm-dd hh24:mi:ss'),set_IsSuccess=1, set_IsOutTime=%d,set_RxPackCount=1  where set_Number='%s'",
               GetSysDateTime(), bTimeOut, pszQryNumber);
    }

	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息[%s]\n",szSql, GetSQLErrorMessage());
		 return EXCEPTION;
	}
    CommitTransaction();
	
	return NORMAL;
}



/* 
 * 分解设置网元参数
 */
RESULT DecodeSetElementParam(SENDPACKAGE *pstruSendPackage)
{
	STR szProperty[2000];
	STR szContent[2000];
	STR szFailContent[2000];
	STR szFailMapId[2000];
	STR szErrorId[2000];
	STR szQrySerial[29];
	INT i;
	
	//初始化变量
	memset(szProperty, 0, sizeof(szProperty));
	memset(szContent, 0, sizeof(szContent));
	memset(szFailMapId, 0, sizeof(szFailMapId));
	memset(szErrorId, 0, sizeof(szErrorId));
	memset(szFailContent, 0, sizeof(szFailContent));
	
	strcpy(szQrySerial, pstruSendPackage->struHead.QA);
	n_gNeId = pstruSendPackage->nNeId;
	n_gTaskId = pstruSendPackage->nTaskId;
	n_gTaskLogId = pstruSendPackage->nTaskLogId;
	
	for (i=0; i< pstruSendPackage->struHead.nObjectCount; i++)
	{
        if (pstruSendPackage->struMapObjList[i].cErrorId == '0')//监控对象标号正确
		{
			//if (strlen(pstruSendPackage->struMapObjList[i].szMapData) > 0)//判断是否为空值,为空不处理
			{
		    sprintf(szProperty, "%s%s,", szProperty, pstruSendPackage->struMapObjList[i].szMapId);
		    sprintf(szContent, "%s%s,", szContent, pstruSendPackage->struMapObjList[i].szMapData);
			}

		}
		else//监控对象标号有错误
		{
		    //用空格分割
		    sprintf(szFailMapId, "%s%s,", szFailMapId, pstruSendPackage->struMapObjList[i].szMapId);
		    sprintf(szErrorId, "%s%c,", szErrorId, pstruSendPackage->struMapObjList[i].cErrorId);
		    sprintf(szFailContent, "%s%s,", szFailContent,pstruSendPackage->struMapObjList[i].szMapData);
 		}
	}
	TrimRightChar(szProperty, ',');
	TrimRightOneChar(szContent, ',');
	
	PrintDebugLog(DBG_HERE, "QrySerial=[%s],Property[%s],Content[%s],FailMapId[%s]\n",
	     szQrySerial, szProperty, szContent, szFailMapId);
	     
	//更新数据库失败内容
	if (strlen(TrimAllSpace(szFailMapId)) > 0)
	{
	    TrimRightChar(szFailMapId, ',');
	    TrimRightChar(szErrorId, ',');
	    SetEleSetLogErrorFromComm(szQrySerial, szFailMapId,  szErrorId, szFailContent);
	    SaveSysErrorLog( n_gNeId, szFailMapId,  szErrorId, pstruSendPackage->szRecvMsg);
	}
	
	
	//更新网元设置记录,进入工厂模式
	if (strcmp(szProperty, "0AFF") == 0)
	{
		sprintf(szFailContent, "%X", szContent[0]);
		UpdateEleFromCommBySetPacket(szQrySerial, szFailContent);
	}
	else
		UpdateEleFromCommBySetPacket(szQrySerial, "");
	
	//更新网元设置信息(对站点查询后返回结果的更新包括网元表和网元辅助表)成功时
	UpdateEleSetLogFromComm(pstruSendPackage, szQrySerial, szProperty, szContent);
	
	return NORMAL;
}


/*
 *  保存特殊命令日志
 */
RESULT InsertSpecialCommandLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
    INT nCommandCode;
    UINT nPesqMosTaskId;
    STR szServerNum[20];

	snprintf(szSql, sizeof(szSql),
	    "insert into sm_SpecailCommandLog(scl_id, scl_qrysetlogid, scl_listid, scl_neid, scl_commandtype,"
	    "scl_command, scl_eventtime, scl_logid, scl_windowlogid) values("
	    "%s,  %s,  1,   %s, %s,"
		" %s, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), %d, '%s') ",
		DemandStrInXmlExt(pstruReqXml, "<omc>/<日志号>"),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<日志号>"),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<命令类型>"),
		
		DemandStrInXmlExt(pstruReqXml, "<omc>/<命令号>"), //流水号既命令号
		GetSysDateTime(),
		atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<登录流水号>")),
		DemandStrInXmlExt(pstruReqXml,"<omc>/<窗体流水号>")
	);
	
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	nCommandCode = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"));
	if (nCommandCode == 4)
	{
	    memset(szSql, 0, sizeof(szSql));
	    sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %s and epm_ObjId = '07B0'", 
	        DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	    PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	    if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    	return EXCEPTION;
	    }
	    if(FetchCursor(&struCursor) == NORMAL)
	    {
	        strcpy(szServerNum, GetTableFieldValue(&struCursor, "epm_CurValue"));
	    }
	    FreeCursor(&struCursor);
	    
	    GetDbSerial(&nPesqMosTaskId, "sm_PesqMosTask");
	    
	    snprintf(szSql, sizeof(szSql),
	        "insert into sm_pesqmostask(tst_id, tst_taskid, tst_eventtime, tst_prior, tst_neid,"
	        "tst_netelnum, tst_iscaller, tst_number, tst_pesqservernum) values("
	        "%d,  0,  to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 0, %s,"
	    	" '%s', 1, %s, '%s') ",
	    	nPesqMosTaskId,
	    	GetSysDateTime(),
	    	DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),

	    	DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"),
	    	DemandStrInXmlExt(pstruReqXml, "<omc>/<日志号>"),
	    	DemandStrInXmlExt(pstruReqXml,"<omc>/<窗体流水号>")//
	    );
	    
	    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	    if(ExecuteSQL(szSql) != NORMAL)
	    {
	    	PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
	    		szSql, GetSQLErrorMessage());
            return EXCEPTION;
	    }
	}
	CommitTransaction();
	
	return NORMAL;  
    
}


RESULT RemortUpdateDbOperate1(PXMLSTRU pstruReqXml)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	int nFulm_Id, nFulm_UpdateMode;
	UINT nFulm_NewId;
	STR szMsg[100];
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select FULM_ID,FULM_UPDATEMODE from sm_firmwareupdatelog_m where FULM_NEID = %s and FULM_LOGID = %d and FULM_WINDOWLOGID = '%s' and FULM_STATUS = %d ",
	        DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"), 
	        atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<登录流水号>")),
	        DemandStrInXmlExt(pstruReqXml, "<omc>/<窗体流水号>"),
	        UNSENDED_STATUS);
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    FreeCursor(&struCursor);
	    if(GetDbSerial(&nFulm_Id, "Sm_FirmwareUpdateLog_M")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取远程升级日志流水号错误\n");
			return EXCEPTION;
		}
	    memset(szSql, 0, sizeof(szSql));
		sprintf(szSql, "insert into sm_firmwareupdatelog_m (FULM_ID, FULM_NEID, PULM_NENAME, FULM_REPEATERID, FULM_DEVICEID, FULM_PROTOCOLTYPEID, PULM_DEVICETYPEID, FULM_TELPHONENUM, FULM_UPDATEMODE, FULM_EVENTTIME, FULM_LOGID, FULM_WINDOWLOGID)"
		               " values(%d, %d, ' ', %u, %d, 1, 141, '%s', 2, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 0, 0)",
		                nFulm_Id,
		                atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>")),
		                atol(DemandStrInXmlExt(pstruReqXml, "<omc>/<站点编号>")),
		                atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<设备编号>")),
		                DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"),
		                GetSysDateTime()
		                );
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
		if(ExecuteSQL(szSql)!=NORMAL)
		{
		 	PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
		 	return EXCEPTION;
		}
	}
	else
	{
		nFulm_Id = atoi(GetTableFieldValue(&struCursor, "FULM_ID"));
		nFulm_UpdateMode = atoi(GetTableFieldValue(&struCursor, "FULM_UPDATEMODE"));
		FreeCursor(&struCursor);
	}
	
	if (nFulm_UpdateMode == 1)
	    strcpy(szMsg, "MCP:B模式下发设置升级模式命令成功!");
	else
	    strcpy(szMsg, "FTP模式下发设置升级模式命令成功!");
	
    bufclr(szSql);
    sprintf(szSql, "update sm_firmwareupdatelog_m set FULM_BEGINTIME = sysdate,FULM_STATUS = %d, FULM_QA= '%s' where FULM_ID= %d",
         SENDSET_STATUS, DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"), nFulm_Id);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
		 return EXCEPTION;
	}
	        
    bufclr(szSql);
    GetDbSerial(&nFulm_NewId, "sm_firmwareupdatelog_d");
    sprintf(szSql, "insert into sm_firmwareupdatelog_d(FULD_ID,FULD_MSG,FULD_MASTERID,FULD_STYLE) values(%d, '%s', %d, %d)",
         nFulm_NewId, szMsg, nFulm_Id, 1);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
		 return EXCEPTION;
	}
    return NORMAL;
}

RESULT RemortUpdateDbOperate2(PSTR pszQA)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	int nFulm_Id, nFulm_UpdateMode;
	UINT nFulm_NewId;
	STR szMsg[100];
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select FULM_ID,FULM_UPDATEMODE from sm_firmwareupdatelog_m where FULM_QA = '%s' ",
	        pszQA);
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    FreeCursor(&struCursor);
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	nFulm_Id = atoi(GetTableFieldValue(&struCursor, "FULM_ID"));
	nFulm_UpdateMode = atoi(GetTableFieldValue(&struCursor, "FULM_UPDATEMODE"));
	FreeCursor(&struCursor);
	
	if (nFulm_UpdateMode == 1)
	    strcpy(szMsg, "MCP:B模式下发启动升级命令成功!开始升级!");
	else
	    strcpy(szMsg, "FTP模式下发启动升级命令成功!开始升级!");
	
	bufclr(szSql);
    sprintf(szSql, "update sm_firmwareupdatelog_m set FULM_STATUS = %d where FULM_ID= %d",
         SENDED_STATUS, nFulm_Id);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
		 return EXCEPTION;
	}
	
    bufclr(szSql);
    GetDbSerial(&nFulm_NewId, "sm_firmwareupdatelog_d");
    sprintf(szSql, "insert into sm_firmwareupdatelog_d(FULD_ID,FULD_MSG,FULD_MASTERID,FULD_STYLE) values(%d, '%s', %d, %d)",
         nFulm_NewId, szMsg, nFulm_Id, 1);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
		 return EXCEPTION;
	}
   
    return NORMAL;
}

//远程升级结果
RESULT RemortUpdateDbOperate3(PXMLSTRU pstruReqXml)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	int nFulm_Id, nFulm_UpdateMode;
	UINT nFulm_NewId;
	STR szMsg[100];
	INT nNewStatus;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select FULM_ID,FULM_UPDATEMODE from sm_firmwareupdatelog_m where FULM_REPEATERID = %s and FULM_DEVICEID = %s and FULM_STATUS = %d  order by FULM_BEGINTIME desc",
	        DemandStrInXmlExt(pstruReqXml, "<omc>/<设备编号>"),  DemandStrInXmlExt(pstruReqXml, "<omc>/<次编号>"), SENDED_STATUS);
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
	nFulm_Id = atoi(GetTableFieldValue(&struCursor, "FULM_ID"));
	nFulm_UpdateMode = atoi(GetTableFieldValue(&struCursor, "FULM_UPDATEMODE"));
	FreeCursor(&struCursor);
	
	if (nFulm_UpdateMode == 1)
	    strcpy(szMsg, "MCP:B模式远程升级结束!已经成功完成升级!");
	else
	    strcpy(szMsg, "FTP模式远程升级结束!已经成功完成升级!");
	/*
	if(atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<UpdateResult>")) == 0)
		 nNewStatus = SUCCESS_STATUS ;// 现状态 1－升级成功
    else
		 nNewStatus = FAILURE_STATUS ;// 现状态 ,2－升级失败
    */
	nNewStatus = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<UpdateResult>"));
    bufclr(szSql);
    sprintf(szSql, "update sm_firmwareupdatelog_m set FULM_VERSIONID = '%s', FULM_UPRESULTSTATUS = %d where FULM_ID= %d",
         DemandStrInXmlExt(pstruReqXml, "<omc>/<VerInfo>"), nNewStatus, nFulm_Id);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
		 return EXCEPTION;
	}
	        
    bufclr(szSql);
    GetDbSerial(&nFulm_NewId, "sm_firmwareupdatelog_d");
    sprintf(szSql, "insert into sm_firmwareupdatelog_d(FULD_ID,FULD_MSG,FULD_MASTERID,FULD_STYLE) values(%d, '%s', %d, %d)",
         nFulm_NewId, szMsg, nFulm_Id, 1);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
		 return EXCEPTION;
	}
	
	if (nNewStatus == 0)
	{
		bufclr(szSql);
    	sprintf(szSql, "update ne_element set ne_ver = '%s',ne_lastupdatetime = to_date( '%s','yyyy-mm-dd hh24:mi:ss')  where ne_neid= %s", 
    	        DemandStrInXmlExt(pstruReqXml, "<omc>/<VerInfo>"),
    	        GetSysDateTime(),
    	        DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n", szSql);
		if(ExecuteSQL(szSql)!=NORMAL)
		{
			 PrintErrorLog(DBG_HERE,"执行 SQL语句[%s]错误信息=[%s]\n",szSql, GetSQLErrorMessage());
			 return EXCEPTION;
		}
	}
	CommitTransaction();
    return NORMAL;
}


/*
 * 用于同步设置站点参数
 *
 */
RESULT SetAllElementParam(int nNeId)
{
    STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	XMLSTRU struXml;
	PXMLSTRU pstruXml=&struXml;
	REPEATER_INFO struRepeater;
	COMMANDHEAD struHead;
	STR szNeObjectList[MAX_BUFFER_LEN];
	STR szNeDataList[MAX_BUFFER_LEN];
	PSTR pszSepParamStr[MAX_OBJECT_NUM];  /* 对象数组*/
	PSTR pszSepContentStr[MAX_OBJECT_NUM];  /* 对象内容数组*/
	INT nObjCount, i, nDeviceTypeId;

	sprintf(szSql,"select ne_neid, ne_CommTypeId,ne_RepeaterId, ne_DeviceId, ne_NeTelNum,ne_OmcIp, ne_ProtocoltypeId,ne_ProtocolDeviceTypeId, ne_OtherDeviceId, ne_DeviceModelId, ne_ServerTelNum,ne_ObjList from ne_Element where ne_neid = %d", nNeId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if (FetchCursor(&struCursor) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE,"执行SQL语句[%s] 失败[%s]\n",szSql,GetSQLErrorMessage());
		FreeCursor(&struCursor);
		return EXCEPTION;
	}
	memset(pstruXml,0,sizeof(XMLSTRU));
	CreateXml(pstruXml,FALSE,OMC_ROOT_PATH,NULL);
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
	nDeviceTypeId = atoi(GetTableFieldValue(&struCursor, "ne_ProtocolDeviceTypeId"));
	InsertInXmlExt(pstruXml,"<omc>/<设备类型>", GetTableFieldValue(&struCursor, "ne_ProtocolDeviceTypeId"),
	               MODE_AUTOGROW|MODE_UNIQUENAME);
	InsertInXmlExt(pstruXml,"<omc>/<其它标识>", GetTableFieldValue(&struCursor, "ne_OtherDeviceId"),
	               MODE_AUTOGROW|MODE_UNIQUENAME);
	InsertInXmlExt(pstruXml,"<omc>/<设备型号>", GetTableFieldValue(&struCursor, "ne_DeviceModelId"),
	               MODE_AUTOGROW|MODE_UNIQUENAME);
	//服务号码
	InsertInXmlExt(pstruXml,"<omc>/<服务号码>", TrimAllSpace(GetTableFieldValue(&struCursor, "ne_ServerTelNum")),
	               MODE_AUTOGROW|MODE_UNIQUENAME);
	//命令暂时为查询:COMMAND_SET
    InsertInXmlExt(pstruXml,"<omc>/<命令号>", "2", MODE_AUTOGROW|MODE_UNIQUENAME);
    InsertInXmlExt(pstruXml,"<omc>/<类型>",  "21", MODE_AUTOGROW|MODE_UNIQUENAME);
	/*
	 * 站点等级为快:OMC_QUICK_MSGLEVEL
	 */
	InsertInXmlExt(pstruXml,"<omc>/<站点等级>", OMC_QUICK_MSGLEVEL, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	FreeCursor(&struCursor);
	
	//取对象列表
	memset(szNeObjectList, 0, sizeof(szNeObjectList));
	memset(szNeDataList, 0, sizeof(szNeDataList));
	if (nDeviceTypeId == 49) //GSM增强型效果监控
		strcpy(szNeObjectList, "'0101','0102','0111','0112','0113','0114','0115','0120','0130','0131','0201','0701','0707','0708','070A','070B','070C','070D','070E','070F','07A1','07A2','07A4','07B0','07E1','07E3','07E4','07E5','07E6','07E9','0806','0807','0808','0809','080A','080B','080C','080D','04D0'");
	else if (nDeviceTypeId == 48) //TD-SCDMA增强型效果监控设备
		strcpy(szNeObjectList, "'0101','0102','0111','0112','0113','0114','0115','0120','0130','0131','0201','02CC','0701','0707','0708','070A','070B','070C','070D','070E','070F','07A1','07A2','07A4','07B0','07C7','07C8','07E1','07E3','07E4','07E5','07E6','07E9','0806','0807','0808','0809','080A','080B','080C','080D','08A0','08A1','08A2','08A3','08A4','04D0'");
	if (GetNeObjectList(nNeId, szNeObjectList, szNeDataList) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"取同步设置参数失败!\n");
		return EXCEPTION;
	}
			
	memset(&struHead, 0, sizeof(COMMANDHEAD));
    memset(&struRepeater, 0, sizeof(REPEATER_INFO));
	struRepeater.nCommType = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<通信方式>"));  
	struRepeater.nRepeaterId = atol(DemandStrInXmlExt(pstruXml,"<omc>/<站点编号>")); 
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
	//分解对象
	ResolveSetParamArrayGprs(szNeObjectList, szNeDataList);
	nObjCount = SeperateString(szNeDataList, '|', pszSepContentStr, MAX_SEPERATE_NUM);
	nObjCount = SeperateString(szNeObjectList, '|', pszSepParamStr, MAX_SEPERATE_NUM);
    for(i=0; i< nObjCount; i++)
	{   
	    if (strlen(pszSepParamStr[i]) >= 4)
	    {
	    	InsertInXmlExt(pstruXml,"<omc>/<监控对象>", pszSepParamStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
	    	InsertInXmlExt(pstruXml,"<omc>/<监控对象内容>", pszSepContentStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
       		SetElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
        	SaveToGprsQueue(pstruXml);
	    }
	}
	
	/*
	 * 先设置站点编号和上报号码，再发送GPRS请求连接
	 * (要测试发送点击多个请求情况)
	 */
	struHead.nCommandCode = COMMAND_SET;
	/*
	InsertInXmlExt(pstruXml,"<omc>/<类型>",  "12", MODE_AUTOGROW|MODE_UNIQUENAME);
	//站点编号和上报号码
    InsertInXmlExt(pstruXml,"<omc>/<监控对象>", "0101,0120", MODE_AUTOGROW|MODE_UNIQUENAME);
    sprintf(szTemp, "%d,%s", struRepeater.nRepeaterId, struRepeater.szNetCenter);
	InsertInXmlExt(pstruXml,"<omc>/<监控对象内容>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
    SetElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
    SaveToMsgQueue(pstruXml);
	SaveEleSetLog(pstruXml);
	
	
	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime((INT)time(NULL)+10), MODE_AUTOGROW|MODE_UNIQUENAME);
	*/
	
	//参数查询设置请求指令
	InsertInXmlExt(pstruXml,"<omc>/<监控对象>", "0804", MODE_AUTOGROW|MODE_UNIQUENAME);
	InsertInXmlExt(pstruXml,"<omc>/<监控对象内容>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
    SetElementParam(M2G_SMS, &struHead, &struRepeater, pstruXml);
    SaveToMsgQueue(pstruXml);
	SaveEleSetLog(pstruXml);
	
	
	DeleteXml(pstruXml);
	
	return NORMAL;
}

/* 
 * 更新主表网元最新时间 add by wwj at 2010.08.03(cqf) for device busy
 */
RESULT UpdateEleLastTime(int nNeId)
{
	STR szSql[MAX_SQL_LEN];
	
	sprintf(szSql, "update ne_element set ne_lastupdatetime = to_date( '%s','yyyy-mm-dd hh24:mi:ss') where ne_NeId = %d", GetSysDateTime(), nNeId);
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
    if(ExecuteSQL(szSql) !=NORMAL ) 
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();
	
	return NORMAL;
}
