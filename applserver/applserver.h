/*
 * 名称: 应用服务器头文件
 *
 */

#ifndef __APPLSERVER_H__
#define __APPLSERVER_H__

#include <cgprotcl.h>
#include <mobile2g.h>

typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned char	uchar;
typedef char	int8;				/* Signed integer >= 8	bits */
typedef short	int16;				/* Signed integer >= 16 bits */
typedef unsigned char	uint8;		/* Short for unsigned integer >= 8  bits */
typedef unsigned short	uint16;		/* Short for unsigned integer >= 16 bits */
typedef int		int32;
typedef unsigned int	uint32;		/* Short for unsigned integer >= 32 bits */
typedef unsigned long ulong;

STR	szGprsRespBuffer[100];		/* gprs应答通讯报文 */
				
#define MAX_MAPOBJECT_NUM			1200*10
#define MAX_MAPCGOBJECT_NUM			1200*7
#define MAX_MAPDASOBJECT_NUM		1200*10
#define MAX_MAPSNMPOBJECT_NUM		1200*4

/*
 * 限制宏定义
 */
#define MAX_PKGFIELD_NUM			200					/* 报文最大字段数 */
#define MAX_FIELDVALUE_LEN			256					/* 报文字段值最大长度 */

/*
 * 交易报文代码定义
 */
#define BS_QUERYSETTRANS		"6000"				    /* bs查询设置交易 */
#define BS_SETTRANS		        "6001"					/* bs设置交易 */
#define BS_TRUNTASK		        "6002"					/* 轮训触发 */
#define TIMERETRUNTASK          "6003"                                      /* 复训触发 */
#define DELIVERTRANS			"7000"					/* 主动上报交易 */
#define SMSSTATSREPORT			"7001"					/* 状态报告交易 */
#define GPRSDELIVE			    "8000"					/* GPRS上报交易 */
#define GPRSREQTRANS			"8001"					/* GPRS查询设置请求交易 */
#define GPRSRESQQRYSET			"8002"					/* GPRS查询设置应答交易 */
#define GPRSOFFLINE				"8003"					/* GPRS脱机交易 */


//定义监控对象长度
#define MAX_OBJCONTEXT_LEN  4000
#define MAX_OBJECT_NUM		1200

/*
 * 告警代码定义
 */
#define ALM_DT_ID  		159     //单通告警id号
#define ALM_DH_ID		160     //掉话告警id号
#define ALM_QHPF_ID		161     //cqt切换频繁告警id号
#define ALM_FTPCX_ID	165     //FTP下载时小区重选频繁告警
#define ALM_DLSB_ID		166	    //GPRS登录失败
#define ALM_SJSB_ID		167	    //GPRS数据上报失败


//定义通信方式

//应答标志编码定义
#define M2G_SUCCESS			 0x00			//成功
#define M2G_FEW_SUCCESS		 0x01		//命令被有条件执行
#define M2G_COMMANDID_ERR		 0x02			//命令编号错
#define M2G_LEN_ERR			 0x03			//监控对象长度错
#define M2G_CRC_ERR			 0x04			//CRC校验错
#define M2G_OTHER_ERR			 0xFE			//其它错误
#define M2G_COMMAND			 0xFF			//命令
#define M2G_ENCODE_ERR		 0xCC			//打包错误
#define M2G_DECODE_ERR		 0xCD			//解包错误

#define M2G_MONITOROBJLEN_SIZE	 0x01			//监控对象标号长度
#define M2G_MONITOROBJLEN_SIZE2	 0x02			//监控对象标号长度
#define M2G_MONITOROBJID_SIZE		 0x02			//监控对象标号长度

/////////////////////////////////////////////////////////////
///命令标示//////////////////////////////////////////////////

//主动上报报文过长时XML BUG
#define MAX_CONTENT_LEN  255
//定时上报发现网元告警对象串过长覆盖neid BUG
#define MAX_ALARMOBJ_LEN  4000

