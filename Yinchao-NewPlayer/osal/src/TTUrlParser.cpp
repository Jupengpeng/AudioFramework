// INCLUDES
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "TTUrlParser.h"
#include <string.h>
#define HTTP_PREFIX_LENGTH	7
#define HTTPS_PREFIX_LENGTH	8

static TTBool isSplashCharacter(TTChar character) 
{
	return character == '/' || character == '\\';
}

static TTBool isDotCharacter(TTChar character) 
{
	return character == '.';
}

static void upperCaseString(const TTChar* head, const TTChar* tail, TTChar* upperCasedString) 
{
	if (head != NULL && tail != NULL)
	{
		do
		{
			*upperCasedString++ = toupper(*head++);
		}while (head < tail);
	}

	*upperCasedString = '\0';
}

static void copyString(const TTChar* head, const TTChar* tail, TTChar* copiedString) 
{
	if (head != NULL && tail != NULL)
	{
		do
		{
			*copiedString++ = *head++;
		}while (head < tail);
	}

	*copiedString = '\0';
}

static const TTChar* ptrToNearestDotFromStringTail(const TTChar* head, const TTChar* tail) 
{
	const TTChar* ptr = tail;
	TTChar character;

	do 
	{
		character = *(--ptr);
	} while (ptr >= head && !isSplashCharacter(character) && !isDotCharacter(character));
	ptr++;

	return character == '.' ? ptr : NULL;
}

static const TTChar* ptrToNearestSplashFromStringTail(const TTChar* head, const TTChar* tail) 
{
	const TTChar* ptr = tail;
	TTChar character;

	do 
	{
		character = *(--ptr);
	} while (ptr >= head && !isSplashCharacter(character));
	ptr++;

	return ptr;
}

static const TTChar* ptrToFirstQuestionMarkOrStringTail(const TTChar* string) 
{
	const TTChar* ptr = strchr(string, '?');
	if (ptr == NULL)
	{
		ptr = string + strlen(string);
	}

	return ptr;
}

static const TTChar* ptrToExtHeadOrStringTail(const TTChar* url) 
{
	const TTChar* extTail = ptrToFirstQuestionMarkOrStringTail(url);

	const TTChar* extHead = ptrToNearestDotFromStringTail(url, extTail);

	return (extHead != NULL) ? extHead - 1 : url + strlen(url);
}


void CTTUrlParser::ParseProtocal(const TTChar* aUrl, TTChar* aProtocal)
{
	const TTChar* protocalTail = strstr(aUrl, "://");
	TTInt protocalLength = 0;
	if (protocalTail != NULL)
	{
		protocalLength = protocalTail - aUrl;
		memcpy(aProtocal, aUrl, protocalLength);
	}
	aProtocal[protocalLength] = '\0';
}

void CTTUrlParser::ParseExtension(const TTChar* aUrl, TTChar* aExtension)
{
	const TTChar* tail = ptrToFirstQuestionMarkOrStringTail(aUrl);

	const TTChar* head = ptrToNearestDotFromStringTail(aUrl, tail);

	upperCaseString(head, tail, aExtension);

	// work around to remove "," in extension. For example,  MP3,1 -> MP3
	TTChar* ptr = strchr(aExtension, ',');
	if (ptr != NULL) 
	{
		*ptr = '\0';
	}
}

void CTTUrlParser::ParseShortName(const TTChar* aUrl, TTChar* aShortName)
{
	const TTChar* shortNameTail = ptrToExtHeadOrStringTail(aUrl);

	const TTChar* shortNameHead = ptrToNearestSplashFromStringTail(aUrl, shortNameTail);

	copyString(shortNameHead, shortNameTail, aShortName);
}

void CTTUrlParser::ParseUrl(const TTChar* aUrl, TTChar* aHost, TTChar* aPath, TTInt& aPort) 
{
	// parse host
	TTChar* hostHead = const_cast<TTChar*> (aUrl);
	if (!strncmp(hostHead, "http://", HTTP_PREFIX_LENGTH))
	{
		hostHead += HTTP_PREFIX_LENGTH;
	}
	else if (!strncmp(hostHead, "https://", HTTPS_PREFIX_LENGTH))
	{
		hostHead += HTTPS_PREFIX_LENGTH;
	}

	TTChar* urlTail = hostHead + strlen(hostHead);
	TTChar* hostTail = strchr(hostHead, '/');
	if (hostTail == NULL)
	{
		hostTail = urlTail;
	}

	TTInt hostLength = hostTail - hostHead;
	memcpy(aHost, hostHead, hostLength);
	aHost[hostLength] = '\0';
    
    TTChar* charColon = strchr(aHost, ':');
    if (charColon) {
        *charColon++ = '\0';
        aPort = atoi(charColon);
    } else {
	      aPort = 80;
	}

	// parse path and port
	aPath[0] = '\0';
	if (hostTail < urlTail) 
	{
		TTChar* pathHead = hostTail + 1;
		TTInt pathLength = urlTail - pathHead;
		memcpy(aPath, pathHead, pathLength);
		aPath[pathLength] = '\0';
	}
}
