#ifndef __ST_URL_UTILS_H__
#define __ST_URL_UTILS_H__
#include "STMacrodef.h"
#include "STTypedef.h"

class STUrlUtils
{
public:
	/**
	* \fn								STInt GetParam(const STChar* aParamStr, const STChar* aKey, STChar* aValue, STInt aValueMaxSize)
	* \brief							获取某个Key所对应的的值
	* \param[in]	aParamStr			整个参数字符串
	* \param[in]    aKey				搜索的Key值
	* \param[out]   aValue				搜索到的Value值
	* \param[out]   aSeparator			分割符号
	* \param[in]    aValueMaxSize		value的最大长度
	* \return							操作状态
	*/
	static STInt						GetParam(const STChar* aParamStr, const STChar* aKey, STChar* aValue, const STChar* aSeparator, STInt aValueMaxSize);		

	/**
	* \fn								void ParseExtension(const STChar* aUrl, STChar* aExtension);
	* \brief							解析Url后缀名。
	* \param[in]	aUrl				Url
	* \param[out]	aExtension			后缀名(没有后缀时，返回空字符)
	*/
	static void							ParseExtension(const STChar* aUrl, STChar* aExtension);

	/**
	* \fn								void ParseShortName(const STChar* aUrl, STChar* aShortName);
	* \brief							解析Url后缀名。
	* \param[in]	aUrl				Url
	* \param[out]	aShortName			Short name.
	*/
	static void							ParseShortName(const STChar* aUrl, STChar* aShortName);

private:
	static STBool IsSplashCharacter(STChar aCharacter);
	static STBool IsDotCharacter(STChar aCharacter);
	static void UpperCaseString(const STChar* aHead, const STChar* aTail, STChar* aUpperCasedString);
	static void CopyString( const STChar* aHead, const STChar* aTail, STChar* aCopiedString);
	static const STChar* PtrToNearestDotFromStringTail(const STChar* aHead, const STChar* aTail);
	static const STChar* PtrToNearestSplashFromStringTail(const STChar* aHead, const STChar* aTail);
	static const STChar* PtrToFirstQuestionMarkOrStringTail(const STChar* aStr);
	static const STChar* PtrToExtHeadOrStringTail(const STChar* aUrl) ;
};

#endif