#pragma pack(1)
//协议头
typedef struct _COMMANDHEAD
{
    int  nCommandLen;          //协议长度
	int  nProtocolType;        //协议类型 2G 3G GSM CDMA 控制命令 
	int  nCommandCode;	       //命令码或者命令类型
    char QA[28];			   //流水号
	int  nObjectCount;         //对象数
	char ProtocolEdition[4];   //协议版本号 
	int  nRiority;             //命令优先级 0=代表普通命令 1=代表轮训命令	
}COMMANDHEAD;


typedef struct _REPEATER_INFO//信息结构
{
    int nCommType;			 //通讯方式
    unsigned int nRepeaterId; //直放站编号
	int nDeviceId;           //设备编号
    char szTelephoneNum[30]; //电话号码
	char szIP[30];			 //IP
	int  nPort;				 //端口号
    char szNetCenter[30];    //服务号码

	int  nDeviceTypeId;         //设备类型
	int  nProtocolDeviceType; //协议设备类型
	int  nConnStatus;  //连接状态
	char szRouteAddr[20];
	char szAddrInfo[51];
	
	char szAlarmObjList[MAX_ALARMOBJ_LEN];   //告警对象
	char szAlarmEnableList[2048];   //告警使能对象
    char szSpecialCode[51];
	char szReserve[51];       //保留
}REPEATER_INFO;

typedef struct _MAPOBJECT
{
    int	 nMapLen;		//监控对象长度
    char cErrorId;       //错误id号
	char szMapId[8+1];		//监控对象 对应表的ID
	char szMapType[20];		//类型
	char szMapData[MAX_OBJCONTEXT_LEN];	//监控对象内容
}MAPOBJECT;

//发送包结构
typedef struct _SENDPACKAGE
{
	COMMANDHEAD struHead;
    REPEATER_INFO struRepeater;
    int nNeId;
    int nAreaId;
    int nTaskId;
    int nTaskLogId;
    int nMaintainLogId;
    int nMsgLogId;
    char szRecvMsg[200];
	MAPOBJECT struMapObjList[MAX_OBJECT_NUM];
}SENDPACKAGE;


typedef enum _PROTOCOLTYPE  //协议类型
{
    PROTOCOL_2G= 1,			//2G协议类型
	PROTOCOL_3G,			//3G协议类型
	PROTOCOL_GSM,			//GSM 协议类型
	PROTOCOL_CDMA,			//CDMA 协议类型
	PROTOCOL_CONTROL,		//控制命令协议类型   

	PROTOCOL_HEIBEI,
	PROTOCOL_XC_CP ,
	PROTOCOL_YINY_CP ,
	PROTOCOL_ZJYDGSM,
	PROTOCOL_XINLITONG,
	
	PROTOCOL_SYD ,          //三元达

	PROTOCOL_SUNWAVE,       //三维
	PROTOCOL_WLK,           //威力克
	PROTOCOL_WUYOU,         //武邮 14
	//add by qgl at 2006-10-26
	PROTOCOL_JINXIN_R9110AC_II2,	//15
	PROTOCOL_JINXIN_RA1000A_LDII2,	//16
	PROTOCOL_JINXIN_1000AW_LDII2,   //17
	PROTOCOL_JINXIN_1000A_LW,        //18
	PROTOCOL_JINXIN_R9122AC,		//19
	PROTOCOL_JINXIN_R9122AC_II2,	//20
	PROTOCOL_JINXIN_BPA9010,		//21
	PROTOCOL_JINXIN_R9110AC,		//22
	PROTOCOL_JINXIN_R9110AS,		//23
	PROTOCOL_JINXIN_S9180,			//24
	PROTOCOL_JINXIN_TPA9010,		//25
	PROTOCOL_JINXIN_TPA9020A,		//26	//修改TPA9020分成A\B两协议
	PROTOCOL_JINXIN_TPA9020B,		//27   //modify by qgl at 2006-11-06
	PROTOCOL_JINXIN_RS2110BC1C2,//28 RS-2110B-C1C2,室内宽带直放站 modify by cqf at 2007-12-29 for 宁夏
    PROTOCOL_JINXIN_M4000BC1C2,//29 M-4000B-C1C2,GSM干线放大器 modify by cqf at 2008-1-8 for 宁夏 

	PROTOCOL_AOWEI=40,				//奥维  add by qgl at 2006-09-13
	PROTOCOL_HEIBEI2=41,				//河北协议支持8位短信 add by qgl at 2006-09-18
	PROTOCOL_WUYOU1=42,				//武邮1.0协议,支持8位短信 add by qgl at 2006-10-17	
	PROTOCOL_XiNZHONG=43,            //鑫众协议,跟三维协议相同 add by cqf at 2008-01-03
	PROTOCOL_FUYOU=44,                //福邮 modify by cqf at 2008-1-8 for 宁夏
	PROTOCOL_SNMP=45,
	PROTOCOL_DAS=46,
	PROTOCOL_JINXIN_DAS=47 
}PROTOCOLTYPE; 

