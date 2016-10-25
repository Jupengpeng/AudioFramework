#include "TTBitReader.h"

TTBitReader::TTBitReader(unsigned char *data, unsigned int size)
    : mData(data),
      mSize(size),
      mReservoir(0),
      mNumBitsLeft(0) 
{
}

TTBitReader::~TTBitReader() 
{
}

void TTBitReader::fillReservoir() 
{
    mReservoir = 0;
    unsigned int  i;
    for (i = 0; mSize > 0 && i < 4; ++i) {
        mReservoir = (mReservoir << 8) | *mData;

        ++mData;
        --mSize;
    }

    mNumBitsLeft = 8 * i;
    mReservoir <<= 32 - mNumBitsLeft;
}

unsigned int  TTBitReader::getBits(unsigned int n) 
{
    unsigned int  result = 0;
    while (n > 0) {
        if (mNumBitsLeft == 0) {
            fillReservoir();
        }

        unsigned int  m = n;
        if (m > mNumBitsLeft) {
            m = mNumBitsLeft;
        }

        result = (result << m) | (mReservoir >> (32 - m));
        mReservoir <<= m;
        mNumBitsLeft -= m;

        n -= m;
    }

    return result;
}

void TTBitReader::skipBits(unsigned int  n) 
{
    while (n > 32) {
        getBits(32);
        n -= 32;
    }

    if (n > 0) {
        getBits(n);
    }
}

void TTBitReader::putBits(unsigned int  x, unsigned int  n) 
{
    while (mNumBitsLeft + n > 32) {
        mNumBitsLeft -= 8;
        --mData;
        ++mSize;
    }

    mReservoir = (mReservoir >> n) | (x << (32 - n));
    mNumBitsLeft += n;
}

unsigned int  TTBitReader::numBitsLeft() const 
{
    return mSize * 8 + mNumBitsLeft;
}

unsigned char *TTBitReader::data() const 
{
    return mData - (mNumBitsLeft + 7) / 8;
}

