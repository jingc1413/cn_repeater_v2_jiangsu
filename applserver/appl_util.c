/*
 * 名称: 应用服务公共函数
 *
 * 修改记录:
 * 2008-9-12 - 付志刚 建立
 */

#include <ebdgdl.h>
#include <omcpublic.h>
#include <mobile2g.h>
#include "applserver.h"

static int nFailTime=0;
static int nMaxMapIdNum;
static int nSequenceNO=1;
static int nQrySetNumber=1;
static MAPOBJECTSTRU struMapObjectList[MAX_MAPOBJECT_NUM];
static MAPOBJECTSTRU_DAS struMapObjectList_Das[MAX_MAPDASOBJECT_NUM];
static MAPOBJECTSTRU_CG struMapObjectList_CG[MAX_MAPCGOBJECT_NUM];
static MAPOBJECTSTRU_SNMP struMapObjectList_SNMP[MAX_MAPSNMPOBJECT_NUM];

extern STR szService[MAX_STR_LEN];
extern STR szDbName[MAX_STR_LEN];
extern STR szUser[MAX_STR_LEN];
extern STR szPwd[MAX_STR_LEN];
	


int strHexToInt(char* strSource) 
{ 
    int nTemp=0; 
    int i,j,len,flen; 

    len = strlen(strSource); 
    flen = --len; 
    for(i = 0; i <= len; i++) 
    { 
        if(strSource[i] > 'g' || strSource[i] < '0' || ( strSource[i] > '9' && strSource[i] < 'A' ) ) 
        { 
            PrintErrorLog(DBG_HERE,"请输入正确的16进制字符串!输入错误 [%s]\n", strSource);
            return -1; 
        } 
        else 
        { 
            int nDecNum; 
            switch(strSource[i]) 
            { 
                case 'a': 
                case 'A': nDecNum = 10; break; 
                case 'b': 
                case 'B': nDecNum = 11; break; 
                case 'c': 
                case 'C': nDecNum = 12; break; 
                case 'd': 
                case 'D': nDecNum = 13; break; 
                case 'e': 
                case 'E': nDecNum = 14; break; 
                case 'f': 
                case 'F': nDecNum = 15; break; 
                case '0': 
                case '1': 
                case '2': 
                case '3': 
                case '4': 
                case '5': 
                case '6': 
                case '7': 
                case '8': 
                case '9': nDecNum = strSource[i] - '0'; break; 
                default: return 0; 
            } 
        
            for(j = flen; j > 0; j-- ) 
            { 
                nDecNum *= 16; 
            } 
            flen--; 
            nTemp += nDecNum; 
        
        } 
    } 
    return nTemp;
}

int IpstrToInt(const char *ip)
{
	int   result = 0;
	int   tmp = 0;
	int   shift = 24;
	const char *pEnd = ip;
	const char *pStart = ip;
	while(*pEnd != '\0')
	{
		while(*pEnd != '.' && *pEnd != '\0')pEnd++;
		tmp = 0;
		while(pStart < pEnd)
		{
			tmp = tmp * 10 + (*pStart - '0');
			pStart++;
		}
		result += (tmp << shift);
		shift -= 8;
		if (*pEnd == '\0')break;
		pStart = pEnd + 1;
		pEnd++;
	}
	return result;
}

char * IntToIpstr (const int ip, char *buf)
{
	sprintf (buf, "%u.%u.%u.%u",(uchar) * ((char *) &ip + 0),(uchar) * ((char *) &ip + 1),
								(uchar) * ((char *) &ip + 2), (uchar) * ((char *) &ip + 3));
	return buf;
} 

/**--------------------
 * @brief 十进制转二进制
 *
 * @param bstr 二进制字符串，不大与32位
 *
 * @return  出错返回-1，成功返回十进制数
 --------------------*/
int bin2dec(char *bstr)
{
	int d = 0;
	unsigned int len = strlen(bstr);

	if (len > 32)
		return -1;  //数位过长
	len--;

	int i = 0;
	for (i = 0; i <= len; i++)
	{
		d += (bstr[i] - '0') * (1 << (len - i));
	}

	return d;
}


/**--------------------
 * @brief 十进制转二进制
 *
 * @param d 十进制数，输入
 * @param bstr 二进制字符串，输出
 *
 * @return 出错返回-1，成功返回字符串长度
 --------------------*/
int dec2bin(int d, char *bstr)
{
	if (d < 0)
		return -1;

	int mod = 0;
	char tmpstr[64];
	bzero(tmpstr, sizeof(tmpstr));
	bzero(bstr, sizeof(bstr));

	int i = 0;
	while (d > 0)
	{
		mod = d % 2;
		d /= 2;
		tmpstr[i] = mod + '0';
		i++;
	}

	//复制字符串
	unsigned int len = strlen(tmpstr);
	for (i = 0; i < len; i++)
	{
		bstr[i] = tmpstr[len - i - 1];
	}

	return (int)len;
}

/**
 * TrimRightOneChar
 * 除去后面的字符(会修改源串)
 *
 * pszInputString 要转变的字符串
 *
 * Returns 转换后的字符串
 */
PSTR TrimRightOneChar(PSTR pszInputString,CHAR cChar)
{
	INT i;

	if(pszInputString==NULL)
		return NULL;

	for(i=strlen(pszInputString)-1;(i>=0)&&(pszInputString[i]==cChar);i--)
	{
		pszInputString[i]=0;
		break;
	}

	return pszInputString;
}


int ReplaceAlarmObjStr(char *sSrc, char *sMatchStr, char *sReplaceStr)
{
        int  StringLen, nMatchLen;
        char sNewString[4000];

        char *FindPos = strstr(sSrc, sMatchStr);
        if( (!FindPos) || (!sMatchStr) )
                return -1;
		nMatchLen = strlen(sMatchStr);
        if( FindPos != NULL && (strcmp(sReplaceStr, "0") == 0 || strcmp(sReplaceStr, "1") == 0))//找到并且取代位置数据合法
        {
                memset(sNewString, 0, sizeof(sNewString));
                StringLen = FindPos - sSrc + nMatchLen + 1;
                strncpy(sNewString, sSrc, StringLen);
                strcat(sNewString, sReplaceStr);
                strcat(sNewString, FindPos + nMatchLen + 2);
                strcpy(sSrc, sNewString);
        }

        return 0;
}

//替换字符串中某个位置的字符
char *ReplaceCharByPos(char *str, char rchar, int pos)
{
    char *p = str+pos-1;
    if (*p)
    {
        *p = rchar;
    }
    return str;
}

time_t MakeITimeFromLastTime(PSTR pszLastTime)
{
	time_t nTime;
	struct tm struTmNow;

	STR szTemp[5];

	if(strlen(pszLastTime)!=19)
		return((time_t)(-1));	

	memset(szTemp,0,sizeof(szTemp));
	memcpy(szTemp,pszLastTime,4);
	struTmNow.tm_year=atoi(szTemp)-1900;

	memset(szTemp,0,sizeof(szTemp));
	memcpy(szTemp,pszLastTime+5,2);
	struTmNow.tm_mon=atoi(szTemp)-1;

	memset(szTemp,0,sizeof(szTemp));
	memcpy(szTemp,pszLastTime+8,2);
	struTmNow.tm_mday=atoi(szTemp);

	memset(szTemp,0,sizeof(szTemp));
	memcpy(szTemp,pszLastTime+11,2);
	struTmNow.tm_hour=atoi(szTemp);

	memset(szTemp,0,sizeof(szTemp));
	memcpy(szTemp,pszLastTime+14,2);
	struTmNow.tm_min=atoi(szTemp);

	memset(szTemp,0,sizeof(szTemp));
	memcpy(szTemp,pszLastTime+17,2);
	struTmNow.tm_sec=atoi(szTemp);

	nTime=mktime(&struTmNow);

	return nTime;
}


