#include "TTOsalConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <signal.h> 
#include <ctype.h>
#include "TTMacrodef.h"
#include <exception>
#include <typeinfo>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include<signal.h>

#include "TTHttpClient.h"
#include "TTHttpReaderProxy.h"
#include "TTLog.h"
#include "TTSysTime.h"
#include "TTUrlParser.h"

#define  CONNECTION_TIMEOUT_IN_SECOND			30	//unit: second
#define  HTTP_HEADER_RECV_TIMEOUT_IN_SECOND		30 //unit: second
#define  HTTP_HEADER_RECV_MAXTIMES              6 //

#define  CONNECT_ERROR_BASE		600		//connect error base value
#define  REQUEST_ERROR_BASE		1000	//request error base value
#define  RESPONSE_ERROR_BASE	1300	//response error base value
#define  DNS_ERROR_BASE			2000	//dns resolve error base value

#define HTTPRESPONSE_INTERUPTER 1304
#define ERROR_CODE_TEST         902
#define INET_ADDR_EXCEPTION     16
#define DNS_TIME_OUT            17
#define DNS_UNKNOWN             18

#define TIME_KEEP_ALIVE  1  // 打开探测
#define TIME_KEEP_IDLE   10  // 开始探测前的空闲等待时长10s
#define TIME_KEEP_INTVL  2  // 发送探测分节的时间间隔 2s
#define TIME_KEEP_CNT    3  // 发送探测分节的次数 3 times

static const TTInt KInvalidSocketHandler = -1;

static const TTInt KInvalidContentLength = -1;

static const TTChar KContentRangeKey[] = {'C', 'o', 'n', 't', 'e', 'n', 't', '-', 'R', 'a', 'n', 'g', 'e', '\0'};
static const TTChar KContentLengthKey[] = {'C', 'o', 'n', 't', 'e', 'n', 't', '-', 'L', 'e', 'n', 'g', 't', 'h', '\0'};
static const TTChar KLocationKey[] = {'L', 'o', 'c', 'a', 't', 'i', 'o', 'n', '\0'};
static const TTChar KTransferEncodingKey[] = {'T', 'r', 'a', 'n', 's', 'f', 'e', 'r', '-', 'E', 'n', 'c', 'o', 'd',  'i', 'n', 'g','\0'};



CTTDNSCache* CTTHttpClient::iDNSCache = NULL;

static const TTInt STATUS_OK = 0;

//static const TTInt Status_Error_Dns = 901;
//static const TTInt Status_Error_Connect = 902;
//static const TTInt Status_Error_Request = 903;
//static const TTInt Status_Error_Response = 904;
static const TTInt Status_Error_ServerConnectTimeOut = 905;

//HttpResponse error ,start from 1300 + 255 +1
static const TTInt Status_Error_HttpResponseTimeOut = 1556;
static const TTInt Status_Error_HttpResponseBadDescriptor = 1557;
static const TTInt Status_Error_HttpResponseArgumentError= 1558;
static const TTInt Status_Error_NoUsefulSocket = 1559;
//static const TTInt Status_Error_HttpResponseSocketError 
//system error: 0~255, and 132~255 are unkown error. 

#define	_ETIMEDOUT	60

TTUint32 gProxyHostIP = 0;
TTInt    gProxyHostPort = 0;
TTChar*  g_AutherKey = NULL;
TTChar*   g_Domain = NULL;

#define ONESECOND         1000
#define WAIT_DNS_INTERNAL 50
#define WAIT_DNS_MAX_TIME 600

TTBool		 gUseProxy = ETTFalse;
//TTBool       gCancle = ETTFalse;
//TTBool       gDnsInit = ETTFalse;
//DNSParam*    gDNSInput = NULL;
//DNSParam*    gDNSOutput = NULL;
//RTTSemaphore gDnsSephemore;
//RTTCritical	 gDnsCritical;
//RTThread     gDnsResolveThread;
//TTBool       gExitNotify = ETTFalse;
//static const TTChar* KDNSThreadName = "DnsResolveThread";

