#include "STOSConfig.h"

#if (defined __ST_OS_WINDOWS__)//WINDOWS
#define KMaxBufferNum  10
#define KTotalPCMBufferSize  (640 * KILO)
#define KFirstPCMBufferSize  (24 * KILO)
#define KPCMBufferSize   (8 * KILO)
#elif (defined __ST_OS_IOS__)
#define KMaxBufferNum  10
#define KFirstPCMBufferSize  (80 * KILO)
#define KPCMBufferSize   (40 * KILO)
#define KTotalPCMBufferSize  (640 * KILO)
#elif (defined __ST_OS_ANDROID__)
#define KMaxBufferNum  10
#define KFirstPCMBufferSize  (64 * KILO)
#define KPCMBufferSize   (48 * KILO)
//#define KTotalPCMBufferSize  (2 * KILO * KILO) 
//modify by bin.liu 增加播放buffer，不然会出现录制赶上播放速度
#define KTotalPCMBufferSize  (2 * KILO * KILO*2) //

#define KRecordBufferNum  (10)

#define KRecordBufferSize (1600)//200ms 2560

#define KRecordLoopBufferSize (256 * KILO)

#endif