RESULT InitMapObjectCache(VOID)
{
	STR szSql[MAX_BUFFER_LEN];
	CURSORSTRU struCursor;
	INT i;

	bufclr(struMapObjectList);
	sprintf(szSql,"select obj_objid,obj_datatype,obj_activetype,obj_oid, obj_datalen from ne_objectslist where ISNULL(obj_objid)=0");
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息为[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	for(i=0;(FetchCursor(&struCursor)==NORMAL)&&(i<MAX_MAPOBJECT_NUM);i++)
	{
		strcpy(struMapObjectList[i].szMapId,(GetTableFieldValue(&struCursor,"obj_objid")));
		strcpy(struMapObjectList[i].szDataType,(GetTableFieldValue(&struCursor,"obj_datatype")));
		strcpy(struMapObjectList[i].szObjType,(GetTableFieldValue(&struCursor,"obj_activetype")));
		//strcpy(struMapObjectList[i].szObjOid,TrimAllSpace(GetTableFieldValue(&struCursor,"obj_oid")));
		struMapObjectList[i].nDataLen = atoi(GetTableFieldValue(&struCursor,"obj_datalen"));
	}
	FreeCursor(&struCursor);

	if(i>=MAX_MAPOBJECT_NUM)
	{
		PrintErrorLog(DBG_HERE,"mapobject Cache太小[%d]\n",MAX_MAPOBJECT_NUM);
		return EXCEPTION;
	}
	nMaxMapIdNum = i;

	return NORMAL;
}

RESULT InitMapObjectCache_Das(VOID)
{
	STR szSql[MAX_BUFFER_LEN];
	CURSORSTRU struCursor;
	INT i;

	bufclr(struMapObjectList_Das);
	sprintf(szSql,"select obj_objid,obj_datatype,obj_activetype,obj_oid, obj_datalen from ne_objectslist_das");
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息为[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	for(i=0;(FetchCursor(&struCursor)==NORMAL)&&(i<MAX_MAPOBJECT_NUM);i++)
	{
		strcpy(struMapObjectList_Das[i].szMapId,(GetTableFieldValue(&struCursor,"obj_objid")));
		strcpy(struMapObjectList_Das[i].szDataType,(GetTableFieldValue(&struCursor,"obj_datatype")));
		strcpy(struMapObjectList_Das[i].szObjType,(GetTableFieldValue(&struCursor,"obj_activetype")));
		strcpy(struMapObjectList_Das[i].szObjOid,(GetTableFieldValue(&struCursor,"obj_oid")));
		struMapObjectList_Das[i].nDataLen = atoi(GetTableFieldValue(&struCursor,"obj_datalen"));
	}
	FreeCursor(&struCursor);
	if(i>=MAX_MAPOBJECT_NUM)
	{
		PrintErrorLog(DBG_HERE,"mapobject Cache太小[%d]\n",MAX_MAPOBJECT_NUM);
		return EXCEPTION;
	}

	return NORMAL;
}


RESULT InitMapObjectCache_CG(VOID)
{
	STR szSql[MAX_BUFFER_LEN];
	CURSORSTRU struCursor;
	INT i;

	bufclr(struMapObjectList_CG);
	sprintf(szSql,"select obj_objid,obj_datatype,obj_datalen,obj_protocoltypeid from ne_objectslist_cg");
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息为[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	for(i=0;(FetchCursor(&struCursor)==NORMAL)&&(i<MAX_MAPCGOBJECT_NUM);i++)
	{
		strcpy(struMapObjectList_CG[i].szMapId,(GetTableFieldValue(&struCursor,"obj_objid")));
		strcpy(struMapObjectList_CG[i].szDataType,(GetTableFieldValue(&struCursor,"obj_datatype")));
		struMapObjectList_CG[i].nDataLen = atoi(GetTableFieldValue(&struCursor,"obj_datalen"));
		struMapObjectList_CG[i].nProtclType = atoi(GetTableFieldValue(&struCursor,"obj_protocoltypeid"));
	}
	FreeCursor(&struCursor);
	if(i>=MAX_MAPCGOBJECT_NUM)
	{
		PrintErrorLog(DBG_HERE,"mapobject Cache太小[%d]\n",MAX_MAPCGOBJECT_NUM);
		return EXCEPTION;
	}

	return NORMAL;
}

RESULT InitMapObjectCache_SNMP(VOID)
{
	STR szSql[MAX_BUFFER_LEN];
	CURSORSTRU struCursor;
	INT i;

	bufclr(struMapObjectList_SNMP);
	sprintf(szSql,"SELECT a.obj_objid,obj_datatype,obj_activetype, b.obj_oid, obj_datalen, obj_devicetype FROM ne_objectslist a, ne_objectsnmp b WHERE a.obj_objid = b.obj_objid");
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息为[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	for(i=0;(FetchCursor(&struCursor)==NORMAL)&&(i<MAX_MAPCGOBJECT_NUM);i++)
	{
		strcpy(struMapObjectList_SNMP[i].szMapId,(GetTableFieldValue(&struCursor,"obj_objid")));
		strcpy(struMapObjectList_SNMP[i].szDataType,(GetTableFieldValue(&struCursor,"obj_datatype")));
		strcpy(struMapObjectList_SNMP[i].szObjType,(GetTableFieldValue(&struCursor,"obj_activetype")));
		strcpy(struMapObjectList_SNMP[i].szObjOid,(GetTableFieldValue(&struCursor,"obj_oid")));
		struMapObjectList_SNMP[i].nDataLen = atoi(GetTableFieldValue(&struCursor,"obj_datalen"));
		struMapObjectList_SNMP[i].nDeviceType = atoi(GetTableFieldValue(&struCursor,"obj_devicetype"));
	}
	FreeCursor(&struCursor);
	if(i>=MAX_MAPSNMPOBJECT_NUM)
	{
		PrintErrorLog(DBG_HERE,"mapobject Cache太小[%d]\n",MAX_MAPSNMPOBJECT_NUM);
		return EXCEPTION;
	}

	return NORMAL;
}


RESULT GetMapIdFromCache(PSTR pszMapId,PSTR pszDataType,PINT pDataLen)
{
	INT i;
	
	for(i=0;(i<MAX_MAPOBJECT_NUM)&&(strlen(struMapObjectList[i].szMapId)>0);i++)
	{		
		if(strcmp(pszMapId,struMapObjectList[i].szMapId)==0)
		{
			strcpy(pszDataType,struMapObjectList[i].szDataType);
			*pDataLen = struMapObjectList[i].nDataLen;
			return NORMAL;
		}
	}
	strcpy(pszDataType,"");				//找不到
	*pDataLen = 0;

	return EXCEPTION;
}

RESULT GetMapIdFromCache2(PSTR pszMapId,PSTR pszDataType,PSTR pszObjType, PINT pDataLen)
{
	INT i;
	
	for(i=0;(i<nMaxMapIdNum)&&(strlen(struMapObjectList[i].szMapId)>0);i++)
	{		
		//if(strcmp(pszMapId,struMapObjectList[i].szMapId)==0)
		if((strlen(pszMapId) == strlen(struMapObjectList[i].szMapId)) && strncmp(pszMapId,struMapObjectList[i].szMapId, strlen(pszMapId))==0)
		{
			strcpy(pszDataType,struMapObjectList[i].szDataType);
			strcpy(pszObjType,struMapObjectList[i].szObjType);
			*pDataLen = struMapObjectList[i].nDataLen;
			return NORMAL;
		}
	}
	strcpy(pszDataType,"");				//找不到
	*pDataLen = 0;

	return EXCEPTION;
}

RESULT GetMapIdFromCache_Das(PSTR pszMapId,PSTR pszDataType,PINT pDataLen)
{
	INT i;
	
	for(i=0;(i<MAX_MAPDASOBJECT_NUM)&&(strlen(struMapObjectList_Das[i].szMapId)>0);i++)
	{		
		if(strcmp(pszMapId,struMapObjectList_Das[i].szMapId)==0)
		{
			strcpy(pszDataType,struMapObjectList_Das[i].szDataType);
			*pDataLen = struMapObjectList_Das[i].nDataLen;
			return NORMAL;
		}
	}
	strcpy(pszDataType,"");				//找不到
	*pDataLen = 0;

	return EXCEPTION;
}


RESULT GetObjOidFromCache_Snmp(PSTR pszMapId,int nDeviceTypeId, PSTR pszDataType,PSTR pszObjOid)
{
	INT i;
	
	for(i=0;(i<MAX_MAPSNMPOBJECT_NUM)&&(strlen(struMapObjectList_SNMP[i].szMapId)>0);i++)
	{		
		if(struMapObjectList_SNMP[i].nDeviceType==nDeviceTypeId &&
			strcmp(pszMapId,struMapObjectList_SNMP[i].szMapId)==0)
		{
			strcpy(pszDataType,struMapObjectList_SNMP[i].szDataType);
			strcpy(pszObjOid,struMapObjectList_SNMP[i].szObjOid);
			return NORMAL;
		}
	}
	strcpy(pszDataType,"");				//找不到
	strcpy(pszObjOid, "");	

	return EXCEPTION;
}

RESULT GetMapIdFromCache_Snmp(PSTR pszObjOid, PSTR pszMapId,PSTR pszObjType)
{
	INT i;
	
	for(i=0;(i<MAX_MAPSNMPOBJECT_NUM)&&(strlen(struMapObjectList_SNMP[i].szMapId)>0);i++)
	{		
		if(strcmp(pszObjOid, struMapObjectList_SNMP[i].szObjOid)==0)
		{
			strcpy(pszObjType, struMapObjectList_SNMP[i].szObjType);
			strcpy(pszMapId, struMapObjectList_SNMP[i].szMapId);
			return NORMAL;
		}
	}
	strcpy(pszObjType,"");				//找不到
	strcpy(pszMapId, "");	

	return EXCEPTION;
}


RESULT GetMapIdFromCache_CG(int nProtclType,PSTR pszMapId,PSTR pszDataType,PINT pDataLen)
{
	INT i;

	for(i=0;(i<MAX_MAPCGOBJECT_NUM)&&(strlen(struMapObjectList_CG[i].szMapId)>0);i++)
	{
		if(struMapObjectList_CG[i].nProtclType==nProtclType &&
			strcmp(pszMapId,struMapObjectList_CG[i].szMapId)==0)
		{
			strcpy(pszDataType,struMapObjectList_CG[i].szDataType);
			*pDataLen = struMapObjectList_CG[i].nDataLen;
			return NORMAL;
		}
	}
	strcpy(pszDataType,"");				//找不到
	*pDataLen = 0;

	return EXCEPTION;
}

int IsNumber(char *str)
{
	int i;
	for ( i = 0 ; i < strlen(str) ; i ++)
		if( !isdigit(str[i]))
			return -1;
	return 0;
}


RESULT DecodeMapDataFromMapId(PSTR pszMapId, PSTR pszOC, PSTR pszMapData, PSTR pszMapType)
{
    STR szDataType[20];
    INT nDataLen;
    STR szMapData[256+1];
    
    memset(szDataType, 0, sizeof(szDataType));
    if (GetMapIdFromCache2(pszMapId, szDataType, pszMapType, &nDataLen) != NORMAL)
    {
    	if (nFailTime ++ > 100) 
    	{
    		PrintDebugLog(DBG_HERE, "pid=%ld  exit\n", getpid());
    		exit(0);
    	}
        PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", pszMapId);
		return EXCEPTION;
    }
    nFailTime = 0;
    //PrintDebugLog(DBG_HERE,"MAPID=%s,DataType=%s,MapType=%s,nDataLen=%d\n",szMapId,szDataType, MapType, nDataLen);
    
    if (nDataLen >= 256) nDataLen = 255;
    
    memset(szMapData, 0, sizeof(szMapData));
    memcpy(szMapData, pszOC, nDataLen);
    	
    if (strncmp(szDataType, "uint1", 5) == 0 )
    {
         UCHAR cMapData;
         cMapData = szMapData[0];
         sprintf(pszMapData, "%u", cMapData);
    }
    else if (strncmp(szDataType, "sint1", 5) == 0)
    {
         sprintf(pszMapData, "%d", szMapData[0]);
    }
    else if (strncmp(szDataType, "uint2", 5) == 0 )
    {
         sprintf(pszMapData, "%u", ReadWORD(szMapData));
    }
    else if (strncmp(szDataType, "sint2", 5) == 0)
    {
         sprintf(pszMapData, "%d", ReadShort(szMapData));
    }
    else if (strncmp(szDataType, "uint4", 5) == 0)
    {
         sprintf(pszMapData, "%lu", ReadDWORD(szMapData));
    }
    else if (strncmp(szDataType, "sint4", 5) == 0)
    {
         sprintf(pszMapData, "%ld", ReadLong(szMapData));
    }
    else if (strncmp(szDataType, "DT", 2) == 0)
    {
         sprintf(pszMapData, "%02x%02x-%02x-%02x %02x:%02x:%02x", szMapData[0], szMapData[1], szMapData[2], szMapData[3], szMapData[4], szMapData[5],szMapData[6]);
    }
    else if (strncmp(szDataType, "DT8", 3) == 0)
    {
         sprintf(pszMapData, "%02x%02x-%02x-%02x %02x:%02x:%02x;%02x", szMapData[0], szMapData[1], szMapData[2], szMapData[3], szMapData[4], szMapData[5],szMapData[6],szMapData[7]);
    }
    else if (strncmp(szDataType, "TIME", 4) == 0)
    {
         sprintf(pszMapData, "%02x:%02x:%02x", szMapData[0], szMapData[1], szMapData[2]);
    }
    else if (strncmp(szDataType, "DT5", 3) == 0)
    {
         sprintf(pszMapData, "%02x:%02x;%02x:%02x;%02x", szMapData[0], szMapData[1], szMapData[2],szMapData[3], szMapData[4]);
    }
    else if (strncmp(szDataType, "IP4", 3) == 0)
    {
         UCHAR cMapData1, cMapData2, cMapData3, cMapData4;
         cMapData1 = szMapData[0];
         cMapData2 = szMapData[1];
         cMapData3 = szMapData[2];
         cMapData4 = szMapData[3];
         sprintf(pszMapData, "%u.%u.%u.%u", cMapData1, cMapData2, cMapData3, cMapData4);
         //sprintf(pszMapData, "%u.%u.%u.%u", szMapData[0], szMapData[1], szMapData[2], szMapData[3]);
    }
    else if (strcmp(szDataType, "IP16") == 0)
    {
         UCHAR cMapData1, cMapData2, cMapData3, cMapData4;
         cMapData1 = szMapData[0];
         cMapData2 = szMapData[1];
         cMapData3 = szMapData[2];
         cMapData4 = szMapData[3];
         sprintf(pszMapData, "%x.%x.%x.%x", cMapData1, cMapData2, cMapData3, cMapData4);
    }
    else if (strcmp(szDataType, "HEX") == 0)
    {
         UCHAR cMapData1, cMapData2, cMapData3, cMapData4;
         cMapData1 = szMapData[0];
         cMapData2 = szMapData[1];
         cMapData3 = szMapData[2];
         cMapData4 = szMapData[3];
         sprintf(pszMapData, "%02x%02x%02x%02x", cMapData1, cMapData2, cMapData3, cMapData4);
    }
    else if (strcmp(szDataType, "OBJ") == 0)//下发对象用 added by wwj at 20100622 (nDataLen = 2)
    {
    	unsigned char cObj1, cObj2;
    	cObj1 = szMapData[1];
    	cObj2 = szMapData[0];
		sprintf(pszMapData, "%04u", ReadWORD(szMapData));
    }
    else if (strcmp(szDataType, "OBJ4") == 0)
    {
		sprintf(pszMapData, "%08lu", ReadDWORD(szMapData));
    }
    else if (strcmp(szDataType, "BIN") == 0)//十进制转二进制字符串
    {
    	
    	dec2bin(ReadDWORD(szMapData), pszMapData);
    }
    else
    {
         memcpy(pszMapData, szMapData, nDataLen);
    }

    if (strstr(szDataType, "-") != NULL)
    {
    	memset(szMapData, 0, sizeof(szMapData));
        strncpy(szMapData, szDataType+6, strlen(szDataType)-6);
        
    	if (strlen(pszMapData) == 0)
            strcpy(pszMapData, "0");
        if (atoi(szMapData) > 0 && strlen(szMapData) <= 2)
        	sprintf(pszMapData, "%.1f", atof(pszMapData) / atoi(szMapData));
        else if (atoi(szMapData) > 0 && strlen(szMapData) == 3)
        	sprintf(pszMapData, "%.2f", atof(pszMapData) / atoi(szMapData));
        else if (atoi(szMapData) > 0 && strlen(szMapData) == 4)
        	sprintf(pszMapData, "%.3f", atof(pszMapData) / atoi(szMapData));
        else
        	sprintf(pszMapData, "%.2f", atof(pszMapData) / atoi(szMapData));
    }
    //以后数据库改变，要删除
    char szMapIdList[1000];
    strcpy(szMapIdList, "0450,046B,046b,0471,0477,047D,047d,0506,0515,051B,051b,0521,0527");
    if (strstr(szMapIdList, pszMapId) != NULL)
    {
        if (strlen(pszMapData) == 0)
            strcpy(pszMapData, "0");
        sprintf(pszMapData, "%.1f", atof(pszMapData) / 10);
    }
    
    return NORMAL;
}

RESULT DecodeMapDataFromOid(PSTR pszObjOid, PSTR pszMapId, PSTR pszMapData, PSTR pszMapType)
{
    PSTR pszSepStr[MAX_OBJECT_NUM];
    
    SeperateString(pszObjOid, '=', pszSepStr, MAX_SEPERATE_NUM);
    
    PrintDebugLog(DBG_HERE, "map[%s]data[%s]\n", pszSepStr[0], pszSepStr[1]);
    
    if (GetMapIdFromCache_Snmp(pszSepStr[0], pszMapId, pszMapType) != NORMAL)
    {
        PrintErrorLog(DBG_HERE, "系统不存在该[%s]Oid\n", pszSepStr[0]);
		return EXCEPTION;
    }
	
	strcpy(pszMapData, pszSepStr[1]);
	
    return NORMAL;
}

/*
 * 根据输入数据类型，长度，解析报文成字符串
 * 应用appl_gprs等数据包解析
 */
RESULT DecodeMapDataFromType(PSTR pszDataType, INT nDataLen, PSTR pszOC, PSTR pszMapData)
{
    STR szMapData[256+1];
    
    memset(szMapData, 0, sizeof(szMapData));
    memcpy(szMapData, pszOC, nDataLen);
    if (strcmp(pszDataType, "UINT1") == 0 )
    {
         UCHAR cMapData;
         cMapData = szMapData[0];
         sprintf(pszMapData, "%u", cMapData);
    }
    else if (strcmp(pszDataType, "SINT1") == 0)
    {
    	 CHAR csMapData;
         csMapData = szMapData[0];
         sprintf(pszMapData, "%d", csMapData);
    }
    else if (strcmp(pszDataType, "UINT2") == 0 )
    {
         sprintf(pszMapData, "%u", ReadWORD(szMapData));
    }
    else if (strcmp(pszDataType, "SINT2") == 0)
    {
         sprintf(pszMapData, "%d", ReadShort(szMapData));
    }
    else if (strcmp(pszDataType, "UINT4") == 0)
    {
         sprintf(pszMapData, "%lu", ReadDWORD(szMapData));
    }
    else if (strcmp(pszDataType, "SINT4") == 0)
    {
         sprintf(pszMapData, "%ld", ReadLong(szMapData));
    }
    else if (strcmp(pszDataType, "DT") == 0)
    {
         if (strlen(szMapData) == 0)
            strcpy(pszMapData, "1980-01-01 01:01:01");
         else
            sprintf(pszMapData, "%02x%02x-%02x-%02x %02x:%02x:%02x", szMapData[0], szMapData[1], szMapData[2], szMapData[3], szMapData[4], szMapData[5],szMapData[6]);
    }
    else if (strcmp(pszDataType, "TIME") == 0)
    {
        if (strlen(szMapData) == 0)
            strcpy(pszMapData, "01:01:01");
        else
            sprintf(pszMapData, "%02x:%02x:%02x", szMapData[0], szMapData[1], szMapData[2]);
    }
    else if (strcmp(pszDataType, "IP4") == 0)
    {
         UCHAR cMapData1, cMapData2, cMapData3, cMapData4;
         cMapData1 = szMapData[0];
         cMapData2 = szMapData[1];
         cMapData3 = szMapData[2];
         cMapData4 = szMapData[3];
         sprintf(pszMapData, "%u.%u.%u.%u", cMapData1, cMapData2, cMapData3, cMapData4);
    }
    else if (strcmp(pszDataType, "IP16") == 0)
    {
         UCHAR cMapData1, cMapData2, cMapData3, cMapData4;
         cMapData1 = szMapData[0];
         cMapData2 = szMapData[1];
         cMapData3 = szMapData[2];
         cMapData4 = szMapData[3];
         sprintf(pszMapData, "%x.%x.%x.%x", cMapData1, cMapData2, cMapData3, cMapData4);
    }
    else
    {
         memcpy(pszMapData, szMapData, nDataLen);
    }
    
    
    return NORMAL;
}


RESULT EncodeMapDataFromMapId(PSTR pszMapId, PSTR pszMapData,  PBYTE pszOC)
{
    PSTR pszSepStr[10];
    STR szMapData[256], szOC[256];
    STR szDataTypeList[100];
    STR szDataType[20], szTemp[10];
    INT nDataLen;
    int nMapData;
    uint16 nTempU16;
    int nTemp;
    short nTemp2;
    
    if (GetMapIdFromCache(pszMapId, szDataType, &nDataLen) != NORMAL)
    {
        PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", pszMapId);
		return EXCEPTION;
    }
    strcpy(szMapData, pszMapData);
    
    if (strstr(szDataType, "-") != NULL)
    {
    	memset(szTemp, 0, sizeof(szTemp));
        strncpy(szTemp, szDataType+6, strlen(szDataType)-6);
        
    	if (strlen(pszMapData) == 0) strcpy(szMapData, "0");
        if (atoi(szTemp) > 0)
        	sprintf(szMapData, "%.0f", atof(pszMapData) * atoi(szTemp));
    }
    char szMapIdList[1000];
    strcpy(szMapIdList, "0450,046B,046b,0471,0477,047D,047d,0506,0515,051B,051b,0521,0527");
    if (strstr(szMapIdList, pszMapId) != NULL)
    {
        if (strlen(pszMapData) == 0) strcpy(szMapData, "0");
        sprintf(szMapData, "%.0f", atof(pszMapData) * 10);
    }
    
    //检查最值范围
    strcpy(szDataTypeList, "uint1,uint2,sint1,sint2,uint4,sint4");
    if (strstr(szDataTypeList, szDataType) != NULL)
    {
        nMapData = atoi(pszMapData);
        if (strcmp(szDataType, "uint1") == 0)
        {
            if (nMapData < 0) strcpy(szMapData, "0");
            if (nMapData > 255) strcpy(szMapData, "255");
        }
        else if (strcmp(szDataType, "uint2") == 0)
        {
            if (nMapData < 0) strcpy(szMapData, "0");
            if (nMapData > 65535) strcpy(szMapData, "65535");
        }
        else if (strcmp(szDataType, "sint1") == 0)
        {
            if (nMapData < -127) strcpy(szMapData, "-127");
            if (nMapData > 127) strcpy(szMapData, "127");
        }
        else if (strcmp(szDataType, "sint2") == 0)
        {
            if (nMapData < -32767) strcpy(szMapData, "-32767");
            if (nMapData > 32767) strcpy(szMapData, "32767");
        }
    }
    //转换
    memset(szOC, 0, sizeof(szOC));
    if (strncmp(szDataType, "uint1", 5) == 0 || strncmp(szDataType, "sint1", 5) == 0)
    {
        szOC[0] = (int8)atoi(szMapData);
    }
    else if (strncmp(szDataType, "uint2", 5) == 0 || strncmp(szDataType, "sint2", 5) == 0)
    {
         nTempU16 = (int16)atoi(szMapData);
         szOC[0] = LOBYTE(nTempU16);
	     szOC[1] = HIBYTE(nTempU16);
         //storeInt2(pszOC, nTempU16);
    }
    else if (strncmp(szDataType, "uint4", 5) == 0 || strncmp(szDataType, "sint4", 5) == 0)
    {
        nTemp = atoi(szMapData);
        szOC[0]=LOBYTE(LOWORD(nTemp));
	    szOC[1]=HIBYTE(LOWORD(nTemp));
	    szOC[2]=LOBYTE(HIWORD(nTemp));
	    szOC[3]=HIBYTE(HIWORD(nTemp));
        // storeInt(pszOC, nTemp);
    }
    else if (strcmp(szDataType, "DT") == 0)
    {
         bufclr(szTemp);
         strncpy(szTemp, szMapData, 2);
         if (IsNumber(szTemp)==0) szOC[0] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+2, 2);
         if (IsNumber(szTemp)==0) szOC[1] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+5, 2);
         if (IsNumber(szTemp)==0) szOC[2] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+8, 2);
         if (IsNumber(szTemp)==0) szOC[3] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+11, 2);
         if (IsNumber(szTemp)==0) szOC[4] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+14, 2);
         if (IsNumber(szTemp)==0) szOC[5] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+17, 2);
         if (IsNumber(szTemp)==0) szOC[6] = strHexToInt(szTemp);
    }
    else if (strcmp(szDataType, "DT8") == 0)
    {
         bufclr(szTemp);
         strncpy(szTemp, szMapData, 2);
         if (IsNumber(szTemp)==0) szOC[0] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+2, 2);
         if (IsNumber(szTemp)==0) szOC[1] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+5, 2);
         if (IsNumber(szTemp)==0) szOC[2] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+8, 2);
         if (IsNumber(szTemp)==0) szOC[3] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+11, 2);
         if (IsNumber(szTemp)==0) szOC[4] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+14, 2);
         if (IsNumber(szTemp)==0) szOC[5] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+17, 2);
         if (IsNumber(szTemp)==0) szOC[6] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+20, 2);
         if (IsNumber(szTemp)==0) szOC[7] = strHexToInt(szTemp);
    }
    else if (strcmp(szDataType, "TIME") == 0)
    {
         bufclr(szTemp);
         strncpy(szTemp, szMapData, 2);
         if (IsNumber(szTemp)==0) szOC[0] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+3, 2);
         if (IsNumber(szTemp)==0) szOC[1] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+6, 2);
         if (IsNumber(szTemp)==0) szOC[2] = strHexToInt(szTemp);
    }
    else if (strcmp(szDataType, "DT5") == 0)
    {
         bufclr(szTemp);
         strncpy(szTemp, szMapData, 2);
         if (IsNumber(szTemp)==0) szOC[0] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+3, 2);
         if (IsNumber(szTemp)==0) szOC[1] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+6, 2);
         if (IsNumber(szTemp)==0) szOC[2] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+9, 2);
         if (IsNumber(szTemp)==0) szOC[3] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+12, 2);
         if (IsNumber(szTemp)==0) szOC[4] = strHexToInt(szTemp);
    }
    else if (strcmp(szDataType, "IP4") == 0)
    {
         nTemp = SeperateString(szMapData, '.', pszSepStr, 4);
         if (nTemp != 4)
         {   
             return 0;
         }
         if (IsNumber(pszSepStr[0])==0) szOC[0] = atoi(pszSepStr[0]);
         if (IsNumber(pszSepStr[1])==0) szOC[1] = atoi(pszSepStr[1]);
         if (IsNumber(pszSepStr[2])==0) szOC[2] = atoi(pszSepStr[2]);
         if (IsNumber(pszSepStr[3])==0) szOC[3] = atoi(pszSepStr[3]);
    }
    else if (strcmp(szDataType, "IP16") == 0)
    {
         nTemp = SeperateString(szMapData, '.', pszSepStr, 4);
         if (nTemp != 4)
         {   
             return 0;
         }
         if (IsNumber(pszSepStr[0])==0) szOC[0] = strHexToInt(pszSepStr[0]);
         if (IsNumber(pszSepStr[1])==0) szOC[1] = strHexToInt(pszSepStr[1]);
         if (IsNumber(pszSepStr[2])==0) szOC[2] = strHexToInt(pszSepStr[2]);
         if (IsNumber(pszSepStr[3])==0) szOC[3] = strHexToInt(pszSepStr[3]);
    }
    else if (strcmp(szDataType, "HEX") == 0)
    {
         bufclr(szTemp);
         strncpy(szTemp, szMapData, 2);
         if (IsNumber(szTemp)==0) szOC[0] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+2, 2);
         if (IsNumber(szTemp)==0) szOC[1] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+4, 2);
         if (IsNumber(szTemp)==0) szOC[2] = strHexToInt(szTemp);
         strncpy(szTemp, szMapData+6, 2);
         if (IsNumber(szTemp)==0) szOC[3] = strHexToInt(szTemp);
    }
    else if (strcmp(szDataType, "OBJ") == 0)//下发对象用 added by wwj at 20100622 (nDataLen = 2)
    {
		nTemp2 = (short)strHexToInt(szMapData);
		szOC[0] = LOBYTE(nTemp2);
		szOC[1] = HIBYTE(nTemp2);
    }
    else if (strcmp(szDataType, "OBJ4") == 0)
    {
		nTemp = strHexToInt(szMapData);
		szOC[0]=LOBYTE(LOWORD(nTemp));
	    szOC[1]=HIBYTE(LOWORD(nTemp));
	    szOC[2]=LOBYTE(HIWORD(nTemp));
	    szOC[3]=HIBYTE(HIWORD(nTemp));
    }
    else if (strcmp(szDataType, "BIN") == 0)//二进制转十进制
    {
    	if (IsNumber(szMapData)==0){
		nTemp = bin2dec(szMapData);
        szOC[0]=LOBYTE(LOWORD(nTemp));
	    szOC[1]=HIBYTE(LOWORD(nTemp));
	    szOC[2]=LOBYTE(HIWORD(nTemp));
	    szOC[3]=HIBYTE(HIWORD(nTemp));
		}
    }
    else
    {
        memcpy(szOC, szMapData, strlen(szMapData));
        //nDataLen = strlen(szMapData);
    }
    memcpy(pszOC, szOC, nDataLen);
    
    return nDataLen;
}

//ASCII码拆分处理(ASCII专程成HEX)
BOOL AscEsc(BYTEARRAY *Pack)
{
 	//对除起始标志和结束标志外的所有数据转义
	int len = (Pack->Len-2)*2+2;
	BYTE pdu[len+1];         //转义
	BYTE m_Pdu[Pack->Len+1]; //原文
    
    memset(m_Pdu, 0, Pack->Len);
    memcpy(m_Pdu, Pack->pPack, Pack->Len);
    
    memset(pdu, 0, len+1);
       
	BYTE Hivalue,Lovalue;
	int i, j;
	for(i=1, j=1; i<Pack->Len-1; i++)
	{
		Hivalue=m_Pdu[i]/16;
        Lovalue=m_Pdu[i]%16;
        Hivalue=Hivalue<10?Hivalue+48:Hivalue+55;
		Lovalue=Lovalue<10?Lovalue+48:Lovalue+55;
		pdu[j++]=Hivalue;
		pdu[j++]=Lovalue;
	}
	pdu[0] = 0x7E;
	pdu[j] = 0x7E;
	
	//清除递进来的数据
    memset(Pack->pPack, 0, len+1);
	Pack->Len = len;
	memcpy(Pack->pPack, pdu, len);
	//PrintDebugLog(DBG_HERE, "生成2G协议报文[%s]\n", Pack->pPack);
	return TRUE;
}

