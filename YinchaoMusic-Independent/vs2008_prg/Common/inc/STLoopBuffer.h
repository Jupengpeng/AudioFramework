#ifndef __ST_LOOP_BUFFER_H__
#define __ST_LOOP_BUFFER_H__

#include "STTypedef.h"
#include "STMacrodef.h"
#include "STLog.h"
#include "STBufferConfig.h"

template<class T>
class STLoopBuffer
{
public:
    STLoopBuffer(STInt aCnt);
    ~STLoopBuffer();

    template<class T2>STInt SetData(T2* aBuffer, STInt aElementCnt);
    template<class T2>STInt GetData(T2* aBuffer, STInt aElementCnt);
    STInt GetElementCnt();
    T Pop();
    STInt Realloc(STInt aCnt);
    void Reset();
    STUint64 GetTotalCnt();

private:
    void LoopPtr();
    STInt GetIdleBytes();
    STInt GetIdleElementCnt();
    
private:
    T* iBasePtr;
    T* iStartPtr;
    T* iEndPtr;
    STInt iMaxCnt;
    STInt iRealCnt;
    STInt iDataCnt;
    STUint64 iTotal;
};

template<class T>
STLoopBuffer<T>::STLoopBuffer(STInt aCnt)
{
    iMaxCnt = aCnt+1;
    iBasePtr = new T[iMaxCnt];
    iStartPtr = iBasePtr;
    iEndPtr = iBasePtr;
    iRealCnt = aCnt;
    iDataCnt = 0;
    iTotal = 0;
}

template<class T>
STLoopBuffer<T>::~STLoopBuffer()
{
    SAFE_DELETE_ARRAY(iBasePtr);
}

template<class T>
inline T STLoopBuffer<T>::Pop()
{
    if(iDataCnt <= 0)
    {
        STLOGE("No data to pop");
        return 0;
    }
    T nVal = (T)*iStartPtr++;
    LoopPtr();
    iDataCnt--;
    return nVal ;
}

template<class T>
STInt STLoopBuffer<T>::Realloc(STInt aCnt)
{
    STInt nCnt = KTotalPCMBufferSize;
    
    while(nCnt < aCnt)
    {
        nCnt += KTotalPCMBufferSize;
    }
    
    T* pTemp = new T[iMaxCnt+nCnt];
    if (pTemp == NULL)
    {
        STLOGE("No memory");
        STASSERT(ESTFalse);
        return STKErrNoMemory;
    }

    STInt nDataCnt = iDataCnt;
    GetData(pTemp, nDataCnt);

    SAFE_DELETE(iBasePtr);
    iBasePtr = pTemp;
    iStartPtr = iBasePtr;
    iEndPtr = iBasePtr + nDataCnt;
    iMaxCnt += nCnt;
    iRealCnt = iMaxCnt - 1;
    iDataCnt = nDataCnt;

    STLOGI("Exit, realloc=%d(byte), new total size=%d(byte)", nCnt, iRealCnt*sizeof(T));

    return STKErrNone;
}

template<class T>
inline void STLoopBuffer<T>::LoopPtr()
{
    if (iEndPtr >= iBasePtr + iMaxCnt)
    {
        iEndPtr = iBasePtr;
    }
    if (iStartPtr >= iBasePtr + iMaxCnt)
    {
        iStartPtr = iBasePtr;
    }
}

template<class T>
template<class T2>
STInt STLoopBuffer<T>::SetData(T2* aBuff, STInt aElementCnt)
{
    STInt nErr = STKErrNone;
    if( GetIdleElementCnt() < aElementCnt)
    {
        if((nErr = Realloc(aElementCnt - GetIdleElementCnt())) != STKErrNone)
        {
            return nErr;
        }
    }

    STInt nCnt = 0;
    if(sizeof(T) < sizeof(T2))
    {
        nCnt = aElementCnt;
        for(int i = 0; i < nCnt; i++)
        {
            *iEndPtr++ = (T)aBuff[i];
            LoopPtr();
        }
    }
    else
    {
        nCnt = aElementCnt*sizeof(T2)/sizeof(T);
        T* pTBuffer = (T*)aBuff;
        for(int i = 0; i < nCnt; i++)
        {
            *iEndPtr++ = pTBuffer[i];
            LoopPtr();
        }
    }
    iDataCnt += nCnt;
    iTotal += nCnt;
    return nErr;
}

template<class T>
template<class T2>
STInt STLoopBuffer<T>::GetData(T2* aBuff, STInt aElementCnt)
{
    int nValidCnt = iDataCnt < aElementCnt ? iDataCnt : aElementCnt;
    for(int i = 0; i < nValidCnt; i++)
    {
        aBuff[i] = (T2)*iStartPtr++;
        LoopPtr();
    }
    iDataCnt -= nValidCnt;
    return nValidCnt;
}

template<class T>
inline void STLoopBuffer<T>::Reset()
{
    iStartPtr = iBasePtr;
    iEndPtr = iBasePtr;
    iDataCnt = 0;
}

template<class T>
inline STInt STLoopBuffer<T>::GetIdleBytes()
{
    return GetIdleElementCnt()/sizeof(T);
}

template<class T>
inline STInt STLoopBuffer<T>::GetIdleElementCnt()
{
    return (iRealCnt - iDataCnt);
}

template<class T>
inline STInt STLoopBuffer<T>::GetElementCnt()
{
    return iDataCnt;
}

template<class T>
inline STUint64 STLoopBuffer<T>::GetTotalCnt()
{
    return iTotal;
}

#endif