//static void* DnsResolveThreadProc(void* aPtr)
//{
//    TTChar   lcoaldomainName[MAXDOMAINNAME] = {NULL};
//    struct addrinfo hints;
//    struct addrinfo *res;
//    int ret;
//    struct	in_addr aIP;
//    //TTBool
//    while (true) {
//        gDnsSephemore.Wait(ONESECOND);
//
//_start:
//        if (gExitNotify) {
//            break;
//        }
//        gDnsCritical.Lock();
//        if (0 != strcmp(lcoaldomainName,  gDNSInput->domainName)) {
//            strcpy(lcoaldomainName, gDNSInput->domainName);
//            gDnsCritical.UnLock();
//        }
//        else{
//            gDnsCritical.UnLock();
//            continue;
//        }
//
//        memset(&hints, 0, sizeof(struct addrinfo));
//        hints.ai_family = AF_INET; /* Allow IPv4 */
//        hints.ai_socktype = SOCK_STREAM;
//        ret = getaddrinfo(lcoaldomainName, NULL,&hints,&res);
//        if (ret == 0 && res != NULL) {
//            aIP = ((struct sockaddr_in*)(res->ai_addr))->sin_addr;
//            freeaddrinfo(res);
//            char* pHostIP = inet_ntoa(aIP);
//            gDnsCritical.Lock();
//            strcpy(gDNSOutput->domainName, lcoaldomainName);
//            gDNSOutput->ip = inet_addr(pHostIP);
//            gDNSOutput->errorcode = 0;
//            gDnsCritical.UnLock();
//        }
//        else{
//            gDnsCritical.Lock();
//            strcpy(gDNSOutput->domainName, lcoaldomainName);
//            gDNSOutput->errorcode = ret;
//            gDnsCritical.UnLock();
//        }
//        
//        //
//        gDnsCritical.Lock();
//        if (0 != strcmp(lcoaldomainName,  gDNSInput->domainName)) {
//             gDnsCritical.UnLock();LOGE("-- new dns comming--");
//            goto _start;
//        }
//        gDnsCritical.UnLock();
//    }
//    
//    if (gExitNotify) {
//        gDnsSephemore.Destroy();
//        gDnsCritical.Destroy();
//        delete gDNSOutput;
//        delete gDNSInput;
//    }
//	return NULL;
//}
//
void SignalHandle(TTInt avalue)
{
  //  gCancle = ETTTrue;
}

TTBool CTTHttpClient::IsRedirectStatusCode(TTUint32 aStatusCode)
{
	return aStatusCode == 301 || aStatusCode == 302
		|| aStatusCode == 303 || aStatusCode == 307;
}

void CTTHttpClient::SetSocketNonBlock(TTInt& aSocketHandle)
{
#ifdef __TT_OS_WINDOWS__
	u_long non_blk = 1;
	ioctlsocket(aSocketHandle, FIONBIO, &non_blk);
#else
	TTInt flags = fcntl(aSocketHandle, F_GETFL, 0);
	fcntl(aSocketHandle, F_SETFL, flags | O_NONBLOCK);
#endif
}

void CTTHttpClient::SetSocketBlock(TTInt& aSocketHandle)
{
#ifdef __TT_OS_WINDOWS__
	u_long non_blk = 0;
	ioctlsocket(aSocketHandle, FIONBIO, &non_blk);
#else
	TTInt flags = fcntl(aSocketHandle, F_GETFL, 0);
	flags &= (~O_NONBLOCK);
	fcntl(aSocketHandle, F_SETFL, flags);
#endif
}

TTInt CTTHttpClient::SetSocketTimeOut(TTInt& aSocketHandle, timeval aTimeOut)
{	
	return setsockopt(aSocketHandle, SOL_SOCKET, SO_RCVTIMEO, (char *)&aTimeOut, sizeof(struct timeval));
}

TTInt CTTHttpClient::WaitSocketReadBuffer(TTInt& aSocketHandle, timeval& aTimeOut)
{
	fd_set fds;
    TTInt nRet;
    TTInt tryCnt =0;
retry:
	FD_ZERO(&fds);
	FD_SET(aSocketHandle, &fds);

	//select : if error happens, select return -1, detail errorcode is in error
    SetStatusCode(0);
    nRet = select(aSocketHandle + 1, &fds, NULL, NULL, &aTimeOut);
	if (nRet > 0 && !FD_ISSET(aSocketHandle, &fds))
	{
		nRet = 0;
	}
	else if(nRet < 0)
	{
		SetStatusCode(errno + RESPONSE_ERROR_BASE);
        if (StatusCode() == HTTPRESPONSE_INTERUPTER && tryCnt == 0 && IsCancel() == ETTFalse)
        {
            tryCnt++;
            goto retry;
        }
	}
    
	return nRet;
}

