/*
 * 名称: 网管系统应用服务器(TD-SCDMA上报部分)
 *
 * 修改记录:
 * 付志刚 -		2009-8-19 创建
 */
 
#include <ebdgdl.h>
#include <omcpublic.h>
#include <applserver.h>


/* 
 * 分解定长报文
 */
static RESULT UnpackFixLen(PSTR pszPacket,PXMLSTRU pstruXml,PFIXLENCFGSTRU pstruCfg)
{
	INT i;
	STR szField[256];
	STR szPath[200];

	for(i=0;pstruCfg[i].pszName!=NULL;i++)
	{
		bufclr(szField);
		DecodeMapDataFromType(pstruCfg[i].pszType, pstruCfg[i].nLen, pszPacket, szField);
		//memcpy(szField,pszPacket,pstruCfg[i].nLen);
		pszPacket+=pstruCfg[i].nLen;
		
		TrimAllSpace(szField);

		sprintf(szPath,"%s/<%s>", OMC_ROOT_PATH, pstruCfg[i].pszName);
		InsertInXmlExt(pstruXml, szPath, szField, MODE_AUTOGROW|MODE_UNIQUENAME);
	}

	return NORMAL;
}

/* 
 * 分解WAP测试结果数据
 */
RESULT DecodeTdWap(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nAssertLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
    FIXLENCFGSTRU struCfg[]=
	{
		{"Times",	"UINT1", 1},          //测试次数
		{"CurIndex",	    "UINT1", 1},  // 当前测试序号
		{"TestTime",	"DT", 7},     	  //开始测试时间
		{"GprsMode",	"UINT1", 1},  	  //数据业务模式
		{"WapTitle",	"STR", 40},       //WAP首页地址
		{"WapType",		"UINT1", 1},       //WAP协议方式
		{"BeginTime",	"DT", 7},     	   //开始发送时间
		{"EndTime",	    "DT", 7},          //结束时间
		{"IsSuccess",	    "UINT1", 1},   //WAP网关登录成功与否
		{"HomeBeginTime",	"DT", 7},      //WAP首页下载开始时间
		{"HomeEndTime",	    "DT", 7},      //WAP首页下载完成时间
		{"DownData",	    "UINT1", 2},   //WAP实际下载数据量
		{"HomeIsSuccess",	"UINT1", 1},   //WAP首页下载成功与否
		{NULL,		"",     0}
	};
	
	
	if(nAssertLen==0)
	{
		for(i=0;struCfg[i].pszName!=NULL;i++)
		{
			nAssertLen+=struCfg[i].nLen;
		}
	}
	if(nDataLen < nAssertLen)
	{
		PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nAssertLen);
		return -1;
	}
	
	if(UnpackFixLen(pszMapData, pstruXml, struCfg)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"解析报文失败\n");
		return -1;
	}
		
	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解WAP测试结果数据报文[%s]\n",szBuffer);
	
    return NORMAL;
}


/* 
 * 分解PESQ下行测试结束上报(客观语音质量评估)
 */
