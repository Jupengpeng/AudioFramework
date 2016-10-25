package com.android.ychao.media.player;

import android.util.Log;

/**
 * 归一化处理
 * 
 * @author
 * @version
 * @since
 */
public final class Normalizer {
	private static final String LOG_TAG = "Normalizer";
	private static final int K_BAND_64 = 64;
	private static final int K_BAND_128 = 128;

	private static final int K_N_LOUD = 100;

	private static final byte[] K_SCALE_64 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4,
			4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7,
			7, 8, 8, 8, 8, 9, 9, 9, 9 };

	private static final byte[] K_SCALE_128 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };

	private static final short[] POW_VALUE = { 0, 4, 14, 30, 54, 82, 118, 162,
			210, 264, 330, 398, 472, 554, 644, 738, 839, 948, 1052, 1102, 1140,
			1192, 1240, 1320, 1380, 1508, 1648, 1798, 1938, 2100, 2257, 2307,
			2364, 2420, 2480, 2510, 2548, 2728, 2916, 3112, 3315, 3527, 3746,
			3972, 4208, 4451, 4702, 4862, 4892, 4920, 4950, 5008, 5056, 5339,
			5632, 5935, 6248, 6572, 6907, 7253, 7610, 7878, 7904, 7998, 8062,
			8120, 8194, 8615, 9051, 9511, 9978, 10452, 10960, 11466, 11902,
			11980, 12012, 12068, 12128, 12796, 13421, 14105, 14814, 15500,
			16313, 17104, 17923, 18020, 18134, 18297, 19138, 20447, 21596,
			22795, 24047, 26674, 28197, 29127, 30203, 32767 };

	static final int K_FREQUENCY_NUMBER = 512;
	private static final int ERR_RAW_DATA_NUM = -2;

	static public void filter(int[] data, int aDstFreqNum) {
		int i = 0;
		data[0] = (2 * data[0] + data[1]) / 3;
		for (i = 1; i < aDstFreqNum; ++i) {
			data[i] = (2 * data[i] + data[i - 1]) / 3;
		}
	}

	/**
	 * 归并频谱数据
	 * 
	 * @param aDstFreq
	 *            归并后的频谱数据
	 * @param aDstFreqNum
	 *            归并后的频谱根数
	 * @param aSrcRawFreq
	 *            归并前的频谱数据
	 * @param aSrcRawNum
	 *            归并前的频谱根数
	 * @return 成功返回0，失败返回负数失败码
	 */
	public static int normalizeFreqBin(int[] aDstFreq, int aDstFreqNum,
			short[] aSrcRawFreq, int aSrcRawNum) {
		if (aDstFreq == null || aSrcRawFreq == null
				|| (aDstFreqNum < 0 || aDstFreqNum > K_BAND_128)
				|| (aSrcRawFreq.length < aSrcRawNum)) {
			Log.d(LOG_TAG, "argument error -1");
			return -1;
		}

		int i = 0, k = 0, t = 0, count = 0;
		int nDelay = 0;
		if (aSrcRawNum < aDstFreqNum) {
			Log.d(LOG_TAG, "argument error -2");
			return ERR_RAW_DATA_NUM;
		}

		if (aSrcRawNum != K_FREQUENCY_NUMBER) {
			Log.d(LOG_TAG, "argument error -3");
			return -3;
		}

		byte[] pScale = null;
		if (aDstFreqNum <= K_BAND_64) {
			pScale = K_SCALE_64;
		} else if (aDstFreqNum <= K_BAND_128) {
			pScale = K_SCALE_128;
		} else {
			Log.d(LOG_TAG, "argument error -4");
			return -4;
		}

		for (k = 0; k < aDstFreqNum; k++) {
			int freqBin = 0, decayedBin;
			count = pScale[k];

			for (i = 0; i < count; i++) {
				freqBin += aSrcRawFreq[t];
				t++;
			}

			for (i = 0; i < K_N_LOUD; ++i) {
				if (POW_VALUE[i] >= freqBin) {
					break;
				}
			}
			freqBin = i;

			if (k > 20) {
				nDelay = 4;
			} else if (k > 10) {
				nDelay = 5;
			} else {
				nDelay = 6;
			}
			decayedBin = aDstFreq[k] - nDelay;

			if (k > 20) {
				aDstFreq[k] = (((aDstFreq[k]) + 2 * ((freqBin > decayedBin) ? freqBin
						: decayedBin)) / 3);
			} else {
				aDstFreq[k] = ((freqBin > decayedBin) ? freqBin : decayedBin);
			}
		}
		filter(aDstFreq, aDstFreqNum);
		return 0;
	}
}