TTInt CTTHttpClient::WaitSocketWriteBuffer(TTInt& aSocketHandle, timeval& aTimeOut)
{
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(aSocketHandle, &fds);

	TTInt ret = select(aSocketHandle + 1, NULL, &fds, NULL, &aTimeOut);
	TTInt err = 0;
	TTInt errLength = sizeof(err);

	if (ret > 0 && FD_ISSET(aSocketHandle, &fds))
	{
        getsockopt(aSocketHandle, SOL_SOCKET, SO_ERROR, (char *)&err, (socklen_t*)&errLength);
		if (err != 0)
		{
			SetStatusCode(err + CONNECT_ERROR_BASE);
			ret = -1;
		}
	}
	else if(ret < 0)
	{
		SetStatusCode(errno + CONNECT_ERROR_BASE);
	}

	return (ret > 0) ? TTKErrNone : ((ret == 0) ? TTKErrTimedOut : TTKErrCouldNotConnect);
}

TTInt CTTHttpClient::Receive(TTInt& aSocketHandle, timeval& aTimeOut, TTChar* aDstBuffer, TTInt aSize)
{
	TTInt nErr = WaitSocketReadBuffer(aSocketHandle, aTimeOut);
	if (nErr > 0)
	{
		nErr = recv(aSocketHandle, aDstBuffer, aSize, 0);
		if (nErr == 0)
		{
			//server close socket
			nErr = TTKErrServerTerminated;
		}
        if (nErr == -1 && errno == _ETIMEDOUT) {
            //network abnormal disconnected
            nErr = TTKErrNetWorkAbnormallDisconneted;
        }
	}
	return nErr;
}

TTInt CTTHttpClient::Read(TTChar* aDstBuffer, TTInt aSize)
{
	if(iState == DISCONNECTED)
		return TTKErrDisconnected;

	struct timeval tTimeout = {0, 500000};
	return Receive(iSocketHandle, tTimeout, aDstBuffer, aSize);
}

TTInt CTTHttpClient::Recv(TTChar* aDstBuffer, TTInt aSize)
{
	struct timeval tTimeout = {0, 500000};//{HTTP_HEADER_RECV_TIMEOUT_IN_SECOND, 0};
    TTInt nErr;
    TTInt retryCnt = HTTP_HEADER_RECV_MAXTIMES;// 6*5s = 30s, total wait time is 30s;
    do{
        nErr = Receive(iSocketHandle, tTimeout, aDstBuffer, aSize);
        retryCnt--;
        if (retryCnt <= 0 || iCancel) {
            break;
        }
    }while (nErr == 0);
    return nErr;
}

TTInt CTTHttpClient::Send(const TTChar* aSendBuffer, TTInt aSize)
{
	if(iState == DISCONNECTED)
		return TTKErrDisconnected;

	TTInt nSend = 0;
	TTInt nTotalsend = 0;
	while(nTotalsend < aSize)
	{
#ifdef __TT_OS_WINDOWS__
		nSend = send(iSocketHandle , aSendBuffer + nTotalsend, aSize - nTotalsend , 0 );
#else
		nSend = write(iSocketHandle, aSendBuffer + nTotalsend, aSize - nTotalsend);
		if (nSend < 0 && errno == EINTR) {
              nSend = 0;        /* and call write() again */
		} 
#endif
		if(nSend < 0)
		{
			SetStatusCode(errno + REQUEST_ERROR_BASE);
			LOGE("send error!%s/n", strerror(errno));
		    return TTKErrCouldNotConnect;
		}

		nTotalsend += nSend;
	}

	return TTKErrNone;
}

