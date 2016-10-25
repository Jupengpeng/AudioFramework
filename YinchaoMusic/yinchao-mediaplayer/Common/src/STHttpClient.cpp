#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include "STMacrodef.h"
#include <typeinfo>
#include <pthread.h>

#include "STHttpClient.h"
#include "STLog.h"
#include "STSysTime.h"

#if defined __ST_OS_ANDROID__
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
//#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#else
#endif

static const STInt KDefaultPort = 80;
static const STInt KMaxHostAddrLen = 256;
static const STInt KMaxHostFileNameLen = 512;
static const STInt KMaxRequestLen = 2048;
static const STInt KInvalidSocketHandler = -1;

static const STInt KInvalidContentLength = -1;

static const STInt KHttpConnectTimeoutS = 30;

static const STChar KContentRangeKey[] = {'C','o','n','t','e','n','t','-','R','a','n','g','e','\0'};
static const STChar KContentLengthKey[] = {'C','o','n','t','e','n','t','-','L','e','n','g','t','h','\0'};
static const STChar KLocationKey[] = {'L','o','c','a','t','i','o','n','\0'};
STChar tHostAddr[KMaxHostAddrLen];
STChar tHostFileName[KMaxHostFileNameLen];
STChar tRequset[KMaxRequestLen];



/********************************************
功能：搜索字符串右边起的第一个匹配字符
********************************************/
static STChar* Rstrchr(STChar* s, STChar x) {
	STInt i = strlen(s);
	if (!(*s))
		return 0;
	while (s[i - 1])
		if (strchr(s + (i - 1), x))
			return (s + (i - 1));
		else
			i--;
	return 0;
}

/********************************************
功能：把字符串转换为全小写
********************************************/
static void ToLowerCase(STChar * s) {
	while (s && *s) {
		*s = tolower(*s);
		s++;
	}
}

/**************************************************************
功能：从字符串src中分析出网站地址和端口，并得到用户要下载的文件
***************************************************************/
static void GetHost(STChar* aServer, STChar* aHost, STChar* aHostFile, STInt* aPort) {
	STChar * pA;
	STChar * pB;
	*aPort = 0;
	if (!(*aServer))
		return;
	pA = aServer;
	if (!strncmp(pA, "http://", strlen("http://")))
		pA = aServer + strlen("http://");
	else if (!strncmp(pA, "https://", strlen("https://")))
		pA = aServer + strlen("https://");
	pB = strchr(pA, '/');
	if (pB) {
		memcpy(aHost, pA, strlen(pA) - strlen(pB));
		if (pB + 1) {
			memcpy(aHostFile, pB + 1, strlen(pB) - 1);
			aHostFile[strlen(pB) - 1] = 0;
		}
	} else
		memcpy(aHost, pA, strlen(pA));
	if (pB)
		aHost[strlen(pA) - strlen(pB)] = 0;
	else
		aHost[strlen(pA)] = 0;
	pA = strchr(aHost, ':');
	if (pA)
	{
		*aPort = atoi(pA + 1);
		*pA='\0';
	}
	else
	{
		*aPort = KDefaultPort;
	}
}

static STInt SetSocketTimeOut(STInt aSocketHandle, timeval aTimeOut)
{	
	return setsockopt(aSocketHandle, SOL_SOCKET, SO_RCVTIMEO, (char *)&aTimeOut, sizeof(struct timeval));
}

STHttpClient::STHttpClient()
:iAborted(ESTFalse)
{
	iContentLength = KInvalidContentLength;
	iSocketHandle = KInvalidSocketHandler;
	iState = DISCONNECTED;
	iUrl = NULL;
	iCritical.Create();
}

STHttpClient::~STHttpClient()
{
	if (iState == CONNECTED)
	{
		Disconnect();
	}
	iCritical.Destroy();
}