//反向ASCII码拆分处理
//Pack 待解析的数据
//m_Pdu 为反向后的结果
BOOL AscUnEsc(BYTEARRAY *Pack)
{
	int Hivalue,Lovalue,temp;
	int i, j;
	char m_Pdu[4096];
	
	bufclr(m_Pdu);
	m_Pdu[0]=0x7E;
	for(i=1,j=1; i<Pack->Len-1; i++)
	{
		temp=Pack->pPack[i];
		Hivalue=(temp>=48&&temp<=57)?temp-48:temp-55;
		temp=Pack->pPack[i+1];
	 	Lovalue=(temp>=48&&temp<=57)?temp-48:temp-55;
		m_Pdu[j] = (Hivalue*16+Lovalue);
		j++;
		i++;
	}
	m_Pdu[j] = 0x7E;
	j++;
	
	//清除递进来的数据
    memset(Pack->pPack, 0, j+1);
	Pack->Len = j;
	memcpy(Pack->pPack, m_Pdu, j);
	    
    
	return TRUE;
}

RESULT ByteSplit(int nProctlType, BYTEARRAY *Pack, char *pszObjList)
{
	UBYTE ubStrPack[200];
	UBYTE *ubPtr;
	UBYTE Hivalue,Lovalue;
	int i, j, k, t;
	char szObjList[MAX_OBJ_LEN];
	char *pszSeperateStr[MAX_SEPERATE_NUM];
	int nSepCount;
	char szDataType[20];
	int nDataLen;
	int nResvSize;
	int nBitIndct = 0;
	int res;
	
	//命令头
	ubPtr = Pack->pPack;
	for (i = 1, j = 1; i <= 12; i++, j++ )
	{
		Hivalue = ubPtr[i]/16;
		Lovalue = ubPtr[i]%16;
		Hivalue = Hivalue<0x0A?Hivalue+0x30:Hivalue+0x37;
		Lovalue = Lovalue<0x0A?Lovalue+0x30:Lovalue+0x37;
		ubStrPack[j++] = Hivalue;
		ubStrPack[j] = Lovalue;
	}
	
	//命令体
	if(pszObjList != NULL)
	{
		strcpy(szObjList, pszObjList);
		strcpy(szObjList, TrimAllSpace(szObjList));
		nSepCount = SeperateString(szObjList, ',',pszSeperateStr, MAX_SEPERATE_NUM);//pszSeperateStr的大小要和MAX_SEPERATE_NUM一致
		for (k=0; k<nSepCount; k++)
		{
			if (strncmp(pszSeperateStr[k], "bytresv", 7) == 0)
			{
				nResvSize = atoi(pszSeperateStr[k]+7);
				strcpy(szDataType, "bytresv");
				nDataLen = nResvSize;
			}
			else if (strncmp(pszSeperateStr[k], "bitresv", 7) == 0)
			{
				strcpy(szDataType, "bitresv");
				if ( ++nBitIndct % 8 == 0)
					nDataLen = 1;
				else
					nDataLen = 0;
			}	
			else
			{	
				res = GetMapIdFromCache_CG(nProctlType, pszSeperateStr[k], szDataType, &nDataLen);
				if (res == NORMAL)
		    	{
		    		if (strcmp(szDataType, "bit") == 0)
				    {
				        if ( ++nBitIndct % 8 == 0)
				        	nDataLen = 1;
				        else 
				        	nDataLen = 0;	
				    }
		    	}
		    	else
		    	{
		    		PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", pszSeperateStr[k]);
					return EXCEPTION;
		    	}
		    }
	    	
	    	
	    	
	    	if (strcmp(szDataType, "str") == 0)
	    	{
	    		memcpy(ubStrPack+j, ubPtr+i, nDataLen);
	    		i += nDataLen;
	    		j += nDataLen;
	    	}
	    	else
	    	{
	    		t = nDataLen;
	    		while(t-->0)
	    		{
					Hivalue = ubPtr[i]/16;
					Lovalue = ubPtr[i]%16;
					Hivalue = Hivalue<0x0A?Hivalue+0x30:Hivalue+0x37;
					Lovalue = Lovalue<0x0A?Lovalue+0x30:Lovalue+0x37;
					ubStrPack[j++] = Hivalue;
					ubStrPack[j] = Lovalue;
					i++, j++;
				}
	    	}
	    		
	    }	    
	}	
	
	//校验码
	k = 2;
	while(k-->0)
	{
		Hivalue = ubPtr[i]/16;
		Lovalue = ubPtr[i]%16;
		Hivalue = Hivalue<0x0A?Hivalue+0x30:Hivalue+0x37;
		Lovalue = Lovalue<0x0A?Lovalue+0x30:Lovalue+0x37;
		ubStrPack[j++] = Hivalue;
		ubStrPack[j] = Lovalue;
		i++, j++;
	}
	
	ubStrPack[0] = 0x58;
	ubStrPack[j] = 0x58;

	memcpy(Pack->pPack, ubStrPack, j+1);
	*(Pack->pPack+j+1) = 0;
	
	return NORMAL;
}

RESULT ByteCombine(int nProctlType, BYTEARRAY *Pack, char *pszCmdObjList)
{   
	UBYTE ubPack[200];
	UBYTE *ubPtr;
	int Hivalue,Lovalue;
	int i, j, k, t;
	char szObjList[MAX_OBJ_LEN];
	char *pszSeperateStr[MAX_SEPERATE_NUM];
	int nSepCount;
	char szDataType[20];
	int nDataLen;
	int nResvSize;
	int nBitIndct = 0;
	int res;
	
	//命令头
	ubPtr = Pack->pPack;
	for (i = 1, j = 1; i <= 12*2; i++, j++ )
	{		
		Hivalue=(ubPtr[i]>=48&&ubPtr[i]<=57)?ubPtr[i]-48:ubPtr[i]-55;
		i++;
	 	Lovalue=(ubPtr[i]>=48&&ubPtr[i]<=57)?ubPtr[i]-48:ubPtr[i]-55;
		ubPack[j] = (Hivalue*16+Lovalue);		
	}
	
	//命令体
	if(strlen(pszCmdObjList) > 0)
	{
		strcpy(szObjList, pszCmdObjList);
		strcpy(szObjList, TrimAllSpace(szObjList));
		nSepCount = SeperateString(szObjList, ',',pszSeperateStr, MAX_SEPERATE_NUM);

		for (k=0; k<nSepCount; k++)
		{		
			if (strncmp(pszSeperateStr[k], "bitresv", 7) == 0)//比特保留位只能为bitresv1形式
			{
				strcpy(szDataType, "bitresv");
				if ( ++nBitIndct % 8 == 0)
					nDataLen = 1;
				else
					nDataLen = 0;
			}	
			else if (strncmp(pszSeperateStr[k], "bytresv", 7) == 0)
			{
				nResvSize = atoi(pszSeperateStr[k]+7);
				strcpy(szDataType, "bytresv");
				nDataLen = nResvSize;
			}	
			else 
			{	
				res = GetMapIdFromCache_CG(nProctlType, pszSeperateStr[k], szDataType, &nDataLen);
				if (res == NORMAL)
		    	{
			        if (strcmp(szDataType, "bit") == 0)
			        {	
			        	if ( ++nBitIndct % 8 == 0)
			        		nDataLen = 1;
			        	else 
			        		nDataLen = 0;	
			        }
		    	}
		    	else
		    	{
		    		PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", pszSeperateStr[k]);
					return EXCEPTION;
		    	}
		    }
	    	////////
	    	if (strcmp(szDataType, "str") == 0)
	    	{
	    		memcpy(ubPack+j, ubPtr+i, nDataLen);
	    		i += nDataLen;
	    		j += nDataLen;
	    	}
	    	else
	    	{
	    		t = nDataLen;
	    		while (t-- >0)
	    		{
					Hivalue=(ubPtr[i]>=48&&ubPtr[i]<=57)?ubPtr[i]-48:ubPtr[i]-55;
					i++;
		 			Lovalue=(ubPtr[i]>=48&&ubPtr[i]<=57)?ubPtr[i]-48:ubPtr[i]-55;
					ubPack[j] = (Hivalue*16+Lovalue);	
					i++, j++;
				}
	    	}
	    }	    
	}	
	
	//校验码
	k = 2;
	while(k-->0)
	{
		Hivalue=(ubPtr[i]>=48&&ubPtr[i]<=57)?ubPtr[i]-48:ubPtr[i]-55;
		i++;
	 	Lovalue=(ubPtr[i]>=48&&ubPtr[i]<=57)?ubPtr[i]-48:ubPtr[i]-55;
	 	ubPack[j] = (Hivalue*16+Lovalue);
		i++, j++;
	}
	
	ubPack[0] = 0x58;
	ubPack[j] = 0x58;
	
	memset(Pack->pPack, 0, j+1);
	memcpy(Pack->pPack, ubPack, j+1);
	Pack->Len = j+1; 
        
	return NORMAL;	
}

/*
*函数介绍：
*输入参数：ObjId-pstruOmcObj-nOmcObjNum
*输出参数：ObjVal
*返回值：
*/
static int GetObjValByObjId(const char *pszMapId, const OMCOBJECT *pstruOmcObj, int nOmcObjNum, char *pszObjVal)
{
	int i;
	int res = -1;

	for(i = 0; i < nOmcObjNum; i ++)
	{
		if (strcmp(pszMapId, pstruOmcObj[i].szObjId) == 0)
		{	
			strcpy(pszObjVal, pstruOmcObj[i].szObjVal);
			res = 0;
			break;
		}	
	}	
	return res;
}

int EncodeCmdBodyFromCmdId(int nProtclType, const char *pszObjList, const OMCOBJECT *pstruOmcObj, int nOmcObjNum,
		UBYTE *pubCmdBody, int nCmdBodyMaxLen, UBYTE *pubCmdBodyLen)
{
	int i, j;
	char  szCmdObjList[MAX_OBJ_LEN];
	int  nSeperateNum;
	char  *pszField[MAX_SEP_NUM];
	char  szDataType[20];
	int  nDataLen;
	char  *pszMapId;
	char  szObjVal[MAX_VAL_LEN];
	int  nBitIndct;
	UWORD  uwTemp;
	UDWORD  udwTemp;
	int  nResvSize;

	strcpy(szCmdObjList, pszObjList);
	strcpy(szCmdObjList, TrimAllSpace(szCmdObjList));
		
	//02B0,02B1,0229,0228,0201,resv2,020A,020B,020E,0202,0215,0212,02D1,02D2,02D3,0214,020F,resv15
	nSeperateNum = SeperateString(szCmdObjList, ',', pszField, MAX_SEP_NUM);	
	
	nBitIndct = 0;
	memset(pubCmdBody, 0, nCmdBodyMaxLen);
	for ( i = 0, j = 0; i < nSeperateNum; i ++ )
	{
		pszMapId = pszField[i];
		if (strncmp(pszMapId, "bytresv", 7) == 0)
		{
			nResvSize = atoi(pszMapId+7);
			memset(pubCmdBody+j, 0, nResvSize);
			j += nResvSize;
			continue;
		}
		if (strncmp(pszMapId, "bitresv", 7) == 0)
		{
			nResvSize = atoi(pszMapId+7);
			nBitIndct += nResvSize;
			if ( nBitIndct % 8 == 0)
				j ++;
			continue;
		}
		
		if (GetMapIdFromCache_CG(nProtclType, pszMapId, szDataType, &nDataLen) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", pszMapId);
			return -1;
		}

		if (GetObjValByObjId(pszMapId, pstruOmcObj, nOmcObjNum, szObjVal) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "应用不存在该[%s]监控量\n", pszMapId);
			return -1;
		}
		
		if (strcmp(szDataType, "bit") == 0)
		{
			pubCmdBody[j] |= ((UBYTE)atoi(szObjVal) << (7 - nBitIndct % 8));
			if ( ++ nBitIndct % 8 == 0)
				j ++;
		}	
		else if (strcmp(szDataType, "sint1") == 0 || strcmp(szDataType, "uint1") == 0)
		{
			pubCmdBody[j] = (UBYTE)atoi(szObjVal); 
			j += 1;
		}
		else if (strcmp(szDataType, "sint2") == 0 || strcmp(szDataType, "uint2") == 0)
		{
			uwTemp = (UWORD)atoi(szObjVal); 
			pubCmdBody[j] = LOWBYTE(uwTemp);
			pubCmdBody[j+1]	= HIGBYTE(uwTemp);
			j += 2;
		}
		else if (strcmp(szDataType, "sint4") == 0 || strcmp(szDataType, "uint4") == 0)
		{
			udwTemp = (UDWORD)atoi(szObjVal); 
			pubCmdBody[j] = LOWBYTE(LOWWORD(udwTemp));
			pubCmdBody[j+1]=HIGBYTE(LOWWORD(udwTemp));
			pubCmdBody[j+2]=LOWBYTE(HIGWORD(udwTemp));
			pubCmdBody[j+3]=HIGBYTE(HIGWORD(udwTemp));
			j += 4;
		}
		else if (strcmp(szDataType, "str") == 0 )//ASCII码格式表示
		{
			int k;
			
			memset(pubCmdBody+j, 0x20, nDataLen);//字符串时不足补空格
			for (k = 0; k < strlen(szObjVal); k++)
			{
				pubCmdBody[j+k] = szObjVal[k];
			}
			
			j += nDataLen;
		}

	}
	*pubCmdBodyLen = j;	

	return 0;

}


int DecodeCmdBodyFromCmdId(int nProtclType, char *pszCmdObjList, CMDHEAD *pCmdHead, 
							UBYTE *pubCmdBody, OMCOBJECT *pstruOmcObj, int *pnOmcObjNum)
{
	UBYTE	ubCmdBodyLen;
	char  szCmdObjList[MAX_OBJ_LEN];
	int  nSeperateNum;
	char  *pszField[MAX_SEP_NUM];
	int i, j, k;
	char  *pszMapId;	
	char  szDataType[20];
	int  nDataLen;
	char  szTemp[100];		
	int  nBitIndct;	
	int nResvSize;
	
	
	/*应答失败*/
	if (pCmdHead->ubAnsFlag != 0x00)
		return (int)(pCmdHead->ubAnsFlag);
	
	ubCmdBodyLen = pCmdHead->ubCmdBodyLen;	

	strcpy(szCmdObjList, pszCmdObjList);
	strcpy(szCmdObjList, TrimAllSpace(szCmdObjList));

	nSeperateNum = SeperateString(szCmdObjList, ',', pszField, MAX_SEP_NUM);
	nBitIndct = 0;	
		
	for ( i = 0, j = 0, k = 0; i < nSeperateNum; i ++ )
	{
		pszMapId = pszField[i];		
		if(strncmp(pszMapId, "bytresv", 7) == 0)
		{
			nResvSize = atoi(pszMapId+7);
			j += nResvSize;
			continue;
		}
		if (strncmp(pszMapId, "bitresv", 7) == 0)
		{
			nResvSize = atoi(pszMapId+7);
			nBitIndct += nResvSize;
			if (nBitIndct % 8 == 0)
			{
				j += 1;
			}
			continue;
		}		
		
		if (GetMapIdFromCache_CG(nProtclType, pszMapId, szDataType, &nDataLen) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", pszMapId);
			return -1;
		}

		strcpy(pstruOmcObj[k].szObjId, pszMapId);
					
		if (strcmp(szDataType, "sint1") == 0)
		{
			sprintf(szTemp, "%d", *((char *)pubCmdBody+j));
			j += 1;
		}
		else if	(strcmp(szDataType, "uint1") == 0)
		{
			sprintf(szTemp, "%u", *(pubCmdBody+j));
			j += 1;
		}
		else if (strcmp(szDataType, "sint2") == 0)
		{
			sprintf(szTemp, "%d", ReadShort(pubCmdBody+j));
			j += 2;
		}
		else if (strcmp(szDataType, "uint2") == 0)
		{
			sprintf(szTemp, "%u", ReadWORD(pubCmdBody+j));
			j += 2;
		}
		else if (strcmp(szDataType, "sint4") == 0)
		{
			sprintf(szTemp, "%ld", ReadLong(pubCmdBody+j));
			j += 4;
		}
		else if (strcmp(szDataType, "uint4") == 0)
		{
			sprintf(szTemp, "%lu", ReadDWORD(pubCmdBody+j));
			j += 4;
		}
		else if (strcmp(szDataType, "bit") == 0)
		{
			sprintf(szTemp, "%d", ((((UBYTE)(*(pubCmdBody+j))) >> (7-nBitIndct%8)) & 0x1));
			if (++ nBitIndct % 8 == 0)
			{
				j += 1;
			}	
		}
		else if (strcmp(szDataType, "str") == 0)
		{
			memset(szTemp, 0, sizeof(szTemp));
			memcpy(szTemp, pubCmdBody+j, nDataLen);
			strcpy(szTemp, TrimAllSpace(szTemp));
			j += nDataLen;
		}

		strcpy(pstruOmcObj[k].szObjVal, szTemp);
		k++;
				
	}
	*pnOmcObjNum = k;
	
	return 0;	
}

//2G协议流水号
int GetCurrent2GSquenue()
{
    STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	
	if (strcmp(getenv("DATABASE"), "mysql") == 0)
		sprintf(szSql,"SELECT auto_increment as itemvalue FROM information_schema.`TABLES` WHERE TABLE_NAME='ne_msgqueue'");
	else	
		sprintf(szSql,"select SEQ_MOBILE2G.NEXTVAL as itemvalue from dual");
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		nSequenceNO=1;
		PrintErrorLog(DBG_HERE,"取自增序列号时出错 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		//2009.5.15 增加数据重连
		CloseDatabase();
		sleep(30);
		
		if(OpenDatabase(szService, szDbName, szUser, szPwd) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "进程[%d]打开数据库发生错误 [%s]\n", getpid(), GetSQLErrorMessage());
			return EXCEPTION;
		}
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor)!=NORMAL)
	{
		FreeCursor(&struCursor);
		nSequenceNO=1;
		return nSequenceNO;
	}
	nSequenceNO=atoi(GetTableFieldValue(&struCursor,"itemvalue"));
	FreeCursor(&struCursor);
	
    
	return nSequenceNO;
}


PSTR GetTypeNumber(PSTR pszType)
{
    static STR szQrySetNumber[19 + 1];
	time_t struTimeNow;
	struct tm struTmNow;

	if(time(&struTimeNow)==(time_t)(-1))
	{
		PrintErrorLog(DBG_HERE,"得到系统时间错误 %s\n",strerror(errno));
		return "";
	}

	memset(szQrySetNumber, 0, sizeof(szQrySetNumber));
	struTmNow=*localtime(&struTimeNow);
	nQrySetNumber >= 999 ? nQrySetNumber=1 : nQrySetNumber ++;
	
	snprintf(szQrySetNumber,sizeof(szQrySetNumber),"%s%04d%02d%02d%02d%02d%02d%03d", pszType,
		struTmNow.tm_year+1900,struTmNow.tm_mon+1,struTmNow.tm_mday, struTmNow.tm_hour,struTmNow.tm_min,
		struTmNow.tm_sec, nQrySetNumber);

	return szQrySetNumber; 
}

PSTR Get2GNumber(PSTR pszType, INT n2G_QB)
{
    static STR szQrySetNumber[30 + 1];
	time_t struTimeNow;
	struct tm struTmNow;

	if(time(&struTimeNow)==(time_t)(-1))
	{
		PrintErrorLog(DBG_HERE,"得到系统时间错误 %s\n",strerror(errno));
		return "";
	}

	memset(szQrySetNumber, 0, sizeof(szQrySetNumber));
	struTmNow=*localtime(&struTimeNow);
		
	snprintf(szQrySetNumber,sizeof(szQrySetNumber),"%s%04d%02d%02d%02d%02d%02d%d", pszType,
		struTmNow.tm_year+1900,struTmNow.tm_mon+1,struTmNow.tm_mday, struTmNow.tm_hour,struTmNow.tm_min,
		struTmNow.tm_sec, n2G_QB);

	return szQrySetNumber; 
}



