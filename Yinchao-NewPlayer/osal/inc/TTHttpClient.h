#ifndef __TT_HTTP_CLIENT_H__
#define __TT_HTTP_CLIENT_H__
#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTOsalConfig.h"
#include "TTDNSCache.h"
#include <string.h>

#ifdef __TT_OS_WINDOWS__
#include <winsock2.h>
#include "Ws2tcpip.h"
#else
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#define MAXDOMAINNAME 256

static const TTInt KMaxHostAddrLen = 256;
static const TTInt KMaxHostFileNameLen = 512;
static const TTInt KMaxRequestLen = 1024;
static const TTInt KMaxLineLength = 2048;

typedef struct _DNSParam{
    TTChar   domainName[MAXDOMAINNAME];
    TTUint32 ip;
    TTInt    errorcode;
    _DNSParam(){
        memset(domainName, 0, MAXDOMAINNAME);
        errorcode = -1;
        ip = 0;
    }
} DNSParam;

class ITTStreamBufferingObserver;
class CTTHttpClient;

typedef TTInt (CTTHttpClient::*_pFunConnect)(ITTStreamBufferingObserver* aObserver, const TTChar* aUrl, TTInt aOffset);


class CTTHttpClient
{
public:
	/**
	* \fn							CTTHttpClient();
	* \brief						构造函数
	*/
	CTTHttpClient();

	/**
	* \fn							~CTTHttpClient();
	* \brief						析构函数
	*/
	~CTTHttpClient();
	
	/**
	* \fn							TTInt Read(TTChar* aDstBuffer, TTInt aSize);
	* \brief						从当前位置读取流
	* \param[in] aDstBuffer			用于填充数据的Buffer		
	* \param[in] aSize			    读取的数据大小	
	* \return						读取的字节数或者错误码
	*/
	TTInt							Read(TTChar* aDstBuffer, TTInt aSize);

	/**
	* \fn							TTInt Recv(TTChar* aDstBuffer, TTInt aSize);
	* \brief						从服务器接收数据
	* \param[in] aDstBuffer			用于接收数据的Buffer		
	* \param[in] aSize			    接收的数据大小	
	* \return						接收的字节数或者错误码
	*/
	TTInt							Recv(TTChar* aDstBuffer, TTInt aSize);

	/**
	* \fn							TTInt Send(TTChar* aDstBuffer, TTInt aSize);
	* \brief						向服务器发送数据
	* \param[in] aSendBuffer		发送数据的Buffer		
	* \param[in] aSize			    发送的数据大小	
	* \return						操作状态
	*/
	TTInt							Send(const TTChar* aSendBuffer, TTInt aSize);

	/**
	* \fn							TTInt Connect(const ITTStreamBufferingObserver* aObserver, const TTChar* aUrl, TTInt aOffset = 0);
	* \brief						连接服务器
	* \param[in] aObserver			回调接口
	* \param[in] aUrl				资源路径
	* \param[in] aOffset			读取偏移
	* \return						操作状态
	*/
	TTInt							Connect(ITTStreamBufferingObserver* aObserver, const TTChar* aUrl, TTInt aOffset = 0);
    
    /**
     * \fn							TTInt ConnectProxyServer(const ITTStreamBufferingObserver* aObserver, const TTChar* aUrl, TTInt aOffset = 0);
     * \brief						连接服务器
     * \param[in] aObserver			回调接口
     * \param[in] aUrl				资源路径
     * \param[in] aOffset			读取偏移
     * \return						操作状态
     */
    TTInt							ConnectViaProxy(ITTStreamBufferingObserver* aObserver, const TTChar* aUrl, TTInt aOffset = 0);
    
	/**
	* \fn							TTInt Disconnect();
	* \brief						断开连接		
	* \return						操作状态
	*/
	TTInt							Disconnect();

	/**
	* \fn							TTInt ContentLength();
	* \brief						获取资源文件大小		
	* \return						资源文件大小(byte)
	*/
	INLINE_C TTInt					ContentLength() { return iContentLength;};	