#if 0
STInt STHttpClient::WaitSocketReadBuffer(timeval timeout)
{
	fd_set fds; 

	FD_ZERO(&fds);
	FD_SET(iSocketHandle, &fds);

	STInt nRet = select(iSocketHandle + 1, &fds, NULL, NULL, &timeout);

	if (nRet > 0 && !FD_ISSET(iSocketHandle, &fds))
	{
		nRet = 0;
	}
	return nRet;
}
#endif
STInt STHttpClient::Read(STChar* aDstBuffer, STInt aSize)
{
#if 0
	struct timeval timeout = {0, 500000};
	STInt nRet = WaitSocketReadBuffer(timeout);
	if (nRet > 0)
	{
		nRet = recv(iSocketHandle, aDstBuffer, aSize, 0);
	}
	return nRet;
#else
	STInt nRet = recv(iSocketHandle, aDstBuffer, aSize, MSG_WAITALL);
	if (nRet < 0)
	{
		STInt nErr = errno;
		if (nErr == EAGAIN || nErr == EWOULDBLOCK)
		{
			nRet = 0;
		}
		else
		{
			STLOGE("recv error: %d", nErr);
		}
	}
	return nRet;
#endif
}

STInt STHttpClient::Recv(STChar* aDstBuffer, STInt aSize)
{
#if 0
	struct timeval timeout = {5, 0};
	STInt nRet = WaitSocketReadBuffer(timeout);
	if (nRet > 0)
	{
		nRet = recv(iSocketHandle, aDstBuffer, aSize, 0);
	}
	else if (nRet == 0)
	{
		STLOGE("read time out!");
	}
#else
	return recv(iSocketHandle, aDstBuffer, aSize, MSG_WAITALL);
#endif
}

STInt STHttpClient::Send(const STChar* aSendBuffer, STInt aSize)
{
	STInt nSend = 0;
	STInt nTotalsend = 0;
	while(nTotalsend < aSize)
	{
		nSend = write(iSocketHandle, aSendBuffer + nTotalsend, aSize - nTotalsend);
		if(nSend == -1)
		{
			STLOGE("send error!%s/n", strerror(errno));
		    return -1;
		}

		nTotalsend += nSend;
	}

	return STKErrNone;
}