TTInt CTTHttpClient::SendRequest(TTInt aPort, TTInt aOffset) 
{
	memset(iRequset, 0, sizeof(iRequset));
	if (aOffset > 0) 
	{
		if(aPort != 80) {
			sprintf(iRequset, "GET /%s HTTP/1.1\r\nHost: %s:%d\r\nRange: bytes=%d-\r\nConnection: keep-alive\r\n\r\n", iHostFileName, iHostAddr, aPort, aOffset);
		} else {
			sprintf(iRequset, "GET /%s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%d-\r\nConnection: keep-alive\r\n\r\n", iHostFileName, iHostAddr, aOffset);
		}
	}
	else
	{
		if(aPort != 80) {
			sprintf(iRequset, "GET /%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: keep-alive\r\n\r\n", iHostFileName, iHostAddr, aPort);
		} else {
			sprintf(iRequset, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", iHostFileName, iHostAddr);
		}
	}

	return Send(iRequset, strlen(iRequset));
}

TTInt CTTHttpClient::ParseResponseHeader(ITTStreamBufferingObserver* aObserver, TTUint32& aStatusCode)
{
	TTInt nErr = ParseHeader(aStatusCode);
	if (nErr == TTKErrNone && aObserver != NULL)
	{
		aObserver->HttpHeaderReceived();
	}
	else if(nErr == TTKErrBadDescriptor)
	{
		iStatusCode = Status_Error_HttpResponseBadDescriptor;
	}
	LOGI("ParseResponseHeader return %d, %ld", nErr, aStatusCode);
	return nErr;
}

TTInt CTTHttpClient::RequireContentLength()
{
	TTInt nErr = TTKErrArgument;

	if(!iTransferBlock)
	{
		return nErr;
	}
	while (ETTTrue)
	{
		nErr = ReceiveLine(iLineBuffer, sizeof(TTChar) * KMaxLineLength);

		if (nErr != TTKErrNone)
		{
			LOGE("CTTHttpClient RecHeader Error:%d", nErr);
			break;
		}

		if (iLineBuffer[0] == '\0')
		{
			continue;
		}
		
		int a= ConvertToValue(iLineBuffer);
		//nErr = ReceiveLine(iLineBuffer, sizeof(TTChar) * KMaxLineLength);
		return a;
	}
	return nErr;
}

TTInt CTTHttpClient::ParseContentLength(TTUint32 aStatusCode)
{
	const TTChar* pKey = (aStatusCode == 206) ? KContentRangeKey : KContentLengthKey;
	TTInt nErr = GetHeaderValueByKey(pKey, iHeaderValueBuffer, sizeof(TTChar)*KMaxLineLength);
	if(iTransferBlock) 
		return TTKErrNone;

	if (TTKErrNone == nErr)
	{
		TTChar *pStart = (aStatusCode == 206) ? strchr(iHeaderValueBuffer, '/') + 1 : iHeaderValueBuffer;
		TTChar* pEnd = NULL;
		TTInt32 nContentLen = strtol(pStart, &pEnd, 10);

		if ((pEnd == iHeaderValueBuffer) || (*pEnd != '\0'))
		{
			LOGE("CTTHttpClient Get ContentLength Error!");
			iStatusCode = Status_Error_HttpResponseArgumentError;
			nErr = TTKErrArgument;
		}
		else
		{
			iContentLength = nContentLen;
			LOGI("nContentLen: %ld", nContentLen);
		}
	}
	return nErr;
}

TTInt CTTHttpClient::ConnectViaProxy(ITTStreamBufferingObserver* aObserver, const TTChar* aUrl, TTInt aOffset)
{
    TTASSERT(aOffset >= 0);
	
	LOGI("CTTHttpClient::ConnectViaProxy");

	if(iWSAStartup)
		return TTKErrCouldNotConnect;
    
	TTASSERT(iState == DISCONNECTED);
    TTChar tLine[3] = {0};
    TTInt nPort;
    TTInt nErr;

	iStatusCode = STATUS_OK;
	iCancel = ETTFalse;
	iTransferBlock = ETTFalse;
	iContentLength = KInvalidContentLength;
    iConnectionTid = pthread_self();
    if (g_Domain != NULL) {
        TTInt nErr = ResolveDNS(aObserver, g_Domain, gProxyHostIP);
        if( nErr != TTKErrNone)
        {
            return nErr;
        }
    }
    
    nErr = ConnectServer(aObserver, gProxyHostIP, gProxyHostPort);
    if( nErr != TTKErrNone)
	{
		return nErr;
	}
    
    CTTUrlParser::ParseUrl(aUrl, iHostAddr, iHostFileName, nPort);
    
	iStatusCode = STATUS_OK;
    TTChar strRequest[1024] = {0};
    
    // sprintf(strRequest, "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\nProxy-Connection: Keep-Alive\r\nContent-Length: 0\r\nProxy-Authorization: Basic MzAwMDAwNDU1MDpCREFBQUQ5QjczOUQzQjNG\r\n\r\n",g_pHostAddr, nPort, g_pHostAddr, nPort);MzAwMDAwNDU1MDpCREFBQUQ5QjczOUQzQjNG
    sprintf(strRequest, "CONNECT %s:%d HTTP/1.1\r\nProxy-Authorization: Basic %s\r\n\r\n",iHostAddr, nPort, g_AutherKey);
    
    //send proxyserver connect request
	nErr = Send(strRequest, strlen(strRequest));
    if (nErr != TTKErrNone)
    {
        return nErr;
    }
    
    TTUint32 nStatusCode;
    //wait for proxyserver connect response
    //response: HTTP/1.1 200 Connection established\r\n
    nErr = ParseResponseHeader(NULL, nStatusCode);
	if (nStatusCode != 200)
	{
        return nErr;
	}
    
    //read \r\n
    Recv(tLine, 2);
    
    return SendRequestAndParseResponse(&CTTHttpClient::ConnectViaProxy, aObserver, aUrl, nPort, aOffset);
}

TTInt CTTHttpClient::Connect(ITTStreamBufferingObserver* aObserver, const TTChar* aUrl, TTInt aOffset)
{
	TTASSERT(aOffset >= 0);

	TTASSERT(iState == DISCONNECTED);

	if(iWSAStartup)
		return TTKErrCouldNotConnect;

	TTInt nPort;
	CTTUrlParser::ParseUrl(aUrl, iHostAddr, iHostFileName, nPort);

	iStatusCode = STATUS_OK;
	iHostIP = 0;
	iCancel = ETTFalse;
	iTransferBlock = ETTFalse;
	iContentLength = KInvalidContentLength;
	iConnectionTid = pthread_self();
	TTInt nErr = ResolveDNS(aObserver, iHostAddr, iHostIP);
	if( nErr != TTKErrNone)
	{
		return nErr;
	}

	nErr = ConnectServer(aObserver, iHostIP, nPort); 
	if( nErr != TTKErrNone)
	{
		return nErr;
	}

	return SendRequestAndParseResponse(&CTTHttpClient::Connect, aObserver, aUrl, nPort, aOffset);
}

TTInt CTTHttpClient::SendRequestAndParseResponse(_pFunConnect pFunConnect, ITTStreamBufferingObserver* aObserver, const TTChar* aUrl, TTInt aPort, TTInt aOffset)
{
	//send get file request
	LOGI("[Connected]: TT_Send_Request_C: offset = %d", aOffset);
    TTInt nErr = SendRequest(aPort, aOffset);
    if (nErr == TTKErrNone)
    {
		TTUint32 nStatusCode = STATUS_OK;
        nErr = ParseResponseHeader(aObserver, nStatusCode);
        if (nErr == TTKErrNone)
        {
            LOGI("[Connected]: TT_Respones_C ,Status Code = %d",nStatusCode);
            if (IsRedirectStatusCode(nStatusCode))
            {
                return Redirect(pFunConnect, aObserver, aOffset);
            }
            else if (nStatusCode == 200 || nStatusCode == 206)
            {
                nErr = ParseContentLength(nStatusCode);
            }
            else
			{
				iStatusCode = nStatusCode;
				nErr = TTKErrCouldNotConnect;
			}
		}
	}
    
    if ((nErr != TTKErrNone) && (iState == CONNECTED))
	{
        nErr = TTKErrCouldNotConnect;
		LOGE("connect failed. Connection is going to be closed");
		Disconnect();
	}
	
	struct timeval nRecvDataTimeOut = {0, 500000};
	SetSocketTimeOut(iSocketHandle, nRecvDataTimeOut);

	return nErr;
}

TTInt CTTHttpClient::Redirect(_pFunConnect pFunConnect, ITTStreamBufferingObserver* aObserver, TTInt aOffset)
{
	TTInt nErr = GetHeaderValueByKey(KLocationKey, iHeaderValueBuffer, sizeof(TTChar)*KMaxLineLength);
	Disconnect();
	if (TTKErrNone == nErr)
	{
		 memcpy(iRedirectUrl,iHeaderValueBuffer,sizeof(iRedirectUrl));
		 return (this->*pFunConnect)(aObserver, iHeaderValueBuffer, aOffset);
	}
	else
	{
		nErr = TTKErrCouldNotConnect;
	}
	return nErr;
}

TTInt CTTHttpClient::ResolveDNS(ITTStreamBufferingObserver* aObserver, TTChar* aHostAddr, TTUint32& aHostIP)
{
	aHostIP = iDNSCache->get(aHostAddr);//aHostAddr
    TTInt loopWaitCnt = 0;
    TTInt i;
    TTBool parseRet;
	if (aHostIP == IP_NOT_FOUND)
	{
        for(i=0;i< strlen(aHostAddr);i++)
        {
            if (aHostAddr[i] == '.' || (aHostAddr[i] >= '0' && aHostAddr[i] <= '9'))
            {
                continue;
            }
            else
                break;
        }
        if (i == strlen(aHostAddr))
        {
            aHostIP = inet_addr(aHostAddr);
            if (aHostIP == INADDR_NONE)
            {
                LOGE("inet_addr error:host = %s",aHostAddr);
                iStatusCode = DNS_ERROR_BASE + INET_ADDR_EXCEPTION;
                return TTKErrCouldNotConnect;
            }
            else
            {
                return TTKErrNone;
            }
        }
          
        if (iCancel)
        {
            return TTKErrCouldNotConnect;
        }
        

	    parseRet = ETTFalse;
		struct addrinfo hints;
		struct addrinfo *res;
		int ret;
		struct	in_addr aIP;

		memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET; /* Allow IPv4 */
        hints.ai_socktype = SOCK_STREAM;
        ret = getaddrinfo(aHostAddr, NULL,&hints,&res);
        if (ret == 0 && res != NULL) {
            aIP = ((struct sockaddr_in*)(res->ai_addr))->sin_addr;
            char* pHostIP = inet_ntoa(aIP);
            aHostIP = inet_addr(pHostIP);
			freeaddrinfo(res);
			parseRet = ETTTrue;
        } else {
			 iStatusCode = DNS_ERROR_BASE + ret;
        }
        
        if (parseRet) {
            char* pHostIP = inet_ntoa(*((struct in_addr*)&aHostIP));
            if (strcmp(pHostIP, aHostAddr) != 0)
            {
                iDNSCache->put(aHostAddr, aHostIP);
            }
            //LOGE("HostIP = %s ",pHostIP);
            if(aObserver != NULL)
            {
                aObserver->DNSDone();
            }
            return TTKErrNone;
        }
              
        //parse fail
        return TTKErrCouldNotConnect;
	}

	return TTKErrNone;
}

TTInt CTTHttpClient::ConnectServer(ITTStreamBufferingObserver* aObserver, TTUint32 nHostIP, TTInt& nPortNum)
{
	if((iSocketHandle = socket(AF_INET, SOCK_STREAM, 0)) == KInvalidSocketHandler)
	{
		LOGE("socket error");
		iStatusCode = Status_Error_NoUsefulSocket;
		return TTKErrCouldNotConnect;
	}
	iState = CONNECTING;

	SetSocketNonBlock(iSocketHandle);
	
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	//bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nPortNum);
	server_addr.sin_addr = *(struct in_addr *)(&nHostIP);
	TTInt nErr = connect(iSocketHandle, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr));
	if (nErr < 0)
	{
		iStatusCode = errno + CONNECT_ERROR_BASE;
#ifdef __TT_OS_WINDOWS__
		if(nErr == -1)
#else
		if (errno == EINPROGRESS)
#endif
		{
			timeval timeout = {CONNECTION_TIMEOUT_IN_SECOND, 0};
			nErr = WaitSocketWriteBuffer(iSocketHandle, timeout);
		}

		if (nErr < 0)
		{
			if (nErr == TTKErrTimedOut)
			{
				iStatusCode = Status_Error_ServerConnectTimeOut;
				iDNSCache->del(iHostAddr); 
			}

			LOGE("connect error. nErr: %d, errorno: %d", nErr, errno);
			Disconnect();
			SetSocketBlock(iSocketHandle);
			return TTKErrCouldNotConnect;
		}
	}	

	SetSocketBlock(iSocketHandle);

	if (aObserver != NULL)
	{
		aObserver->ConnectDone();
	}

	iState = CONNECTED;

	return TTKErrNone;
}

