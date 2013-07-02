/*
 *  PTmasker $Id$
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *  
 *  Takes a set of tiffs and computes their stitching masks
 *
 *  Oct 2006
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

#include <errno.h>


#include "filter.h"
#include "panorama.h"
#include "PTcommon.h"
#include "ptstitch.h"
#include "pttiff.h"
#include "ptfeather.h"
#include "ZComb.h"

#define PT_MASKER_USAGE "PTmasker [options] <tiffFiles>+\n\n"	\
                         "Options:\n"\
                         "\t-p <prefix>\tPrefix for output files (defaults to masked%%4d)\n"\
                         "\t-e <feather>\tSize of the feather (defaults to zero)\n"\
                         "\t-f\t\tForce processing (do not stop at warnings)\n"\
                         "\t-x\t\tDelete source files (use with care)\n"\
                         "\t-q\t\tQuiet run\n"\
                         "\t-h\t\tShow this message\n"\
                         "\t-z\t\tEnable Extended depth of field\n"\
                         "\t-m\t\tFocus estimation mask type\n"\
                         "\t\t\t0  hard-edged masks, mutually exclusive\n"\
                         "\t\t\t1  hard-edged masks, stack of nested masks\n"\
                         "\t\t\t2  blended masks, stack of nested masks\n"\
                         "\t\t\t\t2 is default & strongly recommended -- this option includes a smoothing computation that seems to help a lot.\n"\
                         "\t-w <integer>\t\tFocus estimation window size. Only available if -z\n"\
                         "\t\t\tRecommended value is 0.5%% of image width, e.g. 4 pixels for an 800-pixel image\n."\
                         "\t\t\tComputation cost for focus estimation increases proportional to N^2.  Default w4.\n"\
                         "\t-s <integer>\t\tSmoothing window size,  Only available if -z\n"\
                         "\t\t\tRecommended value is 0.5%% of image width, e.g. 4 pixels for an 800-pixel image\n."\
                         "\t\t\tComputation cost for focus estimation increases proportional to N^2.  Default w4.\n"\
                         "\n"

#define PT_MASKER_VERSION "PTmasker Version " VERSION ", originally written by Helmut Dersch, rewritten by Daniel M German\n"

#define Z_DEFAULT_MASK_TYPE 2
#define Z_DEFAULT_WINDOW_SIZE 4
#define Z_DEFAULT_SMOOTHING_WINDOW_SIZE 4

#define DEFAULT_PREFIX    "masked"

int main(int argc,char *argv[])
{
    int returnValue = -1;
    int opt;
    fullPath *ptrInputFiles   = NULL;
    fullPath *ptrOutputFiles  = NULL;
    
    int counter;
    char outputPrefix[MAX_PATH_LENGTH];
    int filesCount = 0;
    int base = 0;
    int ptForceProcessing = 0;
    int feather = 0;
    int ptDeleteSources = 0;
    int enableFocusEstimation = 0;
    int focusEstimationWindowSize = 0;
    int focusEstimationMaskType = -1;
    int focusEstimationSmoothingWindowSize = 0;

    counter = 0;
    outputPrefix[0] = 0;

    printf(PT_MASKER_VERSION);

    while ((opt = getopt(argc, argv, "p:fqhxe:zw:s:m:")) != -1) {

        // o and f -> set output file
        // h       -> help
        // q       -> quiet?
        // k       -> base image, defaults to first
        // s       -> compute seams
    
        switch(opt) {  // fhoqs    f: 102 h:104  111 113 115  o:f:hsq
        case 'e':
            feather = strtol(optarg, NULL, 10);
            if (errno != 0) {
                PrintError("Illegal value for feather");
                goto end;
            }
            break;
        case 'w':
            focusEstimationWindowSize = strtol(optarg, NULL, 10);
            if (errno != 0 || focusEstimationWindowSize <= 0) {
                PrintError("Illegal value for focus estimation window size [%s]", optarg);
                goto end;
            }
            break;
        case 'm':
            focusEstimationMaskType = strtol(optarg, NULL, 10);
            if (errno != 0 || focusEstimationMaskType  <0 || focusEstimationMaskType > 2) {
              PrintError("Illegal value for focus estimation window type [%s]", optarg);
                goto end;
            }
            break;
        case 's':
            focusEstimationSmoothingWindowSize = strtol(optarg, NULL, 10);
            if (errno != 0 || focusEstimationSmoothingWindowSize <= 0) {
                PrintError("Illegal value for focus estimation smoothing window size [%s]", optarg);
                goto end;
            }
            break;

        case 'p':
            if (strlen(optarg) < MAX_PATH_LENGTH) {
                strcpy(outputPrefix, optarg);
            } else {
                PrintError("Illegal length for output prefix");
                goto end;
            }
            break;
        case 'z':
            enableFocusEstimation = 1;
            break;
        case 'f':
            ptForceProcessing = 1;
            break;
        case 'q':
            ptQuietFlag = 1;
            break;
        case 'x':
            ptDeleteSources = 1;
            break;
        case 'h':
            printf(PT_MASKER_USAGE);
            returnValue = 0;
            goto end;
        default:
            printf(PT_MASKER_USAGE);
            returnValue = 1;
            goto end;
            break;
        }
    }
  
    // check if an output prefix was given
    if (!(*outputPrefix)) {
        strcpy(outputPrefix, "masked");
    }

    filesCount = argc - optind;

  
    if (filesCount < 1) {
        PrintError("No files specified in the command line");
        fprintf(stderr, PT_MASKER_USAGE);
        goto end;
    }

    if (enableFocusEstimation == 0) {
      if (focusEstimationWindowSize != 0 ||
        focusEstimationSmoothingWindowSize != 0 ||
        focusEstimationMaskType != -1)  {
        PrintError("You should specify -z option in order to use options -m -w  or -s");
        goto end;
      }
    } else {
    if (feather == 0) {
        PrintError("-z requires feathering (use -e)");
        goto end;
    }

    if (filesCount == 1) {
        PrintError("-z requires more than one file, disabing -z");
        enableFocusEstimation = 0;
    }
    // At this point we know we are to do Z processing
    // set defaults if no values are given

    if (focusEstimationWindowSize == 0)
        focusEstimationWindowSize = Z_DEFAULT_WINDOW_SIZE;
    if (focusEstimationSmoothingWindowSize == 0)
        focusEstimationSmoothingWindowSize = Z_DEFAULT_SMOOTHING_WINDOW_SIZE;
    if (focusEstimationMaskType == -1)
        focusEstimationMaskType =  Z_DEFAULT_MASK_TYPE;
    }

    // Allocate memory for filenames
    if ((ptrInputFiles = calloc(filesCount, sizeof(fullPath))) == NULL || 
        (ptrOutputFiles = calloc(filesCount, sizeof(fullPath))) == NULL)        {
        PrintError("Not enough memory");
        goto end;
    }

    // GET input file names
    base = optind;
    for (; optind < argc; optind++) {
        char *currentParm;

        currentParm = argv[optind];

        if (StringtoFullPath(&ptrInputFiles[optind-base], currentParm) !=0) { // success
            PrintError("Syntax error: Not a valid pathname");
            goto end;
        }
    }

    // Generate output file names
    if (panoFileOutputNamesCreate(ptrOutputFiles, filesCount, outputPrefix) == 0) {
      goto end;
    }


#ifdef testingfeather
    panoFeatherFile(ptrInputFiles, ptrOutputFiles, feather);
    exit(1);
#endif

    if (! ptForceProcessing) {
	char *temp;
	if ((temp = panoFileExists(ptrOutputFiles, filesCount)) != NULL) {
	    PrintError("Output filename exists %s. Use -f to overwrite", temp);
	    goto end;
	}
	if (filesCount > 1) {
	    if (!panoTiffVerifyAreCompatible(ptrInputFiles, filesCount, TRUE)) {
		PrintError("Input files are not compatible. Use -f to overwrite");
		goto end;
	    }
	}
    }
    if (! ptQuietFlag) printf("Computing seams for %d files\n", filesCount);
	
    if (filesCount == 1) {
      // only do feathering
	if (feather == 0) {
	    PrintError("Only one file specified, nothing to do\n");
	    goto end;
	}
	if (!panoFeatherFile(ptrInputFiles, ptrOutputFiles, feather)) {
	    goto end;
	}

    }
    else { //if (filesCount > 1)
      if (enableFocusEstimation)  {
	  ZCombSetFocusWindowHalfwidth(focusEstimationWindowSize);
	  ZCombSetSmoothingWindowHalfwidth(focusEstimationSmoothingWindowSize);
	  ZCombSetMaskType(focusEstimationMaskType);
	  ZCombSetEnabled();
      }

      if (panoStitchReplaceMasks(ptrInputFiles, ptrOutputFiles, filesCount,
			         feather) != 0) {
	  PrintError("Could not create stitching masks");
	  goto end;
      }
    }

    returnValue = 0; //success

end:
    if (ptDeleteSources && returnValue!=-1 && ptrInputFiles) {
	int i;
	for (i = 0; i < filesCount; i++) {
	    remove(ptrInputFiles[i].name);
	}
    }
    if (ptrInputFiles) free(ptrInputFiles);
    if (ptrOutputFiles) free(ptrOutputFiles);

    return returnValue;
}

