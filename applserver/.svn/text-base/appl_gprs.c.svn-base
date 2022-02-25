/*
 * 名称: 网管系统应用服务器(GPRS上报部分)
 *
 * 修改记录:
 * 付志刚 -		2009-1-9 创建
 */
 
#include <ebdgdl.h>
#include <omcpublic.h>
#include <applserver.h>

//extern char	szGprsRespBuffer[100];		/* gprs应答通讯报文 */

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

static int TimeStampToSeconds(char *timestamp)
{
	char temp[10];
	int dialyear, dialmon, dialday, dialhour, dialmin, dialsec;
	time_t timep;   
    struct tm atm;

	
	memset(temp, 0, sizeof(temp));
	strncpy(temp, timestamp, 4);
	dialyear = atoi(temp);
	
	memset(temp, 0, sizeof(temp));
	strncpy(temp, timestamp+4, 2);
	dialmon = atoi(temp);
	
	memset(temp, 0, sizeof(temp));
	strncpy(temp, timestamp+6, 2);
	dialday = atoi(temp);
	
	memset(temp, 0, sizeof(temp));
	strncpy(temp, timestamp+8, 2);
	dialhour = atoi(temp);
	
	memset(temp, 0, sizeof(temp));
	strncpy(temp, timestamp+10, 2);
	dialmin = atoi(temp);
	
	memset(temp, 0, sizeof(temp));
	strncpy(temp, timestamp+12, 2);
	dialsec = atoi(temp);
      
    atm.tm_year = dialyear-1900;
    atm.tm_mon = dialmon-1;
    atm.tm_mday = dialday;
    atm.tm_hour = dialhour;
    atm.tm_min = dialmin;
    atm.tm_sec = dialsec;
    
    timep = mktime(&atm);
    
    return timep;
}

RESULT GetBatPickTimePoint(const char *pszBeginTime, int nOffset, char *pszTimePoint)
{
	char szDateTime[20];
	int nStartSeconds;
	int nEndSeconds;
	
	//格式 20100714153906
	strcpy(szDateTime, pszBeginTime);
	nStartSeconds = (int)MakeITimeFromSTime(szDateTime);
	nEndSeconds = nStartSeconds + nOffset;
	strcpy(pszTimePoint, MakeSTimeFromITime(nEndSeconds));
	return NORMAL;
}


/* 
 * 分解PESQ下行测试结束上报(客观语音质量评估)
 */
RESULT DecodeGprsPesq(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nAssertLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
    FIXLENCFGSTRU struCfg[]=
	{
		{"DialTime",	"DT", 7},          //拨打日期时间	整数（7）
		{"EndTelNum",	"STR", 20},        // 对端号码	字符串（20）
		{"PesqDialStatus",	"UINT1", 1},   //PESQ接通状态
		{"Lac",	        "UINT2", 2},
		{"Bcch",	    "UINT2", 2},
		{"Bsic",	    "UINT1", 1},
		{"Cid1",	    "UINT2", 2},
		{"Cid2",	    "UINT2", 2},
		{"Cid3",	    "UINT2", 2},
		{"SwitchCount",	    "UINT1", 1},   //切换总次数
		{"RxlSub",	    "SINT1", 1},       //呼叫电平(RXL)
		{"RxqSub",	    "UINT1", 1},       //话音质量(RXQ)
		{NULL,		"",     0}
	};
	
		
	if(nAssertLen==0)
	{
		for(i=0;struCfg[i].pszName!=NULL;i++)
		{
			nAssertLen+=struCfg[i].nLen;
		}
	}


	{
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
	}
	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解PESQ下行测试结束上报报文[%s]\n",szBuffer);
	
    return NORMAL;
}

/* 
 * 分解CQT语音拨测
 */
