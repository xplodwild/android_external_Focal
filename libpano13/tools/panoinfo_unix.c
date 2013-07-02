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

// gcc -Wall panoinfo_unix.c -L.. -lpano13 -o panoinfo

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>

#include"../version.h"
#include"../queryfeature.h"
#include "../panorama.h"

int main(int argc,char *argv[])
{
	int				iResult;
	double			dResult;
	char			sResult[256];
	char			str1[1000];
	char			str2[10000];
	int 			i,bufsize,numfeatures;
	char 		   *name;
	char 		   *value;
	Tp12FeatureType type;


	strcpy(str2, ""); // clean up string 
	if(queryFeatureString (PTVERSION_NAME_FILEVERSION, sResult, sizeof(sResult)/sizeof(sResult[0]) ))
	{
		sprintf(str1, "Panotools version:\t%s\n", sResult );
		strcat(str2 ,str1);
	}

//		if(queryFeatureString (PTVERSION_NAME_LONG, sResult, sizeof(sResult)/sizeof(sResult[0]) ))
//		{
//			sprintf(str1, "pano13 version:\t%s\n\n", sResult );
//			strcat(str2 ,str1);
//		}

	if(queryFeatureString (PTVERSION_NAME_COMMENT, sResult, sizeof(sResult)/sizeof(sResult[0]) ))
	{
		sprintf(str1, "Comment:\t%s\n", sResult );
		strcat(str2 ,str1);
	}

	if(queryFeatureString (PTVERSION_NAME_LEGALCOPYRIGHT, sResult, sizeof(sResult)/sizeof(sResult[0]) ))
	{
		sprintf(str1, "Copyright:\t%s\n\n", sResult );
		strcat(str2 ,str1);
	}

	if(queryFeatureInt ("CPErrorIsDistSphere", &iResult ))
	{
		sprintf(str1, "Optimizer Error:\t%s\n", iResult? "dist sphere" : "dist rect" );
		strcat(str2 ,str1);
	}

	if(queryFeatureDouble ("MaxFFOV", &dResult ))
	{
		sprintf(str1, "Max FoV:\t\t%f\n\n", dResult );
		strcat(str2 ,str1);
	}

	strcat(str2 ,"Feature List:\n\n");
	numfeatures = queryFeatureCount();
	for(i=0; i < numfeatures;i++)
	{
		queryFeatures(i, &name, &type);
		bufsize = queryFeatureString(name, NULL, 0)+1;
		value = (char*)malloc(bufsize);
		queryFeatureString(name, value, bufsize);

		sprintf(str1, "   %s: %s\n", name, value);
		strcat(str2 ,str1);
		free(value);
	}
        printf("%s", str2);
        printf("Number of projections %d\n", panoProjectionFormatCount());
	for (i=0; i< panoProjectionFormatCount(); i++) {
	    int j;
	    pano_projection_features features;
	    if (!panoProjectionFeaturesQuery(i, &features) ) {
		printf("Error trying to retreive features of projection index %d\n", i);
		continue;
	    } 
	    printf("Projection index: %d name: %s\n", features.projection, features.name);
	    printf("\t Max Hfov: %f max Vfov: %f\n", features.maxHFOV, features.maxVFOV);
	    printf("\t number of parms: %d\n", features.numberOfParameters);
	    for (j= 0; j< features.numberOfParameters; j++) {
		printf("\t\tParm %d name %s min value %f max value %f\n", j, features.parm[j].name, features.parm[j].minValue, features.parm[j].maxValue);

	    }
	}

        return 0;
}
