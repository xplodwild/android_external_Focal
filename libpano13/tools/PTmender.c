/*
 *  PTmender 
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *
 *  Dec 2005
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


// TODO
//    Create_Panorama requires some floating point assembly to be interpreted

#define __DEBUG__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef _MSC_VER
#include <unistd.h>
#else
#include "compat_win32/getopt.h"
#endif

#include "tiffio.h"
#include "filter.h"
#include "panorama.h"

#include "PTmender.h"
#include "PTcommon.h"
#include "file.h"
#include "ColourBrightness.h"


// Global variables for the program


int ptDebug = 0;

#define DEFAULT_OUTPUT_NAME "pano"

#define PT_MENDER_VERSION  "PTmender Version " VERSION ", originally written by Helmut Dersch, rewritten by Daniel German\n"

#define PT_MENDER_USAGE "PTmender [options] <script filename> <images>*\n\n"\
                         "Options:\n"\
                         "\t-o <prefix>\tPrefix for output filename, defaults to " DEFAULT_OUTPUT_NAME "\n"\
                         "\t-q\t\tQuiet run\n"\
                         "\t-h\t\tShow this message\n"\
                         "\t-s\t\tSort the filenames provided in the command line in lexicographical order (and only in the command line)\n"\
                         "\n\nIf no images are specified in the command line, then the 'i' lines are used. If 'i' lines do not contain "\
                         "a valid filename then the 'o' lines are used.\n"\
                         "\n"


static int hasPathInfo(char *aName);
static int panoMenderSortingFunction(const void *p1, const void *p2);


static void panoMenderDuplicateScriptFile(char *scriptFileName, char *script, fullPath  *scriptPathName)
{
    FILE* scriptFD;
    int temp;
    
    strcpy(scriptPathName->name, scriptFileName);
      
    // Get a temp filename for the copy of the script
    if (panoFileMakeTemp(scriptPathName) == 0) {
        PrintError("Could not make Tempfile");
        exit(1);
    }
    
    // Copy the script
    if ((scriptFD = fopen(scriptPathName->name, "w")) == NULL) {
        PrintError("Could not open temporary Scriptfile");
        exit(1);
    }
    
    temp = fwrite(script, 1, (int) strlen(script), scriptFD); 
    
    if (strlen(script) != temp) {
        PrintError("Could not write temporary Scriptfile");
        exit(1);
    }
    
    fclose(scriptFD);
    
}

void panoMenderSetFileName(fullPath *ptrImageFileName, char *name, fullPath *scriptFileName)
{
    //Only prepend the path to the script to the filenames if the filenames
    //don't already have path information
    assert(ptrImageFileName != NULL);
    assert(name != NULL);
    assert(scriptFileName != NULL);

    if ( (hasPathInfo(name)) == 0 )
	strcpy(ptrImageFileName->name, scriptFileName->name);
    else
	strcpy(ptrImageFileName->name, "");
    
    InsertFileName(ptrImageFileName, name);
}

static int panoMenderImageFileNamesReadFromScript(fullPath **ptrImageFileNames, fullPath *scriptFileName)
{
    char *script;
    AlignInfo alignInfo;
    int counter;
    int i;

    // We don't have any images yet. We read the Script and load them from it.
    if (ptDebug) {
	fprintf(stderr, "Loading script [%s]\n", scriptFileName->name);
    }
	
    script = LoadScript(scriptFileName);
    
    if (script == NULL) {
	PrintError("Could not load script [%s]", scriptFileName->name);
	return -1;
    }
    
    // parse input script and set up an array of input file names
    if (ParseScript(script, &alignInfo) != 0) {
	// print error
	
	PrintError("Panorama script parsing error");
	return 0;
    }
    
    // The parser of panotools is really broken. To retrieve each
    // input filename it reads the file,
    // finds the first filename, then removes it, and writes the rest of the file again
    // This is done recursively 
    
    counter = alignInfo.numIm;
    
    if (counter != 0) {
	
	// Try to find filenames in input section
	if (ptDebug) {
	    fprintf(stderr, "Found %d images in script file in INPUT section\n", counter);
	}
	// Allocate their space
	if ((*ptrImageFileNames = malloc(512 * counter)) == NULL) {
	    PrintError("Not enough memory");
	    exit(1);
	}
	
	//Iterate over input images and populate input filename array
	for (i = 0; i < counter; i ++) {
	    //If the image filenames don't appear to have any path information, then 
	    //prepend the path to the script (if any) that was specified on the 
	    //command line (Note: this was the only behavior in the original 
	    //PTStitcher.  It has been moved into this conditional block because
	    //the script path could get prepended to an already fully qualified
	    //filename...not very useful.
	    if (ptDebug) {
		fprintf(stderr, "Processing image [%s] from 'i' line %d\n", alignInfo.im[i].name, i);
	    }

	    panoMenderSetFileName(&((*ptrImageFileNames)[i]), alignInfo.im[i].name, scriptFileName);
	    
	    if (ptDebug) {
		fprintf(stderr, "Reading image filename [%s] from 'i' line %d\n", (*ptrImageFileNames)[i].name, i);
	    }
	    
	    
	}
	DisposeAlignInfo(&alignInfo);
    }	
    if (counter == 0) {
	// Sometimes the names of the images are not in the 'o' line, then assume they are
	// in the 'i' line.
	
	
	// create a temporary copy we can overwrite
	fullPath scriptPathName;
	
	counter = numLines(script, 'o');
	
	if (counter == 0) {
	    PrintError("No images found input file script file (there are no 'o' lines nor 'i' lines");
	    exit(1);
	}
	// Allocate their space
	if ((*ptrImageFileNames = malloc(512 * counter)) == NULL) {
	    PrintError("Not enough memory");
	    exit(1);
	}
	
	
	panoMenderDuplicateScriptFile(scriptFileName->name, script, &scriptPathName);
	
	for (i = 0; i < counter; i++) {
	    aPrefs* preferences;
	    if ( (preferences = readAdjustLine(&scriptPathName)) == NULL) {
		PrintError("No 'i' line for image number %d", i);
		exit(1);
	    }
	    
	    if (ptDebug) {
		fprintf(stderr, "Processing image [%s] from 'o' line %d\n", preferences->im.name, i);
	    }

	    panoMenderSetFileName(&((*ptrImageFileNames)[i]), preferences->im.name, scriptFileName);
	    
	    if (ptDebug) {
		fprintf(stderr, "Reading image filename [%s] from 'i' line %d\n",
			(*ptrImageFileNames)[i].name, i);
	    }
	    if (preferences->td != NULL)
		free(preferences->td);
	    
	    if (preferences->ts != NULL)
		free(preferences->ts);
	    
	    free(preferences);
	    
	} // end of for (i = 0; i < counter; i++) {
	free(script); 
	
	remove(scriptPathName.name);
	
    }
    return counter;
}


int main(int argc,char *argv[])
{
    int counter;
    int sort = 0; // do we sort command line filenames?
    fullPath *ptrImageFileNames;

    fullPath scriptFileName;
    fullPath panoFileName;

    int opt;

    char *currentParm;
    ptrImageFileNames = NULL;
    counter = 0;
    strcpy(panoFileName.name, DEFAULT_OUTPUT_NAME);
    strcpy(scriptFileName.name, "");

    printf(PT_MENDER_VERSION);
    
    while ((opt = getopt(argc, argv, "o:f:hsqd")) != -1) {
	
	// o       -> set output file
	// h       -> help?
	// q       -> quiet?
	
	switch(opt) { 
	    
	case 'o':   // specifies output file name
	    if (StringtoFullPath(&panoFileName, optarg) != 0) { 
		PrintError("Syntax error: Not a valid pathname");
		return(-1);
	    }
	    break;
	case 's':
	    sort = 1;
	    break;
	case 'd':
	    ptDebug = 1;
	    break;
	case 'q':
	    ptQuietFlag = 1;
	    break;
      
	case 'h':
	    PrintError(PT_MENDER_USAGE);
	    return -1;
      
	default:
	    break;
	}
    }


    if (ptDebug) {
	fprintf(stderr, "Number of options to process %d\n", argc- optind);
    }

    if (optind - argc == 0) {
	PrintError(PT_MENDER_USAGE);
	return -1;
    }

    // First we get the name of the script
    if (StringtoFullPath(&scriptFileName, argv[optind]) !=0) { // success
	PrintError("Syntax error: Not a valid pathname");
	PrintError(PT_MENDER_USAGE);
	return(-1);
    }

    // Check if we got a filename
    if (strlen(scriptFileName.name) == 0) {
	PrintError("No script name provided\n");
	PrintError(PT_MENDER_USAGE);
	return -1;

    }  // end of if (scriptFileName[0] != 0) {

    if (ptDebug){
	fprintf(stderr,"Script filename %s\n", scriptFileName.name);
    }

    

    // optionally  we receive a list of filenames
    optind ++;
    currentParm = NULL;
    while (optind < argc  ) {
	currentParm = argv[optind];
	optind++;
	counter++;
	
	if((ptrImageFileNames = realloc(ptrImageFileNames, counter * 512)) == NULL) {
	    PrintError("Not enough memory");
	    exit(1);
	}
	
	
	if (StringtoFullPath(&ptrImageFileNames[counter-1], currentParm) != 0) {
	    PrintError("Syntax error: Not a valid pathname");
	    return(-1);
	}
	if (ptDebug){
	    fprintf(stderr,"Getting file from command line %s index %d\n", ptrImageFileNames[counter-1].name, counter);
	}
    } // end of while loop  while (optind < argc  ) {
  
    if (sort && counter > 0) {
	// We have filenames that need to be sorted
	qsort(ptrImageFileNames, counter, 512, panoMenderSortingFunction);;
    }


    // Handle some lack of information
    if (strlen(panoFileName.name) == 0) {
	PrintError("No output filename specified\n");
	PrintError(PT_MENDER_USAGE);
	return -1;
    }
  
    if (counter == 0) {
	counter = panoMenderImageFileNamesReadFromScript(&ptrImageFileNames, &scriptFileName);
    }
    if (counter == 0) {
	PrintError("No images found input file script file (there are no 'o' lines nor 'i' lines");
	exit(1);
    }

    // By now we should have loaded up the input filename array, the output 
    // panorama name, and the name of the script file (copied to a temporary
    // directory).  Now we can create the output image.
    return panoCreatePanorama(ptrImageFileNames, counter, &panoFileName, &scriptFileName);

}


//////////////////////////////////////////////////////////////////////

char* Filename(fullPath* path)
{
    char *temp;
    if ((temp = strrchr(path->name, PATH_SEP)) != NULL) {
	temp++;
    } else {
	temp = path->name;
    }
    return temp;
}


static int hasPathInfo(char *aName)
{
    return ((strchr(aName, PATH_SEP) == NULL) ? 0 : 1);
}

static int panoMenderSortingFunction(const void *p1, const void *p2)
{
  return strcmp(p1, p2);
}
