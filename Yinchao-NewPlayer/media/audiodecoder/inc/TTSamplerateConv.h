#ifndef __SAMPLERATE_CONV_H__
#define __SAMPLERATE_CONV_H__

#include "DataUnit.h"
#include "TTTypedef.h"

class CTTSamplerateConv;

class SamplerateConvFactory
{
public:
	static CTTSamplerateConv* NewConv(int inSamplerate, int outSamplerate, int channels);
};

class CTTSamplerateConv
{
public:
//	CTTSamplerateConv();
//	virtual ~CTTSamplerateConv();

	virtual DataUnit<TTInt16> Conv(DataUnit<TTInt16> inDataUnit) = 0;

protected:
	bool 			iValidX1;
	short 			iX1;
	unsigned long 	iDtb;
	unsigned long 	iTime;
};

#include "aflibConverter.h"
class CTTAflibConv : public CTTSamplerateConv
{
public:
	CTTAflibConv(double factor, int channels);
	~CTTAflibConv();

	virtual DataUnit<TTInt16> Conv(DataUnit<TTInt16> inDataUnit);

private:
	void CreateConvertedBuffer();
	void DestroyConvertedBuffer();

private:
	double			iFactor;
	TTInt16* 		iConvertedBuffer;
	aflibConverter 	iAflibConverter;
};

class CTTNullConv : public CTTSamplerateConv
{
public:
	virtual DataUnit<TTInt16> Conv(DataUnit<TTInt16> inDataUnit);
};
#endif