STInt STHttpClient::Connect(const STChar* aUrl, STInt aOffset)
{
	STASSERT(aOffset >= 0);

	STASSERT(iState == DISCONNECTED);

	STInt nUrlLen = strlen(aUrl);
	STASSERT(iUrl == 0);
	iUrl = (STChar*)malloc(nUrlLen + 1);
	strcpy(iUrl, aUrl);
	ToLowerCase(iUrl);
	memset(tHostAddr, 0, sizeof(tHostAddr));
	memset(tHostFileName, 0, sizeof(tHostFileName));
	memset(tRequset, 0, sizeof(tRequset));

	STInt nPortNum = KDefaultPort;
	GetHost(iUrl, tHostAddr, tHostFileName, &nPortNum);
	struct hostent *host = NULL;
	if((host = gethostbyname(tHostAddr)) == NULL)
	{
		SAFE_FREE(iUrl);
		STLOGE("gethostbyname error: %d", h_errno);
		return STKErrCouldNotConnect;
	}

	iCritical.Lock();
	if(!iAborted && (iSocketHandle = socket(AF_INET,SOCK_STREAM,0)) == KInvalidSocketHandler)
	{
		iCritical.UnLock();
		SAFE_FREE(iUrl);
		STLOGE("socket error:%d", iAborted);
		return STKErrCouldNotConnect;
	}
	iCritical.UnLock();


	struct sockaddr_in server_addr;

	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(nPortNum);
	server_addr.sin_addr=*((struct in_addr *)host->h_addr);
	STLOGI("STHttpClient::connect:%s", aUrl);
	if(connect(iSocketHandle,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr)) == -1)
	{
		SAFE_FREE(iUrl);
		iSocketHandle = KInvalidSocketHandler;
		STLOGE("connect error");
		return STKErrCouldNotConnect;
	}
	
	iState = CONNECTED;

	struct timeval nRecvHeaderTimeOut = {8, 0};
	SetSocketTimeOut(iSocketHandle, nRecvHeaderTimeOut);

	if (aOffset > 0) 
	{
		sprintf(tRequset, "GET /%s HTTP/1.1\r\nHost: %s:%d\r\nRange: bytes=%d-\r\nConnection: keep-alive\r\n\r\n", tHostFileName, tHostAddr, nPortNum, aOffset);
	}
	else
	{
		sprintf(tRequset, "GET /%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: keep-alive\r\n\r\n", tHostFileName, tHostAddr, nPortNum);
	}

	STLOGE("Request:%s", tRequset);

	STInt nErr = Send(tRequset, strlen(tRequset));
	if (nErr == STKErrNone)
	{
		STInt32 nResStatus = 0;
		nErr = ParseHeader(nResStatus);
		if (nErr == STKErrNone)
		{			
			if (IsRedirectStatusCode(nResStatus))
			{
				if (STKErrNone == (nErr = GetHeaderValueByKey(KLocationKey, iHeaderValueBuffer, sizeof(STChar)*KMaxLineLength)))
				{
         			iState = DISCONNECTED;					
					SAFE_FREE(iUrl);					
					return Connect(iHeaderValueBuffer, aOffset);
				}	
         	}					
			else if (nResStatus == 200 || nResStatus == 206)
			{
				if (STKErrNone ==  (nErr = GetHeaderValueByKey(KContentLengthKey, iHeaderValueBuffer, sizeof(STChar)*KMaxLineLength))) //bin.liu时不时解析返回 非STKErrNone ，以后定位可能和网络情况有关
				{
					STChar* pEnd = NULL;
					STInt32 nContentLen = strtol(iHeaderValueBuffer, &pEnd, 10);

					if ((pEnd == iHeaderValueBuffer) || (*pEnd != '\0'))
					{
						//STLOGI("bin.liu  10");
						STLOGE("STHttpClient Get ContentLength Error!");
						nErr = STKErrArgument;
					}
					else
					{
						if ((nResStatus == 200 && nContentLen == aOffset)  /*|| (nResStatus == 206 && nContentLen == aOffset)*/)//modify bin.liu增加处理返回值206
						{
							Disconnect();
							//STLOGI("bin.liu   nResStatus:%d,nContentLen=%d,aOffset=%d",nResStatus, nContentLen,aOffset);
							nErr = STKErrOverflow;
						}
						else 
						{
							//STLOGI("bin.liu  10");
							iContentLength = nContentLen;
							STLOGI("nContentLen:%d", nContentLen);
						}
					}
				}
				else
				{
					STLOGI("GetHeaderValue faild");
				}
			}
			else if (nResStatus == 416)
			{
				Disconnect();
				STLOGI("connect: Overflow");
				return STKErrOverflow;
			}
			else
            {
				//STLOGI("bin.liu  1");
                nErr = STKErrCouldNotConnect;
            }
		}
		else 
		{
			//STLOGI("bin.liu  2");
			nErr = STKErrCouldNotConnect;
		}

	}

	if ((nErr != STKErrNone) && (iState == CONNECTED))
	{
		//STLOGI("bin.liu  3");
        nErr = STKErrCouldNotConnect;
		STLOGE("open failed. Disconnect");
		Disconnect();
	}
	
	if (nErr == STKErrNone)
	{
		struct timeval nRecvDataTimeOut = {0, 500000};
		SetSocketTimeOut(iSocketHandle, nRecvDataTimeOut);
	}

	return nErr;
}

void STHttpClient::Abort()
{
	STLOGE("STHttpClient::Abort");
	iCritical.Lock();
	iAborted = ESTTrue;
	shutdown(iSocketHandle, SHUT_RDWR);
	close(iSocketHandle);
	iCritical.UnLock();
}

STInt STHttpClient::Disconnect()

{
	if ((iState == CONNECTED) && (iSocketHandle != KInvalidSocketHandler))
	{
		STASSERT(iUrl != 0);
		SAFE_FREE(iUrl);
		close(iSocketHandle);
		iSocketHandle = KInvalidSocketHandler;
	}
	return 0;
}

