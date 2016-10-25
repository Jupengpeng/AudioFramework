/**
* File        : TTDirEnum.cpp 
* Created on  : 2011-5-9
* Author      : hu.cao
* Copyright   : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
* Description : CTTDirEnum系统时间实现文件
*/

// INCLUDES
#include "TTOSALConfig.h"
#include "TTDirEnum.h"
#include "TTLog.h"

#if (defined __TT_OS_WINDOWS__)//WINDOWS
#include <Windows.h>

char strMatch( const char *str1, const char *str2) 
{ 
	int slen1 = (int)strlen(str1); 
	int slen2 = (int)strlen(str2); //实际使用时根据strl的长度来动态分配表的内存 
	
	char matchmap[128][128]; 
	memset(matchmap, 0, 128*128); 
	matchmap[0][0] = 1; int i, j, k; 
	
	//遍历目标字符串符串 
	for(i = 1; i<= slen1; ++i) 
	{ 
		//遍历通配符串 
		for(j = 1; j<=slen2; ++j) 
		{ 
			//当前字符之前的字符是否已经得到匹配 
			if(matchmap[i-1][j-1]) 
			{ 
				//匹配当前字符 
				if(str1[i-1] == str2[j-1] || str2[j-1] == '?') 
				{ 
					matchmap[i][j] = 1; 
					//考虑星号在末尾的情况 
					if( i == slen1 && j < slen2) 
					{ 
						for ( k = j+1 ; k <= slen2 ; ++k ) 
						{ 
							if( '*' == str2[k-1]) 
							{ 
								matchmap[i][k] = 1; 
							}
							else
							{ 
								break; 
							} 
						} 
					} 
				}
				else if(str2[j-1] == '*') 
				{ 
					//遇到星号，目标字符串到末尾都能得到匹配 
					for(k = i-1; k<=slen1; ++k) 
					{ 
						matchmap[k][j] = 1; 
					} 
				} 
			} 
		} 
		
		//如果当前字符得到了匹配则继续循环，否则匹配失败 
		for(k = 1; k<=slen2; ++k) 
		{
			if(matchmap[i][k])
			{ 
				break; 
			} 
		} 
		if(k > slen2) 
		{ 
			return 0; 
		} 
	} 
	
	return matchmap[slen1][slen2]; 
}

TTInt CTTDirEnum::EnumDir(RTTPointerArray<TTChar>& aPathArray, const TTChar* aFileDir, const TTChar* aWildCard)
{ 	
	int len = (int)strlen(aFileDir);  
	if(aFileDir==NULL || len<=0) return 0;  

	char path[MAX_PATH];  
	strcpy(path, aFileDir);  
	if(aFileDir[len-1] != '\\') strcat(path, "\\");  
	strcat(path, "*");  

	WIN32_FIND_DATA fd;  
	HANDLE hFindFile = FindFirstFile(path, &fd);  
	if(hFindFile == INVALID_HANDLE_VALUE)  
	{  
		::FindClose(hFindFile); return 0;  
	}  

	char tempPath[MAX_PATH]; 
	BOOL bUserReture=TRUE; 
	BOOL bFinish = FALSE;  
	BOOL bIsDirectory;  

	while(!bFinish)  
	{  
		strcpy(tempPath, aFileDir);  
		if(aFileDir[len-1] != '\\') strcat(tempPath, "\\");  
		strcat(tempPath, fd.cFileName);  

		bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);  

		//如果是.或..  
		if(bIsDirectory && (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0))   
		{
			bFinish = (FindNextFile(hFindFile, &fd) == FALSE);  
			continue;  
		}

		bFinish = (FindNextFile(hFindFile, &fd) == FALSE); 		

		if (strMatch(tempPath, aWildCard))
		{
			char* pDllFile = (char*)malloc((strlen(tempPath) + 1)* sizeof(char));
			TTASSERT(pDllFile != NULL);
			strcpy(pDllFile, tempPath);	 

			aPathArray.Append(pDllFile);
		}
	}  

	::FindClose(hFindFile); 

	return aPathArray.Count();
}

#elif (defined __TT_OS_SYMBIAN__)//SYMBIAN
TTInt CTTDirEnum::EnumDir(RTTPointerArray<TTChar>& aPathArray, const TTChar* aFileDir, const TTChar* aWildCard)
{
	return 0;
}
#elif (defined __TT_OS_ANDROID__)//ANDROID
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <fnmatch.h>

TTInt CTTDirEnum::EnumDir(RTTPointerArray<TTChar>& aPathArray, const TTChar* aFileDir, const TTChar* aWildCard)
{
	DIR *pDir;  
	pDir = opendir(aFileDir);

	LOGI("Codec:%s", aFileDir);
	if(pDir != NULL)  
	{  	
		struct dirent *pDirent;

		while((pDirent = readdir(pDir)) != NULL)
		{
			if((strcmp(pDirent->d_name, ".") == 0) || (strcmp(pDirent->d_name, "..") == 0))
				continue;

			char tempPath[1024] = {0};  //判断是否为文件夹  
			sprintf(tempPath,"%s/%s",aFileDir,pDirent->d_name);  
			struct stat S_stat;
			if((!((lstat(tempPath, &S_stat) < 0) || S_ISDIR(S_stat.st_mode))) && fnmatch(aWildCard, tempPath, FNM_CASEFOLD) == 0)  //打开目录失败，表示非目录
			{  
				char* pDllFile = (char*)malloc((strlen(tempPath) + 1)* sizeof(char));
				TTASSERT(pDllFile != NULL);

				strcpy(pDllFile, tempPath);	 
				aPathArray.Append(pDllFile);
			}
		}

		closedir(pDir); 
	}   

	return aPathArray.Count();
}
#endif

//end of file

