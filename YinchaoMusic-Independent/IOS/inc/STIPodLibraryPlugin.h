#ifndef __ST_IPOD_LIBRARY_PLUGIN_H__
#define __ST_IPOD_LIBRARY_PLUGIN_H__


#include "STBasePlugin.h"

class STIPodLibraryPlugin : public STBasePlugin
{ 
public:
    static STBool               IsSourceValid(const STChar* aUrl);
    
public:
    STIPodLibraryPlugin();
    ~STIPodLibraryPlugin();

public:	
	/**
     * \fn				            STInt InitPlugin(const STChar* aUrl)
     * \param						Url路径
     * \brief				        初始化解码器
     */
    STInt							InitPlugin(const STChar* aUrl);
    
	/**
     * \fn				            void ResetPlugin()
     * \brief				        复位解码器
     */
	void							ResetPlugin();
    
	/**
     * \fn				            STInt StartReading()
     * \brief				        开始读取PCM数据
     * \return						STKErrNone: 成功
     */
	STInt							StartReading();
	
	/**
     * \fn				            STInt Read(STSampleBuffer* aBuffer)
     * \brief				        读取PCM,在aBuffer中
     * \param[out] aBuffer			Buffer指针
     * \return						错误码
     */
	STInt							Read(STSampleBuffer* aBuffer);
    
	/**
     * \fn				            void Seek(STUint aPos)
     * \brief				        Seek操作
     * \param[in]					Seek到的位置，毫秒为单位
     */
	void							Seek(STUint aPos);
    
    /**
     * \fn				            void UnInitPlugin()
     * \brief				        释放解码器
     */
	 void                           UnInitPlugin();
    
protected:
    void                            PreSampleRefRelease();
    STInt                           StartReadingAt(STUint aTime);
    STInt                           ReadNextSampleBuffer();
    void                            ReleaseReader();
    void                            ReleaseAsset();
    STInt                           FillSampleBuffer(STSampleBuffer* aBuffer);
    
private: 
    void*                           iAsset;
    void*                           iAssetReader;
    void*                           iAssetReaderOutput;
    void*                           iCurSampleBufferRef;
    void*                           iAudioArray;
    STInt64                         iTotalReadSize;
    STUint                          iCurSampleBufferDataOffset;
    STUint                          iCurSampleBufferTotalLen;
    STChar*                         iCurSampleBufferDataPtr;
};
#endif