RESULT DecodeTdPesq(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
	static INT nTdLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i, nSpendTime, nSumValue=0, N;
	STR szValueStr[MAX_BUFFER_LEN], szTemp[10];
	
    	
	FIXLENCFGSTRU struTdCfg[]=
	{
		{"Times",	"UINT1", 1},          //测试次数
		{"CurIndex", "UINT1", 1},         // 当前测试序号
		{"网络模式",		"UINT1", 1},      // 网络模式
		{"IsCaller",	"UINT1", 1},      //主被叫标识
		{"EndTelNum",	"STR", 20},        // 对端号码	字符串（20）
		{"DialTime",	"DT", 7},          //拨打日期时间	整数（7）
		{"ConnTime",	"TIME", 3},         //接通时间	整数（3）
		{"PesqDialStatus",	"UINT1", 1},   //PESQ接通状态
		{"SpendTime",	    "UINT2", 2},   //通话时长
		{"Lac",	        "UINT2", 2},
		{"Uarfcn",	    "UINT2", 2},
		{"CellId",	    "UINT2", 2},
		{"Ta",	    	"SINT2", 2},
		{"Txpower",	    "SINT1", 1},
		{"Cid1",	    "UINT2", 2},
		{"Cid2",	    "UINT2", 2},
		{"Cid3",	    "UINT2", 2},
		{"SwitchCount",	"UINT1", 1},   	   //切换总次数
		{NULL,			"",     0}
	};
	
	
	if(nTdLen==0)
	{
		for(i=0;struTdCfg[i].pszName!=NULL;i++)
		{
			nTdLen+=struTdCfg[i].nLen;
		}
	}
	
	{
		if(nDataLen < nTdLen)
		{
			PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nTdLen);
			return -1;
		}
		
		if(UnpackFixLen(pszMapData, pstruXml, struTdCfg)!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"解析报文失败\n");
			return -1;
		}
		pszMapData += nTdLen;  //移到电平明细
		
		nSpendTime = atoi(DemandStrInXmlExt(pstruXml, "<omc>/<SpendTime>"));
		N = (nSpendTime + 1) / 2 ; //N=（通话时长+1）/2，取整
    	memset(szValueStr, 0, sizeof(szValueStr));
		for(i = 0 ; i < N ; i++)
		{
			//呼叫电平(RXL) SUB	有符号整数（1）	单位为dBm ，2秒一次，取平均值取整。
			memset(szTemp, 0, sizeof(szTemp));
			DecodeMapDataFromType("SINT1", 1,  pszMapData, szTemp);
			pszMapData += 1;				
			nSumValue += atoi(szTemp);
			sprintf(szValueStr, "%s%s,", szValueStr, szTemp);
		}
    	
		if(N>0)
		{
		    TrimRightChar(szValueStr, ',');
		    sprintf(szTemp, "%d", nSumValue / N);
		    InsertInXmlExt(pstruXml,"<omc>/<RxlSub>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		}
		else
		{
			InsertInXmlExt(pstruXml,"<omc>/<RxlSub>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
		}
		InsertInXmlExt(pstruXml,"<omc>/<RxlSubData>", szValueStr, MODE_AUTOGROW|MODE_UNIQUENAME);	
	}

	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解PESQ下行测试结束上报报文[%s]\n",szBuffer);
	
    return NORMAL;
}

/* 
 * 分解VP测试结果数据
 */
RESULT DecodeVp(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nTdLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i, nSpendTime, nSumValue=0, N;
	STR szValueStr[MAX_BUFFER_LEN], szTemp[10];
	
   	FIXLENCFGSTRU struTdCfg[]=
	{
		{"Count",	    "UINT1", 1},      //测试次数
		{"CurIndex",	"UINT1", 1},      // 当前测试序号
		{"IsCaller",	"UINT1", 1},      //主被叫标识
		{"EndTelNum",	"STR",  20},      //对端号码
		{"DialTime",	"DT",    7},      //拨打时间
		{"AlertingTime", "DT",    7},      //Alerting时间
		{"IsConnect",	"UINT1", 1},      //是否接通
		{"VideookTime",	"DT",    7},      //视频接入时间
		{"FirstVideoTime",	"DT",    7},      //收到第一个视频帧时间
		{"VideoisConnect",	    "UINT1", 1},  //视频是否接通
		{"VideoendTime",	    "DT", 7},  //视频结束时间
		{"DialendTime",	    "DT", 7},      //电路结束时间
		{"DialStatus",	    "UINT1", 1},  //接通状态
		{"Lac",	        "UINT2", 2},
		{"CellId",	    "UINT2", 2},      //
		{"Uarfcn",	    "UINT2", 2},      //
		{"Cpi",	        "UINT1", 1},          //
		{"Ta",	        "SINT2", 2},          //
		{"TxPower",	    "SINT1", 1},          //
		{"Sir",	    	"SINT1", 1},          //
		{"Rscp",	    "SINT1", 1},          //
		{"Rm",	    	"UINT1", 1},	      //扰码
		{"Midamble",	 "UINT1", 1},	      //
		{"DnSyncCode",	    "UINT1", 1},	 //
		{"UpSyncCode",	    "UINT1", 1},	  //
		{"Iscp",	    	"UINT1", 1},	  //
		{"Cid1",	    	"UINT2", 2},	  //
		{"Cid2",	    	"UINT2", 2},	  //
		{"Cid3",	    	"UINT2", 2},	  //
		{"UarfCn1",	    	"UINT2", 2},	  //
		{"UarfCn2",	    	"UINT2", 2},	  //
		{"UarfCn3",	    	"UINT1", 2},	  //
		{"CellSwitchCount",	"SINT1", 1},  //小区切换总次数
		{"FreqSwitchCount",	"SINT1", 1},      //频点切换总次数
		{NULL,				"",     0}
	};
	
	
	if(nTdLen==0)
	{
		for(i=0;struTdCfg[i].pszName!=NULL;i++)
		{
			nTdLen+=struTdCfg[i].nLen;
		}
	}
	if(nDataLen < nTdLen)
	{
		PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nTdLen);
		return -1;
	}
	
	if(UnpackFixLen(pszMapData, pstruXml, struTdCfg)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"解析报文失败\n");
		return -1;
	}
	pszMapData += nTdLen;  //移到电平明细
		
	nSpendTime = atoi(DemandStrInXmlExt(pstruXml, "<omc>/<SpendTime>"));
	N = (nSpendTime + 1) / 2 ; //N=（通话时长+1）/2，取整
    memset(szValueStr, 0, sizeof(szValueStr));
	for(i = 0 ; i < N ; i++)
	{
		//呼叫电平(RXL) SUB	有符号整数（1）	单位为dBm ，2秒一次，取平均值取整。
		memset(szTemp, 0, sizeof(szTemp));
		DecodeMapDataFromType("SINT1", 1,  pszMapData, szTemp);
		pszMapData += 1;				
		nSumValue += atoi(szTemp);
		sprintf(szValueStr, "%s%s,", szValueStr, szTemp);
	}
    
	if(N>0)
	{
	    TrimRightChar(szValueStr, ',');
	    sprintf(szTemp, "%d", nSumValue / N);
	    InsertInXmlExt(pstruXml,"<omc>/<Dpch>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	else
	{
		InsertInXmlExt(pstruXml,"<omc>/<Dpch>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	InsertInXmlExt(pstruXml,"<omc>/<DpchData>", szValueStr, MODE_AUTOGROW|MODE_UNIQUENAME);		
    
    nSumValue = 0;
	memset(szValueStr, 0, sizeof(szValueStr));
	for(i = 0 ; i < N ; i++)
	{
		//话音质量(RXQ) SUB	整数（1）	0~7，2秒一次，取平均值。(保留1位小数,即比例是10)
		memset(szTemp, 0, sizeof(szTemp));
		DecodeMapDataFromType("UINT1", 1,  pszMapData, szTemp);
		pszMapData += 1;				
		nSumValue += atoi(szTemp);
		sprintf(szValueStr, "%s%s,", szValueStr, szTemp);
	}
    
	if(N>0)
	{
	    TrimRightChar(szValueStr, ',');
	    sprintf(szTemp, "%d", nSumValue / N);
	    InsertInXmlExt(pstruXml,"<omc>/<Bler>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	else
	{
		InsertInXmlExt(pstruXml,"<omc>/<Bler>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	InsertInXmlExt(pstruXml,"<omc>/<BlerData>", szValueStr, MODE_AUTOGROW|MODE_UNIQUENAME);	
	
	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解VP测试结果数据报文[%s]\n",szBuffer);
	
    return NORMAL;
}

/* 
 * 分解CQT语音拨测
 */
RESULT DecodeTdCqt(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nTdLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i, nSpendTime, nSumValue=0, N;
	STR szValueStr[MAX_BUFFER_LEN], szTemp[10];
	
 	FIXLENCFGSTRU struTdCfg[]=
	{
		{"Count",	    "UINT1", 1},      //测试次数
		{"CurIndex",	"UINT1", 1},      // 当前测试序号
		{"网络模式",		"UINT1", 1},      // 网络模式
		{"IsCaller",	"UINT1", 1},      //主被叫标识
		{"EndTelNum",	"STR",  20},      //对端号码
		{"DialTime",	"DT",    7},      //拨打时间
		{"AlertingTime", "DT",    7},      //Alerting时间
		{"DialOkTime",	    "TIME", 3},   //接通时间
		{"DialStatus",	    "UINT1", 1},  //接通状态
		{"SpendTime",	    "UINT2", 2},  //通话时长
		{"Lac",	        "UINT2", 2},
		{"CellId",	    "UINT2", 2},      //
		{"Uarfcn",	    "UINT2", 2},      //
		{"Cpi",	        "UINT1", 1},          //
		{"Ta",	        "SINT2", 2},          //
		{"TxPower",	    "SINT1", 1},          //
		{"Sir",	    	"SINT1", 1},          //
		{"Rscp",	    "SINT1", 1},          //
		{"Rm",	    	"UINT1", 1},	      //扰码
		{"Midamble",	 "UINT1", 1},	      //
		{"DnSyncCode",	    "UINT1", 1},	 //
		{"UpSyncCode",	    "UINT1", 1},	  //
		{"Iscp",	    	"UINT1", 1},	  //
		{"NetMode1",	    "UINT1", 1},	  //网络模式1
		{"Cid1",	    	"UINT2", 2},	  //
		{"NetMode2",	    "UINT1", 1},	  //网络模式2
		{"Cid2",	    	"UINT2", 2},	  //
		{"NetMode3",	    "UINT1", 1},	  //网络模式3
		{"Cid3",	    	"UINT2", 2},	  //
		{"UarfCn1",	    	"UINT2", 2},	  //
		{"UarfCn2",	    	"UINT2", 2},	  //
		{"UarfCn3",	    	"UINT1", 2},	  //
		{"CellSwitchCount",		"SINT1", 1},  //小区切换总次数
		{"FreqSwitchCount",	"SINT1", 1},      //频点切换总次数
		{NULL,				"",     0}
	};
	
	
	if(nTdLen==0)
	{
		for(i=0;struTdCfg[i].pszName!=NULL;i++)
		{
			nTdLen+=struTdCfg[i].nLen;
		}
	}

	{
		if(nDataLen < nTdLen)
		{
			PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nTdLen);
			return -1;
		}
		if(UnpackFixLen(pszMapData, pstruXml, struTdCfg)!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"解析报文失败\n");
			return -1;
		}
		pszMapData += nTdLen;  //移到电平明细
		
		nSpendTime = atoi(DemandStrInXmlExt(pstruXml, "<omc>/<SpendTime>"));
		N = (nSpendTime + 1) / 2 ; //N=（通话时长+1）/2，取整
    	memset(szValueStr, 0, sizeof(szValueStr));
		for(i = 0 ; i < N ; i++)
		{
			//呼叫电平(RXL) SUB	有符号整数（1）	单位为dBm ，2秒一次，取平均值取整。
			memset(szTemp, 0, sizeof(szTemp));
			DecodeMapDataFromType("SINT1", 1,  pszMapData, szTemp);
			pszMapData += 1;				
			nSumValue += atoi(szTemp);
			sprintf(szValueStr, "%s%s,", szValueStr, szTemp);
		}
    	
		if(N>0)
		{
		    TrimRightChar(szValueStr, ',');
		    sprintf(szTemp, "%d", nSumValue / N);
		    InsertInXmlExt(pstruXml,"<omc>/<Dpch>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		}
		else
		{
			InsertInXmlExt(pstruXml,"<omc>/<Dpch>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
		}
		InsertInXmlExt(pstruXml,"<omc>/<DpchData>", szValueStr, MODE_AUTOGROW|MODE_UNIQUENAME);		
    	
    	nSumValue = 0;
		memset(szValueStr, 0, sizeof(szValueStr));
		for(i = 0 ; i < N ; i++)
		{
			//话音质量(RXQ) SUB	整数（1）	0~7，2秒一次，取平均值。(保留1位小数,即比例是10)
			memset(szTemp, 0, sizeof(szTemp));
			DecodeMapDataFromType("UINT1", 1,  pszMapData, szTemp);
			pszMapData += 1;				
			nSumValue += atoi(szTemp);
			sprintf(szValueStr, "%s%s,", szValueStr, szTemp);
		}
    	
		if(N>0)
		{
		    TrimRightChar(szValueStr, ',');
		    sprintf(szTemp, "%d", nSumValue / N);
		    InsertInXmlExt(pstruXml,"<omc>/<Bler>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		}
		else
		{
			InsertInXmlExt(pstruXml,"<omc>/<Bler>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
		}
		InsertInXmlExt(pstruXml,"<omc>/<BlerData>", szValueStr, MODE_AUTOGROW|MODE_UNIQUENAME);
		
		nSumValue = 0;
		memset(szValueStr, 0, sizeof(szValueStr));
		for(i = 0 ; i < N ; i++)
		{
			//单通检测  SUB	整数（1）	0~7，2秒一次，取平均值。(保留1位小数,即比例是10)
			memset(szTemp, 0, sizeof(szTemp));
			DecodeMapDataFromType("UINT1", 1,  pszMapData, szTemp);
			pszMapData += 1;				
			nSumValue += atoi(szTemp);
			sprintf(szValueStr, "%s%s,", szValueStr, szTemp);
		}
    	
		if(N>0)
		{
		    TrimRightChar(szValueStr, ',');
		    sprintf(szTemp, "%d", nSumValue / N);
		    InsertInXmlExt(pstruXml,"<omc>/<Single>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		}
		else
		{
			InsertInXmlExt(pstruXml,"<omc>/<Single>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
		}
		InsertInXmlExt(pstruXml,"<omc>/<SingleData>", szValueStr, MODE_AUTOGROW|MODE_UNIQUENAME);
	}
			
	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解CQT语音拨测报文[%s]\n",szBuffer);
	
    return NORMAL;
}

/* 
 * 分解PING测试结果
 */
RESULT DecodeTdPing(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nAssertLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
    FIXLENCFGSTRU struCfg[]=
	{
		{"Count",	        "UINT1", 1},          //测试次数
		{"CurIndex",	    "UINT1", 1},        //当前测试序号
		{"Time",	        "DT",    7},        //开始测试时间
		{"Result1",	        "UINT1", 1},        //测试结果1
		{"Time1",	        "UINT2", 2},        //测试时间1
		{"Result2",	        "UINT1", 1},        //测试结果2
		{"Time2",	        "UINT2", 2},        //测试时间2
		{"Result3",	        "UINT1", 1},        //测试结果3
		{"Time3",	        "UINT2", 2},        //测试时间3
		{"Result4",	        "UINT1", 1},        //测试结果4
		{"Time4",	        "UINT2", 2},        //测试时间4
		{"Result5",	        "UINT1", 1},        //测试结果5
		{"Time5",	        "UINT2", 2},        //测试时间5
		{"Result6",	        "UINT1", 1},        //测试结果6
		{"Time6",	        "UINT2", 2},        //测试时间6
		{"Result7",	        "UINT1", 1},        //测试结果7
		{"Time7",	        "UINT2", 2},        //测试时间7
		{"Result8",	        "UINT1", 1},        //测试结果8
		{"Time8",	        "UINT2", 2},        //测试时间8
		{"Result9",	        "UINT1", 1},        //测试结果9
		{"Time9",	        "UINT2", 2},        //测试时间9
		{"Result10",	    "UINT1", 1},        //测试结果10
		{"Time10",	        "UINT2", 2},        //测试时间10
		{NULL,		"",     0}
	};
	
	
	if(nAssertLen==0)
	{
		for(i=0;struCfg[i].pszName!=NULL;i++)
		{
			nAssertLen+=struCfg[i].nLen;
		}
	}
	if(nDataLen < nAssertLen)
	{
		PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nAssertLen);
		return -1;
	}
	
	if(UnpackFixLen(pszMapData, pstruXml, struCfg)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"解析报文失败\n");
		return -1;
	}
	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解PING测试结果报文[%s]\n",szBuffer);
	
    return NORMAL;
}


/* 
 * 分解PDP激活测试结果
 */
RESULT DecodeTdPdp(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nAssertLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
    FIXLENCFGSTRU struCfg[]=
	{
		{"Count",	        "UINT1", 1},        //测试次数
		{"CurIndex",	    "UINT1", 1},        //当前测试序号
		{"Time",	        "DT",    7},        //开始测试时间
		{"Result1",	        "UINT1", 1},        //测试结果1
		{"Time1",	        "UINT2", 2},        //测试时间1
		{"Result2",	        "UINT1", 1},        //测试结果2
		{"Time2",	        "UINT2", 2},        //测试时间2
		{"Result3",	        "UINT1", 1},        //测试结果3
		{"Time3",	        "UINT2", 2},        //测试时间3
		{"Result4",	        "UINT1", 1},        //测试结果4
		{"Time4",	        "UINT2", 2},        //测试时间4
		{"Result5",	        "UINT1", 1},        //测试结果5
		{"Time5",	        "UINT2", 2},        //测试时间5
		{"Result6",	        "UINT1", 1},        //测试结果6
		{"Time6",	        "UINT2", 2},        //测试时间6
		{"Result7",	        "UINT1", 1},        //测试结果7
		{"Time7",	        "UINT2", 2},        //测试时间7
		{"Result8",	        "UINT1", 1},        //测试结果8
		{"Time8",	        "UINT2", 2},        //测试时间8
		{"Result9",	        "UINT1", 1},        //测试结果9
		{"Time9",	        "UINT2", 2},        //测试时间9
		{"Result10",	    "UINT1", 1},        //测试结果10
		{"Time10",	        "UINT2", 2},        //测试时间10
		{NULL,		"",     0}
	};
	
	
	if(nAssertLen==0)
	{
		for(i=0;struCfg[i].pszName!=NULL;i++)
		{
			nAssertLen+=struCfg[i].nLen;
		}
	}
	if(nDataLen < nAssertLen)
	{
		PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nAssertLen);
		return -1;
	}
	
	if(UnpackFixLen(pszMapData, pstruXml, struCfg)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"解析报文失败\n");
		return -1;
	}
	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解PDP激活测试结果报文[%s]\n",szBuffer);
	
    return NORMAL;
}

/* 
 * 分解ATTACH测试结果
 */
RESULT DecodeTdAttach(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nAssertLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
    FIXLENCFGSTRU struCfg[]=
	{
		{"Count",	"UINT1", 1},          //测试次数
		{"CurIndex",	        "UINT1", 1},        //当前测试序号
		{"Time",	        "DT", 7},        //开始测试时间
		{"Result1",	        "UINT1", 1},        //测试结果1
		{"Time1",	        "UINT2", 2},        //测试时间1
		{"Result2",	        "UINT1", 1},        //测试结果2
		{"Time2",	        "UINT2", 2},        //测试时间2
		{"Result3",	        "UINT1", 1},        //测试结果3
		{"Time3",	        "UINT2", 2},        //测试时间3
		{"Result4",	        "UINT1", 1},        //测试结果4
		{"Time4",	        "UINT2", 2},        //测试时间4
		{"Result5",	        "UINT1", 1},        //测试结果5
		{"Time5",	        "UINT2", 2},        //测试时间5
		{"Result6",	        "UINT1", 1},        //测试结果6
		{"Time6",	        "UINT2", 2},        //测试时间6
		{"Result7",	        "UINT1", 1},        //测试结果7
		{"Time7",	        "UINT2", 2},        //测试时间7
		{"Result8",	        "UINT1", 1},        //测试结果8
		{"Time8",	        "UINT2", 2},        //测试时间8
		{"Result9",	        "UINT1", 1},        //测试结果9
		{"Time9",	        "UINT2", 2},        //测试时间9
		{"Result10",	    "UINT1", 1},        //测试结果10
		{"Time10",	        "UINT2", 2},        //测试时间10
		{NULL,		"",     0}
	};
	
	
	if(nAssertLen==0)
	{
		for(i=0;struCfg[i].pszName!=NULL;i++)
		{
			nAssertLen+=struCfg[i].nLen;
		}
	}
	if(nDataLen < nAssertLen)
	{
		PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nAssertLen);
		return -1;
	}
	
	if(UnpackFixLen(pszMapData, pstruXml, struCfg)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"解析报文失败\n");
		return -1;
	}
	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解ATTACH测试结果报文[%s]\n",szBuffer);
	
    return NORMAL;
}


/* 
 * 分解MMS测试结果数据
 */
RESULT DecodeTdMms(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
	static INT nTdLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i, nTemp;
	
	
	FIXLENCFGSTRU struTdCfg[]=
	{
		{"Times",	"UINT1", 1},          //测试次数
		{"CurIndex", "UINT1", 1},          // 当前测试序号
		{"网络模式",		"UINT1", 1},      // 网络模式
		{"Title",	"STR", 19},           //测试标题
		{"SendOrReceive",	"UINT1", 1},  //收发端标识
		{"CallNumber",	    "STR", 20},   //对端号码
		{"BeginSendTime",	"DT", 7},     //开始发送时间
		{"SendedTime",	    "DT", 7},     //发送成功时间
		{"SendedIsSuccess",	    "UINT1", 1},   //是否发送成功
		{"BeginPushTime",	    "DT", 7},      //开始接收到push时间
		{"PushedIsSuccess",	    "UINT1", 1},   //push提取是否成功
		{"MmsReceiveTime",	    "DT", 7},      //MMS提取完成时间
		{"MmsReceiveIsSu",	    "UINT1", 1},   //MMS提取是否成功
		{NULL,		"",     0}
	};
	
	
	if(nTdLen==0)
	{
		for(i=0;struTdCfg[i].pszName!=NULL;i++)
		{
			nTdLen+=struTdCfg[i].nLen;
		}
	}

	{
		if(nDataLen < nTdLen)
		{
			PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nTdLen);
			return -1;
		}
		
		if(UnpackFixLen(pszMapData, pstruXml, struTdCfg)!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"解析报文失败\n");
			return -1;
		}
	}
	
	nTemp = atoi(DemandStrInXmlExt(pstruXml, "<omc>/<SendOrReceive>"));
	if (nTemp == 0)
	{
	    if (strcmp(DemandStrInXmlExt(pstruXml, "<omc>/<BeginSendTime>"), "1980-01-01 01:01:01") == 0 
	       && strcmp(DemandStrInXmlExt(pstruXml, "<omc>/<SendedTime>"), "1980-01-01 01:01:01") == 0)
	    {
	        InsertInXmlExt(pstruXml,"<omc>/<SendedIsSuccess>", "-1", MODE_AUTOGROW|MODE_UNIQUENAME);
	    }
	}
	else
	{
	    if (strcmp(DemandStrInXmlExt(pstruXml, "<omc>/<BeginPushTime>"), "1980-01-01 01:01:01") == 0)
	        InsertInXmlExt(pstruXml,"<omc>/<PushedIsSuccess>", "-1", MODE_AUTOGROW|MODE_UNIQUENAME);
	    if (strcmp(DemandStrInXmlExt(pstruXml, "<omc>/<MmsReceiveTime>"), "1980-01-01 01:01:01") == 0)
	        InsertInXmlExt(pstruXml,"<omc>/<MmsReceiveIsSu>", "-1", MODE_AUTOGROW|MODE_UNIQUENAME);
	}
	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解MMS测试结果数据报文[%s]\n",szBuffer);
	
    return NORMAL;
}

/* 
 * 分解当日测试结果汇总表数据
 */
RESULT DecodeTdDayTestResult(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
	static INT nTdLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
  	FIXLENCFGSTRU struTdCfg[]=
	{
		{"Time",	"DT", 7},          				//测试汇总时间
		{"CqtTimes",	        "UINT2", 2},        //CQT互拨测试
		{"AttachTimes",	        "UINT2", 2},        //ATTACH测试
		{"PdpTimes",	        "UINT2", 2},        //PDP测试
		{"PingTimes",	        "UINT2", 2},        //PING测试
		{"FtpUploadTimes",	    "UINT2", 2},        //FTP上载测试
		{"FtpDownloadTimes",	"UINT2", 2},        //FTP下载测试
		{"TimerUploadTimes",	"UINT2", 2},        //定时上报
		{"MmsTimes",	        "UINT2", 2},        //MMS测试
		{"WapTimes",	        "UINT2", 2},        //WAP测试
		{"VpTimes",	        	"UINT2", 2},        //VP测试
		{NULL,		"",     0}
	};
	
	
	if(nTdLen==0)
	{
		for(i=0;struTdCfg[i].pszName!=NULL;i++)
		{
			nTdLen+=struTdCfg[i].nLen;
		}
	}
	
	{
		if(nDataLen < nTdLen)
		{
			PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nTdLen);
			return -1;
		}
		if(UnpackFixLen(pszMapData, pstruXml, struTdCfg)!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"解析报文失败\n");
			return -1;
		}
	}

	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解当日测试结果汇总表数据报文[%s]\n",szBuffer);
	
    return NORMAL;
}

/* 
 * 分解FTP下载测试结果
 */
RESULT DecodeTdFtpDownLoad(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nTdLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
  	FIXLENCFGSTRU struTdCfg[]=
	{
		{"Count",	        "UINT1", 1},          //测试次数
		{"CurIndex",	    "UINT1", 1},        //当前测试序号
		{"Time",	        "DT", 7},           //开始测试时间
		{"DataMode",	    "UINT1", 1},        //数据模式
		{"Result",	        "UINT1", 1},        //FTP下载结果
		{"HsDataLen",	        "UINT2", 2},        //FTP下载数据量
		{"HsSpendTime",	    "UINT2", 2},        //FTP下载时间
		{"TdDataLen",	        "UINT2", 2},        //TD-R4_FTP下载数据量
		{"TdSpendTime",	    "UINT2", 2},        	//TD-R4_FTP下载时长
		{"GsmDataLen",	        "UINT2", 2},        //GSM_FTP下载数据量
		{"GsmSpendTime",	    "UINT2", 2},        //GSM_FTP下载时长
		{"HsDownCount",	"UINT1", 1},        //HSDPA_FTP掉线次数
		{"TdDownCount",	"UINT1", 1},        //TD-R4_FTP掉线次数
		{"GsmDownCount",	"UINT1", 1},        //GSM_FTP掉线次数
		{"NetMode1",	"UINT1", 1},        //网络模式1
		{"Cid1",	"UINT2", 2},        //
		{"NetMode2",	"UINT1", 1},        //网络模式2
		{"Cid2",	"UINT2", 2},        //
		{"NetMode3",	"UINT1", 1},        //网络模式3
		{"Cid3",	"UINT2", 2},        //
		{"CellChooseTimes",	"UINT1", 1},        //小区重选总次数
		{"HsBler",	        "UINT1", 1},        //RLC层平均BLER（％）
		{"TdBler",	        "UINT1", 1},        //RLC层平均BLER（％）
		{"GsmBler",	        "UINT1", 1},        //RLC层平均BLER（％）
		{NULL,		"",     0}
	};
	
	
	if(nTdLen==0)
	{
		for(i=0;struTdCfg[i].pszName!=NULL;i++)
		{
			nTdLen+=struTdCfg[i].nLen;
		}
	}

	{
		if(nDataLen < nTdLen)
		{
			PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nTdLen);
			return -1;
		}
		
		if(UnpackFixLen(pszMapData, pstruXml, struTdCfg)!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"解析报文失败\n");
			return -1;
		}
	}

	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解FTP下载测试结果报文[%s]\n",szBuffer);
	
    return NORMAL;
}

/* 
 * 分解FTP上载测试结果
 */
RESULT DecodeTdFtpUpLoad(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nTdLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
 	
	FIXLENCFGSTRU struTdCfg[]=
	{
		{"Count",	"UINT1", 1},          //测试次数
		{"CurIndex",	        "UINT1", 1},        //当前测试序号
		{"Time",	        "DT", 7},        //开始测试时间
		{"DataMode",	    "UINT1", 1},        //数据业务模式
		{"Result",	        "UINT1", 1},        //FTP上载结果
		{"DataLen",	        "UINT2", 2},        //FTP上载数据量
		{"SpendTime",	    "UINT2", 2},        //FTP上载时间
		{NULL,		"",     0}
	};
	
	
	if(nTdLen==0)
	{
		for(i=0;struTdCfg[i].pszName!=NULL;i++)
		{
			nTdLen+=struTdCfg[i].nLen;
		}
	}

	{
		if(nDataLen < nTdLen)
		{
			PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nTdLen);
			return -1;
		}
		
		if(UnpackFixLen(pszMapData, pstruXml, struTdCfg)!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"解析报文失败\n");
			return -1;
		}
	}

	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解FTP上载测试结果报文[%s]\n",szBuffer);
	
    return NORMAL;
}


