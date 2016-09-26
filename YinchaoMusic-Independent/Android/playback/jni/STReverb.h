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

//������ͷ�ļ�������freeverb���棬��Ȼ����
//#include <sndfile.h>
//#include <sndfile.hh>
//#include <fftw3.h>
using namespace fv3;

//#define DEFAULTMAXVFS 192000
#define DEFAULTMAXVFS 44100
#define DEFAULTFS 44100

#define DEFAULTFS_8K 8000
#define DEFAULTFS_16K 16000
#define BUFSIZE 44100  //��ʱ������ô��

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
	int  size0;					// ���ο�Ĵ�С
	char wave[5];				// "wave"
	char fmt[5];				//"fmt"
	int  size1;					// ��ʽ���С
	short  fmttag;		  		// ���α����ʽ
	short  channel;	  			// �����ļ������е�ͨ����
	int sampl;					// �����ļ��Ĳ�����
	int byte; 					// ƽ��ÿ�벨����Ƶ����Ҫ�ļ�¼���ֽ���
	short  blockalign;    		// һ����������Ҫ���ֽ���
	short  bitpersamples; 		// �����ļ����ݵ�ÿ��������λ��
	char data[5];           	// "data"
	int datasize;             	// ���ݴ�С
	char *dat;					//����
	bool opFlag;                 //�ļ��ɶ����ɼ��ر�־ 
private:
	FILE *m_pFile;
	long audioDataIndex;
public:
	WavFile();
	//WavFile(const char *path);
	WavFile(const char *path,int cacheFlag/*1Ϊֱ�Ӷ��껺�浽data,Ϊ0ֻ��wavͷ���ݲ���*/);
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
	bool init(int dryDB/*����˥��ֵ,��-5*/,int wetDB/*ʪ��˥��ֵ*/,int frameSize/*ÿ�δ������ݴ�С,��shortΪ��λ*/);
	//bool SetCommReverbParam(int roomSize,int feedback,int Delay,int wetRear/*����˥����С*/,int frameSize);
	//void Process(short *input,short *out,int channel);

	//��ͨ����ӿ�
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
	//����������
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

	//��ͨ����
	fv3::nrev_f m_nrev_f;

	int maxvfs; //�������� 
	int currentmaxvfs; //��ǰ��������
	int currentfactor; //��ǰ��������
	int currentfs; //��ǰ������
	long converter_type;

	float conf_idelay;
	float *m_tmpInBuf;
	float *m_tmpOutBuf;

	pfloat_t *m_ofLR; //������������
	pfloat_t *m_ifL, *m_ifR, *m_ofL, *m_ofR;


};

#endif