/*
 * 取自增序号
 */
RESULT GetDbSerial(PUINT pnSerial,PCSTR pszItemName)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	
	sprintf(szSql,"select ide_ItemValue from sys_Identity where ide_Item='%s' ",pszItemName);
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		*pnSerial=0;
		PrintErrorLog(DBG_HERE,"取自增序列号时出错 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		//2009.5.15 增加数据重连
		CloseDatabase();
		sleep(30);
		
		if(OpenDatabase(szService, szDbName, szUser, szPwd) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "进程[%d]打开数据库发生错误 [%s]\n", getpid(), GetSQLErrorMessage());
			return EXCEPTION;
		}
		return EXCEPTION;
		
	}
	if(FetchCursor(&struCursor)!=NORMAL)
	{
		FreeCursor(&struCursor);
		sprintf(szSql,"insert into sys_Identity (ide_Item, ide_ItemValue) values('%s',1)",pszItemName);
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
		if(ExecuteSQL(szSql)!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
			return EXCEPTION;
		}
		*pnSerial=1;
		CommitTransaction();
		return NORMAL;
    }
	*pnSerial=atoi(GetTableFieldValue(&struCursor,"ide_ItemValue"))+1;
	FreeCursor(&struCursor);
	
	sprintf(szSql,"UPDATE sys_Identity SET ide_ItemValue=ide_ItemValue + 1 WHERE ide_Item='%s'",pszItemName);
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"取自增序列号时出错 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();
	
    //整数最大数为2147483645
    if (*pnSerial >= 2147483645 || (*pnSerial >= 32766 &&  strcmp(pszItemName, "Mobile2G") == 0))
    {
        sprintf(szSql,"UPDATE sys_Identity SET ide_ItemValue= 1 WHERE ide_Item='%s'",pszItemName);
	    PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	    if(ExecuteSQL(szSql)!=NORMAL)
	    {
		    PrintErrorLog(DBG_HERE,"取自增序列号时出错 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		    return EXCEPTION;
	    }
	    CommitTransaction();
    }
    
	return NORMAL;
}


/* 
 * 根据手机号取协议类型
 */
int GetProtocolFrom(PSTR pszMobile)
{
	CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	int nProtocol;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select ne_ProtocoltypeId from ne_Element where ne_NeTelNum='%s'", pszMobile);
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
	nProtocol = atoi(GetTableFieldValue(&struCursor, "ne_ProtocoltypeId"));
	FreeCursor(&struCursor);
	return nProtocol;
	
}

/* 
 * 根据mapid取告警名
 */
PSTR GetAlarmName(PSTR pszMapId)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	static STR szAlarmName[100];
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select alm_name from alm_alarm where alm_objid = '%s'", pszMapId);
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return "";
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    FreeCursor(&struCursor);
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", \
					  szSql, GetSQLErrorMessage());
		return "";
	}
	strcpy(szAlarmName,  TrimAllSpace(GetTableFieldValue(&struCursor, "alm_name")));
	FreeCursor(&struCursor);
	return szAlarmName;
}


/* 
 * 根据网元取告警对象列表
 */
PSTR GetAlarmObjList(int nNeId)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	static STR szAlarmObjList[1000];
	TBINDVARSTRU struBindVar;
	
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_INT;
	struBindVar.struBind[0].VarValue.nValueInt = nNeId;
	struBindVar.nVarCount++;
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select ne_AlarmObjList,ne_Name from ne_Element where ne_NeId = :v_0");
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s][%d]\n", szSql, nNeId);
	if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return "";
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    FreeCursor(&struCursor);
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s][%d]没有找到记录\n", szSql, nNeId);
		return "";
	}
	strcpy(szAlarmObjList,  TrimAllSpace(GetTableFieldValue(&struCursor, "ne_AlarmObjList")));
	//strcpy(pszNeName,  TrimAllSpace(GetTableFieldValue(&struCursor, "ne_Name")));
	FreeCursor(&struCursor);
	return szAlarmObjList;
}

/* 
 * 根据网元取告警对象列表
 */
PSTR GetAlarmEnabledObjList(int nNeId)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	static STR szAlarmEnabledObjList[1000];
	TBINDVARSTRU struBindVar;
	
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_INT;
	struBindVar.struBind[0].VarValue.nValueInt = nNeId;
	struBindVar.nVarCount++;
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select ne_AlarmEnabledObjList,ne_Name from ne_Element where ne_NeId = :v_0");
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s][%d]\n", szSql, nNeId);
	if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return "";
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    FreeCursor(&struCursor);
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", \
					  szSql, GetSQLErrorMessage());
		return "";
	}
	strcpy(szAlarmEnabledObjList,  TrimAllSpace(GetTableFieldValue(&struCursor, "ne_AlarmEnabledObjList")));
	FreeCursor(&struCursor);
	return szAlarmEnabledObjList;
}

/* 
 * 根据直放站编号,电话号,设备编号取告警对象列表
 */
RESULT GetAlarmObjList2(UINT nRepeaterId, int nDeviceId, PSTR pszNeTelNum, int *pNeId, PSTR pszNeName, PSTR pszAlarmObjList)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	TBINDVARSTRU struBindVar;
	
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_INT;
	struBindVar.struBind[0].VarValue.nValueInt = nRepeaterId;
	struBindVar.nVarCount++;
	
	struBindVar.struBind[1].nVarType = SQLT_INT;
	struBindVar.struBind[1].VarValue.nValueInt = nDeviceId;
	struBindVar.nVarCount++;
	
	struBindVar.struBind[2].nVarType = SQLT_STR;
	strcpy(struBindVar.struBind[2].VarValue.szValueChar, pszNeTelNum);
	struBindVar.nVarCount++;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select ne_NeId,ne_AlarmObjList,ne_Name from ne_Element where ne_RepeaterId = :v_0 and ne_DeviceId = :v_1 and ne_NeTelNum = :v_2");
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s][%u][%d][%s]\n", szSql, nRepeaterId, nDeviceId, pszNeTelNum);
	if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n",  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    FreeCursor(&struCursor);
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s][%u][%d][%s]没有找到记录\n", szSql, nRepeaterId, nDeviceId, pszNeTelNum);
		return EXCEPTION;
	}
	strcpy(pszAlarmObjList,  TrimAllSpace(GetTableFieldValue(&struCursor, "ne_AlarmObjList")));
	strcpy(pszNeName,  TrimAllSpace(GetTableFieldValue(&struCursor, "ne_Name")));
	*pNeId = atoi(GetTableFieldValue(&struCursor, "ne_NeId"));
	FreeCursor(&struCursor);
	return NORMAL;
}

/* 
 * 根据直放站编号,设备编号取告警对象列表
 */
RESULT GetAlarmObjList3(UINT nRepeaterId, int nDeviceId, int *pNeId, PSTR pszNeName, PSTR pszAlarmObjList)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select ne_NeId,ne_AlarmObjList,ne_Name from ne_Element where ne_RepeaterId = %u and ne_DeviceId = %d ",
	        nRepeaterId, nDeviceId);
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
	strcpy(pszAlarmObjList,  TrimAllSpace(GetTableFieldValue(&struCursor, "ne_AlarmObjList")));
	strcpy(pszNeName,  TrimAllSpace(GetTableFieldValue(&struCursor, "ne_Name")));
	*pNeId = atoi(GetTableFieldValue(&struCursor, "ne_NeId"));
	FreeCursor(&struCursor);
	return NORMAL;
}

/* 
 * 根据直放站编号,电话号,设备编号取告警对象列表
 */
INT GetNeId(UINT nRepeaterId, int nDeviceId, PSTR pszNeTelNum, BOOL *pIsNewNeId)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	static UINT nNeId;
	
	*pIsNewNeId = BOOLFALSE;
	memset(szSql, 0, sizeof(szSql));
	if (strlen(pszNeTelNum) ==0)
	{
	    sprintf(szSql, "select ne_NeId from ne_Element where ne_RepeaterId = %u and ne_DeviceId = %d",
	            nRepeaterId, nDeviceId);
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
	        PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没记录，产生新的网元记录!\n",  szSql);
	    	GetDbSerial(&nNeId, "ne_Element");
	    	*pIsNewNeId = BOOLTRUE;
	    	return nNeId;
	    }
	    nNeId = atoi(GetTableFieldValue(&struCursor, "ne_NeId"));
	    FreeCursor(&struCursor);
	}
	else
	{
	    sprintf(szSql, "select ne_NeId from ne_Element where ne_RepeaterId = %u and ne_NeTelNum = '%s' and ne_DeviceId = %d",
	            nRepeaterId,  pszNeTelNum, nDeviceId);
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
	    	memset(szSql, 0, sizeof(szSql));
	        sprintf(szSql, "select ne_NeId from ne_Element where ne_RepeaterId = %u  and ne_DeviceId = %d",
	                nRepeaterId, nDeviceId);
	        PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	        if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	        {
	        	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
	        				  szSql, GetSQLErrorMessage());
	        	return EXCEPTION;
	        }
	        if(FetchCursor(&struCursor) != NORMAL)
	        {
	        	PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没记录，产生新的网元记录!\n",  szSql);
	    	    GetDbSerial(&nNeId, "ne_Element");
	    	    *pIsNewNeId = BOOLTRUE;
	    	    FreeCursor(&struCursor);
	    	    return nNeId;
	        }
	    }
	    nNeId = atoi(GetTableFieldValue(&struCursor, "ne_NeId"));
	    FreeCursor(&struCursor);
    }
	return nNeId;
}


BOOL ExistNeId(UINT nRepeaterId, int nDeviceId)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	INT nCount;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select count(ne_NeId) as v_count from ne_Element where NE_REPEATERID = %u and NE_DEVICEID= %d", nRepeaterId, nDeviceId);
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
	nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
	FreeCursor(&struCursor);
	if (nCount > 0)
	    return BOOLTRUE;
	else
	    return BOOLFALSE;
}


BOOL ExistDasNeId(UINT nRepeaterId, int nDeviceId, PSTR pszRouteAddr)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	INT nCount;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select count(ne_NeId) as v_count from ne_Element where NE_REPEATERID = %u and NE_DEVICEID = %d and NE_ROUTE= '%s'", nRepeaterId, nDeviceId, pszRouteAddr);
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
	nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
	FreeCursor(&struCursor);
	if (nCount > 0)
	    return BOOLTRUE;
	else
	{
		memset(szSql, 0, sizeof(szSql));
		sprintf(szSql, "select count(ne_NeId) as v_count from ne_Element where NE_REPEATERID = %d and NE_DEVICEID= %d", nRepeaterId, nDeviceId);
		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
		if(SelectTableRecord(szSql, &struCursor) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
						  szSql, GetSQLErrorMessage());
			return EXCEPTION;
		}
		if(FetchCursor(&struCursor) == NORMAL)
		nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
		FreeCursor(&struCursor);
		if (nCount > 0)
	    	return BOOLTRUE;
		else
	    	return BOOLFALSE;
	}
}


BOOL ExistRfidEle(UINT nRepeaterId, PSTR pszRouteAddr)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	INT nCount;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select count(*) as v_count from ne_Rfid where RFI_REPEATERID = %u and RFI_ROUTE= '%s'", nRepeaterId, pszRouteAddr);
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
	nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
	FreeCursor(&struCursor);
	if (nCount > 0)
	    return BOOLTRUE;
	else
	    return BOOLFALSE;
}

/* 
 * 根据直放站编号,电话号,设备编号取告警对象列表
 */
RESULT GetNeInfo(SENDPACKAGE *pstruNeInfo)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	TBINDVARSTRU struBindVar;
	
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_INT;
	struBindVar.struBind[0].VarValue.nValueInt = pstruNeInfo->struRepeater.nRepeaterId;
	struBindVar.nVarCount++;
	
	struBindVar.struBind[1].nVarType = SQLT_INT;
	struBindVar.struBind[1].VarValue.nValueInt = pstruNeInfo->struRepeater.nDeviceId;
	struBindVar.nVarCount++;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select ne_NeId,ne_netelnum,ne_servertelnum,ne_protocoldevicetypeid,ne_ProtocoltypeId, ne_alarmobjlist, ne_areaid from ne_Element where ne_RepeaterId = :v_0 and ne_DeviceId = :v_1");
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s][%u][%d]\n", szSql,pstruNeInfo->struRepeater.nRepeaterId,  pstruNeInfo->struRepeater.nDeviceId);
	if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", szSql, GetSQLErrorMessage());
		//2009.5.15 增加数据重连
		CloseDatabase();
		sleep(30);
		
		if(OpenDatabase(szService, szDbName, szUser, szPwd) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "进程[%d]打开数据库发生错误 [%s]\n", getpid(), GetSQLErrorMessage());
			return EXCEPTION;
		}
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    FreeCursor(&struCursor);
    	PrintErrorLog(DBG_HERE, "执行SQL语句[%s][%u][%d]没有找到记录\n", szSql, pstruNeInfo->struRepeater.nRepeaterId,  pstruNeInfo->struRepeater.nDeviceId);
	    return EXCEPTION;
	}
	pstruNeInfo->nNeId = atoi(GetTableFieldValue(&struCursor, "ne_NeId"));
	pstruNeInfo->nAreaId = atoi(GetTableFieldValue(&struCursor, "ne_areaid"));
	pstruNeInfo->struRepeater.nProtocolDeviceType = atoi(GetTableFieldValue(&struCursor, "ne_protocoldevicetypeid"));
	pstruNeInfo->struHead.nProtocolType = atoi(GetTableFieldValue(&struCursor, "ne_ProtocoltypeId"));
	strcpy(pstruNeInfo->struRepeater.szTelephoneNum, TrimAllSpace(GetTableFieldValue(&struCursor, "ne_netelnum")));
	strcpy(pstruNeInfo->struRepeater.szNetCenter, TrimAllSpace(GetTableFieldValue(&struCursor, "ne_servertelnum")));
	strcpy(pstruNeInfo->struRepeater.szAlarmObjList, TrimAllSpace(GetTableFieldValue(&struCursor, "ne_alarmobjlist")));
	FreeCursor(&struCursor);
	
	return NORMAL;
}

/*
 * 重新分配特服号
 */
RESULT DistServerTelNum(PXMLSTRU pstruReqXml)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
    STR szServTelNum1[20], szServTelNum2[20], szServTelNum3[20],szServTelNum4[20], szServTelNum5[20];
    STR szAgentNo[20];
    
    strcpy(szServTelNum1, DemandStrInXmlExt(pstruReqXml,"<omc>/<服务号码1>"));
    strcpy(szServTelNum2, DemandStrInXmlExt(pstruReqXml,"<omc>/<服务号码2>"));
    strcpy(szServTelNum3, DemandStrInXmlExt(pstruReqXml,"<omc>/<服务号码3>"));
    strcpy(szServTelNum4, DemandStrInXmlExt(pstruReqXml,"<omc>/<服务号码4>"));
    strcpy(szServTelNum5, DemandStrInXmlExt(pstruReqXml,"<omc>/<服务号码5>"));
    
    
    memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select qs_agentno from ne_queuenum where qs_state = '0' order by qs_count");
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	while(FetchCursor(&struCursor) == NORMAL)
	{
	    strcpy(szAgentNo, TrimAllSpace(GetTableFieldValue(&struCursor, "qs_agentno")));
	    if (strcmp(szAgentNo, szServTelNum1) == 0 || strcmp(szAgentNo, szServTelNum2) == 0 ||
	        strcmp(szAgentNo, szServTelNum3) == 0 || strcmp(szAgentNo, szServTelNum4) == 0 ||
	        strcmp(szAgentNo, szServTelNum5) == 0)
	    {
	        InsertInXmlExt(pstruReqXml, "<omc>/<服务号码>", szAgentNo, MODE_AUTOGROW|MODE_UNIQUENAME);
	        FreeCursor(&struCursor);
	        return NORMAL;
	    }
	}
	FreeCursor(&struCursor);
	return NORMAL;
}

RESULT GetDbSequence(PUINT pnSerial,PCSTR pszItemName)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	
	sprintf(szSql,"select %s.NEXTVAL as itemvalue from dual", pszItemName);
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		*pnSerial=0;
		PrintErrorLog(DBG_HERE,"取自增序列号时出错 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		//2009.5.15 增加数据重连
		CloseDatabase();
		sleep(30);
		
		if(OpenDatabase(szService, szDbName, szUser, szPwd) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "进程[%d]打开数据库发生错误 [%s]\n", getpid(), GetSQLErrorMessage());
			return EXCEPTION;
		}
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor)!=NORMAL)
	{
		*pnSerial=0;
		FreeCursor(&struCursor);
		return NORMAL;
	}
	*pnSerial=atoi(GetTableFieldValue(&struCursor,"itemvalue"));
	FreeCursor(&struCursor);
	
	return NORMAL;
}

RESULT GetMySqlSequence(PUINT pnSerial,PCSTR pszTableName)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	
	sprintf(szSql,"SELECT auto_increment as itemvalue FROM information_schema.`TABLES` WHERE TABLE_NAME='%s' and table_schema = '%s'", pszTableName, szDbName);
	if(SelectTableRecord(szSql,&struCursor)!=NORMAL)
	{
		*pnSerial=0;
		PrintErrorLog(DBG_HERE,"取自增序列号时出错 SQL语句[%s]错误信息=[%s]\n",szSql,GetSQLErrorMessage());
		//2009.5.15 增加数据重连
		CloseDatabase();
		sleep(30);
		
		if(OpenDatabase(szService, szDbName, szUser, szPwd) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "进程[%d]打开数据库发生错误 [%s]\n", getpid(), GetSQLErrorMessage());
			return EXCEPTION;
		}
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor)!=NORMAL)
	{
		*pnSerial=0;
		FreeCursor(&struCursor);
		return NORMAL;
	}
	*pnSerial=atoi(GetTableFieldValue(&struCursor,"itemvalue"));
	FreeCursor(&struCursor);
	
	return NORMAL;
}
RESULT SaveToMsgQueue(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	UINT qry_EleQryLogId;		//获取编号
	char szTemp[20];
	char szLastTime[20], szTeleNum[20];

	if (strcmp(getenv("DATABASE"), "mysql") == 0)
	{
		if(GetMySqlSequence(&qry_EleQryLogId, "ne_msgqueue")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取查询设置流水号错误\n");
			return EXCEPTION;
		}
	}
	else
	{
		if(GetDbSequence(&qry_EleQryLogId, "SEQ_MSGQUEUE")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取查询设置流水号错误\n");
			return EXCEPTION;
		}
	}
	strcpy(szTeleNum, DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"));
	if(DemandInXmlExt(pstruReqXml, "<omc>/<定时发送时间>", szLastTime, sizeof(szLastTime)) != NORMAL)
	   	strcpy(szLastTime, "0");
	
	sprintf(szTemp, "%d", qry_EleQryLogId);
	InsertInXmlExt(pstruReqXml, "<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	//if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<服务状态>")) == 0)
	{
		snprintf(szSql, sizeof(szSql),
			"INSERT INTO NE_MSGQUEUE (QS_ID, QS_NEID, QS_REPEATERID, QS_DEVICEID, QS_CONTENT, "
			"QS_PROTOCOLTYPEID, QS_TELEPHONENUM, QS_IP, QS_QRYNUMBER, QS_COMMANDCODE,"
			"QS_DEVICETYPEID, QS_OTHERDEVICEID, QS_DEVICEMODELID, QS_PORT, QS_SERVERTELNUM,"
			"QS_TYPE, QS_EVENTTIME, QS_LEVEL, QS_COMMTYPEID, QS_NETFLAG,"
			"QS_LOGID, QS_WINDOWLOGID, QS_TASKID, QS_TASKLOGID,"
			"QS_LASTTIME, QS_RETRYTIMES, QS_FIRSTID, QS_SECONDID, QS_MSGSTAT) VALUES ( "
			"%d,  %s,  %s,   %s,   '%s',"
			" %s, '%s', '%s', '%s', %s, "
			"'%s', '%s', '%s', %d, '%s', "
			" %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), %s, %d, %d, "
			" %d, '%s',  %d, %d,"
			" '%s', 0, 0, 0, '0')",
			qry_EleQryLogId,
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<设备编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<消息内容>"),  //短信内容
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<协议类型>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"),  //手机号
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点IP>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<命令号>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<设备类型>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<其它标识>"),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<设备型号>"),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<端口号>")),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<服务号码>"),
			
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<类型>")),     //消息类型
			GetSysDateTime(),//DemandStrInXmlExt(pstruReqXml, "<omc>/<时间>"),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<站点等级>"),  //消息级别
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<通信方式>")),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<序列号>")),
			
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<登录流水号>")),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<窗体流水号>"),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<任务号>")),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<任务日志号>")),
			szLastTime );
		
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

