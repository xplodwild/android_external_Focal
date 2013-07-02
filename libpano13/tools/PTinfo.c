/*
 *  PTinfo
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *  
 *  Displays information about an image created with panotools
 *
 *  Nov 2006
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
#include "ptstitch.h"
#include "pttiff.h"

#define PT_INFO_USAGE "PTinfo [options] <tiffFiles>+\n\n"\
                         "Options:\n"\
                         "\t-h\t\tShow this message\n"\
                         "\n"

#define PT_INFO_VERSION "PTinfo Version " VERSION ", originally written by Helmut Dersch, rewritten by Daniel M German\n"

int main(int argc,char *argv[])
{
    int opt;
    fullPath *ptrInputFiles = NULL;
    int i;
    int filesCount = 0;
    int base = 0;

    printf(PT_INFO_VERSION);

    while ((opt = getopt(argc, argv, "p:fqhxe:")) != -1) {

        // o and f -> set output file
        // h       -> help
        // q       -> quiet?
        // k       -> base image, defaults to first
        // s       -> compute seams
    
        switch(opt) {  // fhoqs    f: 102 h:104  111 113 115  o:f:hsq
        case 'q':
            ptQuietFlag = 1;
            break;
        case 'h':
            printf(PT_INFO_USAGE);
            exit(0);
        default:
            break;
        }
    }
  
    filesCount = argc - optind;
  
    if (filesCount < 1) {
        PrintError("No files specified in the command line");
        fprintf(stderr, PT_INFO_USAGE);
        return -1;
    }
    // Allocate memory for filenames
    if ((ptrInputFiles = calloc(filesCount, sizeof(fullPath))) == NULL) { 
        PrintError("Not enough memory");
        return -1;
    }

    base = optind;
    for (; optind < argc; optind++) {
        char *currentParm;

        currentParm = argv[optind];

        if (StringtoFullPath(&ptrInputFiles[optind-base], currentParm) !=0) { // success
            PrintError("Syntax error: Not a valid pathname");
            return(-1);
        }
    }

    for (i=0; i< filesCount ; i++) {
      PrintError("Filename %d %s", i, ptrInputFiles[i].name);
      panoTiffDisplayInfo(ptrInputFiles[i].name);
    }
    
    return 0;
  
}

