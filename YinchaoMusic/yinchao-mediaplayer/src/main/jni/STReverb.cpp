
#include "STReverb.h"
#include "STLog.h"
//add by bin.chen

STReverb::STReverb()
{
	m_impulse == NULL;
	m_opt = FV3_IR_DEFAULT|FV3_IR_SKIP_FILTER|FV3_IR_MONO2STEREO;
	m_pIR = new IR2();

	//普通混响设置
	conf_idelay = 20.0;
	currentmaxvfs = maxvfs = DEFAULTMAXVFS;
	currentmaxvfs = maxvfs;
	converter_type = SRC_ZERO_ORDER_HOLD;
	//converter_type = SRC_SINC_FASTEST;

	currentfs = DEFAULTFS;
	//currentfs = DEFAULTFS_16K;
	currentfactor = maxvfs/currentfs;
	m_nrev_f.setCurrentFs(currentfs);
	m_nrev_f.setOverSamplingFactor(currentfactor, converter_type);

	int iDelay = (int)((float)currentfs*conf_idelay/1000.0f);

	m_nrev_f.setInitialDelay(iDelay);

	//reverbm.setCurrentFs(currentfs);
	//reverbm.setOverSamplingFactor(currentfactor, converter_type);
	std::fprintf(stderr, "freeverb3.cpp: fs = %d[Hz] x %ld\n,iDelay = %d",
		currentfs, m_nrev_f.getOverSamplingFactor(),iDelay);


	m_tmpInBuf = new float[BUFSIZE];
	m_tmpOutBuf = new float[BUFSIZE];
	m_ifL = new float[BUFSIZE/2];
	m_ifR = new float[BUFSIZE/2]; 
	m_ofL = new float[BUFSIZE/2];
	m_ofR = new float[BUFSIZE/2];

	float presetI[][9] = {
		{2.05, -10.0, -2.0,  0.65, 0.3,  0.7,   0.43, 0.9, 20,},
		{3.05, -15.0, -2.0,  0.6,  0.85, 0.85,  0.10, 1.0, 30,},
		{1.0,  -10.0, -2.0,  0.7,  0.4,  0.2,   0.25, 0.9, 3,},
		{1.0,  -10.0, -2.0,  0.6,  0.9,  0.9,   0.10, 1.0, 0,},
		{2.98, -15.0, -2.0,  0.6,  0.21, 0.6,   0.05, 1.0, 40,},
		{1.07, -10.0, -2.0,  0.56,  0.9,  0.9,   0.2, 1.0, 10,},};

	float presetI2[][9] = {
			{4.05, -10.0, -2.0,  0.65, 0.3,  0.7,   0.43, 0.9, 20,},
			{3.05, -15.0, -2.0,  0.6,  0.85, 0.85,  0.10, 1.0, 30,},
			{1.0,  -10.0, -2.0,  0.7,  0.4,  0.2,   0.25, 0.9, 3,},
			{1.0,  -10.0, -2.0,  0.6,  0.9,  0.9,   0.10, 1.0, 0,},
			{2.98, -15.0, -2.0,  0.6,  0.21, 0.6,   0.05, 1.0, 40,},
			{1.07, -10.0, -2.0,  0.56,  0.9,  0.9,   0.2, 1.0, 10,},};

	//设置混响参数
	SetCommReverbParam(presetI[3]);

};

STReverb::~STReverb()
{
	delete[] m_stream;
	delete[] m_iL;
	delete[] m_iR;
	delete[] m_oL;
	delete[] m_oR;
	delete m_pIR;

	delete[] m_tmpInBuf;
	delete[] m_tmpOutBuf;

	delete[] m_ifL;
	delete[] m_ifR;
	delete[] m_ofL;
	delete[] m_ofR;
};

bool STReverb::SetIRParamter(const char*pathName,int drydB,int wetdB,int frameSize)
{
	//m_impulse = new SndfileHandle(pathName);
	m_impulse = new WavFile(pathName,1);
	if(m_impulse == NULL || m_impulse->frames() == 0)
	{
		//std::fprintf(stderr, "ERROR: open PCM file %s.\n",pathName);
		return false;
	}
	pfloat_t * irStream = new pfloat_t[((int)m_impulse->frames())*m_impulse->channels()];
	sf_count_t rcount = m_impulse->readf(irStream, m_impulse->frames());
	if(rcount != m_impulse->frames())
	{
		//std::fprintf(stderr, "ERROR: readf impulse\n");
		delete[] irStream;
		return false;
	}
	pfloat_t * irL = new pfloat_t[(int)m_impulse->frames()];
	pfloat_t * irR = new pfloat_t[(int)m_impulse->frames()];
	splitLR(irStream, irL, irR, m_impulse->frames(), m_impulse->channels());
	m_pIR->loadImpulse(irL, irR, m_impulse->frames());
	delete irL;
	delete irR;

	m_pIR->setdry(drydB);
	m_pIR->setwet(wetdB);

	m_frameSize = frameSize;
	m_stream = new pfloat_t[m_frameSize*2];
	m_iL = new pfloat_t[m_frameSize];
	m_iR = new pfloat_t[m_frameSize];
	m_oL = new pfloat_t[m_frameSize];
	m_oR = new pfloat_t[m_frameSize];

	m_input = new pfloat_t[m_frameSize*2];
	m_out = new pfloat_t[m_frameSize*2];

	return true;
}
bool STReverb::init(int dryDB/*干声衰减值,如-5*/,int wetDB/*湿声衰减值*/,int frameSize/*每次处理数据大小,以short为单位*/)
{
	return true;
};
bool STReverb::SetCommReverbParam(float roomsize,float dry,float wet,float feedback,float damp,float damp2,float damp3,float width)
{
	m_nrev_f.setroomsize(2.05);
	m_nrev_f.setwet(-10.0f);
	m_nrev_f.setdry(-5.0f);
	m_nrev_f.setfeedback(0.65);
	m_nrev_f.setdamp(0.3);
	m_nrev_f.setdamp2(0.7);
	m_nrev_f.setdamp3(0.43);
	m_nrev_f.setwidth(0.9);
	m_nrev_f.mute();
	conf_idelay = 0.0f;
	return true;
};


