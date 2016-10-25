#ifndef __TT_NETWORK_CONFIG__
#define __TT_NETWORK_CONFIG__

enum TTActiveNetWorkType 
{
	EActiveNetWorkNone = 0
	,EActiveNetWorkGPRS = 1
	,EActiveNetWorkWIFI = 2
};

class TTNetWorkConfig 
{
public:
	static TTNetWorkConfig* getInstance();
	static void				release();

	TTActiveNetWorkType getActiveNetWorkType();

	void SetActiveNetWorkType(TTActiveNetWorkType aNetWorkType);

private:
	TTNetWorkConfig();

private:
	TTActiveNetWorkType			iNetWorkType;
	static TTNetWorkConfig*		iNetWorkConfig;				
};

#endif