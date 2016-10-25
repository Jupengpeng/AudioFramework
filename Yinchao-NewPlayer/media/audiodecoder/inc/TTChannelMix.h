#ifndef __CHANNEL_MIX_H__
#define __CHANNEL_MIX_H__

#include "DataUnit.h"
#include "TTTypeDef.h"

#define MIN_MIXED_FRAME_LENGTH 1152

class CTTChannelMixer;

class ChannelMixerFactory
{
public:
	static CTTChannelMixer* NewMixer(int inChannels, int outChannels);
};

class CTTChannelMixer
{
public:
	CTTChannelMixer();
	~CTTChannelMixer();

	virtual DataUnit<TTInt16> Mix(DataUnit<TTInt16> inDataUnit) = 0;

protected:
	void CreateMixedBuffer();
	void DestroyMixedBuffer();

protected:
	TTInt16* 	iMixedBuffer;
};

class CTTStereoMonoMixer: public CTTChannelMixer
{
public:
	virtual DataUnit<TTInt16> Mix(DataUnit<TTInt16> inDataUnit);

private:
	double GetCrossCorrelation(TTInt16* samples, long length);
};

class CTTMonoStereoMixer: public CTTChannelMixer
{
public:
	virtual DataUnit<TTInt16> Mix(DataUnit<TTInt16> inDataUnit);
};

class CTTNullMixer: public CTTChannelMixer
{
public:
	virtual DataUnit<TTInt16> Mix(DataUnit<TTInt16> inDataUnit);
};

#endif