TTInt CTTHttpClient::Disconnect()
{
	if ((iState == CONNECTED || iState == CONNECTING) && (iSocketHandle != KInvalidSocketHandler))
	{
#ifdef __TT_OS_WINDOWS__
		closesocket(iSocketHandle);
#else
		close(iSocketHandle);
#endif
		LOGI("CTTHttpClient::Disconnect closed");

		iSocketHandle = KInvalidSocketHandler;
		iState = DISCONNECTED;
	}
	iTransferBlock = ETTFalse; 
	memset(iRedirectUrl,0,sizeof(iRedirectUrl));
#ifndef __TT_OS_WINDOWS__
	iConnectionTid = 0;
#else
	memset(&iConnectionTid, 0, sizeof(iConnectionTid));
#endif

	iCancel = ETTFalse;
	LOGI("CTTHttpClient::Disconnect return");
	return 0;
}

CTTHttpClient::CTTHttpClient()
: iContentLength(KInvalidContentLength)
, iSocketHandle(KInvalidSocketHandler)
, iState(DISCONNECTED)
, iWSAStartup(0)
{
#ifndef __TT_OS_WINDOWS__
   iConnectionTid = 0;
#endif
   iCancel = ETTFalse;

#ifdef __TT_OS_WINDOWS__
   	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD( 2, 2 );
	iWSAStartup = WSAStartup( wVersionRequested, &wsaData );
#else
	struct sigaction act, oldact;
	act.sa_handler = SignalHandle;
	act.sa_flags = SA_NODEFER; 
	//sigaddset(&act.sa_mask, SIGALRM);
 	sigaction(SIGALRM, &act, &oldact);
	//signal(SIGPIPE, SIG_IGN);
#endif
	memset(&iConnectionTid, 0, sizeof(iConnectionTid));
	memset(iRedirectUrl,0,sizeof(iRedirectUrl));

	iHostIP = 0;
	iStatusCode = 0;
	iCancel = ETTFalse;

	if(iDNSCache == NULL)
	{
		iDNSCache = new CTTDNSCache;
	}
}

