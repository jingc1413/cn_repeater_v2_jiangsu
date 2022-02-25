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
	int nTaskId, nTaskLogId, nAreaId;	/* 任务号 */
	int nProStyle;          /* 处理类型*/
	int nProtocolTypeId, nProtocolDeviceTypeId, nDeviceTypeId;
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
	int nNeCount=0;
	int nNowTime, nTimes=0;
	STR szUploadTime[100];
	STR szTaskQryParm[1000], szBase[4000], szRadio[4000], szAlarm[4000], szAlarmen[4000], szObjList[MAX_BUFFER_LEN];
	PSTR pszTempStr[MAX_MAPOBJECT_NUM];
	INT nSeperateNum;

 	memset(pstruXml,0,sizeof(XMLSTRU));
	if(ImportXml(pstruXml,FALSE,pszCaReqBuffer)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"报文[%s]非法\n",pszCaReqBuffer);
		DeleteXml(pstruXml);
		return -1;
	}
	nTaskId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<taskid>"));
	nAreaId = atoi(DemandStrInXmlExt(pstruXml,"<omc>/<areaid>"));
	
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
	
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "select count(tsk_neid) as ne_count from man_taskneid where tsk_taskid = %d", nTaskId);
    PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
    if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
    if(FetchCursor(&struCursor) == NORMAL)
	{
		nNeCount = atoi(GetTableFieldValue(&struCursor, "ne_count"));
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
             strcpy(szTaskQryParm, TrimAllSpace(GetTableFieldValue(&struCursor, "tkd_qryparam")));
		     strcpy(szBase, TrimAllSpace(GetTableFieldValue(&struCursor, "tkd_base")));
			 strcpy(szRadio, TrimAllSpace(GetTableFieldValue(&struCursor, "tkd_radio")));
			 strcpy(szAlarmen, TrimAllSpace(GetTableFieldValue(&struCursor, "tkd_alarmen")));
			 strcpy(szAlarm, TrimAllSpace(GetTableFieldValue(&struCursor, "tkd_alarm")));
        } 
        else //设置 1为查询:COMMAND_SET
        {
            memset(szSetEleParam, 0, sizeof(szSetEleParam));
          	strcpy(szSetEleParam, TrimAllSpace(GetTableFieldValue(&struCursor, "tkd_setparam")));
            
            memset(szSetEleValue, 0, sizeof(szSetEleValue));
            strcpy(szSetEleValue, TrimAllSpace(GetTableFieldValue(&struCursor, "tkd_setvalue")));
        }
		FreeCursor(&struCursor);
	}
	//取任务明细
	memset(szSql, 0, sizeof(szSql));
	if(nNeCount > 0)//选择设备
		snprintf(szSql, sizeof(szSql), "select b.ne_neid,b.ne_protocoldevicetypeid,b.ne_devicetypeid,b.ne_protocoltypeid,b.ne_devicestatusid, b.ne_commtypeid, b.ne_netelnum, b.ne_telnum1, b.ne_telnum2,b.ne_telnum3,b.ne_telnum4,b.ne_telnum5,"
	  		" b.ne_servertelnum, b.ne_repeaterid, b.ne_deviceid, b.ne_deviceip, b.ne_deviceport, b.ne_devicemodelid, ne_objlist, ne_ActiveCol,ne_ActiveRow, ne_AlarmObjList, ne_AlarmEnabledObjList  from ne_element b where (b.ne_devicestatusid=0 or b.ne_devicestatusid=17) and b.ne_neid in (select tsk_neid from man_taskneid where tsk_taskid = %d)",
	        	nTaskId);
	else
		snprintf(szSql, sizeof(szSql), "select b.ne_neid,b.ne_protocoldevicetypeid,b.ne_devicetypeid,b.ne_protocoltypeid,b.ne_devicestatusid, b.ne_commtypeid, b.ne_netelnum, b.ne_telnum1, b.ne_telnum2,b.ne_telnum3,b.ne_telnum4,b.ne_telnum5,"
	  		" b.ne_servertelnum, b.ne_repeaterid, b.ne_deviceid, b.ne_deviceip, b.ne_deviceport, b.ne_devicemodelid, ne_objlist, ne_ActiveCol,ne_ActiveRow, ne_AlarmObjList, ne_AlarmEnabledObjList  from ne_element b, pre_area a where  b.ne_areaid = a.are_areaid and b.ne_areaid in (select rel_childs_id from pre_area_relate  where rel_parent_id = %d) and (b.ne_devicestatusid=0 or b.ne_devicestatusid=17) and b.ne_devicetypeid <> 146",
	        nAreaId);
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
        strcpy(szServerTelNum,  GetTableFieldValue(&struCursor, "ne_servertelnum"));
        
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
        InsertInXmlExt(pstruXml,"<omc>/<服务号码1>", GetTableFieldValue(&struCursor, "ne_telnum1"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码2>", GetTableFieldValue(&struCursor, "ne_telnum2"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码3>", GetTableFieldValue(&struCursor, "ne_telnum3"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码4>", GetTableFieldValue(&struCursor, "ne_telnum4"), MODE_AUTOGROW|MODE_UNIQUENAME);
        InsertInXmlExt(pstruXml,"<omc>/<服务号码5>", GetTableFieldValue(&struCursor, "ne_telnum5"), MODE_AUTOGROW|MODE_UNIQUENAME);

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
	    strcpy(szObjList, GetTableFieldValue(&struCursor, "ne_objlist"));
	    //PrintDebugLog(DBG_HERE, "szObjList[%s]\n", szObjList);
	    memset(szQryEleParam, 0, sizeof(szQryEleParam));
	    if (nProStyle == 0)  //0为查询:COMMAND_QUERY
	    {
		    //不轮训基本型效果2.0
		     if ((strcmp(szStyle, "1") == 0 || strcmp(szStyle, "2") == 0) && nDeviceTypeId == 146)
			     continue;

             if (strlen(szServerTelNum) == 0 && struRepeater.nCommType != 6)
             {
                 nFailEleCount ++;
                 InsertFailNeid(nTaskLogId, nNeId, "电话号码校验失败", "网元监控中心号码为空");
                 continue;
             }
            
             if (struHead.nProtocolType == PROTOCOL_2G || 
             	struHead.nProtocolType == PROTOCOL_DAS)
             {	
		     	 if (strcmp(szStyle, "213") == 0)//批量查询
		     	 {
			 	     if (strstr(szTaskQryParm, "base") != NULL && strlen(szBase) >= 4)                    
			     	 {
				 	     nSeperateNum = SeperateString(szBase,  ',', pszTempStr, MAX_MAPOBJECT_NUM);
		             	 for(i=0; i< nSeperateNum; i++)
		             	 {
		             	 	memset(szTemp, 0, sizeof(szTemp));
		             	 	strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]) -2);
		             	 	if (strstr(szObjList, szTemp) != NULL)
		                 	sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		                 }
			     	 }
			     	 if (strstr(szTaskQryParm, "radio") != NULL && strlen(szRadio) >= 4)
			     	 {
				 	     nSeperateNum = SeperateString(szRadio,  ',', pszTempStr, MAX_MAPOBJECT_NUM);
		             	 for(i=0; i< nSeperateNum; i++)
		             	 {
		             	 	memset(szTemp, 0, sizeof(szTemp));
		             	 	strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]) -2);
		             	 	if (strstr(szObjList, szTemp) != NULL)
		                 	sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		                 }
			     	 }
		     	 }
		     	 else//快速查询,常规查询
		     	 {
		     	 	 strcpy(szAlarmen, GetTableFieldValue(&struCursor, "ne_AlarmEnabledObjList"));
			 	     strcpy(szAlarm, GetTableFieldValue(&struCursor, "ne_AlarmObjList"));
			     	 //常规轮训考虑增加所有参量
	             }
	             //PrintDebugLog(DBG_HERE, "szAlarm[%s]\n", szAlarm);
	             if (strstr(szTaskQryParm, "alarm") != NULL && strlen(szAlarm) >= 4)
		         {
		         	 nSeperateNum = SeperateString(szAlarm,  ',', pszTempStr, MAX_MAPOBJECT_NUM);
		         	 for(i=0; i< nSeperateNum; i++)
		         	 {
		         	 	memset(szTemp, 0, sizeof(szTemp));
		         	 	strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]) -2);
		             	if (strstr(szObjList, szTemp) != NULL)
		             	sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
		             }
		         }
		         //PrintDebugLog(DBG_HERE, "轮训对象[%s]\n", szQryEleParam);
		         if (strstr(szTaskQryParm, "alarmen") != NULL && strlen(szAlarmen) >= 4)
			     {
				     nSeperateNum = SeperateString(szAlarmen,  ',', pszTempStr, MAX_MAPOBJECT_NUM);
				     for(i=0; i< nSeperateNum; i++)
				     {
				     	 memset(szTemp, 0, sizeof(szTemp));
				 	     strncpy(szTemp, pszTempStr[i], strlen(pszTempStr[i]) -2);
				 	     if (strstr(szObjList, szTemp) != NULL)
				 	     sprintf(szQryEleParam, "%s%s,", szQryEleParam, szTemp);
				     }
			     }
			     //PrintDebugLog(DBG_HERE, "轮训对象[%s]\n", szQryEleParam);
	             if (struRepeater.nCommType == 6)
	             	ResolveQryParamArrayGprs(szQryEleParam);
	             else	
	             	ResolveQryParamArray(szQryEleParam);
	             //为空不轮训     
	             if (strlen(szQryEleParam) == 0) continue;
	             PrintDebugLog(DBG_HERE, "轮训对象[%s]\n", szQryEleParam);
	             
	             if (nTimes++>= 8)//校准发送时间
	        	 {
	        	 	 nNowTime ++;
	        	 	 nTimes = 0;
	        	 	 if (nNowTime - 1000 >=(int)time(NULL))
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
	 	             	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime(nNowTime+i*8000), MODE_AUTOGROW|MODE_UNIQUENAME);
	 	             
		             InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  pszSepParamStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
	            	 
	            	 if (struRepeater.nCommType == 6)
	            	 {
	            	 	QryElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
	            	 	SaveToGprsQueue(pstruXml);
	            	 }
	            	 else if (struRepeater.nCommType == 7)//通信方式：snmp协议
					 {
						struHead.nProtocolType = PROTOCOL_SNMP;
						QryElementParam(M2G_SNMP_TYPE, &struHead, &struRepeater, pstruXml);
			        	//生成设置命令保存
			        	SaveToSnmpQueue(pstruXml);
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
			         if (strcmp(szStyle, "2") == 0 &&  i>= 1)	break; //快速轮训只轮训2个包
		         }
		     }
		     else if (struHead.nProtocolType == PROTOCOL_GSM ||  //add by wwj at 2010.07.28
				struHead.nProtocolType == PROTOCOL_CDMA ||
				struHead.nProtocolType == PROTOCOL_HEIBEI ||
				struHead.nProtocolType == PROTOCOL_HEIBEI2 ||
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
        else //设置 1:COMMAND_SET
        {
        	
            if (strlen(szServerTelNum) == 0 && struRepeater.nCommType != 6)
            {
                nFailEleCount ++;
                InsertFailNeid(nTaskLogId, nNeId, "电话号码校验失败", "网元监控中心号码为空");
                continue;
            }

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
            if (strlen(szSetEleParam) == 0) continue;
            PrintDebugLog(DBG_HERE, "轮训设置对象[%s][%s]\n", szSetEleParam, szSetEleValue);
            
            nObjCount = SeperateString(szSetEleParam, '|', pszSepParamStr, MAX_SEPERATE_NUM);
            nObjCount = SeperateString(szSetEleValue, '|', pszSepValueStr, MAX_SEPERATE_NUM);
            for(i=0; i< nObjCount; i++)
	        {
	        	 if (nTimes++>=2)
	        	 {
	        	 	 nNowTime ++;
	        	 	 nTimes = 0;
	        	 }
	            //监控对象用空格分割
 	            
 	             if (strlen(szUploadTime) == 14)
 	            	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", szUploadTime, MODE_AUTOGROW|MODE_UNIQUENAME);
 	             else 
 	            	InsertInXmlExt(pstruXml,"<omc>/<定时发送时间>", MakeSTimeFromITime(nNowTime + i*10), MODE_AUTOGROW|MODE_UNIQUENAME);
	             InsertInXmlExt(pstruXml,"<omc>/<监控对象>",  pszSepParamStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
	             InsertInXmlExt(pstruXml,"<omc>/<监控对象内容>",  pszSepValueStr[i], MODE_AUTOGROW|MODE_UNIQUENAME);
            	 if (struRepeater.nCommType == 6)
	             {
	            	 SetElementParam(M2G_TCPIP, &struHead, &struRepeater, pstruXml);
	            	 SaveToGprsQueue(pstruXml);
	             }
	             else if (struRepeater.nCommType == 7)//snmp协议
				 {
					struHead.nProtocolType = PROTOCOL_SNMP;
					SetElementParam(M2G_SNMP_TYPE, &struHead, &struRepeater, pstruXml);
		        	//生成设置命令保存
		        	SaveToSnmpQueue(pstruXml);
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