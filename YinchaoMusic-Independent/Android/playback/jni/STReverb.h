//add by bin.chen
#ifndef STREVERB_H_
#define STREVERB_H_
#include <freeverb/irmodels.hpp>
#include <freeverb/irmodel.hpp>
#include <freeverb/irmodel2.hpp>
#include <freeverb/irmodel3.hpp>
#include <freeverb/utils.hpp>
#include <freeverb/nrev.hpp>

#include <freeverb/nrevb.hpp>
#include <freeverb/slot.hpp>
#include <freeverb/fv3_ch_tool.hpp>

//该两个头文件必须在freeverb后面，不然报错
//#include <sndfile.h>
//#include <sndfile.hh>
//#include <fftw3.h>
using namespace fv3;

//#define DEFAULTMAXVFS 192000
#define DEFAULTMAXVFS 44100
#define DEFAULTFS 44100

#define DEFAULTFS_8K 8000
#define DEFAULTFS_16K 16000
#define BUFSIZE 44100  //暂时设置这么大

#ifdef PLUGDOUBLE
typedef fv3::irbase_ IRBASE;
typedef fv3::irmodel_ IR;
typedef fv3::irmodel2_ IR2;
typedef fv3::irmodel3_ IR3;
typedef fv3::irmodels_ IRS;
typedef fv3::utils_ UTILS;
typedef double pfloat_t;
#else
typedef fv3::irbase_f IRBASE;
typedef fv3::irmodel_f IR;
typedef fv3::irmodel2_f IR2;
typedef fv3::irmodel3_f IR3;
typedef fv3::irmodels_f IRS;
typedef fv3::utils_f UTILS;
typedef float pfloat_t;
#endif

#define sf_count_t long

class WavFile
{
private:
	char riff_id[5];			// "RIFF"	
	int  size0;					// 波形块的大小
	char wave[5];				// "wave"
	char fmt[5];				//"fmt"
	int  size1;					// 格式块大小
	short  fmttag;		  		// 波形编码格式
	short  channel;	  			// 波形文件数据中的通道数
	int sampl;					// 波形文件的采样率
	int byte; 					// 平均每秒波形音频所需要的记录的字节数
	short  blockalign;    		// 一个采样所需要的字节数
	short  bitpersamples; 		// 声音文件数据的每个采样的位数
	char data[5];           	// "data"
	int datasize;             	// 数据大小
	char *dat;					//数据
	bool opFlag;                 //文件可读及可加载标志 
private:
	FILE *m_pFile;
	long audioDataIndex;
public:
	WavFile();
	//WavFile(const char *path);
	WavFile(const char *path,int cacheFlag/*1为直接读完缓存到data,为0只读wav头数据不读*/);
	int frames();
	short channels();
	long readf(short *inputStream,int frames);
	long readf(float *inputStream,int frames);
	void reset();
	bool isLoad();
	virtual ~WavFile();
private:
	void ShortToFloatArray(const short *in, float *out, long len);
	void FloatToShortArray (const float *in, short *out, long len);
};

class STReverb
{
public:
	STReverb();
	virtual ~STReverb();
public:
	bool SetIRParamter(const char*pathName,int drydB,int wetdB,int frameSize);
	bool init(int dryDB/*干声衰减值,如-5*/,int wetDB/*湿声衰减值*/,int frameSize/*每次处理数据大小,以short为单位*/);
	//bool SetCommReverbParam(int roomSize,int feedback,int Delay,int wetRear/*混响衰减大小*/,int frameSize);
	//void Process(short *input,short *out,int channel);

	//普通混响接口
	bool SetCommReverbParam(float roomsize,float dry,float wet,float feedback,float damp,float damp2,float damp3,float width);
	bool SetCommReverbParam(float setPara[9]);
	void Process(short *input,short *out,int channel,int frames);
	//Process(short *input,short *out,int channel,int samples)

public:
	void dump(void * v, int t);

	void splitLR(pfloat_t * data, pfloat_t * L, pfloat_t * R,
		int singleSize, int channels);

	void mergeLR(pfloat_t * data, pfloat_t * L, pfloat_t * R,
		int singleSize);

	void dumpLR(pfloat_t * l, pfloat_t * r, int t);

	void FloatToShortArray(const float *in, short *out, long len);
	void ShortToFloatArray(const short *in, float *out, long len);

private:
	int m_roomSize;
	int m_feedBack;
	int m_Delay;
	int m_wetRear;
	//脉冲混响参数
	//SndfileHandle * m_impulse;
	WavFile *m_impulse;
	IRBASE *m_pIR;
	int m_drydB;
	int m_wetdB;
	int m_frameSize;
	//////
	pfloat_t *m_stream, *m_iL, *m_iR, *m_oL, *m_oR;
	pfloat_t *m_input,*m_out;
	unsigned m_opt;

	//普通混响
	fv3::nrev_f m_nrev_f;

	int maxvfs; //最大采样率 
	int currentmaxvfs; //当前最大采样率
	int currentfactor; //当前采样因子
	int currentfs; //当前采样率
	long converter_type;

	float conf_idelay;
	float *m_tmpInBuf;
	float *m_tmpOutBuf;

	pfloat_t *m_ofLR; //左右声道数据
	pfloat_t *m_ifL, *m_ifR, *m_ofL, *m_ofR;


};

#endif