CTTHttpClient::~CTTHttpClient()
{
	if (iState == CONNECTED)
	{
		Disconnect();
	}
#ifdef __TT_OS_WINDOWS__
	WSACleanup();
#endif
}

TTBool CTTHttpClient::IsCancel()
{
    return iCancel;
}

TTInt CTTHttpClient::ParseHeader(TTUint32& aStatusCode)
{
	TTChar tLine[KMaxLineLength];

	TTInt nErr = ReceiveLine(tLine, sizeof(tLine));
	if (nErr != TTKErrNone)
	{
		LOGE("CTTHttpClient Receive Response Error!");
		return nErr;
	}

	TTChar* pSpaceStart = strchr(tLine, ' ');
	if (pSpaceStart == NULL) 
	{
		LOGE("CTTHttpClient Receive Response content Error!");
		return TTKErrBadDescriptor;
	}

	TTChar* pResponseStatusStart = pSpaceStart + 1;
	TTChar* pResponseStatusEnd = pResponseStatusStart;
	while (isdigit(*pResponseStatusEnd)) 
	{
		++pResponseStatusEnd;
	}

	if (pResponseStatusStart == pResponseStatusEnd) 
	{
		return TTKErrBadDescriptor;
	}

	memmove(tLine, pResponseStatusStart, pResponseStatusEnd - pResponseStatusStart);
	tLine[pResponseStatusEnd - pResponseStatusStart] = '\0';

	TTInt32 nResponseNum = strtol(tLine, NULL, 10);
	if ((nResponseNum < 0) || (nResponseNum > 999))
	{
		LOGE("CTTHttpClient Receive Invalid ResponseNum!");
		return TTKErrBadDescriptor;
	}

	aStatusCode = nResponseNum;

	return TTKErrNone;
}

 TTInt CTTHttpClient::ConvertToValue(TTChar * aBuffer)
 {
	 TTInt size = strlen(aBuffer);
	 TTInt i=0;
	 TTInt value = 0;
	 while(i<size)
	 {
		 if(aBuffer[i] >= '0' && aBuffer[i] <= '9')
		 {
			 value = value* 16 +(aBuffer[i]-'0');
		 }
		 else  if(aBuffer[i] >= 'a' && aBuffer[i] <= 'f')
		 {
			 value = value* 16 +(aBuffer[i]-'a' + 10);
		 }
		 else  if(aBuffer[i] >= 'A' && aBuffer[i] <= 'F')
		 {
			 value = value* 16 +(aBuffer[i]-'A' + 10);
		 }
		 else
			 return -1;

		 i++;
	  }
	 return value;
 }

