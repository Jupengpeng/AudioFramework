#ifndef __TTEventThread_H__
#define __TTEventThread_H__

#include <pthread.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "TTCritical.h"
#include "TTCondition.h"
#include "TTList.h"

#define TIME_MIN   (-0x7fffffffffffffff - 1) 
#define TIME_MAX   0x7fffffffffffffff  
/**
*the status of the thread	
*/
typedef enum{
	TT_THREAD_STATUS_INIT				= 0,	/*!<The status is init */
	TT_THREAD_STATUS_RUN				= 1,	/*!<The status is running */
	TT_THREAD_STATUS_PAUSE				= 2,	/*!<The status is paused */
	TT_THREAD_STATUS_STOP				= 3		/*!<The status is stopped*/
}TT_THREAD_STATUS;

typedef enum 
{
	EEventMsg = 1,
	EEventAudioProcoess = 2,
	EEventAudioRender = 3,
	EEventVideoDeocder = 4,
	EEventVideoRender = 5,
	EEventLoad = 6,
	EEventClose = 7,
	EEventStream = 8,
    EEventVideoRenderRepeat = 9,
}EPlayEventType;

class TTBaseEventItem
{
public:
	TTBaseEventItem (TTInt nType, TTInt nMsg = 0, TTInt nVar1 = 0, TTInt nVar2 = 0, void* pVar3 = 0)
		:mType(nType)
		,mID(0)
		,mTime(-1)
		,mMsg(nMsg)
		,mVar1(nVar1)
		,mVar2(nVar2)
		,mVar3(pVar3){

	}

	virtual ~TTBaseEventItem (void){
	}

public:
	virtual void		fire (void) = 0;
	void				setEventType (int nType) {mType = nType;}
	TTInt				getEventType (void) {return mType;}
	void				setEventID (int nID) {mID = nID;}
	TTInt				getEventID (void) {return mID;}
	TTInt				getEventMsg (void) {return mMsg;}
	TTInt64				getTime (void) {return mTime;}
	void				setTime (TTInt64 nTime) {mTime = nTime;}
	void				setEventMsg (TTInt nMsg, TTInt nVar1 = 0, TTInt nVar2 = 0, void* pVar3 = 0) {mMsg = nMsg; mVar1 = nVar1; mVar2 = nVar2; mVar3 = pVar3;}

protected:
	TTInt				mType;
	TTInt				mID;
	TTInt64				mTime;

	TTInt				mMsg;
	TTInt				mVar1;
	TTInt				mVar2;
	void*				mVar3;


public:
	 TTBaseEventItem(const TTBaseEventItem &Item) {
		this->mType = Item.mType;
		this->mID = Item.mID;
		this->mMsg = Item.mMsg;
		this->mVar1 = Item.mVar1;
		this->mVar2 = Item.mVar2;
		this->mVar3 = Item.mVar3;
	}

    TTBaseEventItem &operator=(const TTBaseEventItem &);
};

class TTEndEvent : public TTBaseEventItem
{
public:
	TTEndEvent()
		:TTBaseEventItem(EEventClose) { ; }

	~TTEndEvent() {;}

	virtual void		fire (void) {;}
};

class TTEventThread
{
public:
	TTEventThread(const char * pThreadName);
	virtual ~TTEventThread(void);

	virtual	TTInt				start ();
	virtual	TTInt				stop (void);

	virtual TT_THREAD_STATUS	getStatus (void);

	virtual TTInt				postEventWithRealTime (TTBaseEventItem * pEvent, TTInt64 nRealTime);
	virtual TTInt				postEventWithDelayTime (TTBaseEventItem * pEvent, TTInt64 nDelayTime);

	virtual TTBaseEventItem *	cancelEvent (TTBaseEventItem * pEvent);
	virtual TTBaseEventItem *	cancelEventByID (TTInt nID, bool stopAfterFirstMatch = true);
	virtual TTBaseEventItem * 	cancelEventByType (TTInt nType, bool stopAfterFirstMatch = true);
	virtual TTBaseEventItem * 	cancelEventByMsg (TTInt nMsg, bool stopAfterFirstMatch = true);

	virtual TTInt				cancelAllEvent (void);
	
	virtual TTInt				freeAllEvent (void);

	virtual TTInt				getFullEventNum (TTInt nType);
	virtual TTBaseEventItem *	getEventByType (TTInt nType);

protected:
	void						TTThreadSetName(const char* threadName);

	pthread_t					mThreadId;				
	bool						mThreadExisted;			
	bool						mThreadTerminating;	    

protected:
	RTTCritical			mEventCritical;
	RTTCritical			mStatusCritical;
	RTTCondition		mConditionHeadChanged;
	RTTCondition		mConditionNotEmpty;
	TTInt				mNextEventID;
	TTChar				mName[512];
	TT_THREAD_STATUS	mStatus;

	List<TTBaseEventItem *>	mListFull;
	List<TTBaseEventItem *>	mListFree;

public:
	static	TTInt		eventBaseThreadProc (void* pParam);
	virtual TTInt		eventBaseThreadLoop (void);
};
    
#endif // __TTEventThread_H__