/*
 *  保存PESQ下行测试结束上报日志
 */
RESULT SaveTdPesqLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	UINT tst_Id;
	
	GetDbSerial(&tst_Id, "td_PesqLog");
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into td_pesqlog(td_Id, td_uploadtime, td_neid, td_times, td_curindex,"
		    " td_netmode, td_iscaller, td_endtelnum, td_dialtime, td_pesqdialstatus, "
		    " td_spendtime, td_lac, td_Uarfcn, td_CellId, td_Ta,"
		    " td_Txpower, td_cid1, td_cid2, td_cid3, td_switchcount, "
		    " td_rxlsub, td_rxlsubdata, td_dialoktime) values("
		    
		    " %d,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s, %s, %s,"
		    " %s,  %s, '%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s, "
			" %s, %s,  %s, %s,  %s,"
			" %s, %s,  %s,  %s, %s,"
			" %s, '%s', '%s') ",
			
			tst_Id,
			GetSysDateTime(),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Times>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"), 
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网络模式>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<IsCaller>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<EndTelNum>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<PesqDialStatus>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Lac>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Uarfcn>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CellId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Ta>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Txpower>"),
    	    DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SwitchCount>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<RxlSub>"),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<RxlSubData>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<ConnTime>")
		);
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
 *  保存CQT互拨测试结果上报日志
 */