RESULT SaveToMsgQueue_Tmp(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	UINT qry_EleQryLogId;		//获取编号
	char szTemp[20];
	char szLastTime[20], szTeleNum[20];

	if (strcmp(getenv("DATABASE"), "mysql") == 0)
	{
		if(GetMySqlSequence(&qry_EleQryLogId, "ne_msgqueue")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取查询设置流水号错误\n");
			return EXCEPTION;
		}
	}
	else
	{
		if(GetDbSequence(&qry_EleQryLogId, "SEQ_MSGQUEUE")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取查询设置流水号错误\n");
			return EXCEPTION;
		}
	}
	strcpy(szTeleNum, DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"));
	if(DemandInXmlExt(pstruReqXml, "<omc>/<定时发送时间>", szLastTime, sizeof(szLastTime)) != NORMAL)
	   	strcpy(szLastTime, "0");
	
	sprintf(szTemp, "%d", qry_EleQryLogId);
	InsertInXmlExt(pstruReqXml, "<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	//if (atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<服务状态>")) == 0)
	{
		snprintf(szSql, sizeof(szSql),
			"INSERT INTO TT_MSGQUEUE (QS_ID, QS_NEID, QS_REPEATERID, QS_DEVICEID, QS_CONTENT, "
			"QS_PROTOCOLTYPEID, QS_TELEPHONENUM, QS_IP, QS_QRYNUMBER, QS_COMMANDCODE,"
			"QS_DEVICETYPEID, QS_OTHERDEVICEID, QS_DEVICEMODELID, QS_PORT, QS_SERVERTELNUM,"
			"QS_TYPE, QS_EVENTTIME, QS_LEVEL, QS_COMMTYPEID, QS_NETFLAG,"
			"QS_LOGID, QS_WINDOWLOGID, QS_TASKID, QS_TASKLOGID,"
			"QS_LASTTIME, QS_RETRYTIMES, QS_FIRSTID, QS_SECONDID, QS_MSGSTAT) VALUES ( "
			"%d,  %s,  %s,   %s,   '%s',"
			" %s, '%s', '%s', '%s', %s, "
			"'%s', '%s', '%s', %d, '%s', "
			" %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), %s, %d, %d, "
			" %d, '%s',  %d, %d,"
			" '%s', 0, 0, 0, '0')",
			qry_EleQryLogId,
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<设备编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<消息内容>"),  //短信内容
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<协议类型>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"),  //手机号
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点IP>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<命令号>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<设备类型>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<其它标识>"),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<设备型号>"),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<端口号>")),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<服务号码>"),
			
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<类型>")),     //消息类型
			GetSysDateTime(),//DemandStrInXmlExt(pstruReqXml, "<omc>/<时间>"),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<站点等级>"),  //消息级别
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<通信方式>")),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<序列号>")),
			
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<登录流水号>")),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<窗体流水号>"),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<任务号>")),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<任务日志号>")),
			szLastTime );
		
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

INT Get2GSerial(PSTR pszSerialType, PINT pnSerial)
{
	STR szFileName[MAX_PATH_LEN];
	INT nFileDescriptor;
	INT nSerial, n2g_QB;

	snprintf(szFileName,sizeof(szFileName),"%s/etc/serial.%s",getenv("HOME"),pszSerialType);
	if(access(szFileName,F_OK)<0)
	{
		nFileDescriptor=open(szFileName,O_WRONLY|O_CREAT,0600);
		if(nFileDescriptor<0)
		{
			PrintErrorLog(DBG_HERE,"创建流水文件错误[%s][%s]\n",szFileName,strerror(errno));
			return -1;
		}
		nSerial=1;
		if(GetMySqlSequence(&nSerial, "ne_gprsqueue")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"初始化查询设置流水号错误\n");
		}
		nSerial++;
		write(nFileDescriptor,&nSerial,sizeof(nSerial));
		close(nFileDescriptor);
		return nSerial;
	}
	else
	{
		nFileDescriptor=open(szFileName,O_RDWR);
		if(nFileDescriptor<0)
		{
			PrintErrorLog(DBG_HERE,"打开编号数据库错误[%s][%s]\n",szFileName,strerror(errno));
			return -1;
		}
		SetFileWriteLock(nFileDescriptor,0,sizeof(nSerial));
		lseek(nFileDescriptor,0,SEEK_SET);
		read(nFileDescriptor,&nSerial,sizeof(nSerial));
		lseek(nFileDescriptor,0,SEEK_SET);

		nSerial++;
		//if (nSerial >= 32766) nSerial=1; //2020.10.30 add int 2147483647
		if (nSerial >= 2147483640) nSerial=1;
		write(nFileDescriptor,&nSerial,sizeof(nSerial));
		UnSetFileWriteLock(nFileDescriptor,0,sizeof(nSerial));
		close(nFileDescriptor);
		
		n2g_QB=nSerial%32766+1;
		*pnSerial=nSerial;
		return n2g_QB;
	}
}


RESULT SaveToGprsQueue(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	UINT qry_EleQryLogId;		//获取编号
	char szTemp[20];
	char szLastTime[20], szTeleNum[20];
	//int nRepeaterId, nDeviceId;
	
	if (getenv("WUXIAN")!=NULL)
	{
		RedisHsetAndPush(pstruReqXml);
	}
	else
	{
		if (strcmp(getenv("DATABASE"), "mysql") == 0)
		{
			if(GetMySqlSequence(&qry_EleQryLogId, "ne_gprsqueue")!=NORMAL)
			{
				PrintErrorLog(DBG_HERE,"获取查询设置流水号错误\n");
				return EXCEPTION;
			}
		}
		else
		{
			if(GetDbSequence(&qry_EleQryLogId, "SEQ_MSGQUEUE")!=NORMAL)
			{
				PrintErrorLog(DBG_HERE,"获取查询设置流水号错误\n");
				return EXCEPTION;
			}
		}
		//nRepeaterId=atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<站点编号>"));
		//nDeviceId=atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<设备编号>"));
		
		strcpy(szTeleNum, DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"));
	   	strcpy(szLastTime, "0");
		
		sprintf(szTemp, "%d", qry_EleQryLogId);
		InsertInXmlExt(pstruReqXml, "<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
		{
			memset(szSql, 0, sizeof(szSql));
			snprintf(szSql, sizeof(szSql),
				"INSERT INTO NE_GPRSQUEUE (QS_ID, QS_NEID, QS_REPEATERID, QS_DEVICEID, QS_CONTENT, "
				"QS_PROTOCOLTYPEID, QS_TELEPHONENUM, QS_IP, QS_QRYNUMBER, QS_COMMANDCODE,"
				" QS_DEVICETYPEID, QS_OTHERDEVICEID, QS_DEVICEMODELID, QS_PORT, QS_SERVERTELNUM,"
				"QS_TYPE, QS_EVENTTIME, QS_LEVEL, QS_COMMTYPEID, QS_NETFLAG,"
				"QS_LOGID, QS_WINDOWLOGID, QS_TASKID, QS_TASKLOGID,"
				"QS_LASTTIME, QS_RETRYTIMES, QS_FIRSTID, QS_SECONDID, QS_MSGSTAT) VALUES ( "
				"%d,  %s,  %s,   %s,   '%s',"
				" %s, '%s', '%s', '%s', %s, "
				" %d, '%s', '%s', %d, '%s', "
				" %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), %s, %d, %d, "
				" %d, '%s',  %d, %d,"
				" '%s', 0, 0, 0, '0')",
				qry_EleQryLogId,
				DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<站点编号>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<设备编号>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<消息内容>"),  //短信内容
				
				DemandStrInXmlExt(pstruReqXml, "<omc>/<协议类型>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"),  //手机号
				DemandStrInXmlExt(pstruReqXml, "<omc>/<站点IP>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<命令号>"),
				
				atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<设备类型>")),
				DemandStrInXmlExt(pstruReqXml, "<omc>/<其它标识>"),
				DemandStrInXmlExt(pstruReqXml,"<omc>/<设备型号>"),
				atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<端口号>")),
				DemandStrInXmlExt(pstruReqXml,"<omc>/<服务号码>"),
				
				atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<类型>")),     //消息类型
				GetSysDateTime(),//DemandStrInXmlExt(pstruReqXml, "<omc>/<时间>"),
				DemandStrInXmlExt(pstruReqXml,"<omc>/<站点等级>"),  //消息级别
				atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<通信方式>")),
				atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<序列号>")),
				
				atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<登录流水号>")),
				DemandStrInXmlExt(pstruReqXml,"<omc>/<窗体流水号>"),
				atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<任务号>")),
				atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<任务日志号>")),
				szLastTime );
				
	
				PrintDebugLog(DBG_HERE, "Excute SQL[%s]\n", szSql);
				if(ExecuteSQL(szSql) != NORMAL)
				{
					PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
						szSql, GetSQLErrorMessage());
		    	    return EXCEPTION;
				}
				CommitTransaction();
		}
	}

	return NORMAL;  
    
}

RESULT SaveToSnmpQueue(PXMLSTRU pstruReqXml)
{
    char szSql[MAX_BUFFER_LEN];
	UINT qry_EleQryLogId;		//获取编号
	char szTemp[20];
	char szLastTime[20], szTeleNum[20];
	
	if (strcmp(getenv("DATABASE"), "mysql") == 0)
	{
		if(GetMySqlSequence(&qry_EleQryLogId, "ne_snmpqueue")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取查询设置流水号错误\n");
			return EXCEPTION;
		}
	}
	else
	{
		if(GetDbSequence(&qry_EleQryLogId, "SEQ_MSGQUEUE")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取查询设置流水号错误\n");
			return EXCEPTION;
		}
	}
	
	strcpy(szTeleNum, DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"));
   	strcpy(szLastTime, "0");
	
	sprintf(szTemp, "%d", qry_EleQryLogId);
	InsertInXmlExt(pstruReqXml, "<omc>/<日志号>", szTemp, MODE_AUTOGROW|MODE_UNIQUENAME);
	{
		memset(szSql, 0, sizeof(szSql));
		snprintf(szSql, sizeof(szSql),
			"INSERT INTO NE_SNMPQUEUE (QS_ID, QS_NEID, QS_REPEATERID, QS_DEVICEID, QS_CONTENT, "
			"QS_PROTOCOLTYPEID, QS_TELEPHONENUM, QS_IP, QS_QRYNUMBER, QS_COMMANDCODE,"
			" QS_DEVICETYPEID, QS_OTHERDEVICEID, QS_DEVICEMODELID, QS_PORT, QS_SERVERTELNUM,"
			"QS_TYPE, QS_EVENTTIME, QS_LEVEL, QS_COMMTYPEID, QS_NETFLAG,"
			"QS_LOGID, QS_WINDOWLOGID, QS_TASKID, QS_TASKLOGID,"
			"QS_LASTTIME, QS_RETRYTIMES, QS_FIRSTID, QS_SECONDID, QS_MSGSTAT) VALUES ( "
			"%d,  %s,  %s,   %s,   '%s',"
			" %s, '%s', '%s', '%s', %s, "
			" '%s', '%s', '%s', %d, '%s', "
			" %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), %s, %d, %d, "
			" %d, '%s',  %d, %d,"
			" '%s', 0, 0, 0, '0')",
			qry_EleQryLogId,
			DemandStrInXmlExt(pstruReqXml, "<omc>/<网元编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<设备编号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<消息内容>"),  //短信内容
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<协议类型>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点电话>"),  //手机号
			DemandStrInXmlExt(pstruReqXml, "<omc>/<站点IP>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<流水号>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<命令号>"),
			
			DemandStrInXmlExt(pstruReqXml, "<omc>/<设备类型>"),
			DemandStrInXmlExt(pstruReqXml, "<omc>/<其它标识>"),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<设备型号>"),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<端口号>")),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<服务号码>"),
			
			atoi(DemandStrInXmlExt(pstruReqXml, "<omc>/<类型>")),     //消息类型
			GetSysDateTime(),//DemandStrInXmlExt(pstruReqXml, "<omc>/<时间>"),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<站点等级>"),  //消息级别
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<通信方式>")),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<序列号>")),
			
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<登录流水号>")),
			DemandStrInXmlExt(pstruReqXml,"<omc>/<窗体流水号>"),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<任务号>")),
			atoi(DemandStrInXmlExt(pstruReqXml,"<omc>/<任务日志号>")),
			szLastTime );
		
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

RESULT GetSendPackageInfo(int QB, unsigned int RptId, SENDPACKAGE *pstruSendInfo)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	INT nMsgSerial;
	TBINDVARSTRU struBindVar;
	
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_INT; //SQLT_INT
	struBindVar.struBind[0].VarValue.nValueInt = QB;
	struBindVar.nVarCount++;

	struBindVar.struBind[1].nVarType = SQLT_INT; //SQLT_INT
	struBindVar.struBind[1].VarValue.nValueInt = RptId;
	struBindVar.nVarCount++;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "SELECT QS_ID,QS_PROTOCOLTYPEID,QS_COMMANDCODE, QS_QRYNUMBER,QS_LEVEL, QS_COMMTYPEID, QS_REPEATERID, QS_DEVICEID,QS_TELEPHONENUM, QS_SERVERTELNUM, QS_DEVICETYPEID, QS_OTHERDEVICEID, QS_NEID,QS_TASKID, QS_TASKLOGID FROM NE_MSGQUEUE WHERE QS_NETFLAG = :v_0 AND QS_REPEATERID = :v_1");
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s][%d][%u]\n", szSql, QB, RptId);
	if(BindSelectTableRecord(szSql, &struCursor, &struBindVar) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
	    FreeCursor(&struCursor);
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s][%d][%u]没有找到记录\n", szSql, QB, RptId);
		return EXCEPTION;
	}
	nMsgSerial = atoi(GetTableFieldValue(&struCursor, "QS_ID"));
	pstruSendInfo->struHead.nProtocolType = atoi(GetTableFieldValue(&struCursor, "QS_PROTOCOLTYPEID"));
	pstruSendInfo->struHead.nCommandCode = atoi(GetTableFieldValue(&struCursor, "QS_COMMANDCODE"));

	strcpy(pstruSendInfo->struHead.QA, GetTableFieldValue(&struCursor, "QS_QRYNUMBER"));
	//pstruSendInfo->struHead.nObjectCount = atoi(GetTableFieldValue(&struCursor, "alm_name"));
	pstruSendInfo->struHead.nRiority = atoi(GetTableFieldValue(&struCursor, "QS_LEVEL"));
	
	pstruSendInfo->struRepeater.nCommType = atoi(GetTableFieldValue(&struCursor, "QS_COMMTYPEID"));
	pstruSendInfo->struRepeater.nRepeaterId = atol(GetTableFieldValue(&struCursor, "QS_REPEATERID"));
	pstruSendInfo->struRepeater.nDeviceId = atoi(GetTableFieldValue(&struCursor, "QS_DEVICEID"));
	strcpy(pstruSendInfo->struRepeater.szTelephoneNum, GetTableFieldValue(&struCursor, "QS_TELEPHONENUM"));
	
	strcpy(pstruSendInfo->struRepeater.szNetCenter, GetTableFieldValue(&struCursor, "QS_SERVERTELNUM"));
	pstruSendInfo->struRepeater.nProtocolDeviceType = atoi(GetTableFieldValue(&struCursor, "QS_DEVICETYPEID")); //设备类型
	strcpy(pstruSendInfo->struRepeater.szSpecialCode, GetTableFieldValue(&struCursor, "QS_OTHERDEVICEID"));  //其它标识
	strcpy(pstruSendInfo->struRepeater.szReserve, GetTableFieldValue(&struCursor, "QS_DEVICEMODELID")); //设备型号
	//增加网元编号
	pstruSendInfo->nNeId = atoi(GetTableFieldValue(&struCursor, "QS_NEID"));
	pstruSendInfo->nTaskId = atoi(GetTableFieldValue(&struCursor, "QS_TASKID"));
	pstruSendInfo->nTaskLogId = atoi(GetTableFieldValue(&struCursor, "QS_TASKLOGID"));
	
	//strcpy(szAlarmName,  TrimAllSpace(GetTableFieldValue(&struCursor, "alm_name")));
	FreeCursor(&struCursor);

	//将队列表删除
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_INT; //SQLT_INT
	struBindVar.struBind[0].VarValue.nValueInt = nMsgSerial;
	struBindVar.nVarCount++;
	sprintf(szSql,"delete from NE_MSGQUEUE where QS_ID = :v_0");
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s][%d]\n",szSql, nMsgSerial);
	if(BindExecuteSQL(szSql, &struBindVar)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();
	return NORMAL;
}


RESULT GetGprsPackageInfo(int QB, unsigned int RptId, SENDPACKAGE *pstruSendInfo)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	INT nMsgSerial;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select QS_ID,QS_PROTOCOLTYPEID,QS_COMMANDCODE, QS_QRYNUMBER,QS_LEVEL, QS_COMMTYPEID, QS_REPEATERID, QS_DEVICEID,QS_TELEPHONENUM, QS_SERVERTELNUM, QS_DEVICETYPEID, QS_OTHERDEVICEID, QS_NEID,QS_TASKID, QS_TASKLOGID, QS_IP, QS_PORT from NE_GPRSQUEUE where QS_NETFLAG = %d and QS_REPEATERID = %u",
	        QB, RptId);
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
	nMsgSerial = atoi(GetTableFieldValue(&struCursor, "QS_ID"));
	pstruSendInfo->struHead.nProtocolType = atoi(GetTableFieldValue(&struCursor, "QS_PROTOCOLTYPEID"));
	pstruSendInfo->struHead.nCommandCode = atoi(GetTableFieldValue(&struCursor, "QS_COMMANDCODE"));
	strcpy(pstruSendInfo->struHead.QA, GetTableFieldValue(&struCursor, "QS_QRYNUMBER"));
	//pstruSendInfo->struHead.nObjectCount = atoi(GetTableFieldValue(&struCursor, "alm_name"));
	pstruSendInfo->struHead.nRiority = atoi(GetTableFieldValue(&struCursor, "QS_LEVEL"));
	
	pstruSendInfo->struRepeater.nCommType = atoi(GetTableFieldValue(&struCursor, "QS_COMMTYPEID"));
	//pstruSendInfo->struRepeater.nRepeaterId = atoi(GetTableFieldValue(&struCursor, "QS_REPEATERID"));
	//pstruSendInfo->struRepeater.nDeviceId = atoi(GetTableFieldValue(&struCursor, "QS_DEVICEID"));
	strcpy(pstruSendInfo->struRepeater.szTelephoneNum, GetTableFieldValue(&struCursor, "QS_TELEPHONENUM"));
	strcpy(pstruSendInfo->struRepeater.szNetCenter, GetTableFieldValue(&struCursor, "QS_SERVERTELNUM"));
	pstruSendInfo->struRepeater.nProtocolDeviceType = atoi(GetTableFieldValue(&struCursor, "QS_DEVICETYPEID")); //设备类型
	strcpy(pstruSendInfo->struRepeater.szSpecialCode, GetTableFieldValue(&struCursor, "QS_OTHERDEVICEID"));  //其它标识
	strcpy(pstruSendInfo->struRepeater.szReserve, GetTableFieldValue(&struCursor, "QS_DEVICEMODELID")); //设备型号
	
	strcpy(pstruSendInfo->struRepeater.szIP, GetTableFieldValue(&struCursor, "QS_IP"));
	pstruSendInfo->struRepeater.nPort = atoi(GetTableFieldValue(&struCursor, "QS_PORT")); 
	
	//增加网元编号
	pstruSendInfo->nNeId = atoi(GetTableFieldValue(&struCursor, "QS_NEID"));
	pstruSendInfo->nTaskId = atoi(GetTableFieldValue(&struCursor, "QS_TASKID"));
	pstruSendInfo->nTaskLogId = atoi(GetTableFieldValue(&struCursor, "QS_TASKLOGID"));
	
	//strcpy(szAlarmName,  TrimAllSpace(GetTableFieldValue(&struCursor, "alm_name")));
	FreeCursor(&struCursor);
	
	//将队列表删除
	sprintf(szSql,"delete from NE_GPRSQUEUE where QS_ID=%d",nMsgSerial);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();
	return NORMAL;
}


