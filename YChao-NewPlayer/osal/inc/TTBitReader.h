#ifndef _TT_BIT_READER_H_
#define _TT_BIT_READER_H_

class TTBitReader {
public:
    TTBitReader(unsigned char *data, unsigned int size);
    virtual ~TTBitReader();

    unsigned int getBits(unsigned int n);
    void skipBits(unsigned int n);

    void putBits(unsigned int x, unsigned int n);

    unsigned int numBitsLeft() const;

    unsigned char *data() const;

protected:
    unsigned char *mData;
    unsigned int mSize;

    unsigned int mReservoir;  // left-aligned bits
    unsigned int mNumBitsLeft;

    virtual void fillReservoir();
};

#endif  // _TT_BIT_READER_H_