TTInt CTTHttpClient::GetHeaderValueByKey(const TTChar* aKey, TTChar* aBuffer, TTInt aBufferLen)
{
	TTInt nErr = TTKErrArgument;
    TTBool bIsKeyFound = ETTFalse;
	TTBool bIsKeyChange = ETTFalse;

	if(0 == strcmp(aKey, KContentLengthKey))
		bIsKeyChange = ETTTrue;

	LOGI("CTTHttpClient::GetHeaderValueByKey %s", aKey);
	while (ETTTrue)
	{
		nErr = ReceiveLine(iLineBuffer, sizeof(TTChar) * KMaxLineLength);

		if (nErr != TTKErrNone)
		{
			LOGE("CTTHttpClient RecHeader Error:%d", nErr);
			break;
		}

		if(iTransferBlock)
		{
			if (iLineBuffer[0] == '\0')
			{
				nErr = TTKErrNone;
				break;
			}
			else
				continue;
		}

		if (iLineBuffer[0] == '\0')
		{
			nErr = bIsKeyFound ? TTKErrNone : TTKErrEof;
			break;
		}

		TTChar* pColonStart = strchr(iLineBuffer, ':');
		if (pColonStart == NULL) 
		{
			continue;			
		} 

		TTChar* pEndofkey = pColonStart;

		while ((pEndofkey > iLineBuffer) && isspace(pEndofkey[-1])) 
		{
			--pEndofkey;
		}

		TTChar* pStartofValue = pColonStart + 1;
		while (isspace(*pStartofValue)) 
		{
			++pStartofValue;
		}

		*pEndofkey = '\0';

		if (strncmp(iLineBuffer, aKey, strlen(aKey)) != 0)
		{
			if(bIsKeyChange){
				if (strncmp(iLineBuffer, KTransferEncodingKey, strlen(KTransferEncodingKey)) == 0)
				{
					 iTransferBlock = ETTTrue;
					 iContentLength = 0;
				}
			}
			
			continue;
		}		

		if (aBufferLen > strlen(pStartofValue))
		{
            bIsKeyFound = ETTTrue;
			strcpy(aBuffer, pStartofValue);
		}		
	}
	LOGI("CTTHttpClient::GetHeaderValueByKey return %d", nErr);
	return nErr;
}