RESULT GetSnmpPackageInfo(int nMsgId, SENDPACKAGE *pstruSendInfo)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	INT nMsgSerial;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select QS_ID,QS_PROTOCOLTYPEID,QS_COMMANDCODE, QS_QRYNUMBER,QS_LEVEL, QS_COMMTYPEID, QS_REPEATERID, QS_DEVICEID,QS_TELEPHONENUM, QS_SERVERTELNUM, QS_DEVICETYPEID, QS_OTHERDEVICEID, QS_NEID,QS_TASKID, QS_TASKLOGID, QS_IP, QS_PORT from NE_SNMPQUEUE where QS_ID = %d ", nMsgId);
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
	nMsgSerial = atoi(GetTableFieldValue(&struCursor, "QS_ID"));
	pstruSendInfo->struHead.nProtocolType = atoi(GetTableFieldValue(&struCursor, "QS_PROTOCOLTYPEID"));
	pstruSendInfo->struHead.nCommandCode = atoi(GetTableFieldValue(&struCursor, "QS_COMMANDCODE"));
	strcpy(pstruSendInfo->struHead.QA, GetTableFieldValue(&struCursor, "QS_QRYNUMBER"));
	//pstruSendInfo->struHead.nObjectCount = atoi(GetTableFieldValue(&struCursor, "alm_name"));
	pstruSendInfo->struHead.nRiority = atoi(GetTableFieldValue(&struCursor, "QS_LEVEL"));
	
	pstruSendInfo->struRepeater.nCommType = atoi(GetTableFieldValue(&struCursor, "QS_COMMTYPEID"));
	pstruSendInfo->struRepeater.nRepeaterId = atol(GetTableFieldValue(&struCursor, "QS_REPEATERID"));
	pstruSendInfo->struRepeater.nDeviceId = atoi(GetTableFieldValue(&struCursor, "QS_DEVICEID"));
	strcpy(pstruSendInfo->struRepeater.szTelephoneNum, GetTableFieldValue(&struCursor, "QS_TELEPHONENUM"));
	strcpy(pstruSendInfo->struRepeater.szNetCenter, GetTableFieldValue(&struCursor, "QS_SERVERTELNUM"));
	pstruSendInfo->struRepeater.nProtocolDeviceType = atoi(GetTableFieldValue(&struCursor, "QS_DEVICETYPEID")); //设备类型
	strcpy(pstruSendInfo->struRepeater.szSpecialCode, GetTableFieldValue(&struCursor, "QS_OTHERDEVICEID"));  //其它标识
	strcpy(pstruSendInfo->struRepeater.szReserve, GetTableFieldValue(&struCursor, "QS_DEVICEMODELID")); //设备型号
	
	strcpy(pstruSendInfo->struRepeater.szIP, GetTableFieldValue(&struCursor, "QS_IP"));
	pstruSendInfo->struRepeater.nPort = atoi(GetTableFieldValue(&struCursor, "QS_PORT")); 
	
	//增加网元编号
	pstruSendInfo->nNeId = atoi(GetTableFieldValue(&struCursor, "QS_NEID"));
	pstruSendInfo->nTaskId = atoi(GetTableFieldValue(&struCursor, "QS_TASKID"));
	pstruSendInfo->nTaskLogId = atoi(GetTableFieldValue(&struCursor, "QS_TASKLOGID"));
	
	//strcpy(szAlarmName,  TrimAllSpace(GetTableFieldValue(&struCursor, "alm_name")));
	FreeCursor(&struCursor);
	
	//将队列表删除
	
	sprintf(szSql,"delete from NE_SNMPQUEUE where QS_ID=%d",nMsgSerial);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(ExecuteSQL(szSql)!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败,错误信息[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();
	
	return NORMAL;
}


/*
 * 从系统配置表中获取系统信息
 */
RESULT GetSysParameter(PCSTR pszArgSql,PSTR pszArgValue)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;

	sprintf(szSql,"select PAR_KEYVALUE from SYS_PARAMETER where %s", pszArgSql);
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
		if(GetSQLErrorNo()==NO_FOUND_RECORD)
		{
			PrintErrorLog(DBG_HERE,"系统参数[%s]不存在.请配置\n",pszArgSql);
		}
		else
		{
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s] 失败[%s]\n",szSql,GetSQLErrorMessage());
		}
		FreeCursor(&struCursor);
		strcpy(pszArgValue, "");
		return EXCEPTION;
	}
	strcpy(pszArgValue,TrimAllSpace(GetTableFieldValue(&struCursor,"PAR_KEYVALUE")));
	FreeCursor(&struCursor);

	return NORMAL;
}

	


/*
 * 从系统配置表中获取条数
 */
INT GetSysParameterRow(PCSTR pszArgSql)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	INT nCount;

	sprintf(szSql,"select count(*) as v_count from SYS_PARAMETER where '%s' ", pszArgSql);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s] 失败[%s]\n",szSql,GetSQLErrorMessage());
		FreeCursor(&struCursor);
		return EXCEPTION;
	}
	nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
	FreeCursor(&struCursor);

	return nCount;
}

/*
 * 保存上报信息日志
 */
RESULT SaveToMaintainLog(PSTR pszStyle, PSTR pszMemo, SENDPACKAGE *pstruSendInfo)
{
    char szSql[MAX_BUFFER_LEN];
    UINT nMaintainLogId;
    
    if (strcmp(getenv("DATABASE"), "mysql") == 0)
	{
		if(GetMySqlSequence(&nMaintainLogId, "man_maintainlog")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE,"获取查询设置流水号错误\n");
			return EXCEPTION;
		}
	}
	else
	{
		if(GetDbSequence(&nMaintainLogId, "SEQ_MAINTAINLOG")!=NORMAL)
		{
			PrintErrorLog(DBG_HERE, "获取查询设置流水号错误\n");
			return EXCEPTION;
		}
	}
	pstruSendInfo->nMaintainLogId = nMaintainLogId;
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), 
       "insert into  man_MaintainLog ( mnt_RepeaterId,mnt_DeviceId,mnt_EventTime,mnt_Style,mnt_SIMCard,mnt_Number,mnt_NeId,mnt_CommTypeId,mnt_CenterTel,mnt_ProtocoltypeId,mnt_Memo) "
       "values ( %u, %d,to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s','%s','%s', %d, %d,'%s',%d, '%s') ",
       //pstruSendInfo->nMaintainLogId,  
       pstruSendInfo->struRepeater.nRepeaterId,
       pstruSendInfo->struRepeater.nDeviceId,
       GetSysDateTime(),
       pszStyle,
       pstruSendInfo->struRepeater.szTelephoneNum,
       pstruSendInfo->struHead.QA,
       
       pstruSendInfo->nNeId,
       pstruSendInfo->struRepeater.nCommType,
       pstruSendInfo->struRepeater.szNetCenter,
       pstruSendInfo->struHead.nProtocolType,
       pszMemo
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
 * 保存上报信息日志
 */
RESULT SaveToMaintainLog2(PSTR pszStyle, INT nNeId, COMMANDHEAD *pstruHead, REPEATER_INFO *pstruRepeater)
{
    char szSql[MAX_BUFFER_LEN];
    UINT nMaintainLogId;
    
    if(GetDbSerial(&nMaintainLogId, "man_MaintainLog")!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"获取告警上报流水号错误\n");
		return EXCEPTION;
	}
	//pstruSendInfo->nMaintainLogId = nMaintainLogId;
	
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), 
       "insert into  man_MaintainLog (mnt_MaintainLogId,mnt_RepeaterId,mnt_DeviceId,mnt_EventTime,mnt_Style,mnt_SIMCard,mnt_Number,mnt_NeId,mnt_CommTypeId,mnt_CenterTel,mnt_ProtocoltypeId,mnt_Memo) "
       "values (%d, %d, %d,to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s','%s','%s', %d, %d,'%s',%d, '%s') ",
       nMaintainLogId,  
       pstruRepeater->nRepeaterId,
       pstruRepeater->nDeviceId,
       GetSysDateTime(),
       pszStyle,
       pstruRepeater->szTelephoneNum,
       pstruHead->QA,
       
       nNeId,
       pstruRepeater->nCommType,
       pstruRepeater->szNetCenter,
       pstruHead->nProtocolType,
       ""
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
 * 从GPRS请求开站上报取列表信息
 */
RESULT GetPackInfoFromMainLog(SENDPACKAGE *pstruSendInfo)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	
	sprintf(szSql,"select mnt_simcard, mnt_neid, mnt_centertel from man_maintainlog where mnt_repeaterid = %u and mnt_deviceid = %d and mnt_style = '%s'", pstruSendInfo->struRepeater.nRepeaterId, pstruSendInfo->struRepeater.nDeviceId, "GPRS请求开站上报");
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
        PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", \
					  szSql, GetSQLErrorMessage());
        FreeCursor(&struCursor);
		return EXCEPTION;
	}
	//服务号码
	int nTemp = strlen(TrimAllSpace(GetTableFieldValue(&struCursor,"mnt_centertel")));
	memcpy(pstruSendInfo->struRepeater.szNetCenter, TrimAllSpace(GetTableFieldValue(&struCursor,"mnt_centertel")), nTemp);
	//上报设备手机号
	memcpy(pstruSendInfo->struRepeater.szTelephoneNum, TrimAllSpace(GetTableFieldValue(&struCursor,"mnt_simcard")), 11);
	pstruSendInfo->nNeId = atoi(GetTableFieldValue(&struCursor,"mnt_neid"));

	FreeCursor(&struCursor);
	
	return NORMAL;
}

/*
 * 从协议命令监控量列表命令和对象列表
 */
RESULT GetNeCmdObjects(int nProtocolTypeId,int nProtocolDeviceTypeId, PSTR pszCmdCode, PSTR pszCmdObject)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	STR szCmdCode[4000];
	STR szCmdObject[4000];
    
    bufclr(szCmdCode);
    bufclr(szCmdObject);
	sprintf(szSql,"select cdo_CommandCode, cdo_Objects from ne_CommandObjects where cdo_ProtocolType = %d and cdo_ProtocolDeviceType = %d and cdo_spooltype = 'fast'", nProtocolTypeId, nProtocolDeviceTypeId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while(FetchCursor(&struCursor) == NORMAL)
	{
        sprintf(szCmdCode,"%s%s,", szCmdCode, TrimAllSpace(GetTableFieldValue(&struCursor,"cdo_CommandCode")));
        sprintf(szCmdObject,"%s%s,", szCmdCode, TrimAllSpace(GetTableFieldValue(&struCursor,"cdo_Objects")));
	}
	
	FreeCursor(&struCursor);
    
    strcpy(pszCmdCode, szCmdCode);
    strcpy(pszCmdObject, szCmdObject);
	return NORMAL;
}

/*
 * 从ne_ObjectsList表中取对象类别
 */
RESULT GetNeObjectList(INT nNeId, PSTR pszNeObjectList, PSTR pszNeDataList)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	STR szNeObjectList[MAX_BUFFER_LEN];
    STR szNeDataList[MAX_BUFFER_LEN];
    
    bufclr(szSql);
	snprintf(szSql, sizeof(szSql), 
		"select epm_objid, epm_curvalue from ne_elementparam where epm_neid = %d and epm_objid in (%s) and epm_curvalue != ' '", nNeId,  pszNeObjectList);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while(FetchCursor(&struCursor) == NORMAL)
	{
		sprintf(szNeObjectList,"%s%s,",  szNeObjectList, TrimAllSpace(GetTableFieldValue(&struCursor,"epm_objid")));
		sprintf(szNeDataList,"%s%s,",  szNeDataList, TrimAllSpace(GetTableFieldValue(&struCursor,"epm_curvalue")));
	}
	FreeCursor(&struCursor);
	
	if (strlen(szNeObjectList) == 0)
		return EXCEPTION;
	
	
    bufclr(szSql);
	snprintf(szSql, sizeof(szSql), 
		"select smg_gprsapn, smg_GprsUserName, smg_GprsPassword, smg_Gprs_TimerTestEnable, smg_TimerTestTime, smg_TimerTestCount, smg_TimerTestInterval, smg_AttachEnable,"
		"smg_AttachOverTime, smg_PdpEnable, smg_PdpOverTime, smg_PingEnable, smg_PingOverTime, smg_PingAddress, smg_FtpUploadEnable, smg_FtpDownloadEnable, smg_FtpOverTime, "
		"smg_FtpUploadPath, smg_FtpDownloadPath, smg_FtpUploadGprsFile, smg_FtpDownloadEdgeFile, smg_FtpIp, smg_FtpPort, smg_FtpUserName, smg_FtpPassword, smg_FtpUpdatePath,smg_FtpUpdateFile "
	    " from ne_signermonitorgprs where smg_neid = %d",  nNeId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		FreeCursor(&struCursor);
		return EXCEPTION;
        
	}
	sprintf(szNeObjectList,"%s%s",  szNeObjectList, "0133,0136,0137,07B2,07B3,07B4,07B5,07B6,07B7,07B8,07B9,07BA,07BB,07BC,07BD,07BE,07BF,07C0,07C1,07C2,07C3,0160,0161,0162,0163,0164,0165");
	sprintf(szNeDataList,"%s%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",  szNeDataList, 
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_gprsapn")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_GprsUserName")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_GprsPassword")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_Gprs_TimerTestEnable")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_TimerTestTime")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_TimerTestCount")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_TimerTestInterval")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_AttachEnable")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_AttachOverTime")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_PdpEnable")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_PdpOverTime")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_PingEnable")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_PingOverTime")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_PingAddress")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpUploadEnable")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpDownloadEnable")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpOverTime")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpUploadPath")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpDownloadPath")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpUploadGprsFile")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpDownloadEdgeFile")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpIp")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpPort")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpUserName")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpPassword")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpUpdatePath")),
				TrimAllSpace(GetTableFieldValue(&struCursor,"smg_FtpUpdateFile"))
				);
	FreeCursor(&struCursor);
	
	
    strcpy(pszNeObjectList, szNeObjectList);
    strcpy(pszNeDataList, szNeDataList);
	return NORMAL;
}


//设置任务为正在轮询状态
RESULT SetTaskUsing(int nTaskId)
{
    char szSql[MAX_BUFFER_LEN];
     
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "update man_Task set  tsk_state=0 where tsk_Taskid= %d", nTaskId);
     
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

RESULT SaveToDasList(UINT nRepeaterId, int nDeviceId, PSTR pszRouter, PSTR pszDeviceIp, int nConnStat, int nDeviceTypeId, PSTR pszAddrInfo)
{
    char szSql[MAX_BUFFER_LEN];
     
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "insert into ne_daslist (repead_id, device_id, route_addr, ip_addr, conn_stat, device_typeid, addr_info) values (%u, %d, '%s', '%s', %d, %d, '%s')", 
     nRepeaterId, nDeviceId, pszRouter, pszDeviceIp, nConnStat, nDeviceTypeId, pszAddrInfo);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	
	return NORMAL; 
}

RESULT DeleDasList(UINT nRepeaterId)
{
    char szSql[MAX_BUFFER_LEN];
     
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "delete from ne_daslist where repead_id = %u", nRepeaterId);
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

RESULT DeleRfidList(UINT nRepeaterId, int nDeviceId)
{
    char szSql[MAX_BUFFER_LEN];
     
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "delete from ne_rfidlist where repead_id = %u and device_id = %d", nRepeaterId, nDeviceId);
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


RESULT SaveToRfidList(UINT nRepeaterId, int nDeviceId, PSTR pszRouter, int nConnStat, int nDeviceTypeId)
{
    char szSql[MAX_BUFFER_LEN];
     
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "insert into ne_rfidlist (repead_id, device_id, route_addr, conn_stat, device_typeid ) values (%u, %d, '%s', %d, %d)", 
     nRepeaterId, nDeviceId, pszRouter, nConnStat, nDeviceTypeId);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	
	return NORMAL; 
}


//记录man_TaskLog表日志
RESULT InsertTaskLog(int nTaskId, int *pnTaskLogId, PSTR pszStyle)
{
    char szSql[MAX_BUFFER_LEN];
    UINT nTaskLogId;
    
    if(GetDbSerial(&nTaskLogId, "TaskLogid")!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"获取告警上报流水号错误\n");
		return EXCEPTION;
	}
     
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "insert into man_TaskLog(tkl_TaskLogid,tkl_Taskid,tkl_Begintime, tkl_UserId,tkl_Style) values(%d, %d, to_date('%s','yyyy-mm-dd hh24:mi:ss'), %d, '%s')",
         nTaskLogId, nTaskId, GetSysDateTime(),  -1,  pszStyle);
     
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	*pnTaskLogId = nTaskLogId;
	return NORMAL; 
}

//更新tasklog表发送条数和网元条数
RESULT UpdateTaskLogCount(int nTaskLogId, int nTaskId, int nEleCount, int nTxPackCount, int nFailEleCount)
{
    char szSql[MAX_BUFFER_LEN];
	TBINDVARSTRU struBindVar;
		
	memset(&struBindVar, 0, sizeof(struBindVar));
	struBindVar.struBind[0].nVarType = SQLT_INT; //SQLT_INT
	struBindVar.struBind[0].VarValue.nValueInt = nTxPackCount;
	struBindVar.nVarCount++;
	
	struBindVar.struBind[1].nVarType = SQLT_INT; //SQLT_INT
	struBindVar.struBind[1].VarValue.nValueInt = nEleCount;
	struBindVar.nVarCount++;
	
	struBindVar.struBind[2].nVarType = SQLT_INT; //SQLT_INT
	struBindVar.struBind[2].VarValue.nValueInt = nFailEleCount;
	struBindVar.nVarCount++;
	
	struBindVar.struBind[3].nVarType = SQLT_INT; //SQLT_INT
	struBindVar.struBind[3].VarValue.nValueInt = nTaskLogId;
	struBindVar.nVarCount++;
	
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "update man_TaskLog set tkl_TxPackCount=tkl_TxPackCount + %d, tkl_EleCount=tkl_EleCount + %d, tkl_EleUnusualCount=tkl_EleUnusualCount + %d where tkl_TaskLogid = %d", 
    	nTxPackCount, nEleCount, nFailEleCount, nTaskLogId);
     
    PrintDebugLog(DBG_HERE, "Excute SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
    CommitTransaction();
    /*
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "insert into ne_msgqueue select * from tt_msgqueue where qs_taskid = %d", nTaskId);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
    CommitTransaction();
    
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "delete from tt_msgqueue where qs_taskid = %d", nTaskId);
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
    CommitTransaction();
    */
	return NORMAL; 
}




