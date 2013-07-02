/*
 *  PTtiffdump
 *
 *  This program compares the contents of 2 different tiff files. If
 * the byte is different it outputs it.
 *
 *  May 2005
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

#define PT_TIFF_DUMP_USAGE "PTtiffdump [options] <inputFile> <outputFile>\n\n"\
                         "Options:\n"\
                         "-o\t\tOverwrite output file if it exists\n"\
			 "\t-q\t\tQuiet run\n\t-h\t\tShow this message\n"\
                         "\n"

#define PT_TIFF_DUMP_VERSION "PTuncrop Version " VERSION ", by Daniel M German\n"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _MSC_VER
#include <unistd.h>
#else
#include "compat_win32/getopt.h"
#endif

#include "tiffio.h"
#include "panorama.h"
#include "filter.h"
#include "PTcommon.h"
#include "pttiff.h"


int above = 0;
int below = 0;

int Compare(unsigned char *data1, 
	    unsigned char *data2, 
	    char channel, int x, int y)
{
  char extra;
  if (*data1 != *data2) {
    if (*data1 > *data2)  {
      extra = 'A';
      above++;
    }     else {
      below++;
      extra = 'B';
    }
    printf("%5d,%4d,%c,%3x %x, %c\n", x, y, channel, *data1, *data2, extra);
  }
  return 0;
}


int main(int argc,char *argv[])
{
  int opt;
  int overwrite = 0;
  int filesCount;
  char *inputFile, *otherFile;

  Image im1;
  Image im2;

  unsigned char *data1, *data2;
  int x, y;

  int count;

  //Need enough space for a message to be returned if something goes wrong
  
  printf(PT_TIFF_DUMP_VERSION);

  while ((opt = getopt(argc, argv, "ohq")) != -1) {

// o overwrite
// h       -> help
// q       -> quiet?
    
    switch(opt) {  // fhoqs        f: 102 h:104  111 113 115  o:f:hsq
    case 'o':
      overwrite = 1;
      break;
    case 'q':
      ptQuietFlag = 1;
      break;
    case 'h':
      printf(PT_TIFF_DUMP_USAGE);
      exit(0);
    default:
      break;
    }
  }
  filesCount = argc - optind;

  if (filesCount != 2) {
    printf(PT_TIFF_DUMP_USAGE);
    exit(0);
  }

  inputFile = argv[optind];
  otherFile = argv[optind+1];

  if (panoTiffRead(&im1, inputFile) == 0) {
    PrintError("Unable to open input file");
    goto error;
  }

  if (panoTiffRead(&im2, otherFile) == 0) {
    PrintError("Unable to open input file");
    goto error;
  }

  data1 = *(im1.data);
  data2 = *(im2.data);

  if (im1.width != im2.width ||
      im1.height != im2.height) {
    printf("The files have differente sizes %d,%d and %d,%d\n",
	   (int)im1.width, (int)im1.height, (int)im2.width, (int)im2.height);
  }

  printf("Comparing %d %d pixels\n", (int)im1.width, (int)im1.height);


  count =0;

  for (x=0;x<im1.width;x++) {
    for (y=0;y<im2.height;y++) {


      if (*data1 != 0) {
	count++;
	Compare(data1, data2, 'a', x, y);
	Compare(data1+1, data2+1, 'r', x, y);
	Compare(data1+2, data2+2, 'g', x, y);
	Compare(data1+3, data2+3, 'b', x, y);
	
	
      }


      data1 +=4;
      data2 +=4;
    }
  }
  
  printf("Compared %d pixels %d above %d below \n", count, above, below);
  printf("Percent of different values %5.3f\n", ((above + below) * 100.0) / (count * 3.0));
  return 0;
  

 error:
  return 1;

}

