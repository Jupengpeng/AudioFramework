#ifndef __DATA_UNIT_H__
#define __DATA_UNIT_H__

template <typename T>

class DataUnit
{
public:
	DataUnit();
	DataUnit(T* pointer, long length, long maxLength);
	~DataUnit();

	T* GetPointer();
	void SetPointer(T* pointer);

	unsigned long GetLength();
	void SetLength(unsigned long length);

	unsigned long GetMaxLength();
	void SetMaxLength(unsigned long length);

	void Construct(T* pointer, unsigned long length, unsigned long maxLength);

private:

	T*	iPointer;
	unsigned long iLength;
	unsigned long iMaxLength;
};


template <typename T>
DataUnit<T>::DataUnit()
: iPointer(NULL)
, iLength(0)
, iMaxLength(0)
{
}

template <typename T>
DataUnit<T>::DataUnit(T* pointer, long length, long maxLength)
: iPointer(pointer)
, iLength(length)
, iMaxLength(maxLength)
{
}

template <typename T>
DataUnit<T>::~DataUnit()
{
}

template <typename T>
T* DataUnit<T>::GetPointer()
{
	return iPointer;
}

template <typename T>
void DataUnit<T>::SetPointer(T* pointer)
{
	iPointer = pointer;
}

template <typename T>
unsigned long DataUnit<T>::GetLength()
{
	return iLength;
}

template <typename T>
void DataUnit<T>::SetLength(unsigned long length)
{
	iLength = length;
}

template <typename T>
unsigned long DataUnit<T>::GetMaxLength()
{
	return iMaxLength;
}

template <typename T>
void DataUnit<T>::SetMaxLength(unsigned long maxLength)
{
	iMaxLength = maxLength;
}

template <typename T>
void DataUnit<T>::Construct(T* pointer, unsigned long length, unsigned long maxLength)
{
	SetPointer(pointer);
	SetLength(length);
	SetMaxLength(maxLength);
}

#endif
