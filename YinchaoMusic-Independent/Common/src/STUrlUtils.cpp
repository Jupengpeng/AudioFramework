#include <string.h>
#include <ctype.h>
#include "STUrlUtils.h"

STInt STUrlUtils::GetParam(const STChar* aParamStr, const STChar* aKey, STChar* aValue, const STChar* aSeparator, STInt aValueMaxSize)
{
	STInt nKeyLen = 0;

	if (aParamStr != NULL && strlen(aParamStr) != 0 && aKey != NULL && (nKeyLen = strlen(aKey)) != 0 
		&& aSeparator != NULL && strlen(aSeparator) != 0 && aValue != 0)
	{
		while(ESTTrue) 
		{
			const STChar* pParam = strstr(aParamStr, aKey);
			if (pParam == NULL)
			{
				break;
			}

			if (*(pParam + nKeyLen) == '=')
			{
				const STChar* pValue = pParam + nKeyLen + 1;
				const STChar* pValueEnd = strstr(pValue, aSeparator);

				STInt nValueLen = pValueEnd == NULL ? strlen(pValue) : pValueEnd - pValue;

				if (nValueLen > aValueMaxSize)
				{
					return STKErrOverflow;
				}

				memset(aValue, 0, nValueLen + 1);
				memcpy(aValue, pValue, nValueLen);
				return STKErrNone;
			}

			aParamStr += nKeyLen;
		}
	}

	return STKErrNotFound;
}

STBool STUrlUtils::IsSplashCharacter(STChar aCharacter) 
{
	return aCharacter == '/' || aCharacter == '\\';
}

STBool STUrlUtils::IsDotCharacter(STChar aCharacter) 
{
	return aCharacter == '.';
}

void STUrlUtils::UpperCaseString(const STChar* aHead, const STChar* aTail, STChar* aUpperCasedString) 
{
	if (aHead != NULL && aTail != NULL)
	{
		do
		{
			*aUpperCasedString++ = toupper(*aHead++);
		}while (aHead < aTail);
	}

	*aUpperCasedString = '\0';
}

void STUrlUtils::CopyString(const STChar* aHead, const STChar* aTail, STChar* aCopiedString) 
{
	if (aHead != NULL && aTail != NULL)
	{
		do
		{
			*aCopiedString++ = *aHead++;
		}while (aHead < aTail);
	}

	*aCopiedString = '\0';
}

const STChar* STUrlUtils::PtrToNearestDotFromStringTail(const STChar* aHead, const STChar* aTail) 
{
	const STChar* ptr = aTail;
	STChar character;

	do 
	{
		character = *(--ptr);
	} while (ptr >= aHead && !IsSplashCharacter(character) && !IsDotCharacter(character));
	ptr++;

	return character == '.' ? ptr : NULL;
}

const STChar* STUrlUtils::PtrToNearestSplashFromStringTail(const STChar* aHead, const STChar* aTail) 
{
	const STChar* ptr = aTail;
	STChar character;

	do 
	{
		character = *(--ptr);
	} while (ptr >= aHead && !IsSplashCharacter(character));
	ptr++;

	return ptr;
}

const STChar* STUrlUtils::PtrToFirstQuestionMarkOrStringTail(const STChar* aStr) 
{
	const STChar* ptr = strchr(aStr, '?');
	if (ptr == NULL)
	{
		ptr = aStr + strlen(aStr);
	}

	return ptr;
}

const STChar* STUrlUtils::PtrToExtHeadOrStringTail(const STChar* aUrl) 
{
	const STChar* extTail = PtrToFirstQuestionMarkOrStringTail(aUrl);

	const STChar* extHead = PtrToNearestDotFromStringTail(aUrl, extTail);

	return (extHead != NULL) ? extHead - 1 : aUrl + strlen(aUrl);
}

void STUrlUtils::ParseExtension(const STChar* aUrl, STChar* aExtension)
{
	const STChar* tail = PtrToFirstQuestionMarkOrStringTail(aUrl);

	const STChar* head = PtrToNearestDotFromStringTail(aUrl, tail);

	UpperCaseString(head, tail, aExtension);
};

void STUrlUtils::ParseShortName(const STChar* aUrl, STChar* aShortName)
{
	const STChar* tail = PtrToExtHeadOrStringTail(aUrl);

	const STChar* head = PtrToNearestSplashFromStringTail(aUrl, tail);

	CopyString(head, tail, aShortName);
}