
#define  KNLoud  100
static const char KScale32[32] = {3, 4, 5, 6, 6, 8, 8, 8,//512 -> 32¶Î×ª»»±í¸ñ
									8, 8, 8, 8, 8, 8, 8, 8,
									9, 9, 9, 9, 9, 9, 9, 9,
									9, 9, 9, 9, 9, 9, 9, 9 };

static const short powValue[KNLoud] = {
		0,      4,     14,     30,      54,    82,    118,    162,    210,	  264,     
		330,    398,    472,    554,     644,   738,    839,    948,   1052,	 1102,      
		1140,	 1192,	 1240,	 1320,    1380,	 1508,	 1648,	 1798,	 1938,	 2100,
		2257,   2307,   2364,   2420,	  2480,	 2510,	 2548,	 2728,	 2916,	 3112,
		3315,	 3527,	 3746,	 3972,    4208,	 4451,	 4702,	 4862,	 4892,	 4920,
		4950,	 5008,	 5056,	 5339,	  5632,	 5935,	 6248,	 6572,	 6907,	 7253,
		7610,	 7878,	 7904,	 7998,	  8062,	 8120,	 8194,	 8615,	 9051,	 9511,
		9978,	10452,	10960,  11466,	 11902,	11980,	12012,	12068,	12128,	12796,
		13421,	14105,	14814,	15500,	 16313,	17104,	17923,	18020,	18134,	18297,
		19138,	20447,	21596,	22795,	 24047,	26674,	28197,	29127,	30203,	32767
};

static void filter(int* data, int aDstFreqNum)
{
	int i = 0;
	data[0] = (2*data[0] + data[1]) / 3;
	for (i = 1; i < aDstFreqNum; ++i)
	{
		data[i] = (2*data[i] + data[i-1]) / 3;
	}
}

int NormalizeFreqBin(int *aDstFreq, int aDstFreqNum, short *aSrcRawFreq, int aSrcFreqNum)
{
	const char* pScale = KScale32;

	int i = 0, k = 0, count = 0;

	int *pFreq = aDstFreq;
	short *pSrcRawFreq = aSrcRawFreq;
	int nDelay = 0;

	if (aSrcFreqNum < aDstFreqNum)
	{
		return -1;
	}

	if ((aDstFreqNum != 32) || (aSrcFreqNum != 512))
	{
		return -2;
	}

	for(k = 0; k < aDstFreqNum; k++)
	{
		int freqBin = 0, decayedBin;
		count = *pScale++;

		for(i = 0; i < count; i++)
		{
			freqBin += *pSrcRawFreq++;
		}


		for(i=0; i<KNLoud; ++i)
		{
			if( powValue[i] >= freqBin)
				break;
		}

		freqBin = i;	
		
		if (k > 20)
			nDelay = 4;
		else if (k > 10)
			nDelay = 5;
		else 
			nDelay = 6;		

		decayedBin = *pFreq - nDelay;

		if (k > 20)	
			*pFreq++ = (((*pFreq) + 2 * ((freqBin > decayedBin) ? freqBin : decayedBin)) / 3); 
		else
			*pFreq++ = ((freqBin > decayedBin) ? freqBin : decayedBin); 
			
	}

	filter(aDstFreq, aDstFreqNum);

	return 0;
}