//分解对象数组用空格分开，并校验监控对象是否存在
RESULT ResolveQryParamArrayGprs(PSTR pszQryEleParam)
{
    STR szQryEleParam[MAX_BUFFER_LEN];
    STR szTempParam[MAX_BUFFER_LEN];
    PSTR pszParamStr[MAX_MAPOBJECT_NUM];
	INT nSeperateNum, i;
	INT nTempLen=0;
	STR szDataType[20];
	int nDataLen;
	int nPackLen=17;
	
	memset(szQryEleParam, 0, sizeof(szQryEleParam));
	memset(szTempParam, 0, sizeof(szTempParam));
	strcpy(szQryEleParam, pszQryEleParam);
    nSeperateNum = SeperateString(szQryEleParam,  ',', pszParamStr, MAX_MAPOBJECT_NUM);
    for(i=0; i< nSeperateNum; i++)
	{

        GetMapIdFromCache(pszParamStr[i], szDataType, &nDataLen);
        if (nDataLen == 0) continue;

        nTempLen+=nDataLen+3; 
        
        if (getenv("BANGXUN")!=NULL && nTempLen >= atoi(getenv("BANGXUN"))-nPackLen)
        {
            sprintf(szTempParam,"%s|%s,",  szTempParam, pszParamStr[i]);
            nTempLen = 0;
        }
        else if (nTempLen >= 1500-nPackLen)
        
        //if (nTempLen >= 1500)
        {
            sprintf(szTempParam,"%s|%s,",  szTempParam, pszParamStr[i]);
            nTempLen = 0;
        }
        else
        {
            sprintf(szTempParam,"%s%s,",  szTempParam, pszParamStr[i]);
        }
    }
    memset(pszQryEleParam, 0, MAX_BUFFER_LEN);
    strcpy(pszQryEleParam, TrimRightChar(szTempParam, ','));
    
    return NORMAL;
}


//分解对象数组用空格分开，并校验监控对象是否存在
RESULT ResolveQryParamArray(PSTR pszQryEleParam)
{
    STR szQryEleParam[MAX_BUFFER_LEN];
    STR szTempParam[MAX_BUFFER_LEN];
    PSTR pszParamStr[MAX_MAPOBJECT_NUM];
	INT nSeperateNum, i;
	INT nTempLen=0;
	STR szDataType[20];
	int nDataLen;
	
	memset(szQryEleParam, 0, sizeof(szQryEleParam));
	memset(szTempParam, 0, sizeof(szTempParam));
	strcpy(szQryEleParam, pszQryEleParam);

    nSeperateNum = SeperateString(szQryEleParam,  ',', pszParamStr, MAX_MAPOBJECT_NUM);
    for(i=0; i< nSeperateNum; i++)
	{

        GetMapIdFromCache(pszParamStr[i], szDataType, &nDataLen);
        if (nDataLen == 0) continue;
        if (strlen(pszParamStr[i]) == 8) //4位字节
        	nTempLen+=nDataLen+5;
        else 
        	nTempLen+=nDataLen+3; 
        if (nTempLen > 50)
        {
        	TrimRightChar(szTempParam, ',');
            sprintf(szTempParam,"%s|%s,",  szTempParam, pszParamStr[i]);
            nTempLen = 0;
            nTempLen+=nDataLen+3;
        }
        else
        {
            sprintf(szTempParam,"%s%s,",  szTempParam, pszParamStr[i]);
        }
    }
    memset(pszQryEleParam, 0, MAX_BUFFER_LEN);
    strcpy(pszQryEleParam, TrimRightChar(szTempParam, ','));
//PrintDebugLog(DBG_HERE, "[%s]\n", pszQryEleParam);       
    return NORMAL;
}

RESULT ResolveSetParamArray(PSTR pszSetEleParam, PSTR pszSetEleValue)
{
    STR szTempParam[MAX_BUFFER_LEN];
    STR szTempValue[MAX_BUFFER_LEN];
    PSTR pszParamStr[MAX_MAPOBJECT_NUM];
    PSTR pszValueStr[MAX_MAPOBJECT_NUM];
	INT nSeperateNum, i;
	INT nTempLen=0;
	STR szDataType[20];
	int nDataLen;

	memset(szTempParam, 0, sizeof(szTempParam));
	memset(szTempValue, 0, sizeof(szTempValue));
		
    nSeperateNum = SeperateString(pszSetEleParam,  ',', pszParamStr, MAX_MAPOBJECT_NUM);
    nSeperateNum = SeperateString(pszSetEleValue,  ',', pszValueStr, MAX_MAPOBJECT_NUM);
    for(i=0; i< nSeperateNum; i++)
	{

        GetMapIdFromCache(pszParamStr[i], szDataType, &nDataLen);
        if (nDataLen == 0) continue;
        if (strlen(pszParamStr[i]) == 8)
        	nTempLen+=nDataLen+5;
        else
        	nTempLen+=nDataLen+3; 
        if (nTempLen > 50)
        {
        	TrimRightChar(szTempParam, ',');
        	TrimRightOneChar(szTempValue, ',');
            sprintf(szTempParam,"%s|%s,",  szTempParam, pszParamStr[i]);
            sprintf(szTempValue,"%s|%s,",  szTempValue, pszValueStr[i]);
            nTempLen = 0;
            nTempLen+=nDataLen+3; 
        }
        else
        {
            sprintf(szTempParam,"%s%s,",  szTempParam, pszParamStr[i]);
            sprintf(szTempValue,"%s%s,",  szTempValue, pszValueStr[i]);
        }
    }
    memset(pszSetEleParam, 0, MAX_BUFFER_LEN);
    strcpy(pszSetEleParam, TrimRightChar(szTempParam, ','));
    
    memset(pszSetEleValue, 0, MAX_BUFFER_LEN);
    strcpy(pszSetEleValue, TrimRightOneChar(szTempValue, ','));
    
    return NORMAL;
}


RESULT ResolveSetParamArrayGprs(PSTR pszSetEleParam, PSTR pszSetEleValue)
{
    STR szTempParam[MAX_BUFFER_LEN];
    STR szTempValue[MAX_BUFFER_LEN];
    PSTR pszParamStr[MAX_MAPOBJECT_NUM];
    PSTR pszValueStr[MAX_MAPOBJECT_NUM];
	INT nSeperateNum, i;
	INT nTempLen=0;
	STR szDataType[20];
	int nDataLen;

	memset(szTempParam, 0, sizeof(szTempParam));
	memset(szTempValue, 0, sizeof(szTempValue));
		
    nSeperateNum = SeperateString(pszSetEleParam,  ',', pszParamStr, MAX_MAPOBJECT_NUM);
    nSeperateNum = SeperateString(pszSetEleValue,  ',', pszValueStr, MAX_MAPOBJECT_NUM);
    for(i=0; i< nSeperateNum; i++)
	{

        GetMapIdFromCache(pszParamStr[i], szDataType, &nDataLen);
        if (nDataLen == 0) continue;
        
        nTempLen+=nDataLen+3; 
        if (nTempLen > 512)
        {
        	TrimRightChar(szTempParam, ',');
        	TrimRightOneChar(szTempValue, ',');
            sprintf(szTempParam,"%s|%s,",  szTempParam, pszParamStr[i]);
            sprintf(szTempValue,"%s|%s,",  szTempValue, pszValueStr[i]);
            nTempLen = 0;
            nTempLen+=nDataLen+3; 
        }
        else
        {
            sprintf(szTempParam,"%s%s,",  szTempParam, pszParamStr[i]);
            sprintf(szTempValue,"%s%s,",  szTempValue, pszValueStr[i]);
        }
    }
    memset(pszSetEleParam, 0, MAX_BUFFER_LEN);
    strcpy(pszSetEleParam, TrimRightChar(szTempParam, ','));
    
    memset(pszSetEleValue, 0, MAX_BUFFER_LEN);
    strcpy(pszSetEleValue, TrimRightOneChar(szTempValue, ','));
    
    return NORMAL;
}


//记录man_tasklogunsendneid表日志
RESULT InsertFailNeid(int nTaskLogId, int nNeId, PSTR pszReason, PSTR pszEleState)
{
    char szSql[MAX_BUFFER_LEN];
    UINT nTluId;
    
    if(GetDbSerial(&nTluId, "man_tasklogunsendneid")!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"获取告警上报流水号错误\n");
		return EXCEPTION;
	}
    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "insert into man_tasklogunsendneid(tlu_id,tlu_tasklogid,tlu_neid,tlu_reason,tlu_elestate) values(%d, %d, %d, '%s', '%s')",
         nTluId, nTaskLogId, nNeId, pszReason,  pszEleState);
     
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

//取设备状态
PSTR GetDeviceStatu(int nDeviceStatus)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	static STR szEleState[100];
	
	sprintf(szSql,"select ds_name  from ne_DeviceStatusId where ds_id = %d", nDeviceStatus);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return "";
	}
	if (FetchCursor(&struCursor) == NORMAL)
	{
        strcpy(szEleState, TrimAllSpace(GetTableFieldValue(&struCursor,"ds_name")));
	}
	
	FreeCursor(&struCursor);
	return szEleState;
}
//判断任务是否到结束时间
BOOL getTaskStopUsing(int nTaskId)
{
    STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	STR szEleState[10];
	
	sprintf(szSql,"select tsk_state,tsk_isuse from man_Task where tsk_Taskid = %d", nTaskId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return BOOLFALSE;
	}
	if (FetchCursor(&struCursor) == NORMAL)
	{
        strcpy(szEleState, TrimAllSpace(GetTableFieldValue(&struCursor,"tsk_state")));
	}
	FreeCursor(&struCursor);
	if (atoi(szEleState) == 0)//正在运行
	    return BOOLFALSE;
	else
	    return BOOLTRUE;
}


//保存到ne_maplist表中
RESULT SaveToMapList(PSTR pszQrySerial, PSTR pszMapId)
{
    char szSql[MAX_BUFFER_LEN];

    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "insert into  ne_maplist (qs_qrynumber,qs_mapid) values ('%s', '%s')",
         pszQrySerial, pszMapId);
     
    //PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		//PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",	szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
    
	return NORMAL; 
}

RESULT SaveToMap0001List(PSTR pszQrySerial, PSTR pszMapId, PSTR pszMap0001)
{
    char szSql[MAX_BUFFER_LEN];

    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "insert into  ne_maplist (qs_qrynumber,qs_mapid, qs_mcpid) values ('%s', '%s', '%s')",
         pszQrySerial, pszMapId, pszMap0001);
     
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		//PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",	szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
    
	return NORMAL; 
}

RESULT GetMcpIdFromParam(INT nNeId, PSTR pszMapId,PSTR pszMcpId)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;

	sprintf(szSql, "select epm_McpId from ne_ElementParam where epm_NeId = %d and epm_objid ='%s'", nNeId, pszMapId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s] 失败[%s]\n",szSql,GetSQLErrorMessage());
		FreeCursor(&struCursor);
		return EXCEPTION;
	}
	strcpy(pszMcpId,TrimAllSpace(GetTableFieldValue(&struCursor,"epm_McpId")));
	FreeCursor(&struCursor);

	return NORMAL;
}


RESULT GetMapIdFromParam(UINT nRepeaterId, PSTR pszMcpId, PSTR pszMapId)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;

	sprintf(szSql, "SELECT epm_ObjId FROM ne_ElementParam a JOIN ne_element b ON a.EPM_NEID = b.NE_NEID WHERE a.epm_mcpid ='%s' AND b.NE_REPEATERID = %u",
	  pszMcpId, nRepeaterId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
		//PrintErrorLog(DBG_HERE,"执行SQL语句[%s] 失败[%s]\n",szSql,GetSQLErrorMessage());
		FreeCursor(&struCursor);
		return EXCEPTION;
	}
	strcpy(pszMapId,TrimAllSpace(GetTableFieldValue(&struCursor,"epm_ObjId")));
	FreeCursor(&struCursor);

	return NORMAL;
}



//保存到ne_maplist表中
RESULT GetMapId0009List(PSTR pszQrySerial, PSTR pszMapId0009List)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
	STR szMapId0009List[MAX_BUFFER_LEN*2];
	STR szMapId[10], szDataType[20];
	INT nDataLen;
    
    bufclr(szMapId0009List);
	sprintf(szSql,"select qs_mapid  from ne_maplist where qs_qrynumber = '%s'", pszQrySerial);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while(FetchCursor(&struCursor) == NORMAL)
	{
	    strcpy(szMapId, TrimAllSpace(GetTableFieldValue(&struCursor,"qs_mapid")));
	    if (GetMapIdFromCache(szMapId, szDataType, &nDataLen) != NORMAL)
        {
            PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", szMapId);
		    continue;
        }
        if (strlen(szMapId0009List)+ 10 < MAX_BUFFER_LEN*2)
        sprintf(szMapId0009List,"%s%s,",  szMapId0009List, szMapId);
	}
	
	FreeCursor(&struCursor);
	TrimRightChar(szMapId0009List, ',');
	if (strlen(szMapId0009List) == 0)
		return EXCEPTION;
    strcpy(pszMapId0009List, szMapId0009List);
    
	/* 
	 * 防止重复发 2010.6.29
	 
    memset(szSql, 0, sizeof(szSql));
    sprintf(szSql,"delete from ne_maplist where qs_qrynumber = '%s'", pszQrySerial);
     
    PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	
    CommitTransaction();
    */
    
	return NORMAL; 
}

RESULT GetJinXinMapId0009List(PSTR pszQrySerial, PSTR pszMapId0009List)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
	STR szMapId0009List[MAX_BUFFER_LEN*2];
	STR szMapId[10], szDataType[20];
	INT nDataLen;
    
    bufclr(szMapId0009List);
	sprintf(szSql,"select qs_mapid  from ne_maplist where qs_qrynumber = '%s' and qs_mapid not in (select qs_mapid from ne_jinxin)", pszQrySerial);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while(FetchCursor(&struCursor) == NORMAL)
	{
	    strcpy(szMapId, TrimAllSpace(GetTableFieldValue(&struCursor,"qs_mapid")));
	    if (GetMapIdFromCache(szMapId, szDataType, &nDataLen) != NORMAL)
        {
            PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", szMapId);
		    continue;
        }
        if (strlen(szMapId0009List)+ 10 < MAX_BUFFER_LEN*2)
        sprintf(szMapId0009List,"%s%s,",  szMapId0009List, szMapId);
	}
	
	FreeCursor(&struCursor);
	TrimRightChar(szMapId0009List, ',');
	if (strlen(szMapId0009List) == 0)
		return EXCEPTION;
    strcpy(pszMapId0009List, szMapId0009List);
    	    
	return NORMAL; 
}

RESULT GetMapId0001List(PSTR pszQrySerial, PSTR pszMapId0001List, PSTR pszMcpIdList)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
	STR szMapId0001List[MAX_BUFFER_LEN*2], szMcpIdList[MAX_BUFFER_LEN];
	STR szMapId[10], szMcpId[10], szDataType[20];
	INT nDataLen;
    
    bufclr(szMapId0001List);
    bufclr(szMcpIdList);
	sprintf(szSql,"select qs_mapid, qs_mcpid  from ne_maplist where qs_qrynumber = '%s'", pszQrySerial);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while(FetchCursor(&struCursor) == NORMAL)
	{
		//映射id
		strcpy(szMcpId, GetTableFieldValue(&struCursor,"qs_mcpid"));
	    strcpy(szMapId, GetTableFieldValue(&struCursor,"qs_mapid"));
	    
	    if (GetMapIdFromCache(szMapId, szDataType, &nDataLen) != NORMAL)
        {
            PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", szMapId);
		    continue;
        }
        
        sprintf(szMapId0001List,"%s%s,",  szMapId0001List, szMapId);
        if (strlen(szMcpId) > 0)
        	sprintf(szMcpIdList,"%s%s,",  szMcpIdList, szMcpId);
        else
        	sprintf(szMcpIdList,"%s%s,",  szMcpIdList, szMapId);
	}
	
	FreeCursor(&struCursor);
	
	TrimRightChar(szMapId0001List, ',');
	TrimRightChar(szMcpIdList, ',');
	if (strlen(szMapId0001List) == 0)
		return EXCEPTION;
    strcpy(pszMapId0001List, szMapId0001List);
    strcpy(pszMcpIdList, szMcpIdList);
    
	/* 
	 * 防止重复发 2010.6.29
	 */
    memset(szSql, 0, sizeof(szSql));
    sprintf(szSql,"delete from ne_maplist where qs_qrynumber = '%s'", pszQrySerial);
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

RESULT GetDasMapIdList(int nDeviceTypeId, PSTR pszMapId0009List)
{
    char szSql[MAX_BUFFER_LEN];
    CURSORSTRU struCursor;
	STR szMapId0009List[4000];
	STR szMapId[10], szDataType[20];
	INT nDataLen;
    
    bufclr(szMapId0009List);
	sprintf(szSql,"SELECT dvo_objid FROM ne_DeviceObjects WHERE  dvo_DeviceTypeId = %d", nDeviceTypeId);
	PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	while(FetchCursor(&struCursor) == NORMAL)
	{
	    strcpy(szMapId, TrimAllSpace(GetTableFieldValue(&struCursor,"dvo_objid")));
	    
	    if (GetMapIdFromCache(szMapId, szDataType, &nDataLen) != NORMAL)
        {
            PrintErrorLog(DBG_HERE, "系统不存在该[%s]监控量\n", szMapId);
		    continue;
        }
        sprintf(szMapId0009List,"%s%s,",  szMapId0009List, szMapId);
	}
	
	FreeCursor(&struCursor);
	TrimRightChar(szMapId0009List, ',');
	if (strlen(szMapId0009List) == 0)
		return EXCEPTION;
    strcpy(pszMapId0009List, szMapId0009List);
        
	return NORMAL; 
}