bool STReverb::SetCommReverbParam(float setPara[9])
{
	m_nrev_f.setroomsize(setPara[0]);
	m_nrev_f.setwet(setPara[1]);
	m_nrev_f.setdry(setPara[2]);
	m_nrev_f.setfeedback(setPara[3]);
	m_nrev_f.setdamp(setPara[4]);
	m_nrev_f.setdamp2(setPara[5]);
	m_nrev_f.setdamp3(setPara[6]);
	m_nrev_f.setwidth(setPara[7]);
	//idelay = conf[8];
	m_nrev_f.mute();
	return true;
};



// void STReverb::Process(short *input,short *out,int channel)
// {
// 	//int count = 0;
// 	unsigned long acount = 0;
// 
// 	splitLR((pfloat_t*)input, (pfloat_t*)m_iL, (pfloat_t*)m_iR, m_frameSize, channel);
// 	m_pIR->processreplace(m_iL,m_iR,m_oL,m_oR,m_frameSize,m_opt);
// 	//dumpLR(oL,oR,m_frameSize);
// 
// 	//pfloat_t * buf = new pfloat_t[2*m_frameSize];
// 	mergeLR((pfloat_t*)out,(pfloat_t*)m_oL,(pfloat_t*)m_oR,m_frameSize);
// 	dump(out, sizeof(pfloat_t)*m_frameSize*2);
// 
// };

// void STReverb::Process(short *input,short *out,int channel)
// {
// 	if(m_impulse != NULL)
// 	{
// 		//int count = 0;
// 		unsigned long acount = 0;
// 		for(int i = 0;i<m_frameSize;i++)
// 		{
// 			m_input[i] = input[i];
// 		}
// 
// 		splitLR((pfloat_t*)m_input, (pfloat_t*)m_iL, (pfloat_t*)m_iR, m_frameSize, channel);
// 		m_pIR->processreplace(m_iL,m_iR,m_oL,m_oR,m_frameSize,m_opt);
// 		//dumpLR(oL,oR,m_frameSize);
// 
// 		//pfloat_t * buf = new pfloat_t[2*m_frameSize];
// 		mergeLR((pfloat_t*)m_out,(pfloat_t*)m_oL,(pfloat_t*)m_oR,m_frameSize);
// 		for(int i = 0;i<m_frameSize*channel;i++)
// 		{
// 			out[i] = m_out[i];
// 		}
// 		dump(out, sizeof(short)*m_frameSize*2);
// 	}
// };

void STReverb::Process(short *input,short *out,int channel,int samples)
{

	ShortToFloatArray(input,m_tmpInBuf,samples*channel/*input数组大小*/);
	fv3::splitChannelsV(channel, samples, m_tmpInBuf, m_ifL, m_ifR); //声道分离


	//STLOGE("chenbin===STReverb::Process*==,start");
	m_nrev_f.processreplace(m_ifL,m_ifR,m_ofL,m_ofR,samples);
	//STLOGE("chenbin===STReverb::Process*==,end");
	fv3::mergeChannelsV(channel,samples,m_tmpOutBuf, m_ofL, m_ofR);//声道合并
	FloatToShortArray(m_tmpOutBuf,out,samples*channel);

};


void STReverb::ShortToFloatArray(const short *in, float *out, long len)
{
	while (len)
	{
		len -- ;
		out [len] = (float) (in [len] / (1.0 * 0x8000));
	} ;
	return ;
}

void STReverb::FloatToShortArray (const float *in, short *out, long len)
{
	double scaled_value ;
	while (len)
	{
		len -- ;
		scaled_value = in [len] * (8.0 * 0x10000000) ;
		if (scaled_value >= (1.0 * 0x7FFFFFFF))
		{
			out [len] = 32767 ;
			continue ;
		} ;
		if (scaled_value <= (-8.0 * 0x10000000))
		{
			out [len] = -32768 ;
			continue ;
		} ;
		out [len] = (short)(lrintf (scaled_value) >> 16) ;
	} ;
}