STInt STHttpClient::ParseHeader(STInt32& aResponseStatus)
{
	STChar tLine[KMaxLineLength];

	STInt nErr = ReceiveLine(tLine, sizeof(tLine));
	if (nErr != STKErrNone)
	{
		STLOGE("STHttpClient Receive ResponseStaus Error!");
		return nErr;
	}

	STChar* pSpaceStart = strchr(tLine, ' ');
	if (pSpaceStart == NULL) 
	{
		STLOGE("STHttpClient Receive ResponseStaus Error!");
		return STKErrBadDescriptor;
	}

	STChar* pResponseStatusStart = pSpaceStart + 1;
	STChar* pResponseStatusEnd = pResponseStatusStart;
	while (isdigit(*pResponseStatusEnd)) 
	{
		++pResponseStatusEnd;
	}

	if (pResponseStatusStart == pResponseStatusEnd) 
	{
		return STKErrBadDescriptor;
	}

	memmove(tLine, pResponseStatusStart, pResponseStatusEnd - pResponseStatusStart);
	tLine[pResponseStatusEnd - pResponseStatusStart] = '\0';

	STInt32 nResponseNum = strtol(tLine, NULL, 10);
	if ((nResponseNum < 0) || (nResponseNum > 999))
	{
		STLOGE("STHttpClient Receive Invalid ResponseStaus!");
		return STKErrBadDescriptor;
	}

	aResponseStatus = nResponseNum;

	STLOGI("HttpClient ParseHeader Response:%d", nResponseNum);

	return STKErrNone;
}

STInt STHttpClient::GetHeaderValueByKey(const STChar* aKey, STChar* aBuffer, STInt aBufferLen)
{
	STInt nErr = STKErrArgument;
    STBool bIsKeyFound = ESTFalse;

	while (ESTTrue)
	{
		nErr = ReceiveLine(iLineBuffer, sizeof(STChar) * KMaxLineLength);
		STLOGE("Response:%s", iLineBuffer);
		if (nErr != STKErrNone)
		{
			STLOGE("STHttpClient RecHeader Error:%d", nErr);
			break;
		}

		if (iLineBuffer[0] == '\0')
		{
			nErr = bIsKeyFound ? STKErrNone : STKErrEof;
			break;
		}

		STChar* pColonStart = strchr(iLineBuffer, ':');
		if (pColonStart == NULL) 
		{
			continue;			
		} 

		STChar* pEndofkey = pColonStart;

		while ((pEndofkey > iLineBuffer) && isspace(pEndofkey[-1])) 
		{
			--pEndofkey;
		}

		STChar* pStartofValue = pColonStart + 1;
		while (isspace(*pStartofValue)) 
		{
			++pStartofValue;
		}

		*pEndofkey = '\0';

		if (strncmp(iLineBuffer, aKey, strlen(aKey)) != 0)
		{
			continue;
		}		

		if (aBufferLen > strlen(pStartofValue))
		{
            bIsKeyFound = ESTTrue;
			strcpy(aBuffer, pStartofValue);
		}		
	}
	return nErr;
}

STInt STHttpClient::ReceiveLine(STChar* aLine, STInt aSize)
{
	if (iState != CONNECTED) {
		return STKErrNotReady;
	}



	STBool bSawCR = ESTFalse;
	STInt nLength = 0;
	STChar *log = new STChar[aSize];
	memset(log, 0, sizeof(STChar) * aSize);

	while (ESTTrue) 
	{
		char c;
		size_t n = Recv(&c, 1);
		if (n <= 0) 
		{
			//Disconnect();
			strncpy(log, aLine, nLength);
			delete []log;
			return STKErrDisconnected;
		} 

		if (bSawCR && c == '\n') 
		{
			aLine[nLength - 1] = '\0';
			strncpy(log, aLine, nLength);
			delete []log;
			return STKErrNone;
		}

		bSawCR = (c == '\r');

		if (nLength + 1 >= aSize) 
		{
			delete []log;
			return STKErrOverflow;
		}

		aLine[nLength++] = c;
	}
	delete []log;
}

STBool STHttpClient::IsRedirectStatusCode(STInt aResStatus)
{
	return aResStatus == 301 || aResStatus == 302
		|| aResStatus == 303 || aResStatus == 307;
}

void STHttpClient::SetReceiveTimeout(STInt aSecond)
{
	if (aSecond < 0) 
	{
		aSecond = 0;
	}

	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = aSecond;
	setsockopt(iSocketHandle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}
