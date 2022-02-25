/*
 * 名称: 定时服务器头文件
 * 
 * 修改记录:
 * 2008-11-18    付志刚   - 建立
 */
 
#ifndef	__TIMESERVER_H__
#define	__TIMESERVER_H__

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
    BOOL IsNewAlarm;              // 新告警标志
	int NeId; 

	STR szSystemName[19+1] ;      //   系统名称，各系统简称，参见附录A，本字段填写：“覆盖延伸系统”
	STR szSystemVendor[19+1];     //   系统开发厂家：“三维通信”
	STR szAlarmId[19+1];          //        网管告警ID，网管系统简称_告警ID活动告警和清除告警ID为同一个：由我公司自行产生唯一ID号即可

	STR szOriAlarmId[10+1];       //     原始告警号：告警名称相对应的编号
	STR szAlarmTitle[100+1];       //     告警标题：即告警名称
	STR szAlarmCreateTime[19+1];  //   告警产生时间	格式为：年-月-日 小时:分:秒，年为4位数，小时为24小时制：格式YYYY-MM-DD HH:MM:SS
	STR szNeType[100+1];           //         网元类型
	STR szNeName[200+1];           //         网元名称
	STR szNeVendor[10+1];         //       网元厂家
	STR szAlarmLevel[10+1];       //     告警级别
	STR szAlarmType[10+1] ;       //      告警类型：无线告警
	STR szAlarmRedefLevel[10+1] ; //  重定义告警级别：空白
	STR szAlarmRedefType[10+1];   //  重定义告警类型：空白
	STR szAlarmLocation[20+1];    //  告警定位：站点编号,设备编号（以逗号分隔，站点编号为8位16进制数，设备编号为2位16进制数）
	STR szAlarmDeviceId[10+1];    // 设备子编号
	STR szAlarmTelnum[20+1];      //监控电话号码
	STR szAlarmDetail[400+1] ;     //    告警描述：空白
	STR szAlarmRegion[100+1];      //    告警地区地市：地级市（注：三个字，如杭州市、丽水市…）
	STR szSystemLevel[20+1];	   //   系统等级
	STR szSystemNo[20+1];			// 系统编号
	STR szSystemTitle[100+1];		// 系统名称标题
	STR szSiteId[20+1];				//基站小区号
	STR szSiteName[100+1];			//基站名称
	STR szCellId[20+1];				//小区号
	STR szLac[20+1];				//LAC
	STR szExtendInfo[400+1] ;      //     扩展信息，用于扩展。要求以"字段名称:值"的方式提供，多个字段间用回车换行分隔	:空白

	STR szAlarmStatus[1+1];      //    告警状态，1：自动清除；2：手工清除；
	STR szStatusTime[19+1];       //     状态改变时间，格式为：年-月-日 小时:分:秒，年为4位数，小时为24小时制，同上
}YIYANGSTRU;


#endif
