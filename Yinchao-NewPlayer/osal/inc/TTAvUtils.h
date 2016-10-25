#ifndef _TT_AV_UTILS_H_
#define _TT_AV_UTILS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "TTMediadef.h"
#include "TTBitReader.h"

enum {
    kAVCProfileBaseline      = 0x42,
    kAVCProfileMain          = 0x4d,
    kAVCProfileExtended      = 0x58,
    kAVCProfileHigh          = 0x64,
    kAVCProfileHigh10        = 0x6e,
    kAVCProfileHigh422       = 0x7a,
    kAVCProfileHigh444       = 0xf4,
    kAVCProfileCAVLC444Intra = 0x2c
};

// Optionally returns sample aspect ratio as well.
void FindAVCDimensions(
        TTBuffer* pInBuffer,
        int *width, int *height, int* numRef,
        int *sarWidth = NULL, int *sarHeight = NULL);

unsigned int parseUE(TTBitReader *br);

bool IsAVCReferenceFrame(TTBuffer* pInBuffer);

int GetAACFrameSize(
        unsigned char* pBuffer, unsigned int size, int *frame_size,
        int *out_sampling_rate, int *out_channels);

int GetMPEGAudioFrameSize(
        unsigned char* pBuf, unsigned int *frame_size,
        int *out_sampling_rate, int *out_channels,
        int *out_bitrate, int *out_num_samples);

#endif  // _TT_AV_UTILS_H_