RESULT SaveTdCqtLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	int nNeId;
	UINT nAlarmLogId;
	UINT tst_Id;
	int nDialStatus;
	char szCondition[100], szAlarmObjList[100], szTemp[10], szAlarm[2];
	PSTR pszSeperateStr[MAX_OBJECT_NUM];
	int nAlarm1=0, nAlarm2=0, nAlarm3=0;
	int nCount, i,nAlarmCount=0;
	
	bufclr(szAlarmObjList);
	strcpy(szAlarmObjList, DemandStrInXmlExt(pstruReqXml, "<omc>/<告警对象>"));
	if (strlen(szAlarmObjList) > 0)
	{
		nCount= SeperateString(szAlarmObjList, ',',pszSeperateStr, MAX_SEPERATE_NUM);
		for(i=0; i< nCount; i++)
		{
			strcpy(szTemp, pszSeperateStr[i]);
			if(strlen(szTemp) < 6 ) 
			{
				if (i == 0) nAlarm1 = 0;
				else if(i == 1) nAlarm2 = 0;
				else if(i == 2) nAlarm3 = 0;
			}
			else 
			{
				if ( !isdigit(szTemp[5]) ) strcpy(szAlarm , "0");
				else strncpy(szAlarm, szTemp+5, 1);
				
				if (i == 0) nAlarm1 = atoi(szAlarm);
				else if(i == 1) nAlarm2 = atoi(szAlarm);
				else if(i == 2) nAlarm3 = atoi(szAlarm);
			 }
		}
	}
	
	GetDbSerial(&tst_Id, "td_CqtTestLog");
	

	{
		snprintf(szSql, sizeof(szSql),
	    	" insert into td_CqtTestLog(td_Id, td_uploadtime, td_netmode, td_Count, td_CurIndex,"
	    	" td_IsCaller, td_EndTelNum,td_DialTime,td_AlertingTime, td_DialOkTime,"
	    	" td_DialStatus,td_SpendTime, td_lac, td_CellId, td_Uarfcn,"
	    	" td_Cpi, td_Ta, td_TxPower, td_Sir, td_Rscp,"
	    	" td_Rm,td_Midamble,td_DnSyncCode,td_UpSyncCode,td_Iscp,"
	    	" td_NetMode1,td_Cid1,td_NetMode2,td_Cid2,td_NetMode3,"
	    	" td_Cid3,td_UarfCn1,td_UarfCn2,td_UarfCn3,td_CellSwitchCount,"
	    	" td_FreqSwitchCount,td_Dpch,td_Bler, td_DpchData, td_BlerData, td_single, td_singledata,"
	    	" td_tel, td_areaid, td_alarm1, td_alarm2, td_alarm3, td_neid) values("
	    	
	    	" %d,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,   %s,   %s,"
			" %s,  '%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), '%s',"
			" %s, %s,  %s, %s,  %s,"
			" %s, %s,  %s, %s,  %s,"
			" %s, %s,  %s, %s,  %s,"
			" %s, %s,  %s, %s,  %s,"
			" %s, %s,  %s, %s,  %s,"
			" %s, %s,  %s, '%s', '%s', %s, '%s', "
			" '%s', %d, %d, %d, %d, %d) ",
			
			tst_Id,
			GetSysDateTime(),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网络模式>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"), 
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<IsCaller>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<EndTelNum>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<AlertingTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialOkTime>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialStatus>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Lac>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CellId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Uarfcn>"),
			
        	DemandStrInXmlExt(pstruReqXml, "<omc>/<Cpi>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Ta>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TxPower>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Sir>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Rscp>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Rm>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Midamble>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DnSyncCode>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<UpSyncCode>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Iscp>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<NetMode1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<NetMode2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<NetMode3>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<UarfCn1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<UarfCn2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<UarfCn3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CellSwitchCount>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<FreqSwitchCount>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Dpch>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bler>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DpchData>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<BlerData>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Single>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SingleData>"),
			
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<地区编号>")),
			nAlarm1, nAlarm2, nAlarm3,
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"))
		);
	}
	
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	STR szCollectDateTime[20];
	STR szCollectDate[20];
	bufclr(szCollectDateTime);
	bufclr(szCollectDate);
	strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<DialTime>"));
	strncpy(szCollectDate, szCollectDateTime, 10);
	

	//数据汇总处理, CQT累加1
    memset(szSql, 0, sizeof(szSql));
    nDialStatus = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<DialStatus>"));
    nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
    if (nDialStatus == 2)//BUSY
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set CQT_BUSY_TIMES=CQT_BUSY_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nNeId, szCollectDate);
	else if (nDialStatus == 4)//NO CARRIER
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set CQT_NO_CARRIER_TIMES=CQT_NO_CARRIER_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nNeId, szCollectDate);
	else if (nDialStatus == 6)//单通
	{
		sprintf(szSql, "par_SectionName ='alarmYJ' and par_KeyName = 'alarmYJ1'");
		GetSysParameter(szSql, szCondition);
		if (strcmp(szCondition, "TRUE") == 0  && ExistAlarmLog(ALM_DT_ID, nNeId, &nAlarmCount) == BOOLFALSE)
		{
			if(GetDbSerial(&nAlarmLogId, "AlarmLog")!=NORMAL)
			{
				PrintErrorLog(DBG_HERE,"获取告警日志流水号错误\n");
				return EXCEPTION;
			}
			snprintf(szSql, sizeof(szSql), "insert into  alm_AlarmLog (alg_AlarmLogId,alg_AlarmId,alg_NeId,alg_AlarmTime,alg_AlarmStatusId,alg_AlarmObjId, alg_compresscount, alg_AlarmInfo)"
        	     " VALUES(%d, %d, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 1, '%s', 1, '单通次数%d')", 
        	     nAlarmLogId, ALM_DT_ID, nNeId, GetSysDateTime(), "F902", nAlarmCount);
    		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
    		if(ExecuteSQL(szSql) !=NORMAL) 
			{
				PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
				return EXCEPTION;
			}
			CommitTransaction();
		}
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set CQT_SINGLE_TIMES=CQT_SINGLE_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
				nNeId, szCollectDate);

	}
	else if (nDialStatus == 5)//掉话
	{
		sprintf(szSql, "par_SectionName ='alarmYJ' and par_KeyName = 'alarmYJ2'");
		GetSysParameter(szSql, szCondition);
		if (strcmp(szCondition, "TRUE") == 0  && ExistAlarmLog(ALM_DH_ID, nNeId, &nAlarmCount) == BOOLFALSE)
		{
			if(GetDbSerial(&nAlarmLogId, "AlarmLog")!=NORMAL)
			{
				PrintErrorLog(DBG_HERE,"获取告警日志流水号错误\n");
				return EXCEPTION;
			}
			snprintf(szSql, sizeof(szSql), "insert into  alm_AlarmLog (alg_AlarmLogId,alg_AlarmId,alg_NeId,alg_AlarmTime,alg_AlarmStatusId,alg_AlarmObjId, alg_compresscount, alg_AlarmInfo)"
        	     " VALUES(%d, %d, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 1, '%s', 1, '掉话次数%d')", 
        	     nAlarmLogId, ALM_DH_ID, nNeId, GetSysDateTime(), "F903", nAlarmCount);
    		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
    		if(ExecuteSQL(szSql) !=NORMAL) 
			{
				PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
				return EXCEPTION;
			}
			CommitTransaction();
		}
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set CQT_DROP_TIMES=CQT_DROP_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nNeId, szCollectDate);
	}
	else if (nDialStatus == 1)//接通
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set CQT_LINK_TIMES=CQT_LINK_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nNeId, szCollectDate);
	else if (nDialStatus == 0)//正常
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set CQT_NORMAL_TIMES=CQT_NORMAL_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nNeId, szCollectDate);
    if (strlen(szSql) > 1)
    {
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
    	    return EXCEPTION;
		}
		CommitTransaction();
	}
	
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SwitchCount>")) > 3)
	{
		//切换频繁告警 大于3次
		sprintf(szSql, "par_SectionName ='alarmYJ' and par_KeyName = 'alarmYJ3'");
		GetSysParameter(szSql, szCondition);
		if (strcmp(szCondition, "TRUE") == 0 && ExistAlarmLog(ALM_QHPF_ID, nNeId, &nAlarmCount) == BOOLFALSE)
		{
			if(GetDbSerial(&nAlarmLogId, "AlarmLog")!=NORMAL)
			{
				PrintErrorLog(DBG_HERE,"获取告警日志流水号错误\n");
				return EXCEPTION;
			}
			snprintf(szSql, sizeof(szSql), "insert into  alm_AlarmLog (alg_AlarmLogId,alg_AlarmId,alg_NeId,alg_AlarmTime,alg_AlarmStatusId,alg_AlarmObjId, alg_compresscount, alg_AlarmInfo)"
        	     " VALUES(%d, %d, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 1, '%s', 1, '切换次数%d')", 
        	     nAlarmLogId, ALM_QHPF_ID, nNeId, GetSysDateTime(), "F904", nAlarmCount);
    		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
    		if(ExecuteSQL(szSql) !=NORMAL) 
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
 *  保存MMS测试结果上报日志
 */
RESULT SaveTdMmsTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	CURSORSTRU struCursor;
	UINT tst_Id;
	int nNeId;
	
	GetDbSerial(&tst_Id, "td_MmsTestLog");

	{
		if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SendOrReceive>")) == 1)//是发送端
		{
			snprintf(szSql, sizeof(szSql),
			    " insert into td_MmsTestLog(td_Id,td_Times,td_CurIndex,td_Title,td_SendOrReceive,"
			    " td_CallNumber, td_BeginSendTime,td_SendedTime,td_SendedIsSuccess,td_BeginPushTime,"
			    " td_PushedIsSuccess,td_MmsReceiveTime,td_MmsReceiveIsSu,td_UpdateTime,td_NeId,"
			    " td_AreaId, td_netmode) values("
			    " %d,   %s,  %s,   '%s',   %s,"
				" '%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),"
				" %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %d,"
				" %d,  %s) ",
				
				tst_Id,
				DemandStrInXmlExt(pstruReqXml, "<omc>/<Times>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<Title>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<SendOrReceive>"), 
				
				DemandStrInXmlExt(pstruReqXml, "<omc>/<CallNumber>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<BeginSendTime>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<SendedTime>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<SendedIsSuccess>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<BeginPushTime>"),
				
				
				DemandStrInXmlExt(pstruReqXml, "<omc>/<PushedIsSuccess>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<MmsReceiveTime>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<MmsReceiveIsSu>"),
				GetSysDateTime(),
				atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>")),
				
				atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<地区编号>")),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<网络模式>")
			);
		}
		else
		{
			snprintf(szSql, sizeof(szSql),
			    " insert into td_MmsTestLog2(td_Id,td_Times,td_CurIndex,td_Title,td_SendOrReceive,"
			    " td_CallNumber, td_BeginSendTime,td_SendedTime,td_SendedIsSuccess,td_BeginPushTime,"
			    " td_PushedIsSuccess,td_MmsReceiveTime,td_MmsReceiveIsSu,td_UpdateTime,td_NeId,"
			    " td_AreaId, td_netmode) values("
			    " %d,   %s,  %s,   '%s',   %s,"
				" '%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),"
				" %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %d,"
				" %d,  %s) ",
				
				tst_Id,
				DemandStrInXmlExt(pstruReqXml, "<omc>/<Times>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<Title>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<SendOrReceive>"), 
				
				DemandStrInXmlExt(pstruReqXml, "<omc>/<CallNumber>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<BeginSendTime>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<SendedTime>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<SendedIsSuccess>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<BeginPushTime>"),
				
				
				DemandStrInXmlExt(pstruReqXml, "<omc>/<PushedIsSuccess>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<MmsReceiveTime>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<MmsReceiveIsSu>"),
				GetSysDateTime(),
				atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>")),
				
				atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<地区编号>")),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<网络模式>")
			);
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
	
	STR szCollectDateTime[20];
	STR szCollectDate[20];
	bufclr(szCollectDateTime);
	bufclr(szCollectDate);
	nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	
	//数据汇总处理, MMS彩信上报累加1
	memset(szSql, 0, sizeof(szSql));
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SendOrReceive>")) == 1)//是发送端
	{
		strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<BeginSendTime>"));
		strncpy(szCollectDate, szCollectDateTime, 10);
				
		if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SendedIsSuccess>")) == 1)//发送成功
			snprintf(szSql, sizeof(szSql), "update td_data_collect_report set MMS_SEND_TIMES=MMS_SEND_TIMES+1,MMS_DTD_TIMES=MMS_DTD_TIMES+1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
				nNeId, szCollectDate);
	    else
	    	snprintf(szSql, sizeof(szSql), "update td_data_collect_report set MMS_DTD_TIMES=MMS_DTD_TIMES+1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
				nNeId, szCollectDate);
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
					szSql, GetSQLErrorMessage());
        	return EXCEPTION;
		}
	}
	
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SendOrReceive>")) == 0 &&
		atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<PushedIsSuccess>")) == 1)  //是接受端
	{
			STR szCallNumber[100], szTemp[20];
			strcpy(szCallNumber, DemandStrInXmlExt(pstruReqXml, "<omc>/<CallNumber>"));
			strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<MmsReceiveTime>"));
			strncpy(szCollectDate, szCollectDateTime, 10);
			
			if (strlen(szCallNumber) != 11)
			{
				if (strlen(szCallNumber) < 11) return EXCEPTION;
				bufclr(szTemp);
				strncpy(szTemp, szCallNumber+strlen(szCallNumber)-11, 11);
				bufclr(szCallNumber);
				strncpy(szCallNumber, szTemp, 11);
			}
			
			sprintf(szSql, "select ne_NeId from ne_Element where ne_netelnum = '%s'", szCallNumber);
    
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
	    	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没记录!\n",  szSql);
	    		return EXCEPTION;
	    	}
	    	nNeId = atoi(GetTableFieldValue(&struCursor, "ne_NeId"));
	    	FreeCursor(&struCursor);
	    	
	    	snprintf(szSql, sizeof(szSql), "update td_data_collect_report set MMS_RECEIVE_TIMES=MMS_RECEIVE_TIMES+1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
				nNeId, szCollectDate);
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