typedef enum _PROTOCOL_2G_COMMAND //2G命令编码
{	
    COMMAND_QUERY=1,              //查询
	COMMAND_SET=2,                //设置
	COMMAND_UP = 3,			      //上报
	COMMAND_QUERY_MAPLIST = 9,    //查询监控量列表
	COMMAND_FCTPRM_QRY = 0x11,     //原工厂参数查询 178  改为 切换监控软件版本 0x11 2014/1/8 modify
	COMMAND_FACTORY_MODE = 176,   //进入工厂模式
	COMMAND_FCTPRM_SET = 179,     //工厂参数设置   add  at 2008-12-15 for factory mode
    COMMAND_PRJPRM_QRY = 180,     //工程参数查询   add  at 2008-12-15 for factory mode
	COMMAND_PRJPRM_SET = 181      //工程参数设置   add  at 2008-12-15 for factory mode
}PROTOCOL_2G_COMMAND;

typedef enum _2G_UP_TYPE //2G上报类型
{	
    DEVICE_ALARM=1,              //1:设备告警
	OPENSTATION=2,               //2：开站
	PERSONPATROL = 3,			 //3：巡检
	DEVICEREPAIR = 4,           //4：设备修复
	DEVICECHANGED = 5,           //5：设备配置变化
	DEVICEMONITOR = 0x20,        //效果监控
	DEVICESTARTUP = 0xCA         //202:效果监控请求上报
}UP_TYPE;


typedef enum _REMORT_UPDATE_STATUS //远程升级状态
{
	UNSENDED_STATUS=0,  //短信未发送
	SUCCESS_STATUS=1,   //升级成功
	FAILURE_STATUS=2,   //升级失败
	SENDSET_STATUS=3,   //短信已下发
    SENDED_STATUS = 5   //短信已回复下发成功
}REMORT_UPDATE_STATUS;


typedef enum _COMMTYPE //通讯方式
{
	M2G_RS232_TYPE=1,	//1
	M2G_DATA_TYPE,		//2
	M2G_SMS_TYPE,		//3
	M2G_TCPIP_TYPE,		//4
	M2G_GPRS_TYPE,		//5
	M2G_UDP_TYPE,		//6
	M2G_SNMP_TYPE		//7
}COMMTYPE;

#pragma pack()


//////////////////////////////////////////////

typedef struct _ALARMPARAMETER
{
    int nNeId;		
	unsigned int nRepeaterId;		
	short nDeviceId;
	char szNeTelNum[20];
	char szAlarmObjId[10];
	char szAlarmTime[20];
	BOOL bIsNewAlarm ;//true为发生告警，false告警取消
	int nAlarmLogId;
	char szAlarmId[10];
	char szAlarmLevelId[10];
	char szAlarmName[50];  //告警名
	char szAlarmLevelName[50];
	char szNeName[50];  //网元名称
	char szAlarmObjList[4000];  //告警对象列表
	char szAlarmInfo[1000];    //告警信息

	int nAlarmAutoConfirmSuccess;  //告警自动确认状态 1：成功
	int nAlarmAutoClearSuccess ;  // 告警自动清除状态 1：成功
}ALARMPARAMETER;


typedef struct
{
	STR szMapId[9];
	STR szDataType[10];
	STR szObjType[20];
	STR szObjOid[50];
	int nDataLen;
}MAPOBJECTSTRU;

typedef struct
{
	STR szMapId[5];
	STR szDataType[10];
	int nDataLen;
	int nProtclType;
}MAPOBJECTSTRU_CG;

