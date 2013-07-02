/*
 *  PTblender $id$
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *  
 *  Implements the colour and brightness correction originally found
 *  in PTStitcher.
 *
 *  Jan 2006
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
 *
 *  Author: Daniel M German dmgerman at uvic doooot ca
 * 
 */


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

#ifndef _MSC_VER
#include <unistd.h>
#else
#include "compat_win32/getopt.h"
#endif


#include "filter.h"
#include "panorama.h"
#include "PTcommon.h"
#include "ColourBrightness.h"
#include "pttiff.h"

#define DEFAULT_PREFIX "blended%04d"

#define PT_BLENDER_USAGE "PTblender [options] <tiffFiles>+\n\n"\
                         "Options:\n"\
                         "  -p <prefix>\tPrefix for output filename. Defaults to blended%%4d\n"\
                         "  -k <index>\tIndex to image to use as a reference (0-based, defaults to 0)\n"\
                         "  -t [0,1,2]\tType of colour correction:\n"\
                         "  \t\t\t 0 full (default), 1 brightness only, 2 colour only\n"\
                         "  -c\t\tOutput curves smooth (Output 1 per each corrected file)\n"\
                         "  -m\t\tOutput curves arbitrary map (Output 1 per each corrected file)\n"\
                         "  -f\t\tForce processing (ignore warnings)\n"\
                         "  -x\t\tDelete source files (use with care)\n"\
                         "  -q\t\tQuiet run\n"\
                         "  -h\t\tShow this message\n"\
                         "\n"

#define PT_BLENDER_VERSION "PTblender Version " VERSION ", originally written by Helmut Dersch, rewritten by Daniel M German\n"


int main(int argc,char *argv[])
{
    int returnValue = -1;
    int opt;
    int referenceImage = 0;
    fullPath *ptrInputFiles   = NULL;
    fullPath *ptrOutputFiles  = NULL;

    int counter;
    char outputPrefix[MAX_PATH_LENGTH];
    char *endPtr;
    int filesCount = 0;
    char tempString[MAX_PATH_LENGTH];
    int base = 0;
    int outputCurvesType = 0; // if 1 => create Photoshop curve files (.acv)
    int typeCorrection = 0;
    int ptForceProcessing = 0;
    int ptDeleteSources = 0;

    counter = 0;

    printf(PT_BLENDER_VERSION);


    strcpy(outputPrefix, DEFAULT_PREFIX);

    while ((opt = getopt(argc, argv, "p:k:t:fqcmh")) != -1) {

	// o and f -> set output file
	// h       -> help
	// q       -> quiet?
	// k       -> base image, defaults to first
    
	switch(opt) {  // fhoqs        f: 102 h:104  111 113 115  o:f:hsq
	case 'p':
	    if (strlen(optarg) < MAX_PATH_LENGTH) {
		strcpy(outputPrefix, optarg);
	    } else {
		PrintError("Illegal length for output prefix");
		goto end;
	    }
	    break;
	case 'k':
	    referenceImage = strtol(optarg, &endPtr, 10);
	    if (errno != 0) {
		PrintError("Invalid integer in -k option");
		goto end;
	    }
	    break;
	case 't':
	    typeCorrection = strtol(optarg, &endPtr, 10);
	    if (errno != 0 || (typeCorrection < 0 || typeCorrection > 2)) {
		PrintError("Invalid integer in -t option");
		goto end;
	    }
	    break;
	case 'f':
	    ptForceProcessing = 1;
	case 'q':
	    ptQuietFlag = 1;
	    break;
	case 'c':
	    if (outputCurvesType == CB_OUTPUT_CURVE_ARBITRARY) {
		PrintError("Can't use both -c and -m options");
		goto end;
	    }
	    outputCurvesType = CB_OUTPUT_CURVE_SMOOTH;
	    break;
	case 'm':
	    if (outputCurvesType == CB_OUTPUT_CURVE_SMOOTH) {
		PrintError("Can't use both -c and -m options");
		goto end;
	    }
	    outputCurvesType = CB_OUTPUT_CURVE_ARBITRARY;
	    break;
	case 'x':
	    ptDeleteSources = 1;
            break;
	case 'h':
	    printf(PT_BLENDER_USAGE);
        returnValue = 0;
	    goto end;
	default:
	    break;
	}
    }
  
    filesCount = argc - optind;
  
    if ((ptrInputFiles = calloc(filesCount, sizeof(fullPath))) == NULL || 
	(ptrOutputFiles = calloc(filesCount, sizeof(fullPath))) == NULL)  {
	PrintError("Not enough memory");
	goto end;
    }

    base = optind;
    for (; optind < argc; optind++) {
	char *currentParm;

	currentParm = argv[optind];

	if (StringtoFullPath(&ptrInputFiles[optind-base], currentParm) !=0) { // success
	    PrintError("Syntax error: Not a valid pathname");
	    goto end;
	}
    }

    if (filesCount <= 0) {
	PrintError("No files specified in the command line");
	fprintf(stderr, PT_BLENDER_USAGE);
	goto end;
    }

    if (referenceImage < 0 || referenceImage >= filesCount) {
	PrintError(tempString, "Illegal reference image number %d. It should be between 0 and %d\n", 
		referenceImage, filesCount-1);
	goto end;
    }

    //We can't output curves for type 1 or 2 corrections
    if (outputCurvesType != 0) {
	if (typeCorrection!= 0) {
	    PrintError("Output of curves is not supported for correction type %d", typeCorrection);
	    goto end;
	}
    }
    if (panoFileOutputNamesCreate(ptrOutputFiles, filesCount, outputPrefix) == 0) {
	goto end;
    }



    if (!ptForceProcessing) {
	char *temp;
	if ((temp = panoFileExists(ptrOutputFiles, filesCount)) != NULL) {
	    PrintError("Output filename(s) exists. Use -f to overwrite");
	    goto end;
	}

	if (!panoTiffVerifyAreCompatible(ptrInputFiles, filesCount, TRUE)) {
	    PrintError("TIFFs are not compatible");
	    goto end;
	}
    }

    if (! ptQuietFlag) printf("Colour correcting photo using %d as a base type %d\n", referenceImage, typeCorrection);

    ColourBrightness(ptrInputFiles, ptrOutputFiles, filesCount, referenceImage, typeCorrection, outputCurvesType);

    returnValue = 0; // success

end:

    if (ptDeleteSources && returnValue!=-1 && ptrInputFiles) {
	int i;
	for (i = 0; i < filesCount; i++) {
	    remove(ptrInputFiles[i].name);
	}
    }
    free(ptrInputFiles);
    free(ptrOutputFiles);

    return returnValue;  
}