/*
 *  保存当日测试结果汇总日志
 */
RESULT SaveTdDayTestResultLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	UINT tst_Id;
	
	GetDbSerial(&tst_Id, "td_DayTestResultLog");
	
	snprintf(szSql, sizeof(szSql),
	    " insert into td_DayTestResultLog(td_Id,td_Time,td_CqtTimes,td_AttachTimes,td_PdpTimes,"
	    " td_PingTimes, td_FtpUploadTimes,td_FtpDownloadTimes,td_TimerUploadTimes,td_MmsTimes,"
	    " td_WapTimes, td_VpTimes, td_UpdateTime,td_NeId) values("
	    " %d,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,   %s,   %s,"
	    "  %s,  %s, %s,   %s, %s,"
	    "  %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s)",

		
		tst_Id,
		GetSysDateTime(),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<CqtTimes>"),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<AttachTimes>"),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<PdpTimes>"), 
		
		DemandStrInXmlExt(pstruReqXml, "<omc>/<PingTimes>"),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<FtpUploadTimes>"),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<FtpDownloadTimes>"),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<TimerUploadTimes>"),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<MmsTimes>"),
		
		DemandStrInXmlExt(pstruReqXml, "<omc>/<WapTimes>"),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<VpTimes>"),
		GetSysDateTime(),
		DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>")
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