RESULT DecodeGprsCqt(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nAssertLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i, nSpendTime, nSumValue=0, N, nSingle;
	STR szValueStr[MAX_BUFFER_LEN], szTemp[10];
	
    FIXLENCFGSTRU struCfg[]=
	{
		{"Count",	    "UINT1", 1},      //测试次数
		{"CurIndex",	"UINT1", 1},      // 当前测试序号
		{"IsCaller",	"UINT1", 1},      //主被叫标识
		{"EndTelNum",	"STR",  20},      //对端号码
		{"DialTime",	"DT",    7},      //拨打时间
		{"DialOkTime",	    "TIME", 3},   //接通时间
		{"DialStatus",	    "UINT1", 1},  //接通状态
		{"SpendTime",	    "UINT2", 2},  //通话时长
		{"Lac",	        "UINT2", 2},
		{"Bcch",	    "UINT2", 2},      //
		{"Bsic",	    "UINT1", 1},      //
		{"Ta",	        "UINT1", 1},          //
		{"Pl",	        "UINT1", 1},          //
		{"Cid1",	    "UINT2", 2},          //
		{"Cid2",	    "UINT2", 2},          //
		{"Cid3",	    "UINT2", 2},          //
		{"SwitchCount",	"UINT1", 1},      //切换总次数
		{NULL,		"",     0}
	};
	
	
	if(nAssertLen==0)
	{
		for(i=0;struCfg[i].pszName!=NULL;i++)
		{
			nAssertLen+=struCfg[i].nLen;
		}
	}
	
	{
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
		pszMapData += nAssertLen;  //移到电平明细
		
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
    	
    	nSumValue = 0;
		memset(szValueStr, 0, sizeof(szValueStr));
		for(i = 0 ; i < N ; i++)
		{
			//话音质量(RXQ) SUB	整数（1）	0~7，2秒一次，取平均值。(保留1位小数,即比例是10)
			memset(szTemp, 0, sizeof(szTemp));
			DecodeMapDataFromType("UINT1", 1,  pszMapData, szTemp);
			pszMapData += 1;
			if (atoi(szTemp) >= 8)
			{				
				nSumValue += atoi(szTemp)/10;
				sprintf(szValueStr, "%s%d,", szValueStr, atoi(szTemp)/10);
			}
			else
			{
				nSumValue += atoi(szTemp);
			    sprintf(szValueStr, "%s%s,", szValueStr, szTemp);
			}
		}
    	
		if(N>0)
		{
		    TrimRightChar(szValueStr, ',');
		    sprintf(szTemp, "%d", nSumValue / N);
		    InsertInXmlExt(pstruXml,"<omc>/<Rxq>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		}
		else
		{
			InsertInXmlExt(pstruXml,"<omc>/<Rxq>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
		}
		InsertInXmlExt(pstruXml,"<omc>/<RxqData>", szValueStr, MODE_AUTOGROW|MODE_UNIQUENAME);
		
		nSingle = 0;
		memset(szValueStr, 0, sizeof(szValueStr));
		for(i = 0 ; i < N ; i++)
		{
			//单通检测  SUB	整数（1）	0~7，2秒一次，取平均值。(保留1位小数,即比例是10)
			memset(szTemp, 0, sizeof(szTemp));
			DecodeMapDataFromType("UINT1", 1,  pszMapData, szTemp);
			pszMapData += 1;				
			if (atoi(szTemp) == 0)	nSingle ++;
			//nSumValue += atoi(szTemp);
			sprintf(szValueStr, "%s%s,", szValueStr, szTemp);
		}
    	
		if(N>0)
		{
		    TrimRightChar(szValueStr, ',');
		    //sprintf(szTemp, "%d", nSumValue / N);
		    if (nSingle > N/2) 
		    	InsertInXmlExt(pstruXml,"<omc>/<Single>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
		    else
		    	InsertInXmlExt(pstruXml,"<omc>/<Single>", "1", MODE_AUTOGROW|MODE_UNIQUENAME);
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
 * 分解MMS测试结果数据
 */
RESULT DecodeMms(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nAssertLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i, nTemp;
	
    FIXLENCFGSTRU struCfg[]=
	{
		{"Times",	"UINT1", 1},          //测试次数
		{"No",	    "UINT1", 1},          // 当前测试序号
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
	
	
	if(nAssertLen==0)
	{
		for(i=0;struCfg[i].pszName!=NULL;i++)
		{
			nAssertLen+=struCfg[i].nLen;
		}
	}


	{
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
 * 分解WAP测试结果数据
 */
RESULT DecodeWap(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
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
 * 分解当日测试结果汇总表数据
 */
RESULT DecodeDayTestResult(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nAssertLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
    FIXLENCFGSTRU struCfg[]=
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
		{"WapTimes",	        "UINT2", 2},        //WAP测试 gsm2.0增加
		{NULL,		"",     0}
	};
	
	
	if(nAssertLen==0)
	{
		for(i=0;struCfg[i].pszName!=NULL;i++)
		{
			nAssertLen+=struCfg[i].nLen;
		}
	}

	{
		if(nDataLen < nAssertLen-2)//兼容gsm1.0
		{
			PrintErrorLog(DBG_HERE,"输入报文长度[%d]有误 报文长度应为[%d]\n",nDataLen, nAssertLen);
			return -1;
		}
		if(UnpackFixLen(pszMapData, pstruXml, struCfg)!=NORMAL)
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
RESULT DecodeFtpDownLoad(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nAssertLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
    FIXLENCFGSTRU struCfg[]=
	{
		{"Count",	        "UINT1", 1},          //测试次数
		{"CurIndex",	    "UINT1", 1},        //当前测试序号
		{"Time",	        "DT", 7},        //开始测试时间
		{"Result",	        "UINT1", 1},        //FTP下载结果
		{"DataLen",	        "UINT2", 2},        //FTP下载数据量
		{"SpendTime",	    "UINT2", 2},        //FTP下载时间
		{"Cid1",	"UINT2", 2},        //
		{"Bcch1",	"UINT2", 2},        //
		{"Cid2",	"UINT2", 2},        //
		{"Bcch2",	"UINT2", 2},        //
		{"Cid3",	"UINT2", 2},        //
		{"Bcch3",	"UINT2", 2},        //
		{"CellChooseTimes",	"UINT1", 1},        //小区重选总次数
		{"Bler",	        "UINT1", 1},        //RLC层平均BLER（％）
		{"TimesLot",	    "UINT1", 1},        //平均时隙使用数量
		{"Cs1",	        "UINT1", 1},        //GPRS 编码使用比例
		{"Cs2",	        "UINT1", 1},        //
		{"Cs3",	        "UINT1", 1},        //
		{"Cs4",	        "UINT1", 1},        //
		{"Mc1",	        "UINT1", 1},        //EDGE编码使用比例
		{"Mc2",	        "UINT1", 1},        //
		{"Mc3",	        "UINT1", 1},        //
		{"Mc4",	        "UINT1", 1},        //
		{"Mc5",	        "UINT1", 1},        //
		{"Mc6",	        "UINT1", 1},        //
		{"Mc7",	        "UINT1", 1},        //
		{"Mc8",	        "UINT1", 1},        //
		{"Mc9",	        "UINT1", 1},        //
		{NULL,		"",     0}
	};
	
		
	if(nAssertLen==0)
	{
		for(i=0;struCfg[i].pszName!=NULL;i++)
		{
			nAssertLen+=struCfg[i].nLen;
		}
	}

	{	
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
	}
	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解FTP下载测试结果报文[%s]\n",szBuffer);
	
    return NORMAL;
}

/* 
 * 分解FTP上载测试结果
 */
RESULT DecodeFtpUpLoad(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
{
    static INT nAssertLen=0;
    STR szBuffer[MAX_BUFFER_LEN];
	INT i;
	
    FIXLENCFGSTRU struCfg[]=
	{
		{"Count",	"UINT1", 1},          //测试次数
		{"CurIndex",	        "UINT1", 1},        //当前测试序号
		{"Time",	        "DT", 7},        //开始测试时间
		{"Result",	        "UINT1", 1},        //FTP上载结果
		{"DataLen",	        "UINT2", 2},        //FTP上载数据量
		{"SpendTime",	    "UINT2", 2},        //FTP上载时间
		{NULL,		"",     0}
	};
	
	if(nAssertLen==0)
	{
		for(i=0;struCfg[i].pszName!=NULL;i++)
		{
			nAssertLen+=struCfg[i].nLen;
		}
	}

	{
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
	}
	memset(szBuffer, 0, sizeof(szBuffer));
	ExportXml(pstruXml, szBuffer, sizeof(szBuffer));
	PrintDebugLog(DBG_HERE,"分解FTP上载测试结果报文[%s]\n",szBuffer);
	
    return NORMAL;
}

/* 
 * 分解PING测试结果
 */
RESULT DecodeGprsPing(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
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
RESULT DecodeGprsPdp(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
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
RESULT DecodeGprsAttach(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml)
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
 *  保存PESQ下行测试结束上报日志
 */
RESULT SaveGprsPesqLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	UINT tst_Id;
	int nRxqSub;
	
	nRxqSub = atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<RxqSub>"))/10;
	
	GetDbSerial(&tst_Id, "sm_PesqLog");
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_pesqlog(tst_Id, tst_uploadtime, tst_neid, tst_call2center, tst_dialtime,"
		    " tst_endtelnum, tst_pesqdialstatus, tst_lac, tst_bcch, tst_bsic,"
		    "tst_cid1, tst_cid2, tst_cid3, tst_switchcount, tst_rxlsub, tst_rxqsub) values("
		    "%d,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,   %s,   to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),"
			" '%s', %s,  %s, %s,  %s,"
			" %s,  %s, %s,  %s, %s,  %d) ",
			
			tst_Id,
			GetSysDateTime(),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<呼叫方向>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialTime>"), 
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<EndTelNum>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<PesqDialStatus>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Lac>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bcch>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bsic>"),
			
    	    DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SwitchCount>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<RxlSub>"),
			nRxqSub
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
RESULT SaveGprsCqtLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	int nNeId;
	UINT nAlarmLogId;
	UINT tst_Id;
	int nDialStatus;
	char szCondition[100], szAlarmObjList[100], szTemp[10], szAlarm[2];
	PSTR pszSeperateStr[MAX_OBJECT_NUM];
	int nAlarm1=0, nAlarm2=0, nAlarm3=0;
	int nCount, i, nAlarmCount=0;
	
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
	
	GetDbSerial(&tst_Id, "sm_CqtTestLog");
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_CqtTestLog(tst_Id, tst_uploadtime, tst_neid, tst_Count, tst_CurIndex,"
		    " tst_IsCaller, tst_endneid,tst_EndTelNum,tst_DialTime,tst_DialOkTime,"
		    " tst_DialStatus,tst_SpendTime, tst_lac, tst_bcch, tst_bsic,"
		    " tst_cid1, tst_cid2, tst_cid3, tst_switchcount, tst_RxlSub,"
		    " tst_RxlSubData,tst_Rxq,tst_RxqData,tst_Ta,tst_Pl, tst_single, tst_singledata, "
		    " tt_tel, tt_alarm1, tt_alarm2, tt_alarm3, tt_areaid) values("
		    " %d,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,   %s,   %s,"
			" %s, %s,  '%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  '%s',"
			" %s, %s,  %s, %s,  %s,"
			" %s, %s,  %s, %s,  %s,"
			" '%s',  %s, '%s',  %s, %s, %s, '%s', "
			" '%s', %d, %d, %d, %d) ",
			
			tst_Id,
			GetSysDateTime(),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"), 
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<IsCaller>"),
			"0",
			DemandStrInXmlExt(pstruReqXml, "<omc>/<EndTelNum>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialOkTime>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DialStatus>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Lac>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bcch>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bsic>"),
			
    	    DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SwitchCount>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<RxlSub>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<RxlSubData>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Rxq>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<RxqData>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Ta>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Pl>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Single>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SingleData>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"),
			nAlarm1, nAlarm2, nAlarm3,
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
		snprintf(szSql, sizeof(szSql), "update data_collect_report set CQT_BUSY_TIMES=CQT_BUSY_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nNeId, szCollectDate);
	else if (nDialStatus == 4)//NO CARRIER
		snprintf(szSql, sizeof(szSql), "update data_collect_report set CQT_NO_CARRIER_TIMES=CQT_NO_CARRIER_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
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
		//2009.9.10 增加区分主叫和被叫单通
		if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<IsCaller>")) == 1)
			snprintf(szSql, sizeof(szSql), "update data_collect_report set CQT_SINGLE_TIMES=CQT_SINGLE_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
				nNeId, szCollectDate);
		else
			snprintf(szSql, sizeof(szSql), "update data_collect_report set CQT_SINGLE_TIMES2=CQT_SINGLE_TIMES2 + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
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
		snprintf(szSql, sizeof(szSql), "update data_collect_report set CQT_DROP_TIMES=CQT_DROP_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nNeId, szCollectDate);
	}
	else if (nDialStatus == 1)//接通
	{
		if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<Single>")) == 0)// 50%单通监测
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
			//2009.9.10 增加区分主叫和被叫单通
			if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<IsCaller>")) == 1)
				snprintf(szSql, sizeof(szSql), "update data_collect_report set CQT_SINGLE_TIMES=CQT_SINGLE_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
					nNeId, szCollectDate);
			else
				snprintf(szSql, sizeof(szSql), "update data_collect_report set CQT_SINGLE_TIMES2=CQT_SINGLE_TIMES2 + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
					nNeId, szCollectDate);
		}
		else //正常接通
		{
			snprintf(szSql, sizeof(szSql), "update data_collect_report set CQT_LINK_TIMES=CQT_LINK_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
				nNeId, szCollectDate);
		}
	}
	else if (nDialStatus == 0)//正常
		snprintf(szSql, sizeof(szSql), "update data_collect_report set CQT_NORMAL_TIMES=CQT_NORMAL_TIMES + 1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
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
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SwitchCount>")) > 0)//更新切换次数
	{
		snprintf(szSql, sizeof(szSql), "update data_collect_report set CQT_SWITCH_TIMES=CQT_SWITCH_TIMES + %d where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SwitchCount>")), nNeId, szCollectDate);
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
RESULT SaveMmsTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
    char szSql2[MAX_BUFFER_LEN];
	CURSORSTRU struCursor;
	UINT tst_Id;
	int nNeId;
	
	GetDbSerial(&tst_Id, "sm_MmsTestLog");

	{
		if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SendOrReceive>")) == 1)//是发送端
		{
			snprintf(szSql, sizeof(szSql),
			    " insert into sm_MmsTestLog(tst_Id,tst_Times,tst_No,tst_Title,tst_SendOrReceive,"
			    " tst_CallNumber, tst_BeginSendTime,tst_SendedTime,tst_SendedIsSuccess,tst_BeginPushTime,"
			    " tst_PushedIsSuccess,tst_MmsReceiveTime,tst_MmsReceiveIsSu,tst_UpdateTime,tst_NeId, tst_AreaId) values("
			    " %d,   %s,  %s,   '%s',   %s,"
				" '%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),"
				" %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %d, %d) ",
				
				tst_Id,
				DemandStrInXmlExt(pstruReqXml, "<omc>/<Times>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<No>"),
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
				atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<地区编号>"))
			);
			PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
			if(ExecuteSQL(szSql) != NORMAL)
			{
				PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
					szSql, GetSQLErrorMessage());
        		return EXCEPTION;
			}
			CommitTransaction();
		}
		else
		{
			snprintf(szSql2, sizeof(szSql2),
			    " insert into sm_MmsTestLog2(tst_Id,tst_Times,tst_No,tst_Title,tst_SendOrReceive,"
			    " tst_CallNumber, tst_BeginSendTime,tst_SendedTime,tst_SendedIsSuccess,tst_BeginPushTime,"
			    " tst_PushedIsSuccess,tst_MmsReceiveTime,tst_MmsReceiveIsSu,tst_UpdateTime,tst_NeId, tst_AreaId) values("
			    " %d,   %s,  %s,   '%s',   %s,"
				" '%s', to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),"
				" %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %d, %d) ",
				
				tst_Id,
				DemandStrInXmlExt(pstruReqXml, "<omc>/<Times>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<No>"),
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
				atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<地区编号>"))
			);
		}
	}
	
	
	
	STR szCollectDateTime[20];
	STR szCollectDate[20];
	bufclr(szCollectDateTime);
	bufclr(szCollectDate);
	nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	
	//数据汇总处理, MMS彩信上报累加1
	
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SendOrReceive>")) == 1)//是发送端
	{
		strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<BeginSendTime>"));
		strncpy(szCollectDate, szCollectDateTime, 10);
				
		if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SendedIsSuccess>")) == 1)//发送成功
			snprintf(szSql, sizeof(szSql), "update data_collect_report set MMS_SEND_TIMES=MMS_SEND_TIMES+1,MMS_DTD_TIMES=MMS_DTD_TIMES+1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
				nNeId, szCollectDate);
	    else
	    	snprintf(szSql, sizeof(szSql), "update data_collect_report set MMS_DTD_TIMES=MMS_DTD_TIMES+1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
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
			STR szCallNumber[100], szTemp[20], szTitle[100], szSendedTime[30];
			
			strcpy(szCallNumber, DemandStrInXmlExt(pstruReqXml, "<omc>/<CallNumber>"));
			strcpy(szTitle, DemandStrInXmlExt(pstruReqXml, "<omc>/<Title>"));//测试标题
			
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
			//查找网元编号
			sprintf(szSql, "select ne_NeId from ne_Element where ne_netelnum = '%s' and ne_devicestatusid=0", szCallNumber);
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
	    	
	    	//查找发送端数据
			sprintf(szSql, "select to_char(tst_sendedtime, 'yyyy-mm-dd hh24:mi:ss') as tst_sendedtime from sm_mmstestlog  where tst_title = '%s' and tst_neid=%d", szTitle,  nNeId);
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
	    		//保留该记录
	    		strcpy(szGprsRespBuffer, "9999");
	    		
	    	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没记录!\n",  szSql);
	    		return EXCEPTION;
	    	}
	    	strcpy(szSendedTime, GetTableFieldValue(&struCursor, "tst_sendedtime"));
	    	FreeCursor(&struCursor);
	    	
	    	//查找发送端明细，再插入
	    	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql2);
			if(ExecuteSQL(szSql2) != NORMAL)
			{
				PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
					szSql2, GetSQLErrorMessage());
        		return EXCEPTION;
			}
			CommitTransaction();
	    	
	    	STR szBeginPushTime[30], szMmsReceiveTime[30];
	    	INT nPushTime, nRecvTime;
	    	
	    	strcpy(szBeginPushTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<BeginPushTime>"));
	    	strcpy(szMmsReceiveTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<MmsReceiveTime>"));
	    	
	    	nPushTime = MakeITimeFromLastTime(szBeginPushTime) - MakeITimeFromLastTime(szSendedTime);
	    	nRecvTime = MakeITimeFromLastTime(szMmsReceiveTime) - MakeITimeFromLastTime(szSendedTime);
	    	
	    	if (nPushTime < 0 || nPushTime > 300) nPushTime = 20;
	    	if (nRecvTime < 0 || nRecvTime > 300) nRecvTime = 20;
	    		
	    	memset(szSql, 0, sizeof(szSql));
	    	//push成功和累加时间
	    	snprintf(szSql, sizeof(szSql), "update data_collect_report set MMS_PUSH_SUCCESS_TIMES=MMS_PUSH_SUCCESS_TIMES + 1, MMS_PUSH_SUM_TIME = MMS_PUSH_SUM_TIME + %d", nPushTime);
				    	
	    	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<MmsReceiveIsSu>")) == 1)//提取成功次数
	    	{
	    		sprintf(szSql, "%s, MMS_RECEIVE_TIMES=MMS_RECEIVE_TIMES+1, MMS_RECEIVE_SUM_TIME=MMS_RECEIVE_SUM_TIME + %d", szSql, nRecvTime);

			}
			sprintf(szSql, "%s where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')", szSql, 
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
RESULT SaveDayTestResultLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	UINT tst_Id;
	
	GetDbSerial(&tst_Id, "sm_DayTestResultLog");

	snprintf(szSql, sizeof(szSql),
	    " insert into sm_DayTestResultLog(tst_Id,tst_Time,tst_CqtTimes,tst_AttachTimes,tst_PdpTimes,tst_PingTimes,"
	    " tst_FtpUploadTimes,tst_FtpDownloadTimes,tst_TimerUploadTimes,tst_MmsTimes,tst_WapTimes, tst_UpdateTime,tst_NeId) values("
	    " %d,  to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,   %s,   %s,  %s, "
		" %s, %s,  %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s)",

		
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
 *  保存当日测试结果汇总日志
 */
RESULT SaveWapTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	UINT tst_Id;
	STR szCollectDateTime[20], szCollectDate[20];
	STR szBeginTime[20], szEndTime[20];
	INT nHomeTime, nNeId;
	
	GetDbSerial(&tst_Id, "sm_WapTestLog");

	{
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_WapTestLog(tst_Id,tst_Times,tst_CurIndex, tst_TestTime,tst_GprsMode, "
		    " tst_WapTitle,tst_WapType, tst_BeginTime,tst_EndTime,tst_IsSuccess, "
		    " tst_HomeBeginTime,tst_HomeEndTime,tst_DownData, tst_HomeIsSuccess, tst_NeId,"
		    " tst_AreaId) values("
		    " %d, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s,"
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
	
	/*
	 * 更新数据汇总表
	 */
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "update data_collect_report set WAP_TEST_TIMES=WAP_TEST_TIMES+1");
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<IsSuccess>")) == 1)//是否登录成功
	{
	
		bufclr(szCollectDateTime);
		bufclr(szCollectDate);
		nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
		
		strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<TestTime>"));
		strncpy(szCollectDate, szCollectDateTime, 10);
				
		sprintf(szSql, "%s, WAP_LOGIN_TIMES=WAP_LOGIN_TIMES+1", szSql);
		
		if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<HomeIsSuccess>")) == 1)//主页下载成功
		{
			strcpy(szEndTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<HomeEndTime>"));
			strcpy(szBeginTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<HomeBeginTime>"));
			nHomeTime = MakeITimeFromLastTime(szEndTime) - MakeITimeFromLastTime(szBeginTime);
			if (nHomeTime > 0 && atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<DownData>")) > 0)
				sprintf(szSql,"%s, WAP_HOME_TIME=WAP_HOME_TIME+%d, WAP_HOME_DATA=WAP_HOME_DATA+ %d, WAP_DOWN_TIMES=WAP_DOWN_TIMES+1",
					szSql,  nHomeTime, atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<DownData>")));
		}
		
	}
	sprintf(szSql, "%s where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
				szSql, nNeId, szCollectDate);
	     
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

RESULT ImSaveWapTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	UINT tst_Id;
	
	GetDbSerial(&tst_Id, "sm_WapTestLog");
	
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<GprsMode>")) == 3 ||
		atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<GprsMode>")) == 4)
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into im_WapTestLog(im_Id,im_Times,im_CurIndex, im_TestTime,im_GprsMode, "
		    " im_WapTitle,im_WapType, im_BeginTime,im_EndTime,im_IsSuccess, "
		    " im_HomeBeginTime,im_HomeEndTime,im_DownData, im_HomeIsSuccess, im_NeId,"
		    " im_AreaId) values("
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
	else
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into im_WapTestLog(im_Id,im_Times,im_CurIndex, im_TestTime,im_GprsMode, "
		    " im_WapTitle,im_WapType, im_BeginTime,im_EndTime,im_IsSuccess, "
		    " im_HomeBeginTime,im_HomeEndTime,im_DownData, im_HomeIsSuccess, im_NeId,"
		    " im_AreaId) values("
		    " %d, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s,"
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
	return NORMAL;  
}
/*
 *  保存GPRS测试结果上报日志
 */
RESULT SaveGprsTestLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	{
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_GprsTestLog(tst_Id,tst_TaskId,tst_UpLoadTime,tst_NeId,tst_GprsMode,"
		    " tst_Attach, tst_Pdp,tst_Ping,tst_FtpUpLoad,tst_FtpDownLoad) values("
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
RESULT SaveGprsAttachLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
    int i, nResult=0, nTime=0;
    char szPath[100];
    int nNeId, nGprsMode;
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_GprsAttachLog(tst_Id, tst_Count, tst_CurIndex, tst_Time,tst_Result1,"
		    " tst_Time1,tst_Result2,tst_Time2,tst_Result3,tst_Time3,"
		    " tst_Result4,tst_Time4,tst_Result5,tst_Time5,tst_Result6,"
		    " tst_Time6,tst_Result7,tst_Time7,tst_Result8,tst_Time8,"
		    " tst_Result9,tst_Time9,tst_Result10,tst_Time10) values("
		    
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
	snprintf(szSql, sizeof(szSql), "update data_collect_report set ATTACH_TEST_TIMES=ATTACH_TEST_TIMES+10, ATTACH_SUCCESS_TIMES=ATTACH_SUCCESS_TIMES+%d, "
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

RESULT ImSaveGprsAttachLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into im_AttachTestLog(im_Id, im_Count, im_CurIndex, im_Time,im_Result1,"
		    " im_Time1,im_Result2,im_Time2,im_Result3,im_Time3,"
		    " im_Result4,im_Time4,im_Result5,im_Time5,im_Result6,"
		    " im_Time6,im_Result7,im_Time7,im_Result8,im_Time8,"
		    " im_Result9,im_Time9,im_Result10,im_Time10, im_NeId) values("
		    
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
			" %s,  %s, %s, %s, %d)",
			
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
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time10>"),
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
	

	return NORMAL;  
}
/*
 *  保存PDP激活测试结果
 */
RESULT SaveGprsPdpLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	int i, nResult=0, nTime=0;
	char szPath[100];
	int nNeId;
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_GprsPdpLog(tst_Id, tst_Count, tst_CurIndex, tst_Time,tst_Result1,"
		    " tst_Time1,tst_Result2,tst_Time2,tst_Result3,tst_Time3,"
		    " tst_Result4,tst_Time4,tst_Result5,tst_Time5,tst_Result6,"
		    " tst_Time6,tst_Result7,tst_Time7,tst_Result8,tst_Time8,"
		    " tst_Result9,tst_Time9,tst_Result10,tst_Time10) values("
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
	snprintf(szSql, sizeof(szSql), "update data_collect_report set PDP_TEST_TIMES=PDP_TEST_TIMES+10, PDP_SUCCESS_TIMES=PDP_SUCCESS_TIMES+%d, "
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

RESULT ImSaveGprsPdpLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into im_PdpTestLog(im_Id, im_Count, im_CurIndex, im_Time,im_Result1,"
		    " im_Time1,im_Result2,im_Time2,im_Result3,im_Time3,"
		    " im_Result4,im_Time4,im_Result5,im_Time5,im_Result6,"
		    " im_Time6,im_Result7,im_Time7,im_Result8,im_Time8,"
		    " im_Result9,im_Time9,im_Result10,im_Time10, im_NeId) values("
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
			" %s,  %s, %s, %s, %d)",
			
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
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time10>"),
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
	
	return NORMAL;  
}
/*
 *  保存PING测试结果
 */
RESULT SaveGprsPingLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	int i, nResult=0, nTime=0;
	char szPath[100];
	int nNeId;
	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_GprsPingLog(tst_Id, tst_Count, tst_CurIndex, tst_Time,tst_Result1,"
		    " tst_Time1,tst_Result2,tst_Time2,tst_Result3,tst_Time3,"
		    " tst_Result4,tst_Time4,tst_Result5,tst_Time5,tst_Result6,"
		    " tst_Time6,tst_Result7,tst_Time7,tst_Result8,tst_Time8,"
		    " tst_Result9,tst_Time9,tst_Result10,tst_Time10) values("
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
	snprintf(szSql, sizeof(szSql), "update data_collect_report set PING_TEST_TIMES=PING_TEST_TIMES+10, PING_SUCCESS_TIMES=PING_SUCCESS_TIMES+%d, "
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

RESULT ImSaveGprsPingLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	
	{
		snprintf(szSql, sizeof(szSql),
		    " insert into im_PingTestLog(im_Id, im_Count, im_CurIndex, im_Time,im_Result1,"
		    " im_Time1,im_Result2,im_Time2,im_Result3,im_Time3,"
		    " im_Result4,im_Time4,im_Result5,im_Time5,im_Result6,"
		    " im_Time6,im_Result7,im_Time7,im_Result8,im_Time8,"
		    " im_Result9,im_Time9,im_Result10,im_Time10, im_NeId) values("
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
			" %s,  %s, %s, %s, %d)",
			
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
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time10>"),
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
	
	return NORMAL;  
}
/*
 *  保存FTP上载测试结果
 */
RESULT SaveFtpUpLoadLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
    int nNeId;
	int nSpeedTime, nDataLen;

	{
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_FtpUpLoadLog(tst_Id, tst_Count, tst_CurIndex, tst_Time,tst_Result,"
		    " tst_DataLen,tst_SpendTime) values("
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
			" %s, %s)",
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TstId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DataLen>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>")
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
		snprintf(szSql, sizeof(szSql), "update data_collect_report set FTP_TEST_TIMES=FTP_TEST_TIMES+1, FTP_SUCCESS_TIMES=FTP_SUCCESS_TIMES+1,"
	    	"FTP_SPEED_TIMES=FTP_SPEED_TIMES+%d, FTP_DATA_LEN=FTP_DATA_LEN+%d where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nSpeedTime, nDataLen, nNeId, szCollectDate);
	}
	else
	{
		snprintf(szSql, sizeof(szSql), "update data_collect_report set FTP_TEST_TIMES=FTP_TEST_TIMES+1 "
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

RESULT ImSaveFtpUpLoadLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
 

	{
		snprintf(szSql, sizeof(szSql),
		    " insert into im_FtpUpLoadLog(im_Id, im_Count, im_CurIndex, im_Time,im_Result,"
		    " im_DataLen,im_SpendTime, im_NeId) values("
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
			" %s, %s, %d)",
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TstId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DataLen>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>"),
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

	return NORMAL;  
}

/*
 *  保存FTP下载测试结果
 */
RESULT SaveFtpDownLoadLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	int nSpeedTime, nDataLen;
	int nNeId, nAlarmCount=0;
	char szCondition[100];
	UINT nAlarmLogId;

	{
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_FtpDownLoadLog(tst_Id, tst_Count, tst_CurIndex, tst_Time,tst_Result,"
		    " tst_DataLen,tst_SpendTime, tst_Cid1,tst_Bcch1,tst_Cid2,"
		    " tst_Bcch2,tst_Cid3,tst_Bcch3,tst_CellChooseTimes,tst_Bler,"
		    " tst_TimesLot,tst_Cs1,tst_Cs2,tst_Cs3,tst_Cs4,"
		    " tst_Mc1,tst_Mc2,tst_Mc3,tst_Mc4,tst_Mc5,"
		    " tst_Mc6,tst_Mc7,tst_Mc8,tst_Mc9, tst_neid) values("
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
			" %s, %s,  %s, %s, %d)",
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TstId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DataLen>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bcch1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid2>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bcch2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bcch3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CellChooseTimes>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bler>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TimesLot>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cs1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cs2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cs3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cs4>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc5>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc6>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc7>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc8>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc9>"),
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
	strcpy(szCollectDateTime, DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"));
	strncpy(szCollectDate, szCollectDateTime, 10);
	
	
	//数据汇总处理 FTP_UPLOG
	nNeId = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"));
	if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<Result>")) == 1)
	{
		nSpeedTime = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>"));
		nDataLen = atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<DataLen>"));
		snprintf(szSql, sizeof(szSql), "update data_collect_report set FTP2_TEST_TIMES=FTP2_TEST_TIMES+1, FTP2_SUCCESS_TIMES=FTP2_SUCCESS_TIMES+1,"
	    	"FTP2_SPEED_TIMES=FTP2_SPEED_TIMES+%d, FTP2_DATA_LEN=FTP2_DATA_LEN+%d where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
			nSpeedTime, nDataLen, nNeId, szCollectDate);
	}
	else
	{
		snprintf(szSql, sizeof(szSql), "update data_collect_report set FTP2_TEST_TIMES=FTP2_TEST_TIMES+1 "
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

RESULT ImSaveFtpDownLoadLog(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];


	{
		snprintf(szSql, sizeof(szSql),
		    " insert into im_FtpDownLoadLog(im_Id, im_Count, im_CurIndex, im_Time,im_Result,"
		    " im_DataLen,im_SpendTime, im_Cid1,im_Bcch1,im_Cid2,"
		    " im_Bcch2,im_Cid3,im_Bcch3,im_CellChooseTimes,im_Bler,"
		    " im_TimesLot,im_Cs1,im_Cs2,im_Cs3,im_Cs4,"
		    " im_Mc1,im_Mc2,im_Mc3,im_Mc4,im_Mc5,"
		    " im_Mc6,im_Mc7,im_Mc8,im_Mc9, im_neid) values("
		    " %s, %s, %s, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %s,"
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
		    " %s, %s,  %s, %s, %s, "
			" %s, %s,  %s, %s, %d)",
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TstId>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Count>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CurIndex>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Time>"), 
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Result>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<DataLen>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<SpendTime>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bcch1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid2>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bcch2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cid3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bcch3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<CellChooseTimes>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Bler>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<TimesLot>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cs1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cs2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cs3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Cs4>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc1>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc2>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc3>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc4>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc5>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc6>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc7>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc8>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<Mc9>"),
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
	
	
	return NORMAL;  
}

/*
 *  保存GPRS测试结果上报日志
 */
RESULT SaveTimerUploadLog(INT nNeId, PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];

	UINT up_Id;
	
	GetDbSerial(&up_Id, "timeruploadlog");
	
	{
			
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_TimerUploadLog(UP_ID,UP_UPLOADTIME,UP_NEID,UP_POWERALARM,UP_BATTERALARM,UP_RXLALARM,"
		    " UP_LAC,UP_MCC,UP_MNC,UP_CELLID,UP_BSIC,UP_BCCH,"
		    " UP_BCCHRXLEV,UP_C1,UP_C2,UP_TA,UP_POWERLEVEL,UP_TS,"
		    " UP_CELLIDN1,UP_CELLIDN2,UP_CELLIDN3,UP_CELLIDN4,UP_CELLIDN5,UP_CELLIDN6,"
		    " UP_BSICN1,UP_BSICN2,UP_BSICN3,UP_BSICN4,UP_BSICN5,UP_BSICN6,"
		    " UP_BCCHN1,UP_BCCHN2,UP_BCCHN3,UP_BCCHN4,UP_BCCHN5,UP_BCCHN6,"
		    " UP_BCCHRXLEVN1,UP_BCCHRXLEVN2,UP_BCCHRXLEVN3,UP_BCCHRXLEVN4,UP_BCCHRXLEVN5,UP_BCCHRXLEVN6,"
		    " UP_C1N1,UP_C1N2,UP_C1N3,UP_C1N4,UP_C1N5,UP_C1N6,"
		    " UP_C2N1,UP_C2N2,UP_C2N3,UP_C2N4,UP_C2N5,UP_C2N6,"
		    " UP_COLLECTTIME,UP_INTERVALTIME) values("
		    " %d, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %d, %s, %d, %s,"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
			" to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s)",
    	
			up_Id,
			GetSysDateTime(),
			nNeId,
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0301>"), 
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<03CC>")),//0304  改为 03CC (电池故障改为低电压) 2010.2.8
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0704>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0508>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0728>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0507>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<050C>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0509>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<050A>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<050B>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<073B>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<073C>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<073D>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<073E>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<073F>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0710>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0711>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0712>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0713>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0714>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0715>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0716>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0717>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0718>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0719>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071A>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071B>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071C>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071D>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071E>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071F>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0720>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0721>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0722>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0723>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0724>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0725>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0726>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0727>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0750>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0751>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0752>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0753>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0754>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0755>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0756>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0757>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0758>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0759>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<075A>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<075B>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0150>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<070B>")
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
	snprintf(szSql, sizeof(szSql), "update data_collect_report set SCHEDULE_FACT_TIMES=SCHEDULE_FACT_TIMES+1 where ne_neid = %d and collect_date = to_date('%s', 'yyyy-mm-dd')",
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
 *  保存基本行效果监控2.0 GPRS测试结果上报日志
 */
RESULT SaveTimerUploadLog_jb2(INT nNeId, PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
    int i;
	char szTemp[20];
	/*
	 * 四个外部告警
	 */	
	int nExternalAlarm[5];
	char szExternalAlarm[5][5] = { "0320", "0321", "0322", "0323", ""};//对应外部告警1234
	int nAlarmId[5] = {22, 23, 24, 25, -1};
	
	int nExtAlarmId;
	TBINDVARSTRU struBindVar;
	
	for(i = 0; i < 4; i ++)
	{
    	//依alarmid取ExtAlarmId
    	memset(&struBindVar, 0, sizeof(struBindVar));
		struBindVar.struBind[0].nVarType = SQLT_INT;
		struBindVar.struBind[0].VarValue.nValueInt = nNeId;
		struBindVar.nVarCount++;
		
		struBindVar.struBind[1].nVarType = SQLT_INT;
		struBindVar.struBind[1].VarValue.nValueInt = nAlarmId[i];
		struBindVar.nVarCount++;
		
    	sprintf(szSql, "select EAS_EXTALARMID from alm_extalarmset where eas_neid = :v_0 and eas_alarmid = :v_1");
    	PrintDebugLog(DBG_HERE, "开始执行SQL[%s][%d][%d]\n", szSql, nNeId, nAlarmId[i]);
		if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
    	{
    		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
    					  szSql, GetSQLErrorMessage());
    		return EXCEPTION;
    	}
    	if(FetchCursor(&struCursor) != NORMAL)
    	{
    		nExtAlarmId = 0;
    		//FreeCursor(&struCursor);
    	    //PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没记录!\n",  szSql);
    		//return EXCEPTION;
    	}
    	else
    	{
    		nExtAlarmId = atoi(GetTableFieldValue(&struCursor, "EAS_EXTALARMID"));
    	}
    	
    	FreeCursor(&struCursor);
    	
    	if(nExtAlarmId > 0 && nExtAlarmId <= 4)
    	{	
    		sprintf(szTemp, "<omc>/<%s>", &szExternalAlarm[i][0]);
    		PrintDebugLog(DBG_HERE, "[%s]\n", szTemp);
    		nExternalAlarm[nExtAlarmId-1] = atoi(DemandStrInXmlExt(pstruReqXml, szTemp));
    	}
    	else
    	{
    		nExternalAlarm[nExtAlarmId-1] = 0;
    		
    		//PrintDebugLog(DBG_HERE, "外部告警ID[%d]超过范围\n", nExtAlarmId);
    		//return EXCEPTION;
    	}	
    	
	}	
	
	/*
	 * 存储定时上报数据
	 */
	UINT up_Id;
	
	GetDbSerial(&up_Id, "timeruploadlog");
	
	{
			
		snprintf(szSql, sizeof(szSql),
		    " insert into sm_TimerUploadLog(UP_ID,UP_UPLOADTIME,UP_NEID,UP_POWERALARM,UP_BATTERALARM,UP_RXLALARM,"
		    " UP_LAC,UP_MCC,UP_MNC,UP_CELLID,UP_BSIC,UP_BCCH,"
		    " UP_BCCHRXLEV,UP_C1,UP_C2,UP_TA,UP_POWERLEVEL,UP_TS,"
		    " UP_CELLIDN1,UP_CELLIDN2,UP_CELLIDN3,UP_CELLIDN4,UP_CELLIDN5,UP_CELLIDN6,"
		    " UP_BSICN1,UP_BSICN2,UP_BSICN3,UP_BSICN4,UP_BSICN5,UP_BSICN6,"
		    " UP_BCCHN1,UP_BCCHN2,UP_BCCHN3,UP_BCCHN4,UP_BCCHN5,UP_BCCHN6,"
		    " UP_BCCHRXLEVN1,UP_BCCHRXLEVN2,UP_BCCHRXLEVN3,UP_BCCHRXLEVN4,UP_BCCHRXLEVN5,UP_BCCHRXLEVN6,"
		    " UP_C1N1,UP_C1N2,UP_C1N3,UP_C1N4,UP_C1N5,UP_C1N6,"
		    " UP_C2N1,UP_C2N2,UP_C2N3,UP_C2N4,UP_C2N5,UP_C2N6,"
		    " UP_COLLECTTIME,UP_INTERVALTIME, up_outalarm1, up_outalarm2, up_outalarm3, up_outalarm4) values("
		    " %d, to_date('%s', 'yyyy-mm-dd hh24:mi:ss'),  %d, '%s', %d, '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
		    " '%s', '%s', '%s', '%s', '%s', '%s',"
			" to_date('%s', 'yyyy-mm-dd hh24:mi:ss'), %s, %d, %d, %d, %d)",
    	
			up_Id,
			GetSysDateTime(),
			nNeId,
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0301>"), 
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<03CC>")),//0304  改为 03CC (电池故障改为低电压) 2010.2.8
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0704>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0508>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0728>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0507>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<050C>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0509>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<050A>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<050B>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<073B>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<073C>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<073D>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<073E>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<073F>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0710>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0711>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0712>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0713>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0714>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0715>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0716>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0717>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0718>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0719>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071A>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071B>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071C>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071D>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071E>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<071F>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0720>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0721>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0722>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0723>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0724>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0725>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0726>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0727>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0750>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0751>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0752>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0753>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0754>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0755>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0756>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0757>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0758>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0759>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<075A>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<075B>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<0150>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<070B>"),
			
			nExternalAlarm[0],nExternalAlarm[1],
			nExternalAlarm[2],nExternalAlarm[3]
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
	
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_INT;
	struBindVar.struBind[0].VarValue.nValueInt = nNeId;
	struBindVar.nVarCount++;
	
	struBindVar.struBind[1].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[1].VarValue.szValueChar, szCollectDate);
	struBindVar.nVarCount++;
		
	//数据汇总处理, 定时上报累加1
	snprintf(szSql, sizeof(szSql), "update data_collect_report set SCHEDULE_FACT_TIMES=SCHEDULE_FACT_TIMES+1 where ne_neid = :v_0 and collect_date = to_date(:v_1, 'yyyy-mm-dd')");
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s][%d][%s]\n", szSql,	nNeId, szCollectDate);
	if(BindExecuteSQL(szSql, &struBindVar) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	return NORMAL;  
}



RESULT InitBatPick(PXMLSTRU pstruXml)
{
	CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	
	int nNeId;
	char BPObj[9];	
	char szBPBeginTime[20];
	
	int nQryHis;
	int nBPId;
	
	nNeId = atoi(DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//网元
	strcpy(BPObj, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象内容>"));//批采对象
	
	//取参
	if (strlen(BPObj) == 4)
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %s and epm_objid ='0601'", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//批采开始时间
	else
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %s and epm_objid ='00000601'", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//批采开始时间
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
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    return EXCEPTION;

	}
	strcpy(szBPBeginTime, GetTableFieldValue(&struCursor, "epm_CurValue"));//批采开始时间
	FreeCursor(&struCursor);
	//查询历史重复记录
	sprintf(szSql, "select count(*) from SM_BATPICKLOG where BP_NEID = %d and BP_OBJECT ='%s'"
					" and BP_BEGINTIME <= to_date('%s', 'yyyy-mm-dd hh24:mi:ss') and BP_BEGINTIME >= to_date('%s', 'yyyy-mm-dd hh24:mi:ss')", 
					nNeId, BPObj, szBPBeginTime, szBPBeginTime);
					
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
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    return EXCEPTION;
	}
	nQryHis = atoi(GetTableFieldValue(&struCursor, "count(*)"));
	FreeCursor(&struCursor);
	if (nQryHis > 0 )
	{
		sprintf(szSql, "select BP_ID from SM_BATPICKLOG where BP_NEID = %d and BP_OBJECT ='%s'"
					" and BP_BEGINTIME <= to_date('%s', 'yyyy-mm-dd hh24:mi:ss') and BP_BEGINTIME >= to_date('%s', 'yyyy-mm-dd hh24:mi:ss')", 
					nNeId, BPObj, szBPBeginTime, szBPBeginTime);
					
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
		    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
		    				  szSql, GetSQLErrorMessage());
		    return EXCEPTION;
		}
		
		nBPId = atoi(GetTableFieldValue(&struCursor, "BP_ID"));
		FreeCursor(&struCursor);
		
		sprintf(szSql,"delete from SM_BATPICKLOG where BP_ID = %d",  nBPId);
	
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
	        return EXCEPTION;
		}
		
		sprintf(szSql,"delete from SM_BATPICKTMP where BP_ID = %d",  nBPId);
	
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
	        return EXCEPTION;
		}
		
		sprintf(szSql,"delete from SM_BATPICKDAT where BP_ID = %d",  nBPId);
	
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


RESULT GetBatPickCount(PXMLSTRU pstruXml)
{
	CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	
	int nNeId;
	char BPObj[9];	
	char szBPBeginTime[20];
	
	int nQryHis;
	int nBPId;
	
	nNeId = atoi(DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//网元
	strcpy(BPObj, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象内容>"));//批采对象
	
	//取参
	if (strlen(BPObj) == 4)
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %s and epm_objid ='0601'", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//批采开始时间
	else
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %s and epm_objid ='00000601'", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//批采开始时间
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
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    return EXCEPTION;

	}
	strcpy(szBPBeginTime, GetTableFieldValue(&struCursor, "epm_CurValue"));//批采开始时间
	FreeCursor(&struCursor);
	//查询历史重复记录
	sprintf(szSql, "select count(*) as cnt from sm_batpickdat where BP_NEID = %d and BP_OBJECT ='%s'"
					" and BP_PICKUPTIME <= to_date('%s', 'yyyy-mm-dd hh24:mi:ss') and BP_PICKUPTIME >= to_date('%s', 'yyyy-mm-dd hh24:mi:ss')", 
					nNeId, BPObj, szBPBeginTime, szBPBeginTime);
					
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
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    return EXCEPTION;
	}
	nQryHis = atoi(GetTableFieldValue(&struCursor, "cnt"));
	FreeCursor(&struCursor);
	
	if (nQryHis > 0)	
		return NORMAL;
	else
		return EXCEPTION;
}


RESULT SaveBatPickLog(PXMLSTRU pstruXml)
{
	CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	UINT nBatPickId;
	char szBPBeginTime[20];
	int nBPSpan;
	char szTemp[50];
	char szBPObj[9];
	
	strcpy(szBPObj, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象内容>"));
	
	//取参
	if (strlen(szBPObj) == 4)
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %s and epm_objid ='0601'", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//批采开始时间
	else
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %s and epm_objid ='00000601'", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//批采开始时间
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    FreeCursor(&struCursor);				  
	    return EXCEPTION;

	}
	strcpy(szBPBeginTime, GetTableFieldValue(&struCursor, "epm_CurValue"));
	if (strlen(szBPObj) == 4)
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %s and epm_objid ='0602'", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//批采时长
	else
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %s and epm_objid ='00000602'", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//批采时长
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());		  
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    FreeCursor(&struCursor);				  
	    return EXCEPTION;

	}
	nBPSpan = atoi(GetTableFieldValue(&struCursor, "epm_CurValue"));
	FreeCursor(&struCursor);
	
	//记录批采主参数
	GetDbSerial(&nBatPickId, "sm_BatPickQry");
	
	sprintf(szSql,
	    "insert into SM_BATPICKLOG(BP_ID, BP_NEID, BP_BEGINTIME, BP_SPAN, BP_OBJECT) values("
	    "%d,  %s,  to_date( '%s','yyyy-mm-dd hh24:mi:ss'), %d, '%s' ) ",
		nBatPickId,
		DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"),
		szBPBeginTime,
		nBPSpan,
		szBPObj
	);
	
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	
	//临时批采查询表记录
	sprintf(szTemp, "Ne%s%s", 
			DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"), 
			DemandStrInXmlExt(pstruXml, "<omc>/<序列号>"));
	sprintf(szSql,
	    "insert into SM_BATPICKTMP(BP_ID, BP_NECOMID, BP_RELATIVEBEGINTIME, BP_OBJECT) values("
	    "%d,  '%s',  to_date( '%s','yyyy-mm-dd hh24:mi:ss'), '%s') ",
		nBatPickId,
		szTemp,
		szBPBeginTime, 
		szBPObj
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

int CheckBatPickTmp(PXMLSTRU pstruXml)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
    	
	STR szTemp[100];
	int nCount;
	
	sprintf(szTemp, "Ne%s%s", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"), DemandStrInXmlExt(pstruXml, "<omc>/<通信包标识>"));
	
	sprintf(szSql, "select count(*) as cnt from SM_BATPICKTMP where BP_NECOMID = '%s'", szTemp);//批采开始时间
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());		
		FreeCursor(&struCursor);	    				  
	    return EXCEPTION;

	}
	nCount = atoi(GetTableFieldValue(&struCursor, "cnt"));
	FreeCursor(&struCursor);
	
	return nCount;
}



RESULT SaveBatPickDat(PXMLSTRU pstruXml, char *pszValue, int nLen)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
	
	//取批采开始时间
	int nId;
	STR szBPBeginTime[20];
	STR szTemp[100];
	sprintf(szTemp, "Ne%s%s", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"), DemandStrInXmlExt(pstruXml, "<omc>/<通信包标识>"));
	
	sprintf(szSql, "select BP_ID, to_char(BP_RELATIVEBEGINTIME, 'yyyymmddhh24miss') as bp_rbt from SM_BATPICKTMP where BP_NECOMID = '%s'", szTemp);//批采开始时间
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
		InsertInXmlExt(pstruXml,"<omc>/<批采查询ID>", "0", MODE_AUTOGROW|MODE_UNIQUENAME);
		
		FreeCursor(&struCursor);	    				  
	    return EXCEPTION;

	}
	nId = atoi(GetTableFieldValue(&struCursor, "BP_ID"));
	sprintf(szTemp, "%d", nId);
	InsertInXmlExt(pstruXml,"<omc>/<批采查询ID>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	strcpy(szBPBeginTime, GetTableFieldValue(&struCursor, "bp_rbt"));
	FreeCursor(&struCursor);
	
	//保存批采数据
	int i, j;
	STR szBatPickValue[2000];
	STR szMapId[10];
	STR szDataType[10];
	int nDataLen;
	char szOC[100];
	char szMapData[100];
	int nBatPickCnt = 0;
	STR szBPTimePoint[20];
	
	strcpy(szMapId, DemandStrInXmlExt(pstruXml, "<omc>/<批采对象>"));
//	strcpy(szBatPickValue, DemandStrInXmlExt(pstruXml, "<omc>/<批采数值>"));
	memset(szBatPickValue, 0, sizeof(szBatPickValue));
	memcpy(szBatPickValue, pszValue, nLen);	

	if (GetMapIdFromCache(szMapId, szDataType, &nDataLen) != NORMAL)
	{
	        PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", szMapId);
		    return EXCEPTION;
	}

	if (nDataLen != 0)
	{	
		nBatPickCnt = nLen/nDataLen;
	}

	for (i = 0, j = 0; i < nBatPickCnt; i ++)
	{
		memset(szOC, 0, sizeof(szOC));
		memcpy(szOC, szBatPickValue+j, nDataLen);
		j += nDataLen;
		DecodeMapDataFromMapId(szMapId, szOC, szMapData, szTemp);
		
		GetBatPickTimePoint(szBPBeginTime, i*2, szBPTimePoint);

		sprintf(szSql,
		    " insert into SM_BATPICKDAT(BP_ID, BP_NEID, BP_OBJECT, BP_PICKUPTIME, BP_PICKUPVALUE) values("
		    " %d, %s, '%s', to_date('%s', 'yyyymmddhh24miss'),  '%s')",
			nId,
			DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruXml, "<omc>/<批采对象>"),
			MakeSTimeFromITime((INT)time(NULL)-i*2),
			//szBPTimePoint,
			szMapData
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
	
	//更新查询时间
	sprintf(szSql,
	    " update SM_BATPICKLOG set BP_UPDATETIME = sysdate"
	    " where BP_ID = %d",  
	    atoi(DemandStrInXmlExt(pstruXml, "<omc>/<批采查询ID>"))
	);
	
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	
	CommitTransaction();	
		
	
	//记录下次收包批采开始时间
	GetBatPickTimePoint(szBPBeginTime, nBatPickCnt*2, szBPTimePoint);
	InsertInXmlExt(pstruXml,"<omc>/<批采相对开始时间>", szBPTimePoint, MODE_AUTOGROW|MODE_UNIQUENAME);
	
	return NORMAL;	
	
}


RESULT UpdateBatPickTmp(PXMLSTRU pstruXml)
{
	char szSql[MAX_BUFFER_LEN];
	char szTemp[100];
	
	sprintf(szTemp, "Ne%s%s", 
			DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"), 
			DemandStrInXmlExt(pstruXml, "<omc>/<序列号>"));

	sprintf(szSql,
	    "update SM_BATPICKTMP set BP_NECOMID = '%s', BP_RELATIVEBEGINTIME = to_date('%s', 'yyyy-mm-dd hh24:mi:ss')"
	    " where BP_ID = %d",  
	    szTemp,
	    //DemandStrInXmlExt(pstruXml, "<omc>/<批采相对开始时间>"),
	    GetSysDateTime(),
	    atoi(DemandStrInXmlExt(pstruXml, "<omc>/<批采查询ID>"))
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


RESULT DeleteBatPickTmp(PXMLSTRU pstruXml)
{
    char szSql[MAX_BUFFER_LEN];
    STR szTemp[100];
	
	sprintf(szTemp, "Ne%s%s", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"), DemandStrInXmlExt(pstruXml, "<omc>/<通信包标识>"));

	sprintf(szSql, "delete from SM_BATPICKTMP where BP_NECOMID = '%s'", szTemp);
	
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}		
	
	CommitTransaction();
	
	return NORMAL;
}


RESULT CheckShiXi(PXMLSTRU pstruXml)
{
	CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	
	int nNeId;
	
	int nQryHis;
	int nBPId;
	
	nNeId = atoi(DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));//网元
		
	//查询历史重复记录
	sprintf(szSql, "select SX_ID from SM_SXLOG where SX_NEID = %d and SX_MAPID = '%s'", nNeId, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象>"));
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
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
	    return EXCEPTION;
	}
	nBPId = atoi(GetTableFieldValue(&struCursor, "SX_ID"));
	FreeCursor(&struCursor);
	
	if (nBPId > 0 )
	{
				
		sprintf(szSql,"delete from SM_SXLOG where SX_ID = %d",  nBPId);
	
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
	        return EXCEPTION;
		}
		
		sprintf(szSql,"delete from SM_SXTMP where SX_ID = %d",  nBPId);
	
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
	        return EXCEPTION;
		}
		
		sprintf(szSql,"delete from SM_SXDAT where SX_ID = %d",  nBPId);
	
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


RESULT SaveShiXiLog(PXMLSTRU pstruXml)
{
	STR szSql[MAX_SQL_LEN];
	UINT nBatPickId;
	char szTemp[50];
	char szBPObj[5];
	
	strcpy(szBPObj, DemandStrInXmlExt(pstruXml, "<omc>/<监控对象内容>"));
	
	//记录批采主参数
	GetDbSerial(&nBatPickId, "sm_ShiXiQry");
	
	sprintf(szSql,
	    "insert into SM_SXLOG(SX_ID, SX_NEID, SX_MAPID, sx_setvalue) values("
	    "%d,  %s, '%s', %d) ",
		nBatPickId,
		DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"),
		DemandStrInXmlExt(pstruXml, "<omc>/<监控对象>"),
		atoi(szBPObj)
	);
	
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	
	//临时批采查询表记录
	sprintf(szTemp, "Ne%s%s", 
			DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"), 
			DemandStrInXmlExt(pstruXml, "<omc>/<序列号>"));
	sprintf(szSql,
	    "insert into SM_SXTMP(SX_ID, SX_NECOMID) values("
	    "%d,  '%s') ",
		nBatPickId,
		szTemp
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


int CheckShiXiPack(PXMLSTRU pstruXml)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
    	
	STR szTemp[100];
	int nCount;
	
	sprintf(szTemp, "Ne%s%s", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"), DemandStrInXmlExt(pstruXml, "<omc>/<通信包标识>"));
	
	sprintf(szSql, "select count(*) as cnt from SM_SXTMP where SX_NECOMID = '%s'", szTemp);//批采开始时间
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());		
		FreeCursor(&struCursor);	    				  
	    return EXCEPTION;

	}
	nCount = atoi(GetTableFieldValue(&struCursor, "cnt"));
	FreeCursor(&struCursor);
	
	return nCount;
}

RESULT SaveShiXiDat(PXMLSTRU pstruXml, char *pszValue, int nLen)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
    char szDateTime[20];
	
	//取时间
	int nId;
	STR szTemp[100];
	sprintf(szTemp, "Ne%s%s", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"), DemandStrInXmlExt(pstruXml, "<omc>/<通信包标识>"));
	
	sprintf(szSql, "select SX_ID from SM_SXTMP where SX_NECOMID = '%s'", szTemp);//开始时间
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());
		FreeCursor(&struCursor);	    				  
	    return EXCEPTION;

	}
	nId = atoi(GetTableFieldValue(&struCursor, "SX_ID"));
	FreeCursor(&struCursor);

	//strcpy(szDateTime, GetSysDateTime()); 
	sprintf(szDateTime, "%s%s", GetSystemDate(), GetSystemTime()); 
	
	//保存批采数据
	int i, j;
	STR szBatPickValue[1000];
	int nDataLen;
	unsigned char uchOC;
	int nBatPickCnt = 0;
	STR szBPTimePoint[20];
	
	memset(szBatPickValue, 0, sizeof(szBatPickValue));
	memcpy(szBatPickValue, pszValue, nLen);	

	nDataLen = 1;
	nBatPickCnt = nLen/nDataLen;

	for (i = 0, j = nLen-1; i < nBatPickCnt; i ++, j--)
	{
		uchOC = *(szBatPickValue+j);
		
		sprintf(szTemp, "%d", uchOC);
		
		GetBatPickTimePoint(szDateTime, i*15*60, szBPTimePoint);

		sprintf(szSql,
		    " insert into SM_SXDAT(SX_ID, SX_NEID, SX_MAPID, SX_TIME, SX_VALUE) values("
		    " %d, %d, '%s', to_date('%s', 'yyyymmddhh24miss'),  '%s')",
			nId,
			atoi(DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>")),
			DemandStrInXmlExt(pstruXml, "<omc>/<监控对象>"),
			//szBPTimePoint,
			MakeSTimeFromITime((INT)time(NULL)-i*15*60),
			szTemp
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
	
	//更新查询时间
	sprintf(szSql,
	    " update SM_SXLOG set SX_UPDATETIME = to_date('%s', 'yyyy-mm-dd hh24:mi:ss')"
	    " where SX_ID = %d",  
	    szDateTime, nId
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

RESULT DeleteShiXiTmp(PXMLSTRU pstruXml)
{
    char szSql[MAX_BUFFER_LEN];
    STR szTemp[100];
	
	sprintf(szTemp, "Ne%s%s", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"), DemandStrInXmlExt(pstruXml, "<omc>/<通信包标识>"));

	sprintf(szSql, "delete from SM_SXTMP where SX_NECOMID = '%s'", szTemp);
	
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}		
	
	CommitTransaction();
	
	return NORMAL;
}


RESULT GetBatPickMap8(int nNeId, PSTR pszMapObject)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
    char szTemp[100];
	
	sprintf(szSql, "SELECT epm_curvalue FROM ne_elementparam WHERE epm_neid = %d AND epm_objid = '00000603'", nNeId);
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());		
		FreeCursor(&struCursor);	    				  
	    return EXCEPTION;

	}
	strcpy(szTemp, GetTableFieldValue(&struCursor, "epm_curvalue"));
	
	FreeCursor(&struCursor);
	
	sprintf(pszMapObject, "%08x", atoi(szTemp));
	return NORMAL;
}

RESULT GetBatPickMap4(int nNeId, PSTR pszMapObject)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
    char szTemp[100];
	
	sprintf(szSql, "SELECT epm_curvalue FROM ne_elementparam WHERE epm_neid = %d AND epm_objid = '0603'", nNeId);
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	    				  szSql, GetSQLErrorMessage());		
		FreeCursor(&struCursor);	    				  
	    return EXCEPTION;

	}
	strcpy(szTemp, GetTableFieldValue(&struCursor, "epm_curvalue"));
	
	FreeCursor(&struCursor);
	
	sprintf(pszMapObject, "%04x", atoi(szTemp));
	return NORMAL;
}


RESULT UpdateEleQryLogErrorFromBatPick(PSTR pszQryNumber, PSTR pszErrMapId)
{
    STR szSql[MAX_SQL_LEN];
    CURSORSTRU struCursor;
	STR szErrorMemo[200], szFailContent[MAX_BUFFER_LEN];
	INT nSeperateNum, i, nErrorId;
	TBINDVARSTRU struBindVar;
    
    memset(szFailContent, 0, sizeof(szFailContent));
 	sprintf(szFailContent, "%s:没有批采数据,",  pszErrMapId);
 	
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[0].VarValue.szValueChar, szFailContent);
	struBindVar.nVarCount++;
	
	struBindVar.struBind[1].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[1].VarValue.szValueChar, pszQryNumber);
	struBindVar.nVarCount++;
		
	memset(szSql, 0, sizeof(szSql));
    sprintf(szSql," update man_eleqrylog set qry_Content='', qry_FailContent=:v_0 where qry_Number= :v_1");
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