RESULT RecordAgentRecNum()
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	STR szCfgSeg[100];
	STR szTemp[MAX_STR_LEN];
	INT nProcCount, i, nMsgCount=0;
	
	memset(szSql, 0, sizeof(szSql));
    sprintf(szSql,"delete from ne_queuenum");
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	
	if (GetCfgItem("cmpp.cfg","CMPP程序","最大并发数",szTemp) != NORMAL)
        return EXCEPTION;
    nProcCount = atoi(szTemp);
    for(i=1; i< nProcCount+1; i++)
    {
    	sprintf(szCfgSeg, "CMPP进程%d", i);
    	if (GetCfgItem("cmpp.cfg", szCfgSeg, "CMPP服务号码",szTemp) != NORMAL)
        	return EXCEPTION;
        sprintf(szSql,"select count(*) as msgcount from ne_msgqueue where qs_servertelnum='%s' and qs_msgstat='0'", szTemp);
		PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
		if(SelectTableRecord(szSql,&struCursor) != NORMAL)
		{
			PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
			return BOOLFALSE;
		}
		if (FetchCursor(&struCursor) == NORMAL)
		{
    	    nMsgCount=atoi(GetTableFieldValue(&struCursor,"msgcount"));
		}
		FreeCursor(&struCursor);
		
		memset(szSql, 0, sizeof(szSql));
    	sprintf(szSql,"insert into ne_queuenum values ('%s', %d, '0')", szTemp, nMsgCount);
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


BOOL getAgentState(PSTR pszAgentNo)
{
    STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	STR szAgentState[10];
	
	sprintf(szSql,"select qs_state from ne_queuenum where qs_agentno = '%s'", pszAgentNo);
	//PrintDebugLog(DBG_HERE,"执行SQL语句[%s]\n",szSql);
	if(SelectTableRecord(szSql,&struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return BOOLFALSE;
	}
	if (FetchCursor(&struCursor) == NORMAL)
	{
        strcpy(szAgentState, GetTableFieldValue(&struCursor,"qs_state"));
	}
	else
	{
		strcpy(szAgentState, "1");
	}
	FreeCursor(&struCursor);
	if (atoi(szAgentState) == 0)//状态正常
	    return BOOLTRUE;
	else
	    return BOOLFALSE;
}


//保存到ne_delivecrc表中
RESULT SaveToRecordDeliveCrc(UINT nRepeaterId, INT nDeviceId, INT nNetFlag, INT nType)
{
    char szSql[MAX_BUFFER_LEN];

    memset(szSql, 0, sizeof(szSql));
    snprintf(szSql, sizeof(szSql), "insert into  ne_delivecrc values (%u, %d, %d, %d, sysdate)",
         nRepeaterId, nDeviceId, nNetFlag, nType);
     
    //PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
    
	return NORMAL; 
}
/*
 *  cqt配对管理
 *  pszTelNum 电话号码, pszCallNum 主叫号码 pszBeCallNum 对端号码
 */
RESULT CqtMathchJob(int nNeId, PSTR pszTelNum, PSTR pszCallNum, PSTR pszBeCallNum)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	int nFailed=-1, nMatchesState=0;
	//STR szTelNum1[20], szCallNum1[20], szBeCallNum1[20];
	//STR szTelNum2[20], szCallNum2[20], szBeCallNum2[20];
	int nNeId2=-1, nTempNeId=-1, nTempNeId1, nTempNeId2;
	int nFindCount=0, nCount;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select ne_devicestatusid from ne_element  where ne_neid = %d and ne_devicestatusid = 0", nNeId);
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
	    return EXCEPTION;
	}
	FreeCursor(&struCursor);
	
	if (strlen(pszCallNum) == 0)
	{
		memset(szSql, 0, sizeof(szSql));
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %d and epm_objid ='07A0'", nNeId);
		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
		if(SelectTableRecord(szSql, &struCursor) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
						  szSql, GetSQLErrorMessage());
			return EXCEPTION;
		}
		if(FetchCursor(&struCursor) == NORMAL)
		{
		    strcpy(pszCallNum, GetTableFieldValue(&struCursor, "epm_CurValue"));
		}
		FreeCursor(&struCursor);
	}
	if (strlen(pszBeCallNum) == 0)
	{
		memset(szSql, 0, sizeof(szSql));
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %d and epm_objid ='070C'", nNeId);
		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
		if(SelectTableRecord(szSql, &struCursor) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
						  szSql, GetSQLErrorMessage());
			return EXCEPTION;
		}
		if(FetchCursor(&struCursor) == NORMAL)
		{
		    strcpy(pszBeCallNum, GetTableFieldValue(&struCursor, "epm_CurValue"));
		}
		FreeCursor(&struCursor);
	}
	
	//配对解除,其中一台设备没返回，导致配对数目不一致 2009.10.26 
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select MCH_NEID, MCH_NEID2 from NE_MATCHES  where (MCH_NEID=%d or MCH_NEID2=%d) and MCH_STATE = 1", nNeId, nNeId);
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) == NORMAL)
	{
	    nTempNeId1 = atoi(GetTableFieldValue(&struCursor, "MCH_NEID"));
	    nTempNeId2 = atoi(GetTableFieldValue(&struCursor, "MCH_NEID2"));
	    FreeCursor(&struCursor);
	    
	    if (nTempNeId1 == nNeId)
	    	nTempNeId = nTempNeId2;
	    else if (nTempNeId2 == nNeId)
	    	nTempNeId = nTempNeId1;
	    
	    nFailed = 5; //没有配对
        nMatchesState = 0;
        //插入一条另一个站点没配对记录
		snprintf(szSql, sizeof(szSql), "insert into NE_MATCHES(MCH_NEID, MCH_STATE,MCH_COLLECT_DATE,MCH_FAILSTATE, MCH_NEID2)"
    	    " values( %d, %d, sysdate, %d, -1)",
    	     nTempNeId, nMatchesState, nFailed);
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
		FreeCursor(&struCursor);
	}
	
	//删除旧记录
	snprintf(szSql, sizeof(szSql), "delete from NE_MATCHES where MCH_NEID=%d or MCH_NEID2=%d",
         nNeId, nNeId);
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
	 	PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
     	return EXCEPTION;
	}
	CommitTransaction();
	
	if (strcmp(pszCallNum, pszBeCallNum) !=0)
	{
		nFailed = 3; //2个号码不一致
        nMatchesState = 0;
		//插入已经配对成功记录
		snprintf(szSql, sizeof(szSql), "insert into NE_MATCHES(MCH_NEID, MCH_STATE,MCH_COLLECT_DATE,MCH_FAILSTATE, MCH_NEID2)"
            " values( %d, %d, sysdate, %d, %d)",
    	     nNeId, nMatchesState, nFailed, nNeId2);
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
		 	PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
    	 	return EXCEPTION;
		}
		CommitTransaction();
		return EXCEPTION;
	}
	if (strlen(pszCallNum) == 0 && strlen(pszBeCallNum) == 0)
	{
		nFailed = 1; //号码为空
        nMatchesState = 0;
		//插入已经配对成功记录
		snprintf(szSql, sizeof(szSql), "insert into NE_MATCHES(MCH_NEID, MCH_STATE,MCH_COLLECT_DATE,MCH_FAILSTATE, MCH_NEID2)"
            " values( %d, %d, sysdate, %d, %d)",
    	     nNeId, nMatchesState, nFailed, nNeId2);
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
		 	PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
    	 	return EXCEPTION;
		}
		CommitTransaction();
		return EXCEPTION;
	}
	
	//先查是否配对有记录
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql,"select ne_neid from ne_element t join ne_elementparam pa on t.ne_neid= pa.epm_neid "
                  "join ne_elementparam pa1 on t.ne_neid= pa1.epm_neid "
                  "where pa.epm_objid = '07A0' "
                  "and pa1.epm_objid = '070C'"
                  "and pa.epm_curvalue = '%s' "
                  "and pa1.epm_curvalue = '%s' "
                  "and t.ne_netelnum = '%s'"
                  "and t.ne_neid != %d and ne_devicestatusid=0", 
                  pszTelNum, pszTelNum,  pszCallNum, nNeId);
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) == NORMAL)
	{
		nNeId2 = atoi(GetTableFieldValue(&struCursor, "ne_neid"));
		nFindCount++;
	}
	FreeCursor(&struCursor);
	
	
	
	if (nFindCount == 0)//没配对
	{
		memset(szSql, 0, sizeof(szSql));
		sprintf(szSql, "select count(ne_NeId) as v_count from ne_Element where ne_NeId != %d and ne_netelnum='%s' and ne_devicestatusid=0", nNeId, pszCallNum);
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
		nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
		FreeCursor(&struCursor);
		
		if (nCount > 0)
		{
			nFailed = 5; //没有配对
        	nMatchesState = 0;
    	}
    	else
    	{
    		nFailed = 2; //对端电话号码错误，不存在
        	nMatchesState = 0;
    	}

	}
	else if (nFindCount == 1)//找到配对
	{	
		//查找配多情况
		nFailed = -1; //配对
        nMatchesState = 1;
        
        snprintf(szSql, sizeof(szSql), "delete from NE_MATCHES where MCH_NEID=%d", nNeId2);
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
		 	PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
    	 	return EXCEPTION;
		}
		CommitTransaction();
	}
	else //一配多情况
	{
		nFailed = 4;
        nMatchesState = 0;
        nNeId2 = -1;
	}
	
	//插入已经配对成功记录
	snprintf(szSql, sizeof(szSql), "insert into NE_MATCHES(MCH_NEID, MCH_STATE,MCH_COLLECT_DATE,MCH_FAILSTATE, MCH_NEID2)"
        " values( %d, %d, sysdate, %d, %d)",
         nNeId, nMatchesState, nFailed, nNeId2);
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

RESULT CqtMathchJob2(int nNeId, PSTR pszTelNum, PSTR pszBeCallNum)
{
	STR szSql[MAX_SQL_LEN];
	CURSORSTRU struCursor;
	int nFailed=-1, nMatchesState=0;
	int nNeId2=-1, nTempNeId=-1, nTempNeId1, nTempNeId2;
	int nFindCount=0, nCount;
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select ne_devicestatusid from ne_element  where ne_neid = %d and ne_devicestatusid = 0", nNeId);
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
	    return EXCEPTION;
	}
	FreeCursor(&struCursor);
	
	
	if (strlen(pszBeCallNum) == 0)
	{
		memset(szSql, 0, sizeof(szSql));
		sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %d and epm_objid ='070C'", nNeId);
		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
		if(SelectTableRecord(szSql, &struCursor) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
						  szSql, GetSQLErrorMessage());
			return EXCEPTION;
		}
		if(FetchCursor(&struCursor) == NORMAL)
		{
		    strcpy(pszBeCallNum, GetTableFieldValue(&struCursor, "epm_CurValue"));
		}
		FreeCursor(&struCursor);
	}
	
	//配对解除,其中一台设备没返回，导致配对数目不一致 2009.10.26 
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select MCH_NEID, MCH_NEID2 from NE_MATCHES  where (MCH_NEID=%d or MCH_NEID2=%d) and MCH_STATE = 1", nNeId, nNeId);
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) == NORMAL)
	{
	    nTempNeId1 = atoi(GetTableFieldValue(&struCursor, "MCH_NEID"));
	    nTempNeId2 = atoi(GetTableFieldValue(&struCursor, "MCH_NEID2"));
	    FreeCursor(&struCursor);
	    
	    if (nTempNeId1 == nNeId)
	    	nTempNeId = nTempNeId2;
	    else if (nTempNeId2 == nNeId)
	    	nTempNeId = nTempNeId1;
	    
	    nFailed = 5; //没有配对
        nMatchesState = 0;
        //插入一条另一个站点没配对记录
		snprintf(szSql, sizeof(szSql), "insert into NE_MATCHES(MCH_NEID, MCH_STATE,MCH_COLLECT_DATE,MCH_FAILSTATE, MCH_NEID2)"
    	    " values( %d, %d, sysdate, %d, -1)",
    	     nTempNeId, nMatchesState, nFailed);
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
		FreeCursor(&struCursor);
	}
	
	//删除旧记录
	snprintf(szSql, sizeof(szSql), "delete from NE_MATCHES where MCH_NEID=%d or MCH_NEID2=%d",
         nNeId, nNeId);
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
	 	PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
     	return EXCEPTION;
	}
	CommitTransaction();
	
	
	//先查是否配对有记录
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql,"select ne_neid from ne_element t join ne_elementparam pa on t.ne_neid= pa.epm_neid "
                  "where pa.epm_objid = '070C' "
                  "and pa.epm_curvalue = '%s' "
                  "and t.ne_netelnum = '%s' "
                  "and t.ne_neid != %d and ne_devicestatusid=0", 
                  pszTelNum, pszBeCallNum, nNeId);
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) == NORMAL)
	{
		nNeId2 = atoi(GetTableFieldValue(&struCursor, "ne_neid"));
		nFindCount++;
	}
	FreeCursor(&struCursor);
	
	
	
	if (nFindCount == 0)//没配对
	{
		memset(szSql, 0, sizeof(szSql));
		sprintf(szSql, "select count(ne_NeId) as v_count from ne_Element where ne_NeId != %d and ne_netelnum='%s' and ne_devicestatusid=0", nNeId, pszBeCallNum);
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
		nCount = atoi(GetTableFieldValue(&struCursor, "v_count"));
		FreeCursor(&struCursor);
		
		if (nCount > 0)
		{
			nFailed = 5; //没有配对
        	nMatchesState = 0;
    	}
    	else
    	{
    		nFailed = 2; //对端电话号码错误，不存在
        	nMatchesState = 0;
    	}

	}
	else if (nFindCount == 1)//找到配对
	{	
		//查找配多情况
		nFailed = -1; //配对
        nMatchesState = 1;
        
        snprintf(szSql, sizeof(szSql), "delete from NE_MATCHES where MCH_NEID=%d", nNeId2);
		PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
		if(ExecuteSQL(szSql) != NORMAL)
		{
		 	PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
				szSql, GetSQLErrorMessage());
    	 	return EXCEPTION;
		}
		CommitTransaction();
	}
	else //一配多情况
	{
		nFailed = 4;
        nMatchesState = 0;
        nNeId2 = -1;
	}
	
	//插入已经配对成功记录
	snprintf(szSql, sizeof(szSql), "insert into NE_MATCHES(MCH_NEID, MCH_STATE,MCH_COLLECT_DATE,MCH_FAILSTATE, MCH_NEID2)"
        " values( %d, %d, sysdate, %d, %d)",
         nNeId, nMatchesState, nFailed, nNeId2);
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

BOOL ExistAlarmLog(int nAlarmId, int nNeId, PINT pnAlarmCount)
{
	CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	INT nAlarmLogId, nAlarmCount;
	STR szNewCount[10], szOldCount[10];
	
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select alg_alarmlogid, alg_compresscount from alm_AlarmLog where alg_NeId = %d and alg_AlarmId= %d and alg_AlarmTime>to_date( '%s 000000','yyyymmdd hh24miss') and alg_AlarmTime<to_date( '%s 235959','yyyymmdd hh24miss')", 
		nNeId, nAlarmId, GetSystemDate(), GetSystemDate());
	PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
	if(SelectTableRecord(szSql, &struCursor) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", \
					  szSql, GetSQLErrorMessage());
		return EXCEPTION;
	}
	if(FetchCursor(&struCursor) != NORMAL)
	{
		*pnAlarmCount =1;
	    FreeCursor(&struCursor);
	    return BOOLFALSE;

	}
	nAlarmCount = atoi(GetTableFieldValue(&struCursor, "alg_compresscount"));
	nAlarmLogId = atoi(GetTableFieldValue(&struCursor, "alg_alarmlogid"));
	FreeCursor(&struCursor);
	
	sprintf(szOldCount, "%d", nAlarmCount);
	sprintf(szNewCount, "%d", nAlarmCount+1);
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "update alm_AlarmLog set alg_compresscount = alg_compresscount + 1, alg_AlarmInfo=REPLACE(alg_AlarmInfo, '%s', '%s') where alg_alarmlogid = %d", szOldCount, szNewCount, nAlarmLogId);
	PrintDebugLog(DBG_HERE, "开始执行SQL[%s]\n", szSql);
	if(ExecuteSQL(szSql) != NORMAL)
	{
		PrintErrorLog(DBG_HERE, "执行SQL语句失败[%s][%s]\n",
			szSql, GetSQLErrorMessage());
        return EXCEPTION;
	}
	CommitTransaction();
	
	*pnAlarmCount = nAlarmCount+1;
	
	return BOOLTRUE;

}

RESULT InsertMosTask(PXMLSTRU pstruXml)
{
	CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	UINT nPesqMosTaskId;
	STR szServerNum[20];
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select epm_CurValue from ne_ElementParam where epm_NeId = %s and epm_objid ='07B0'", DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"));
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
	strcpy(szServerNum, GetTableFieldValue(&struCursor, "epm_CurValue"));
	FreeCursor(&struCursor);
	
	GetDbSerial(&nPesqMosTaskId, "sm_PesqMosTask");    
	snprintf(szSql, sizeof(szSql),
	    "insert into sm_pesqmostask(tst_id, tst_taskid, tst_eventtime, tst_prior, tst_neid,"
	    "tst_netelnum, tst_iscaller, tst_pesqservernum) values("
	    "%d,  1,  to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 0, %s,"
		" '%s', 1, '%s') ",
		nPesqMosTaskId,
		GetSysDateTime(),
		DemandStrInXmlExt(pstruXml, "<omc>/<网元编号>"),

		DemandStrInXmlExt(pstruXml, "<omc>/<站点电话>"),
		szServerNum
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

RESULT SaveToAlarmLog(int nAlarmId, int nNeId, int nAlarmCount)
{
	UINT nAlarmLogId;
	STR szSql[MAX_SQL_LEN];
	
	if(GetDbSerial(&nAlarmLogId, "AlarmLog")!=NORMAL)
	{
		PrintErrorLog(DBG_HERE,"获取告警日志流水号错误\n");
		return EXCEPTION;
	}
	if (nAlarmId == ALM_DLSB_ID)
		snprintf(szSql, sizeof(szSql), "insert into  alm_AlarmLog (alg_AlarmLogId,alg_AlarmId,alg_NeId,alg_AlarmTime,alg_AlarmStatusId,alg_AlarmObjId, alg_compresscount, alg_AlarmInfo)"
         " VALUES(%d, %d, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 1, '%s', 1, 'GPRS登陆失败次数%d')", 
         nAlarmLogId, ALM_DLSB_ID, nNeId, GetSysDateTime(), "F911", nAlarmCount);
    else if (nAlarmId == ALM_SJSB_ID)
    	snprintf(szSql, sizeof(szSql), "insert into  alm_AlarmLog (alg_AlarmLogId,alg_AlarmId,alg_NeId,alg_AlarmTime,alg_AlarmStatusId,alg_AlarmObjId, alg_compresscount, alg_AlarmInfo)"
         " VALUES(%d, %d, %d, to_date( '%s','yyyy-mm-dd hh24:mi:ss'), 1, '%s', 1, 'GPRS数据上报失败次数%d')", 
         nAlarmLogId, ALM_SJSB_ID, nNeId, GetSysDateTime(), "F912", nAlarmCount);
    
    PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
    if(ExecuteSQL(szSql) !=NORMAL) 
	{
		PrintErrorLog(DBG_HERE,"执行SQL语句[%s]失败[%s]\n",szSql,GetSQLErrorMessage());
		return EXCEPTION;
	}
	CommitTransaction();
	
	return NORMAL;
}


/* 
 * 根据直放站编号,电话号,设备编号取告警对象列表
 */
RESULT GetDeviceIp(UINT nRepeaterId, int nDeviceId, PSTR pszDeviceIp, int *pPort)
{
    CURSORSTRU struCursor;
	STR szSql[MAX_SQL_LEN];
	char szDeviceIp[30];
	int nDevicePort;
	
	if (getenv("WUXIAN")!=NULL)
	{
		if (GetRedisDeviceIpInfo(nRepeaterId, nDeviceId, szDeviceIp, &nDevicePort)!= NORMAL)
		{
			PrintErrorLog(DBG_HERE, "GetRedisDeviceIpInfo is null, [%u,%d]\n", \
						  nRepeaterId, nDeviceId);
			return EXCEPTION;
		}
		strcpy(pszDeviceIp, szDeviceIp);
		*pPort = nDevicePort;
	}
	else
	{
		memset(szSql, 0, sizeof(szSql));
		sprintf(szSql, "select qs_deviceip,qs_port from ne_deviceip where qs_RepeaterId = %u and qs_DeviceId = %d", nRepeaterId, nDeviceId);
		PrintDebugLog(DBG_HERE, "执行SQL语句[%s]\n", szSql);
		if(SelectTableRecord(szSql, &struCursor) != NORMAL)
		{
			PrintErrorLog(DBG_HERE, "执行SQL语句[%s]错误, 信息为[%s]\n", szSql, GetSQLErrorMessage());
			return EXCEPTION;
		}
		if(FetchCursor(&struCursor) != NORMAL)
		{
		    FreeCursor(&struCursor);
			PrintErrorLog(DBG_HERE, "执行SQL语句[%s]没有找到记录\n", szSql, GetSQLErrorMessage());
			return EXCEPTION;
		}
		strcpy(pszDeviceIp,  TrimAllSpace(GetTableFieldValue(&struCursor, "qs_deviceip")));
		*pPort = atoi(GetTableFieldValue(&struCursor, "qs_port"));
		FreeCursor(&struCursor);
	}
	return NORMAL;
}



