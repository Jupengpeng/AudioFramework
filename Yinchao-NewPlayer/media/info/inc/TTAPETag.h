#ifndef __TTAPETAG_H__

#include <string.h>
#include "TTTypedef.h"

class ITTDataReader;

#define CURRENT_APE_TAG_VERSION                 2000

/*****************************************************************************************
Footer (and header) flags
*****************************************************************************************/
#define APE_TAG_FLAG_CONTAINS_HEADER            (1 << 31)
#define APE_TAG_FLAG_CONTAINS_FOOTER            (1 << 30)
#define APE_TAG_FLAG_IS_HEADER                  (1 << 29)

#define APE_TAG_FLAGS_DEFAULT                   (APE_TAG_FLAG_CONTAINS_FOOTER)

/*****************************************************************************************
The footer at the end of APE tagged files (can also optionally be at the front of the tag)
*****************************************************************************************/
#define APE_TAG_FOOTER_BYTES					32

class APE_TAG_FOOTER
{
protected:

	char m_cID[8];              // should equal 'APETAGEX'    
	int m_nVersion;             // equals CURRENT_APE_TAG_VERSION
	int m_nSize;                // the complete size of the tag, including this footer (excludes header)
	int m_nFields;              // the number of fields in the tag
	int m_nFlags;               // the tag flags
	char m_cReserved[8];        // reserved for later use (must be zero)

public:

	APE_TAG_FOOTER(int nFields = 0, int nFieldBytes = 0)
	{
		memcpy(m_cID, "APETAGEX", 8);
		memset(m_cReserved, 0, 8);
		m_nFields = nFields;
		m_nFlags = APE_TAG_FLAGS_DEFAULT;
		m_nSize = nFieldBytes + APE_TAG_FOOTER_BYTES;
		m_nVersion = CURRENT_APE_TAG_VERSION;
	}

	int GetTotalTagBytes() { return m_nSize + (GetHasHeader() ? APE_TAG_FOOTER_BYTES : 0); }
	int GetFieldBytes() { return m_nSize - APE_TAG_FOOTER_BYTES; }
	int GetFieldsOffset() { return GetHasHeader() ? APE_TAG_FOOTER_BYTES : 0; }
	int GetNumberFields() { return m_nFields; }
	bool GetHasHeader() { return (m_nFlags & APE_TAG_FLAG_CONTAINS_HEADER);}
	bool GetIsHeader() { return (m_nFlags & APE_TAG_FLAG_IS_HEADER);}
	int GetVersion() { return m_nVersion; }

	bool GetIsValid(bool bAllowHeader)
	{
		bool bValid = (strncmp(m_cID, "APETAGEX", 8) == 0) && 
			(m_nVersion <= CURRENT_APE_TAG_VERSION) &&
			(m_nFields <= 65536) &&
			(m_nSize >= APE_TAG_FOOTER_BYTES) &&
			(GetFieldBytes() <= (1024 * 1024 * 16));

		if (bValid && !bAllowHeader && GetIsHeader())
			bValid = false;

		return bValid;
	}
};

/**
* \fn                       TTInt APETagSize();
* \brief                    获取APE信息头长度
* \param [in] aDataReader	ITTDataReader对象引用
* \return                   如果存在APE信息头，返回信息头长度；如果不存在APE信息头，返回0
*/
TTInt						APETagSize(ITTDataReader& aDataReader);

#endif