void STReverb::dump(void * v, int t)
{
	unsigned char * p = (unsigned char *)v;
	for(int i = 0;i < t;i ++)
	{
		std::fprintf(stdout, "%c", p[i]);
	}
}

void STReverb::splitLR(pfloat_t * data, pfloat_t * L, pfloat_t * R,
					   int singleSize, int channels)
{
	for(int t = 0; t < singleSize; t++)
	{
		L[t] = data[t*channels+0];
		if(channels > 1)
			R[t] = data[t*channels+1];
		else
			R[t] = L[t];
	}
}

void STReverb::mergeLR(pfloat_t * data, pfloat_t * L, pfloat_t * R,
					   int singleSize)
{
	for(int t = 0;t < singleSize;t ++)
	{
		data[t*2+0] = L[t];
		data[t*2+1] = R[t];
	}
}

void STReverb::dumpLR(pfloat_t * l, pfloat_t * r, int t)
{
	pfloat_t * buf = new pfloat_t[2*t];
	mergeLR(buf,l,r,t);
	dump(buf, sizeof(pfloat_t)*t*2);
	delete[] buf;
}

WavFile::WavFile()
{
	//riff_id[4]='\0',wave[4]='\0',fmt[4]='\0',data[4]='\0';
	m_pFile = NULL;
	audioDataIndex = 0;
	opFlag = false;
};
WavFile::WavFile(const char *path,int cacheFlag/*1为直接读完缓存到data,为0只读wav头数据不读*/)
{

	opFlag = false;
	riff_id[4]='\0',wave[4]='\0',fmt[4]='\0',data[4]='\0';
	//ifstream infile(path,ios::binary);//文件输入流
	m_pFile = fopen(path,"rb");
	if(m_pFile != NULL)
	{
		fread((char *)riff_id,1,4,m_pFile);		
		fread((char *)&size0,1,4,m_pFile);
		fread((char *)wave,1,4,m_pFile);
		fread((char *)fmt,1,4,m_pFile);
		fread((char *)&size1,1,4,m_pFile);
		fread((char *)&fmttag,1,2,m_pFile);
		fread((char *)&channel,1,2,m_pFile);
		fread((char *)&sampl,1,4,m_pFile);
		fread((char *)&byte,1,4,m_pFile);
		fread((char *)&blockalign,1,2,m_pFile);
		fread((char *)&bitpersamples,1,2,m_pFile);
		fread((char *)data,1,4,m_pFile);
		fread((char *)&datasize,1,4,m_pFile);
		audioDataIndex = ftell(m_pFile);
		if(cacheFlag)
		{
			dat=(char *)new char[sizeof(char)*datasize];
			fread((char *)dat,1,datasize,m_pFile);
		}
		else
		{
			dat = NULL;
		}
		opFlag = true;
	}
	else
	{
		opFlag = false;

	}
	//std::fprintf(stderr, "bitpersamples=%d,sampl=%d\n",bitpersamples,sampl);
	return ;
}
int WavFile::frames()
{
	int iframe = datasize/2/channel;
	return iframe;
}
short WavFile::channels()
{
	return channel;
}
long WavFile::readf(short *inputStream,int frames)
{

	short *p = inputStream;
	short *tp = p;
	long remainSize = frames*channel*2; //剩余字节大小（要读取的）
	long currentSize = 0;
	while((!feof(m_pFile)) && (remainSize > 0))
	{
		long size = fread(tp,1,remainSize,m_pFile);
		tp += size/2;
		remainSize -= size;
		currentSize += size;
	}
	return (long)currentSize/2/channel;
}

long WavFile::readf(float *inputStream,int frames)
{

	short *p = new short[frames*channel];
	short *tp = p;
	long remainSize = frames*channel*2; //剩余字节大小（要读取的）
	long currentSize = 0;
	while((!feof(m_pFile)) && (remainSize > 0))
	{
		long size = fread(tp,1,remainSize,m_pFile);
		remainSize -= size;
		tp += size/2;
		currentSize += size;
	}

	ShortToFloatArray(p,inputStream,currentSize/2);
	delete[] p;
	return (long)currentSize/2/channel;
}

void WavFile::reset()
{
	fseek(m_pFile,audioDataIndex, SEEK_SET);
}

WavFile::~WavFile()
{
	if(m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
	delete []dat;
};

bool WavFile::isLoad()
{
	return opFlag;
};

void WavFile::ShortToFloatArray(const short *in, float *out, long len)
{
	while (len)
	{
		len -- ;
		out [len] = (float) (in [len] / (1.0 * 0x8000));
	} ;
	return ;
}

void WavFile::FloatToShortArray (const float *in, short *out, long len)
{
	double scaled_value ;
	while (len)
	{
		len -- ;
		scaled_value = in [len] * (8.0 * 0x10000000) ;
		if (scaled_value >= (1.0 * 0x7FFFFFFF))
		{
			out [len] = 32767 ;
			continue ;
		} ;
		if (scaled_value <= (-8.0 * 0x10000000))
		{
			out [len] = -32768 ;
			continue ;
		} ;
		out [len] = (short)(lrintf (scaled_value) >> 16) ;
	} ;
}