typedef struct
{
	STR szMapId[5];
	STR szDataType[10];
	STR szObjType[20];
	STR szObjOid[50];
	int nDataLen;
	int nDeviceType;
}MAPOBJECTSTRU_SNMP;

typedef struct
{
	STR szMapId[9];
	STR szDataType[10];
	STR szObjType[20];
	STR szObjOid[50];
	int nDataLen;
}MAPOBJECTSTRU_DAS;

typedef struct
{
	PSTR pszName;
	UINT nLen;
}DIVCFGSTRU;
typedef DIVCFGSTRU * PDIVCFGSTRU;

typedef struct
{
	PSTR pszName;
	PSTR pszType;
	UINT nLen;
}FIXLENCFGSTRU;
typedef FIXLENCFGSTRU * PFIXLENCFGSTRU;



/*
 * 函数定义
 */
//appl_process
int RecvCaReqPacket(int nSock, char *pszCaReqBuffer, char *TradeNo);
int RecvCa8801ReqPacket(int sockfd, char *psCaReqBuffer, char *psTradeNo);
int SendCaRespPacket(int nSock, char *pszCaRespBuffer, int nCommBufferLen);
int SendCa8801RespPacket(int nSock, char *pszCaRespBuffer, int nCommBufferLen);
RESULT DecodeAndProcessSms(PSTR pszUndecode,PSTR pszTelephone,PSTR pszNetCenterNum);
RESULT DecodeAndProcessGprs(PSTR pszUndecode, INT nLen);

//appl_qryandset
RESULT DecodeQryElementParam(SENDPACKAGE *pstruSendPackage);
RESULT DecodeQryOnTime(SENDPACKAGE *pstruSendPackage, BOOL bGprsTime, BOOL bBatchQuery);
RESULT DecodeSetElementParam(SENDPACKAGE *pstruSendPackage);
RESULT DecodeQueryMapList(SENDPACKAGE *pstruSendPackage);
RESULT QryElementParam(INT nCommType, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater, PXMLSTRU pstruXml);
RESULT SetElementParam(INT nCommType, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater, PXMLSTRU pstruXml);
RESULT QueryMapList(INT nCommType, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater, PXMLSTRU pstruXml);
RESULT SaveSysErrorLog(INT nNeId, PSTR pszErrMapId,PSTR pszErrorId, PSTR pszRecvMsg);
RESULT UpdateEleFromCommByPacket(PSTR pszQryNumber, PSTR pszContent, PSTR pszAlarmName, PSTR pszAlarmValue);
RESULT UpdateEleSetLogFromComm(SENDPACKAGE *pstruSendPackage, PSTR pszQryNumber, PSTR pszProperty, PSTR pszContent);
RESULT UpdateEleFromCommBySetPacket(PSTR pszQryNumber, PSTR pszProperty);
RESULT UpdateEleQryLogFromComm(SENDPACKAGE *pstruSendPackage, PSTR pszProperty, PSTR pszContent);
RESULT UpdateEleQryLogOnTime(SENDPACKAGE *pstruSendPackage, PSTR pszProperty, PSTR pszContent);
RESULT UpdateEleObjList(SENDPACKAGE *pstruSendPackage,  PSTR pszObjectList, PSTR pszProvinceId, INT nUpdateWay);
RESULT QryEleFristTime(int nNeId, int nCommType);
RESULT SaveEleSetLog(PXMLSTRU pstruReqXml);
RESULT InsertFailNeid(int nTaskLogId, int nNeId, PSTR pszReason, PSTR pszEleState);
PSTR GetDeviceStatu(int nDeviceStatus);
RESULT UpdateTaskLogCount(int nTaskLogId, int nTaskId, int nEleCount, int nTxPackCount, int nFailEleCount);
BOOL getTaskStopUsing(int nTaskId);
BOOL ExistNeId(unsigned int nRepeaterId, int nDeviceId);
RESULT InsertSpecialCommandLog(PXMLSTRU pstruReqXml);
RESULT RemortUpdateDbOperate1(PXMLSTRU pstruReqXml);
RESULT RemortUpdateDbOperate2(PSTR pszQA);
RESULT RemortUpdateDbOperate3(PXMLSTRU pstruReqXml);
RESULT SetAllElementParam(int nNeId);
RESULT UpdateEleLastTime(int nNeId);