/*
 *  保存GPRS测试结果上报日志
 */
RESULT SaveTdPsTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	//int tst_Id;
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into td_PsTestLog(td_Id,td_TaskId,td_UpLoadTime,td_NeId,td_GprsMode,"
		    " td_Attach, td_Pdp,td_Ping,td_FtpUpLoad,td_FtpDownLoad) values("
		    " %s, 0, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,   %s, "
			" %s, %s,  %s, %s, %s)",
    	
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TstId>"),
			GetSysDateTime(),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<GprsMode>"), 
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Attach>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Pdp>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Ping>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<FtpUpLoad>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<FtpDownLoad>")
		);
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
 *  保存ATTACH测试结果
 */
RESULT SaveTdAttachTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
    int i, nResult=0, nTime=0;
    char szPath[100];
    int nNeId, nGprsMode;
	
	{ 
		snprintf(szSql, sizeof(szSql),
		    " insert into td_AttachTestLog(td_Id, td_Count, td_CurIndex, td_Time,td_Result1,"
		    " td_Time1,td_Result2,td_Time2,td_Result3,td_Time3,"
		    " td_Result4,td_Time4,td_Result5,td_Time5,td_Result6,"
		    " td_Time6,td_Result7,td_Time7,td_Result8,td_Time8,"
		    " td_Result9,td_Time9,td_Result10,td_Time10) values("

		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
			" %s,  %s, %s, %s)",
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TstId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result1>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result3>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time3>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result5>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time5>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result6>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time6>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result7>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time7>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result8>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time8>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result9>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time9>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result10>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time10>")
		);
	}
	
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	STR szCollectDateTime[20];
	STR szCollectDate[20];
	bufclr(szCollectDateTime);
	bufclr(szCollectDate);
	strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"));
	strncpy(szCollectDate, szCollectDateTime, 10);
		
	//数据汇总处理, ATTACH上报累加1
	for(i=1; i<11; i++)
	{
		sprintf(szPath, "<omc>/<Result%d>", i);
		if (atoi(DemandStrInXmlExt(pstruReqXml, szPath)) == 1)
		{
			nResult++;
			sprintf(szPath, "<omc>/<Time%d>", i);
			nTime = nTime + atoi(DemandStrInXmlExt(pstruReqXml, szPath));
		}
		
	}
	nGprsMode = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<GprsMode>"));
	nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	snprintf(szSql, sizeof(szSql), "update td_data_collect_report set ATTACH_TEST_TIMES=ATTACH_TEST_TIMES+10, ATTACH_SUCCESS_TIMES=ATTACH_SUCCESS_TIMES+%d, "
	    "ATTACH_SUM_TIME=ATTACH_SUM_TIME+%d, TST_GPRSMODE=%d  where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
		nResult, nTime, nGprsMode, nNeId, szCollectDate);
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
 *  保存PDP激活测试结果
 */
RESULT SaveTdPdpTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	int i, nResult=0, nTime=0;
	char szPath[100];
	int nNeId;
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into td_PdpTestLog(td_Id, td_Count, td_CurIndex, td_Time,td_Result1,"
		    " td_Time1,td_Result2,td_Time2,td_Result3,td_Time3,"
		    " td_Result4,td_Time4,td_Result5,td_Time5,td_Result6,"
		    " td_Time6,td_Result7,td_Time7,td_Result8,td_Time8,"
		    " td_Result9,td_Time9,td_Result10,td_Time10) values("
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
			" %s,  %s, %s, %s)",
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TstId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result1>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result3>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time3>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result5>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time5>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result6>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time6>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result7>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time7>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result8>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time8>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result9>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time9>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result10>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time10>")
		);
	}

	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	STR szCollectDateTime[20];
	STR szCollectDate[20];
	bufclr(szCollectDateTime);
	bufclr(szCollectDate);
	strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"));
	strncpy(szCollectDate, szCollectDateTime, 10);
	
	
	//数据汇总处理, ATTACH上报累加1
	for(i=1; i<11; i++)
	{
		sprintf(szPath, "<omc>/<Result%d>", i);
		if (atoi(DemandStrInXmlExt(pstruReqXml, szPath)) == 1)
		{
			nResult++;
			sprintf(szPath, "<omc>/<Time%d>", i);
			nTime = nTime + atoi(DemandStrInXmlExt(pstruReqXml, szPath));
		}
		
	}
	nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	snprintf(szSql, sizeof(szSql), "update td_data_collect_report set PDP_TEST_TIMES=PDP_TEST_TIMES+10, PDP_SUCCESS_TIMES=PDP_SUCCESS_TIMES+%d, "
	    "PDP_SUM_TIME=PDP_SUM_TIME+%d where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
		nResult, nTime, nNeId, szCollectDate);
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
 *  保存PING测试结果
 */