	/**
	* \fn							void Interrupt();
	* \brief						发送signal给connect thread，中断其网络操作
	*/
	void							Interrupt();

	/**
	* \fn							void ReleaseDNSCache()
	* \brief						释放DNS缓存
	*/
	static	void					ReleaseDNSCache();

	/**
	* \fn							TTUint32 HostIP();
	* \brief						获取连接主机的IP
	* \return						连接主机的IP
	*/
	TTUint32						HostIP();

	/**
	* \fn							TTUint32 StatusCode();
	* \brief						获取连接响应状态码
	* \return						连接响应状态码
	*/
	TTUint32						StatusCode();


	TTInt							HttpStatus();

	/**
	* \fn							void SetStatusCode();
	* \brief						设定连接响应状态码
	*/
	void							SetStatusCode(TTUint32 aCode);
    
    TTBool							IsCancel();

    void                            SetSocketCheckForNetException();

	TTBool							IsTtransferBlock(){return iTransferBlock;};

	TTInt							RequireContentLength();

	TTInt							ConvertToValue(TTChar * aBuffer);

	TTChar*							GetRedirectUrl();
	

private:
	TTBool				IsRedirectStatusCode(TTUint32 aStatusCode);
	TTInt				ReceiveLine(TTChar* aLine, TTInt aSize);
	TTInt				ParseHeader(TTUint32& aStatusCode);
	TTInt				GetHeaderValueByKey(const TTChar* aKey, TTChar* aBuffer, TTInt aBufferLen);

	TTInt				ConnectServer(ITTStreamBufferingObserver* aObserver, TTUint32 nHostIP, TTInt& nPortNum);
	TTInt				ResolveDNS(ITTStreamBufferingObserver* aObserver, TTChar* aHostAddr, TTUint32& aHostIP);
	TTInt				SendRequest(TTInt aPort, TTInt aOffset);
	TTInt				ParseResponseHeader(ITTStreamBufferingObserver* aObserver, TTUint32& aStatusCode);
    
	TTInt				Redirect(_pFunConnect pFunConnect, ITTStreamBufferingObserver* aObserver, TTInt aOffset);
	TTInt				SendRequestAndParseResponse(_pFunConnect pFunConnect, ITTStreamBufferingObserver* aObserver, const TTChar* aUrl, TTInt aPort, TTInt aOffset);
	
	TTInt				ParseContentLength(TTUint32 aStatusCode);

	TTInt				Receive(TTInt& aSocketHandle, timeval& aTimeOut, TTChar* aDstBuffer, TTInt aSize);
	TTInt				WaitSocketWriteBuffer(TTInt& aSocketHandle, timeval& aTimeOut);
	TTInt				WaitSocketReadBuffer(TTInt& aSocketHandle, timeval& aTimeOut);
	TTInt				SetSocketTimeOut(TTInt& aSocketHandle, timeval aTimeOut);

	void				SetSocketBlock(TTInt& aSocketHandle);
	void				SetSocketNonBlock(TTInt& aSocketHandle);

private:
	enum State {
	        DISCONNECTED,
	        CONNECTING,
	        CONNECTED
	    };

	State		iState;
	TTInt		iSocketHandle;
	TTInt		iContentLength;
	TTInt		iWSAStartup;

	TTChar		iLineBuffer[KMaxLineLength];
	TTChar		iHeaderValueBuffer[KMaxLineLength];
	pthread_t   iConnectionTid;

	TTChar		iHostAddr[KMaxHostAddrLen];
	TTChar		iHostFileName[KMaxHostFileNameLen];
	TTChar		iRequset[KMaxRequestLen];

	static CTTDNSCache* iDNSCache;
	TTUint32	iHostIP;
	TTUint32	iStatusCode;
    TTBool		iCancel;

	TTBool		iTransferBlock;
	TTChar		iRedirectUrl[KMaxLineLength];
};

#endif
