#include "STBasePlugin.h"
#include "STOSConfig.h"
STBasePlugin::STBasePlugin()
	: iReadStatus(ESTReadStatusNotReady)
	, iDuration(0)
{
}

const STMediaInfo& STBasePlugin::GetMediaInfo() const
{
	return iMediaInfo;
}

STBasePlugin::~STBasePlugin()
{
}

void STBasePlugin::UnInitPlugin()
{
	iDuration = 0;
}

STUint STBasePlugin::Duration()
{
	return iDuration;
}

STInt STBasePlugin::GetDownloadPercent()
{
	return  STKErrNotSupported; //等着被重写
}

STReadStatus STBasePlugin::ReadStatus()
{
	return iReadStatus;
}

STInt STBasePlugin::Switch2Stream(const STChar* aStreamName)
{
	return STKErrNotSupported;
}

const STChar* STBasePlugin::GetStreamName(STInt aIndex)
{
	return NULL;
}

void STBasePlugin::Abort()
{

}