RESULT SaveTdPingTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	int i, nResult=0, nTime=0;
	char szPath[100];
	int nNeId;
	
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into td_PingTestLog(td_Id, td_Count, td_CurIndex, td_Time,td_Result1,"
		    " td_Time1,td_Result2,td_Time2,td_Result3,td_Time3,"
		    " td_Result4,td_Time4,td_Result5,td_Time5,td_Result6,"
		    " td_Time6,td_Result7,td_Time7,td_Result8,td_Time8,"
		    " td_Result9,td_Time9,td_Result10,td_Time10) values("
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
			" %s,  %s, %s, %s)",
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TstId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result1>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result3>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time3>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result5>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time5>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result6>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time6>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result7>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time7>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result8>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time8>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result9>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time9>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result10>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time10>")
		);
	}
	
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	STR szCollectDateTime[20];
	STR szCollectDate[20];
	bufclr(szCollectDateTime);
	bufclr(szCollectDate);
	strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"));
	strncpy(szCollectDate, szCollectDateTime, 10);
	
	
	//数据汇总处理, ATTACH上报累加1
	for(i=1; i<11; i++)
	{
		sprintf(szPath, "<omc>/<Result%d>", i);
		if (atoi(DemandStrInXmlExt(pstruReqXml, szPath)) == 1)
		{
			nResult++;
			sprintf(szPath, "<omc>/<Time%d>", i);
			nTime = nTime + atoi(DemandStrInXmlExt(pstruReqXml, szPath));
		}
		
	}
	nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	snprintf(szSql, sizeof(szSql), "update td_data_collect_report set PING_TEST_TIMES=PING_TEST_TIMES+10, PING_SUCCESS_TIMES=PING_SUCCESS_TIMES+%d, "
	    "PING_SUM_TIME=PING_SUM_TIME+%d where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
		nResult, nTime, nNeId, szCollectDate);
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
 *  保存FTP上载测试结果
 */
RESULT SaveTdFtpUpLoadLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
    int nNeId;
	int nSpeedTime, nDataLen;

	{
		snprintf(szSql, sizeof(szSql),
		    " insert into td_FtpUpLoadLog(td_Id, td_Count, td_CurIndex, td_Time,td_Result,"
		    " td_DataLen,td_SpendTime, td_Datamode) values("
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
			" %s, %s, %s)",
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TstId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DataLen>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DataMode>")
		);
	}
		
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	STR szCollectDateTime[20];
	STR szCollectDate[20];
	bufclr(szCollectDateTime);
	bufclr(szCollectDate);
	strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"));
	strncpy(szCollectDate, szCollectDateTime, 10);
	
	
	//数据汇总处理 FTP_UPLOG
	nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<Result>")) == 1)
	{
		nSpeedTime = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>"));
		nDataLen = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<DataLen>"));
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set FTP_TEST_TIMES=FTP_TEST_TIMES+1, FTP_SUCCESS_TIMES=FTP_SUCCESS_TIMES+1,"
	    	"FTP_SPEED_TIMES=FTP_SPEED_TIMES+%d, FTP_DATA_LEN=FTP_DATA_LEN+%d where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nSpeedTime, nDataLen, nNeId, szCollectDate);
	}
	else
	{
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set FTP_TEST_TIMES=FTP_TEST_TIMES+1 "
	    	"where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nNeId, szCollectDate);
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
 *  保存FTP下载测试结果
 */
RESULT SaveTdFtpDownLoadLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	int nSpeedTime, nDataLen;
	int nNeId, nAlarmCount=0;
	char szCondition[100];
	UINT nAlarmLogId;
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into td_FtpDownLoadLog(td_Id, td_Count, td_CurIndex, td_Time,td_Result,"
		    " td_HsDataLen,td_HsSpendTime,  td_TdDataLen,td_TdSpendTime,  td_GsmDataLen,"
		    " td_GsmSpendTime, td_HsDownCount, td_TdDownCount,td_GsmDownCount,  td_NetMode1,"
		    " td_Cid1,td_NetMode2,td_Cid2, td_NetMode3, td_Cid3, td_CellChooseTimes,"
		    " td_HsBler, td_TdBler, td_GsmBler, td_AreaId, td_Neid, td_DataMode) values("
		    
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, %s,"
			" %s, %s,  %s, %d, %d, %d)",
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TstId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<HsDataLen>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<HsSpendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TdDataLen>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TdSpendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<GsmDataLen>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<GsmSpendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<HsDownCount>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TdDownCount>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<GsmDownCount>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<NetMode1>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<NetMode2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<NetMode3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CellChooseTimes>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<HsBler>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TdBler>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<GsmBler>"),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>")),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<地区编号>")),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<DataMode>")) 
		);
	}

	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	STR szCollectDateTime[20];
	STR szCollectDate[20];
	bufclr(szCollectDateTime);
	bufclr(szCollectDate);
	strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"));
	strncpy(szCollectDate, szCollectDateTime, 10);
	
	
	//数据汇总处理 FTP_UPLOG
	nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<Result>")) == 1)
	{
		nSpeedTime = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>"));
		nDataLen = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<DataLen>"));
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set FTP2_TEST_TIMES=FTP2_TEST_TIMES+1, FTP2_SUCCESS_TIMES=FTP2_SUCCESS_TIMES+1,"
	    	"FTP2_SPEED_TIMES=FTP2_SPEED_TIMES+%d, FTP2_DATA_LEN=FTP2_DATA_LEN+%d where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nSpeedTime, nDataLen, nNeId, szCollectDate);
	}
	else
	{
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set FTP2_TEST_TIMES=FTP2_TEST_TIMES+1 "
	    	"where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nNeId, szCollectDate);
	}
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<CellChooseTimes>")) > 3)
	{
		//切换频繁告警 大于3次
		sprintf(szSql, "par_SectionName ='alarmYJ' and par_KeyName = 'alarmYJ7'");
		GetSysParameter(szSql, szCondition);
		if (strcmp(szCondition, "TRUE") == 0 && ExistAlarmLog(ALM_FTPCX_ID, nNeId, &nAlarmCount) == BOOLFALSE)
		{
			if(GetDbSerial(&nAlarmLogId, "AlarmLog")!=NORMAL)
			{
				PrintErrorLog(DBG_HERE,"获取告警日志流水号错误\n");
				return EXCEPTION;
			}
			snprintf(szSql, sizeof(szSql), "insert into  alm_AlarmLog (alg_AlarmLogId,alg_AlarmId,alg_NeId,alg_AlarmTime,alg_AlarmStatusId,alg_AlarmObjId, alg_compresscount, alg_AlarmInfo)"
        	     " VALUES(%d, %d, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 1, '%s', 1, '重选次数%d')", 
        	     nAlarmLogId, ALM_QHPF_ID, nNeId, GetSysDateTime(), "F904", nAlarmCount);
    		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
    		if(ExecuteSQL(szSql) !=NORMAL) 
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
 *  保存当日测试结果汇总日志
 */
RESULT SaveVpTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
    char szAlarmObjList[100], szTemp[10], szAlarm[2];
	PSTR pszSeperateStr[MAX_OBJECT_NUM];
	int nAlarm1=0, nAlarm2=0, nAlarm3=0;
	int nCount, i;
	UINT tst_Id;
	int nNeId;
	
	bufclr(szAlarmObjList);
	strcpy(szAlarmObjList, DemandStrInXmlExt(pstruReqXml, "<omc>/<告警对象>"));
	if (strlen(szAlarmObjList) > 0)
	{
		nCount= SeperateString(szAlarmObjList, ',',pszSeperateStr, MAX_SEPERATE_NUM);
		for(i=0; i< nCount; i++)
		{
			strcpy(szTemp, pszSeperateStr[i]);
			if(strlen(szTemp) < 6 ) 
			{
				if (i == 0) nAlarm1 = 0;
				else if(i == 1) nAlarm2 = 0;
				else if(i == 2) nAlarm3 = 0;
			}
			else 
			{
				if ( !isdigit(szTemp[5]) ) strcpy(szAlarm , "0");
				else strncpy(szAlarm, szTemp+5, 1);
				
				if (i == 0) nAlarm1 = atoi(szAlarm);
				else if(i == 1) nAlarm2 = atoi(szAlarm);
				else if(i == 2) nAlarm3 = atoi(szAlarm);
			 }
		}
	}
	
	GetDbSerial(&tst_Id, "td_VpTestLog");
	
	snprintf(szSql, sizeof(szSql),
	    	" insert into td_VpTestLog(td_Id, td_uploadtime, td_Count, td_CurIndex,td_IsCaller,"
	    	" td_EndTelNum,td_DialTime,td_AlertingTime, td_isConnect, td_videooktime,"
	    	" td_firstvideotime, td_videoisConnect, td_videoendtime, td_dialendtime, td_DialStatus,"
	    	"  td_lac, td_CellId, td_Uarfcn, td_Cpi, td_Ta,"
	    	"  td_TxPower, td_Sir, td_Rscp, td_Rm,td_Midamble,"
	    	" td_DnSyncCode,td_UpSyncCode,td_Iscp, td_Cid1,td_Cid2,"

	    	" td_Cid3,td_UarfCn1,td_UarfCn2,td_UarfCn3,td_CellSwitchCount,"
	    	" td_FreqSwitchCount,td_Dpch,td_Bler, td_DpchData, td_BlerData,"
	    	" td_tel, td_areaid, td_alarm1, td_alarm2, td_alarm3, td_neid) values("
	    	
	    	" %d,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,   %s, %s, "
			" '%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), "
			" to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s,"
			"  %s,  %s, %s, %s, %s,"
			"  %s, %s,  %s, %s, %s, "
			" %s, %s,  %s, %s,  %s,"

			" %s, %s,  %s, %s,  %s,"
			" %s, %s,  %s, '%s', '%s',"
			" '%s', %d, %d, %d, %d, %d) ",
			
			tst_Id,
			GetSysDateTime(),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<IsCaller>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<EndTelNum>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<AlertingTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<IsConnect>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<VideookTime>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<FirstVideoTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<VideoisConnect>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<VideoendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialStatus>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Lac>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CellId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Uarfcn>"),
        	DemandStrInXmlExt(pstruReqXml, "<omc>/<Cpi>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Ta>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TxPower>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Sir>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Rscp>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Rm>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Midamble>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DnSyncCode>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<UpSyncCode>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Iscp>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid2>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<UarfCn1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<UarfCn2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<UarfCn3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CellSwitchCount>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<FreqSwitchCount>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Dpch>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bler>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DpchData>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<BlerData>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<地区编号>")),
			nAlarm1, nAlarm2, nAlarm3,
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"))
		);
	
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	STR szCollectDateTime[20];
	STR szCollectDate[20];
	bufclr(szCollectDateTime);
	bufclr(szCollectDate);
	strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<DialTime>"));
	strncpy(szCollectDate, szCollectDateTime, 10);
	

	//数据汇总处理, CQT累加1
    memset(szSql, 0, sizeof(szSql));
    nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));

	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<IsConnect>")) == 1)
	{
		sprintf(szSql,  "update td_data_collect_report set VP_DIAL_TIMES=VP_DIAL_TIMES + 1");
		
		if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<IsConnect>")) == 1)
			sprintf(szSql,  " %s, VP_VIDEO_TIMES=VP_VIDEO_TIMES +1", szSql);
		if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<DialStatus>")) == 1)//
			sprintf(szSql,  " %s, VP_SUCCESS_TIMES=VP_SUCCESS_TIMES + 1", szSql);
		
		sprintf(szSql,  "%s where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			szSql, nNeId, szCollectDate);
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
        	return EXCEPTION;
		}
		CommitTransaction();
	}
	
	
	return NORMAL;  
}

