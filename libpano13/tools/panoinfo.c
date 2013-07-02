/*
 *  PanoInfo Demo app
 *
 *  Display info from pano13 dll/library
 *
 *  May 2004
 *
 *  Jim Watters (jimwatters AT rogers DOT com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// gcc -opanoinfo panoinfo.c


#include <stdio.h>
#include <windows.h>
#include"..\version.h"
#include"..\queryfeature.h"

typedef int (*PROC_QF)			(int ,char** ,Tp12FeatureType* );
typedef int (*PROC_QFNUM)		(void);
typedef int (*PROC_QFINT)		(const char *, int *); 
typedef int (*PROC_QFDOUBLE)	(const char *, double *);
typedef int (*PROC_QFSTRING)	(const char *, char *, const int);


int main(int argc,char *argv[])
{
	HINSTANCE		hDll		= NULL;
	PROC_QF			pfQF		= NULL;
	PROC_QFNUM		pfQFNum		= NULL;
	PROC_QFINT		pfQFInt		= NULL;
	PROC_QFDOUBLE	pfQFDouble	= NULL;
	PROC_QFSTRING	pfQFString	= NULL;

	int				iResult;
	double			dResult;
	char			sResult[256];
	char			str1[1000];
	char			str2[10000];

	hDll = LoadLibrary("pano13.dll");
	if(!hDll)
	{
		MessageBox((HWND)NULL, "Could not load dll", "panoinfo", MB_ICONEXCLAMATION);
		goto cleanup;
	}

	pfQF		= (PROC_QF) GetProcAddress( hDll, "queryFeatures" );
	pfQFNum		= (PROC_QFNUM) GetProcAddress( hDll, "queryFeatureCount" );
	pfQFInt     = (PROC_QFINT) GetProcAddress( hDll, "queryFeatureInt" );
	pfQFDouble  = (PROC_QFDOUBLE) GetProcAddress( hDll, "queryFeatureDouble" );
	pfQFString  = (PROC_QFSTRING) GetProcAddress( hDll, "queryFeatureString" );

	str2[0] = '\0';
	if(!pfQF)
	{
		strcat(str2 ,"Error: The 'queryFeatures' funtion not pressent\n");
	}
	if(!pfQFNum)
	{
		strcat(str2 ,"Error: The 'queryFeatureCount' funtion not pressent\n");
	}
	if(!pfQFString)
	{
		strcat(str2 ,"Error: The 'queryFeatureString' funtion not pressent\n");
	}
	if(!pfQFInt)
	{
		strcat(str2 ,"Error: The 'queryFeatureInt' funtion not pressent\n");
	}
	if(!pfQFDouble)
	{
		strcat(str2 ,"Error: The 'queryFeatureDouble' funtion not pressent\n");
	}


	if(pfQFString)
	{
		if((pfQFString) (PTVERSION_NAME_FILEVERSION, sResult, sizeof(sResult)/sizeof(sResult[0]) ))
		{
			sprintf(str1, "pano13 file version:\t%s\n", sResult );
			strcat(str2 ,str1);
		}

//		if((pfQFString) (PTVERSION_NAME_LONG, sResult, sizeof(sResult)/sizeof(sResult[0]) ))
//		{
//			sprintf(str1, "pano13 version:\t%s\n\n", sResult );
//			strcat(str2 ,str1);
//		}

		if((pfQFString) (PTVERSION_NAME_COMMENT, sResult, sizeof(sResult)/sizeof(sResult[0]) ))
		{
			sprintf(str1, "Comment:\t%s\n", sResult );
			strcat(str2 ,str1);
		}

		if((pfQFString) (PTVERSION_NAME_LEGALCOPYRIGHT, sResult, sizeof(sResult)/sizeof(sResult[0]) ))
		{
			sprintf(str1, "Copyright:\t%s\n\n", sResult );
			strcat(str2 ,str1);
		}

	}

	if(pfQFInt)
	{
		if((pfQFInt) ("CPErrorIsDistSphere", &iResult ))
		{
			sprintf(str1, "Optimizer Error:\t%s\n", iResult? "dist sphere" : "dist rect" );
			strcat(str2 ,str1);
		}
	}

	if(pfQFDouble)
	{
		if((pfQFDouble) ("MaxFFOV", &dResult ))
		{
			sprintf(str1, "Max FoV:\t\t%f\n\n", dResult );
			strcat(str2 ,str1);
		}

	}

	if(pfQFNum && pfQF && pfQFString)
	{
		int i,bufsize,numfeatures;
		char *name;
		char *value;
		Tp12FeatureType type;

		strcat(str2 ,"Feature List:\n\n");

		numfeatures = pfQFNum();
		for(i=0; i < numfeatures;i++)
		{
			pfQF(i, &name, &type);
			bufsize = pfQFString(name, NULL, 0)+1;
			value = (char*)malloc(bufsize);
			pfQFString(name, value, bufsize);

			sprintf(str1, "   %s: %s\n", name, value);
			strcat(str2 ,str1);

			free(value);
		}
	}

	MessageBox((HWND)NULL, str2, "pano13.dll properties and features", MB_OK);


cleanup:
	if(hDll)
		FreeLibrary(hDll);
	exit(1);
}
