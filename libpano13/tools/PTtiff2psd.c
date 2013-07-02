/*
 *  PTtiff2psd
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *
 *  Converts a set of TIFF files into a Photoshop PSD file
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

#ifndef _MSC_VER
#include <unistd.h>
#else
#include "compat_win32/getopt.h"
#endif

#include <assert.h>
#include <errno.h>


#include "filter.h"
#include "panorama.h"
#include "PTcommon.h"
#include "ColourBrightness.h"
#include "ptstitch.h"
#include "pttiff.h"
#include "file.h"

#define PT_TIFF2PSD_USAGE "PTtiff2psd [options] <tiffFiles>+\n\n"\
                         "Options:\n"\
                         "\t-o <filename>\t\tOutput filename (default merged.psd)\n"\
                         "\t-b <blendingmode>\tSpecify blending mode for layers (use -h to display them)\n"\
                         "\t-f\t\tForce processing (do not stop at warnings)\n"\
                         "\t-s\t\t\tStack them\n"\
                         "\t-q\t\t\tQuiet run\n"\
                         "\t-r\t\t\tReverse layers\n"\
                         "\t-8\t\t\tReduce image to 8bit per channel\n"\
                         "\t-B\t\t\tForce Big, PSB file format\n"\
                         "\t-h\t\t\tShow this message\n"\
                         "\n"

#define PT_TIFF2PSD_VERSION "PTtiff2psd Version " VERSION ", based on code by Helmut Dersch, rewritten by Daniel M German and Jim Watters\n"

#define DEFAULT_OUTPUT_FILENAME "merged.psd"

int main(int argc,char *argv[])
{
    int opt;
    char *endPtr;
    fullPath *ptrInputFiles;
    int counter;
    fullPath outputFilename;
    int filesCount;
    int base = 0;
    int reverseLayers = 0;
    int i;
    int temp;
    int ptForceProcessing = 0;

    pano_flattening_parms flatteningParms;
  
    // clean up struct
    bzero(&flatteningParms, sizeof(flatteningParms));

    ptrInputFiles = NULL;
    counter = 0;

    printf(PT_TIFF2PSD_VERSION);

    if (StringtoFullPath(&outputFilename, DEFAULT_OUTPUT_FILENAME)) {
      PrintError("Not a valid pathnamefor output filename  [%s]", DEFAULT_OUTPUT_FILENAME);
      return(-1);
    }

    while ((opt = getopt(argc, argv, "o:sb:qhfmr8B")) != -1) {

	// o and f -> set output file
	// h       -> help
	// q       -> quiet?
	// k       -> base image, defaults to first
    
	switch(opt) {  // fhoqs        f: 102 h:104  111 113 115  o:f:hsq
	case 'o':
	    if (StringtoFullPath(&outputFilename, optarg) !=0) { // success
		PrintError("Not a valid pathname for output filename");
		return(-1);
	    }
	    break;
	case 'b':
	    temp = strtol(optarg, &endPtr, 10);
	    if (errno != 0 || (temp < 0 || temp >= PSD_NUMBER_BLENDING_MODES)) {
		PrintError("Invalid value in blending mode. Use -h to see possible values ");
		return -1;
	    }
	    printf("Here %d\n", temp);
	    flatteningParms.psdBlendingMode = temp;
	    break;
	case 's':
	    flatteningParms.stacked = 1;
	    break;
	case 'f':
	    ptForceProcessing = 1;
	    break;
	case 'r':
	    reverseLayers = 1;
	    break;
  case '8':
    flatteningParms.force8bit = 1;
    break;
  case 'B':
    flatteningParms.forceBig = 1;
    break;
	case 'q':
	    ptQuietFlag = 1;
	    break;
	case 'h':
	    printf(PT_TIFF2PSD_USAGE);
	    printf("\tValid blending modes:\n");
	    for (i=0;i<PSD_NUMBER_BLENDING_MODES;i++) {
		printf("\t%2d\t%s\n", i, psdBlendingModesNames[i]);
	    }
	    exit(0);
	default:
	    break;
	}
    }


  
    filesCount = argc - optind;
  
    if ((ptrInputFiles = calloc(filesCount, sizeof(fullPath))) == NULL) {
	PrintError("Not enough memory");
	return -1;
    }

    base = optind;
    for (; optind < argc; optind++) {
	char *currentParm;
	int index;

	currentParm = argv[optind];

	// By default files are layered with the first at the bottom, and last at the top
	// This option reverses that
	index = optind - base;
	if (reverseLayers) {
	    index = filesCount - 1 - index;
	    //just in case
	    assert(index >= 0);
	    assert(index < filesCount);
	} 
    
	if (StringtoFullPath(&ptrInputFiles[index], currentParm) !=0) { // success
	    PrintError("Syntax error: Not a valid pathname");
	    return(-1);
	}
    }

    if (filesCount <= 0) {
	PrintError("No files specified in the command line");
	fprintf(stderr, PT_TIFF2PSD_USAGE);
	return -1;
    }

    if (!ptForceProcessing)  {
	if (filesCount > 1 && !panoTiffVerifyAreCompatible(ptrInputFiles, filesCount, TRUE)) {
	    PrintError("TIFFs are not compatible. Use -f to force processing");
	    return -1;
	}
    }

    // Finally create the PSD

    if (!ptQuietFlag) {
	char tempString[MAX_PATH_LENGTH + 40];
	sprintf(tempString, "Creating output file %s", outputFilename.name);
	Progress(_initProgress, tempString);
    }

    //if (panoPSDCreate(ptrInputFiles, filesCount, &outputFilename, &flatteningParms) != 0) {
    if (panoCreateLayeredPSD(ptrInputFiles, filesCount, &outputFilename, &flatteningParms) != 0) {
	PrintError("Error while creating PSD file");
	return -1;
    }

    free(ptrInputFiles);

    return 0;

}