/*
 *  保存TD测试结果上报日志
 */
RESULT SaveTdTimerUploadLog(INT nNeId, PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	UINT up_Id;
	
	GetDbSerial(&up_Id, "timeruploadlog");
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into td_TimerUploadLog(TD_ID,TD_UPLOADTIME,TD_NEID,TD_POWERALARM,TD_BATTERALARM,TD_RXLALARM,"
		    " TD_COLLECTTIME, TD_LAC,TD_MCC,TD_MNC,TD_CELLID,TD_RAC,"
		    " TD_UARFCN,TD_CPI,TD_PRSCP,TD_ISCP,TD_PathLoss,TD_SIR,"
		    " TD_Midamble, TD_DnSyncCode, TD_UpSyncCode,TD_NetMode,"
		    " TD_RSCPN1,TD_RSCPN2,TD_RSCPN3,TD_RSCPN4,TD_RSCPN5,TD_RSCPN6,"
		    " TD_ISCPN1,TD_ISCPN2,TD_ISCPN3,TD_ISCPN4,TD_ISCPN5,TD_ISCPN6,"
		    " TD_SIRN1,TD_SIRN2,TD_SIRN3,TD_SIRN4,TD_SIRN5,TD_SIRN6,"
		    " TD_PathLossN1,TD_PathLossN2,TD_PathLossN3,TD_PathLossN4,TD_PathLossN5,TD_PathLossN6,"
		    " TD_UarfcnN1,TD_UarfcnN2,TD_UarfcnN3,TD_UarfcnN4,TD_UarfcnN5,TD_UarfcnN6,"
		    " TD_CPIN1,TD_CPIN2,TD_CPIN3,TD_CPIN4,TD_CPIN5,TD_CPIN6"
		    " ) values("
		    
		    " %d, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %d, %d, %d, %d,"
		    " to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', %d, "
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s')",
    	
			up_Id,
			GetSysDateTime(),
			nNeId,
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<0301>")), 
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<0304>")),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<0704>")),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0150>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0508>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0728>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0507>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<050C>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08B0>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08B1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08B2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08B3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08B4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08B5>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08B7>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08B9>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08BA>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08BB>"),
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<0779>")),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08BC>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08BD>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08BE>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08BF>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08C0>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08C1>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08C2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08C3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08C4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08C5>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08C6>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08C7>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08C8>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08C9>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08CA>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08CB>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08CC>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08CD>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08CE>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08CF>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08D0>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08D1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08D2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08D3>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08D4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08D5>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08D6>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08D7>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08D8>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08D9>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08DB>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08DC>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08DE>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08DF>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08E0>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<08E1>")
		);

	}
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	STR szCollectDateTime[20];
	STR szCollectDate[20];
	bufclr(szCollectDateTime);
	bufclr(szCollectDate);
	strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<0150>"));
	strncpy(szCollectDate, szCollectDateTime, 10);
	
	//数据汇总处理, 定时上报累加1
	snprintf(szSql, sizeof(szSql), "update td_data_collect_report set SCHEDULE_FACT_TIMES=SCHEDULE_FACT_TIMES+1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
		nNeId, szCollectDate);
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
 *  保存当日测试结果汇总日志
 */
RESULT SaveTdWapTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	UINT tst_Id;
	
	GetDbSerial(&tst_Id, "sm_WapTestLog");
	//td
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into td_WapTestLog(td_Id,td_Times,td_CurIndex, td_TestTime,td_GprsMode, "
		    " td_WapTitle,td_WapType, td_BeginTime,td_EndTime,td_IsSuccess, "
		    " td_HomeBeginTime,td_HomeEndTime,td_DownData, td_HomeIsSuccess, td_NeId,"
		    " td_AreaId) values("
		    " %d,  %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
		    " '%s', %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s,"
		    " to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s, %s, %s,"
		    "  %d)",
    	
			tst_Id,
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Times>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TestTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<GprsMode>"), 
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<WapTitle>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<WapType>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<BeginTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<EndTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<IsSuccess>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<HomeBeginTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<HomeEndTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DownData>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<HomeIsSuccess>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<地区编号>"))
		);
	}
		
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	memset(szSql, 0, sizeof(szSql));
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<IsSuccess>")) == 1)//是否登录成功
	{
		STR szCollectDateTime[20], szCollectDate[20];
		STR szBeginTime[20], szEndTime[20];
		INT nHomeTime, nNeId;
		
		bufclr(szCollectDateTime);
		bufclr(szCollectDate);
		nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
		
		strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<TestTime>"));
		strncpy(szCollectDate, szCollectDateTime, 10);
				
		snprintf(szSql, sizeof(szSql), "update td_data_collect_report set WAP_LOGIN_TIMES=WAP_LOGIN_TIMES+1");
		
		if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<HomeIsSuccess>")) == 1)//主页显示成功
		{
			strcpy(szEndTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<HomeEndTime>"));
			strcpy(szBeginTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<HomeBeginTime>"));
			nHomeTime = MakeITimeFromLastTime(szEndTime) - MakeITimeFromLastTime(szBeginTime);
			
			snprintf(szSql, sizeof(szSql),"%s, WAP_HOME_TIME=WAP_HOME_TIME+%d, WAP_HOME_DATA=WAP_HOME_DATA+ %d, WAP_DOWN_TIMES=WAP_DOWN_TIMES+1",
				szSql,  nHomeTime, atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<DownData>")));
		}
		snprintf(szSql, sizeof(szSql), "%s where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
				szSql, nNeId, szCollectDate);
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
        	return EXCEPTION;
		}
		CommitTransaction();
	}
	return NORMAL;  
}