//appl_gprs
RESULT DecodeGprsPesq(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT DecodeGprsCqt(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);

RESULT DecodeFtpDownLoad(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT DecodeFtpUpLoad(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT DecodeGprsPing(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT DecodeGprsPdp(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT DecodeGprsAttach(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT DecodeMms(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT DecodeDayTestResult(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT DecodeWap(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT DecodeVp(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveGprsPesqLog(PXMLSTRU pstruReqXml);
RESULT SaveGprsCqtLog(PXMLSTRU pstruReqXml);
RESULT SaveFtpDownLoadLog(PXMLSTRU pstruReqXml);
RESULT SaveFtpUpLoadLog(PXMLSTRU pstruReqXml);
RESULT SaveGprsPingLog(PXMLSTRU pstruReqXml);
RESULT SaveGprsPdpLog(PXMLSTRU pstruReqXml);
RESULT SaveGprsAttachLog(PXMLSTRU pstruReqXml);
RESULT SaveGprsTestLog(PXMLSTRU pstruReqXml);
RESULT SaveMmsTestLog(PXMLSTRU pstruReqXml);
RESULT SaveDayTestResultLog(PXMLSTRU pstruReqXml);
RESULT SaveTimerUploadLog(INT nNeId, PXMLSTRU pstruReqXml);
RESULT SaveWapTestLog(PXMLSTRU pstruReqXml);
RESULT SaveVpTestLog(PXMLSTRU pstruReqXml);

RESULT ImSaveFtpDownLoadLog(PXMLSTRU pstruReqXml);
RESULT ImSaveFtpUpLoadLog(PXMLSTRU pstruReqXml);
RESULT ImSaveGprsAttachLog(PXMLSTRU pstruReqXml);
RESULT ImSaveGprsPdpLog(PXMLSTRU pstruReqXml);
RESULT ImSaveGprsPingLog(PXMLSTRU pstruReqXml);
RESULT ImSaveWapTestLog(PXMLSTRU pstruReqXml);

RESULT InitBatPick(PXMLSTRU pstruXml);// 批采
RESULT SaveBatPickLog(PXMLSTRU pstruXml);// 批采
int CheckBatPickTmp(PXMLSTRU pstruXml);// 批采
RESULT SaveBatPickDat(PXMLSTRU pstruXml, char *, int);// 批采
RESULT UpdateBatPickTmp(PXMLSTRU pstruXml);// 批采
RESULT DeleteBatPickTmp(PXMLSTRU pstruXml);// 批采

RESULT CheckShiXi(PXMLSTRU pstruXml);
RESULT SaveShiXiLog(PXMLSTRU pstruXml);
int CheckShiXiPack(PXMLSTRU pstruXml);
RESULT SaveShiXiDat(PXMLSTRU pstruXml, char *pszValue, int nLen);
RESULT DeleteShiXiTmp(PXMLSTRU pstruXml);

//appl_td
RESULT DecodeTdPesq(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveTdPesqLog(PXMLSTRU pstruReqXml);
RESULT DecodeTdCqt(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveTdCqtLog(PXMLSTRU pstruReqXml);
RESULT DecodeTdDayTestResult(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveTdDayTestResultLog(PXMLSTRU pstruReqXml);
RESULT DecodeTdMms(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveTdMmsTestLog(PXMLSTRU pstruReqXml);

RESULT DecodeTdFtpDownLoad(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveTdFtpDownLoadLog(PXMLSTRU pstruReqXml);
RESULT DecodeTdFtpUpLoad(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveTdFtpUpLoadLog(PXMLSTRU pstruReqXml);
RESULT DecodeTdPing(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveTdPingTestLog(PXMLSTRU pstruReqXml);
RESULT DecodeTdPdp(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveTdPdpTestLog(PXMLSTRU pstruReqXml);
RESULT DecodeTdAttach(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveTdAttachTestLog(PXMLSTRU pstruReqXml);

RESULT DecodeTdWap(PSTR pszMapData, INT nDataLen, PXMLSTRU pstruXml);
RESULT SaveTdWapTestLog(PXMLSTRU pstruReqXml);

RESULT SaveTdPsTestLog(PXMLSTRU pstruReqXml);
RESULT SaveTdTimerUploadLog(INT nNeId, PXMLSTRU pstruReqXml);


//appl_alarm
RESULT InitAlarmPara(int nNeId);
RESULT DealNewAlarm(PXMLSTRU pstruXml);
RESULT AlarmComeback(PXMLSTRU pstruXml);
RESULT AlarmFrequent(int nMaintainLogId, PSTR pszCondition);
RESULT TransferAlarm();
/*
 * appl_util
 */
PSTR TrimRightOneChar(PSTR pszInputString,CHAR cChar);
int ReplaceAlarmObjStr(char *sSrc, char *sMatchStr, char *sReplaceStr);
int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr);
char *ReplaceCharByPos(char *str, char rchar, int pos);
time_t MakeITimeFromLastTime(PSTR pszLastTime);
int GetCurrent2GSquenue();
PSTR GetTypeNumber(PSTR pszType);
PSTR Get2GNumber(PSTR pszType, INT n2G_QB);
RESULT GetDbSerial(PUINT pnSerial,PCSTR pszItemName);
RESULT GetDbSequence(PUINT pnSerial,PCSTR pszItemName);
RESULT GetMySqlSequence(PUINT pnSerial,PCSTR pszTableName);
int GetProtocolFrom(PSTR pszMobile);
PSTR GetAlarmName(PSTR pszMapId);
RESULT GetSendPackageInfo(int QB, unsigned int RptId, SENDPACKAGE *pstruSendInfo);
RESULT SaveToMsgQueue(PXMLSTRU pstruReqXml);
RESULT SaveToMsgQueue_Tmp(PXMLSTRU pstruReqXml);
RESULT SaveToGprsQueue(PXMLSTRU pstruReqXml);
RESULT SaveEleQryLog(PXMLSTRU pstruReqXml);
RESULT SaveToMaintainLog(PSTR pszStyle, PSTR pszMemo, SENDPACKAGE *pstruSendInfo);
RESULT SaveToMaintainLog2(PSTR pszStyle, INT nNeId, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater);
RESULT GetPackInfoFromMainLog(SENDPACKAGE *pstruSendInfo);
RESULT GetGprsPackageInfo(int QB, unsigned int RptId, SENDPACKAGE *pstruSendInfo);
RESULT GetSysParameter(PCSTR pszArgSql,PSTR pszArgValue);
PSTR GetAlarmObjList(int nNeId);
PSTR GetAlarmEnabledObjList(int nNeId);
RESULT GetAlarmObjList2(unsigned int nRepeaterId, int nDeviceId, PSTR pszNeTelNum, int *pNeId, PSTR pszNeName, PSTR pszAlarmObjList);
RESULT GetAlarmObjList3(unsigned int nRepeaterId, int nDeviceId, int *pNeId, PSTR pszNeName, PSTR pszAlarmObjList);
RESULT GetNeObjectList(INT nNeId, PSTR pszNeObjectList, PSTR pszNeDataList);
RESULT GetNeCmdObjects(int nProtocolTypeId,int nProtocolDeviceTypeId, PSTR pszCmdCode, PSTR pszCmdObject);
RESULT ResolveQryParamArray(PSTR pszQryEleParam);
RESULT ResolveQryParamArrayGprs(PSTR pszQryEleParam);
RESULT ResolveSetParamArray(PSTR pszSetEleParam, PSTR pszSetEleValue);
RESULT ResolveSetParamArrayGprs(PSTR pszSetEleParam, PSTR pszSetEleValue);
RESULT InsertTaskLog(int nTaskId, int *pnTaskLogId, PSTR pszStyle);
RESULT SetTaskUsing(int nTaskId);
INT GetNeId(unsigned int nRepeaterId, int nDeviceId, PSTR pszNeTelNum, BOOL *pIsNewNeId);
RESULT GetNeInfo(SENDPACKAGE *pstruNeInfo);
int strHexToInt(char* strSource);
RESULT InitMapObjectCache(VOID);
RESULT InitMapObjectCache_CG(VOID);
RESULT GetMapIdFromCache(PSTR pszMapId,PSTR pszDataType,PINT pDataLen);
RESULT GetMapIdFromCache2(PSTR pszMapId,PSTR pszDataType,PSTR pszObjType, PINT pDataLen);
RESULT GetMapIdFromCache_CG(int, PSTR pszMapId,PSTR pszDataType,PINT pDataLen);
RESULT DecodeMapDataFromType(PSTR pszDataType,INT nDataLen, PSTR pszOC, PSTR pszMapData);
RESULT DecodeMapDataFromMapId(PSTR pszMapId, PSTR pszOC, PSTR pszMapData, PSTR pszMapType);
RESULT EncodeMapDataFromMapId(PSTR pszMapId, PSTR pszMapData,  PBYTE pszOC);
RESULT SaveToMapList(PSTR pszQrySerial, PSTR pszMapId);
RESULT GetMapId0009List(PSTR pszQrySerial, PSTR pszMapId0009List);
RESULT DistServerTelNum(PXMLSTRU pstruReqXml);
RESULT RecordAgentRecNum();
BOOL getAgentState(PSTR pszAgentNo);
RESULT SaveToRecordDeliveCrc(unsigned int nRepeaterId, INT nDeviceId, INT nNetFlag, INT nType);
RESULT CqtMathchJob(int nNeId, PSTR pszTelNum, PSTR pszCallNum, PSTR pszBeCallNum);
RESULT CqtMathchJob2(int nNeId, PSTR pszTelNum, PSTR pszBeCallNum);
BOOL ExistAlarmLog(int nAlarmId, int nNeId, PINT pnAlarmCount);
RESULT InsertMosTask(PXMLSTRU pstruXml);
RESULT SaveToAlarmLog(int nAlarmId, int nNeId, int nAlarmCount);
RESULT GetDeviceIp(unsigned int nRepeaterId, int nDeviceId, PSTR pszDeviceIp, int *pPort);
BOOL AscEsc(BYTEARRAY *Pack);
BOOL AscUnEsc(BYTEARRAY *Pack);
RESULT ByteSplit(int, BYTEARRAY *Pack, char *pszObjList);
RESULT ByteCombine(int, BYTEARRAY *Pack, char *pszCmdObjList);
int EncodeCmdBodyFromCmdId(int, const char *pszObjList, const OMCOBJECT *pstruOmcObj, int nOmcObjNum,
		UBYTE *pubCmdBody, int nCmdBodyMaxLen, UBYTE *pubCmdBodyLen);
int DecodeCmdBodyFromCmdId(int, char *pszCmdObjList, CMDHEAD *pCmdHead, 
		UBYTE *pubCmdBody, OMCOBJECT *pstruOmcObj, int *pnOmcObjNum);
			
//appl_redis
RESULT InitRedisMQ_cfg();
RESULT ConnectRedis();
RESULT FreeRedisConn();
RESULT GetRedisPackageInfo(int nQB, SENDPACKAGE *pstruSendInfo, PXMLSTRU  pstruXml);
RESULT GetRedisDeviceIpInfo(unsigned int nRepeaterId, int nDeviceId,PSTR pszDeviceIp, int *pPort);
RESULT PublishToMQ(PXMLSTRU  pstruXml, int nReqFlag);
RESULT PublishToRedis(PXMLSTRU  pstruXml, int nReqFlag);
RESULT RedisHsetAndPush(PXMLSTRU  pstruXml);
RESULT HmsetEleParam(PSTR pszHmsetParam);
RESULT PushElementParamQueue(int nTaskLogId, PSTR pszEleParamSQL);
RESULT PushElementSQL(PSTR pszEleSQL);
RESULT PushEffectControl(int nTaskLogId, PSTR pszEffSQL);
int RedisPingInterval(int timeout);
RESULT GetEleRespQueue(PSTR pszRespQueue);
RESULT HmsetPoolTaskLog(int nTaskLogId, int nFailTimes, int nStyle);

#endif
