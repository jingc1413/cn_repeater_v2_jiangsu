/*
 * 名称: 定时服务器头文件
 * 
 * 修改记录:
 * 2008-11-18    付志刚   - 建立
 */
 
#ifndef	__NORTHSEND_H__
#define	__NORTHSEND_H__

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>

#define	MAX_TIMESVR_NUM	100
#define	SLEEP_TIMESVR_TIME	30

#define	TASKRUNING_STATE	0   //任务运行
#define	NORMAL_STATE	2       //任务正常
#define	PAUSE_STATE		3       //任务挂起，暂停

/*
 * 告警代码定义
 */
#define ALM_MOS_ID  		162     //语音MOS值低告警
#define ALM_FTPDOWN_ID		163     //FTP下载速率低告警
#define ALM_MMS_ID			164     //彩信端到端发送成功率低告警
#define ALM_FTPCELL_ID		165     //FTP下载时小区重选频繁告警


typedef struct tagYIYANGSTRU
{
	STR szAlarmLogId[20+1];
	STR szNeId[20+1];
	STR szNeType[100+1];           //         网元类型
	STR szNeName[200+1];           //         网元名称
	STR szNeVendor[100+1];         //       网元厂家

	STR szAlarmId[100+1];          //   网管告警ID，网管系统简称_告警ID活动告警和清除告警ID为同一个：由我公司自行产生唯一ID号即可
	STR szAlarmTitle[200+1];       //     告警标题：即告警名称
	STR szAlarmCreateTime[19+1];  //   告警产生时间	格式为：年-月-日 小时:分:秒，年为4位数，小时为24小时制：格式YYYY-MM-DD HH:MM:SS
	STR szAlarmClearTime[19+1];    //告警清除时间
	STR szAlarmLevel[20+1];       //     告警级别
	STR szAlarmType[30+1] ;       //      告警类型：无线告警
	STR szAlarmObjId[9+1];         //
	STR szStandardAlarmName[200+1]; //标准告警名
	STR szProbableCauseTxt[200+1] ; //可能原因
	
	STR szAlarmLocation[100+1];    //  告警定位：
		
	STR szAlarmStatusId[1+1];      //    告警状态，0：网元自动清除 1：自动清除；2：手工清除；
	
	
	STR szSiteId[20+1];				//基站小区号
	STR szSiteName[100+1];			//基站名称
	
	STR szSystemName[19+1] ;      //   系统名称，各系统简称，参见附录A，本字段填写：“覆盖延伸系统”
	STR szSystemVendor[19+1];     //   系统开发厂家：“三维通信”
	STR szSystemLevel[20+1];	   //   系统等级
	STR szSystemNo[20+1];			// 系统编号
	
	STR szAlarmRegion[100];
	STR szAlarmCounty[100];
	
	STR szExtendInfo[100+1] ;      //     扩展信息，用于扩展
}YIYANGSTRU;


#endif
