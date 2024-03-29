/*
 * 名称: GPRS采集服务器
 *
 * 修改记录:
 * 付志刚 -		2008-12-31 创建
 */
#include <ebdgdl.h>
#include <ebdgml.h>
#include <omcpublic.h>
#include <applserver.h>
#include <mobile2g.h>

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
	fprintf(stderr, "覆盖延伸系统(GPRS采集服务程序)版本号V8.0.0\n");
	fprintf(stderr, "%s start 启动程序\n"
					"%s stop 停止程序\n"
					"%s -h 显示本信息\n", pszProg, pszProg, pszProg);
}


/* 
 * 处理GPRS上报交易 
 */
RESULT ProcessGprsDeliveTrans(INT nSock, PSTR pszCaReqBuffer, INT nLen)
{
	DecodeAndProcessGprs(pszCaReqBuffer, nLen);
    
	return NORMAL;
}


/* 
 * GPRS服务请求处理 
 */
RESULT GprsReqWork(INT nSock, PVOID pArg)
{
	STR szCaReqBuffer[MAX_BUFFER_LEN];		/* 渠道请求通讯报文 */
	INT nRecvLen;	
	
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
    }
	while(TRUE)
	{
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
	    
	    ProcessGprsDeliveTrans(nSock, szCaReqBuffer, nRecvLen);
		
		
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
	//fprintf(stderr, "\tWelcome to use the network management system(gprsserver)\n");
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
	
    if (GetCfgItem("gprsserv.cfg","GPRSSERV","ListenPort",szTemp) != NORMAL)
        	return EXCEPTION;
    	nGprsPort = atoi(szTemp);
	
	memset(&struPoolSvr,0,sizeof(struPoolSvr));
	//设备服务端口
	struPoolSvr.nListenPort[0]=nGprsPort;
	struPoolSvr.nListenPort[1]=-1;
	
	SetTcpPoolMinNum(2);
	if (GetCfgItem("gprsserv.cfg","GPRSSERV","MaxProcesses",szTemp) != NORMAL)
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