TTInt CTTHttpClient::ReceiveLine(TTChar* aLine, TTInt aSize)
{
	if (iState != CONNECTED) {
		return TTKErrNotReady;
	}

	TTBool bSawCR = ETTFalse;
	TTInt nLength = 0;
	TTChar log[2048];
	memset(log, 0, sizeof(TTChar) * 2048);

	while (ETTTrue) 
	{
		char c;
		TTInt n = Recv(&c, 1);
		if (n <= 0) 
		{
			strncpy(log, aLine, nLength);
			LOGI("log: %s, logLength: %d", log, nLength);
			if (n == 0)
			{
				iStatusCode = Status_Error_HttpResponseTimeOut;
				return TTKErrTimedOut;
			}
			else
			{
				return TTKErrCouldNotConnect;
			}
		} 

		if (bSawCR && c == '\n') 
		{
			aLine[nLength - 1] = '\0';
			//strncpy(log, aLine, nLength);
			//LOGI("log: %s, logLength: %d", log, nLength);
			return TTKErrNone;
		}

		bSawCR = (c == '\r');

		if (nLength + 1 >= aSize) 
		{
			return TTKErrOverflow;
		}

		aLine[nLength++] = c;
	}
}

void  CTTHttpClient::Interrupt()
{
#ifndef __TT_OS_WINDOWS__
	if (iConnectionTid > 0 && !pthread_equal(iConnectionTid, pthread_self()))
	{
		int pthread_kill_err = pthread_kill(iConnectionTid, 0);
		if((pthread_kill_err != ESRCH) && (pthread_kill_err != EINVAL))
		{
			pthread_kill(iConnectionTid, SIGALRM);
			LOGI("sent interrupt signal");
		}
	}
#endif
	
	iCancel = ETTTrue;
}

TTUint32 CTTHttpClient::HostIP()
{
	return iHostIP;
}

TTUint32 CTTHttpClient::StatusCode()
{
	return iStatusCode;
}

TTInt CTTHttpClient::HttpStatus()
{
	return (TTInt)iState;
}

void CTTHttpClient::SetStatusCode(TTUint32 aCode)
{
    //just for test, will delete later!
    if (aCode == ERROR_CODE_TEST || aCode == ERROR_CODE_TEST + 2) {
        aCode = aCode<<1;
    }
    
	iStatusCode = aCode;
}

void CTTHttpClient::ReleaseDNSCache()
{
	SAFE_DELETE(iDNSCache);
}

void  CTTHttpClient::SetSocketCheckForNetException()
{
#ifdef __TT_OS_IOS__
    int keepalive = TIME_KEEP_ALIVE;
    int keepidle = TIME_KEEP_IDLE;
    int keepintvl = TIME_KEEP_INTVL;
    int keepcnt = TIME_KEEP_CNT;
    if(iSocketHandle != KInvalidSocketHandler){
        setsockopt(iSocketHandle, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof (keepalive));
        setsockopt(iSocketHandle, IPPROTO_TCP, TCP_KEEPALIVE, (void *) &keepidle, sizeof (keepidle));
        setsockopt(iSocketHandle, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepintvl, sizeof (keepintvl));
        setsockopt(iSocketHandle, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepcnt, sizeof (keepcnt));
    }
#endif
}

TTChar* CTTHttpClient::GetRedirectUrl()
{
	if(strlen(iRedirectUrl) == 0)
		return NULL;
	else
		return iRedirectUrl;
}
