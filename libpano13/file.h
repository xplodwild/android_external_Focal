/*
 *  file.h
 *
 *  Many of the routines are based on the program PTStitcher by Helmut
 *  Dersch.
 * 
 *  Copyright Helmut Dersch and Daniel M. German
 *  
 *  July 2006
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

#ifndef __FILE_H__

#define __FILE_H__
// these are defined in file.c

PANO13_IMPEX int panoImageRead(Image * im, fullPath * sfile);
PANO13_IMPEX int panoFileMakeTemp(fullPath * path);

// and these are defined in bmp.c, jpeg.c, hdrfile.c, png.c, and ppm.c
// but there is no point in creating a file for each one of them

PANO13_IMPEX int panoBMPRead(Image *im, fullPath *sfile );
PANO13_IMPEX int panoJPEGRead(Image * im, fullPath * sfile);
PANO13_IMPEX int panoHDRRead(Image *im, fullPath *sfile );
PANO13_IMPEX int panoPNGRead(Image *im, fullPath *sfile );
PANO13_IMPEX int panoPPMRead(Image * im, fullPath * sfile);

	
typedef struct {
  int stacked;             // 1 if images are stacked
  int psdBlendingMode;     // for psd output, photoshop/gimp blending mode
  int force8bit;           // 1 if images should be forced to 8bit per channel
  int forceBig;            // 1 if the images should be forced to create PSB instead of PSD
} pano_flattening_parms;

typedef struct {
  int type;             //
  int forceProcessing;
} pano_cropping_parms;


	
enum {
      PSD_NORMAL, 
      PSD_COLOR,
      PSD_DARKEN,
      PSD_DIFFERENCE,
      PSD_DISSOLVE,
      PSD_HARD_LIGHT,
      PSD_HUE,
      PSD_LIGHTEN,
      PSD_LUMINOSITY,
      PSD_MULTIPLY,
      PSD_OVERLAY,
      PSD_SOFT_LIGHT,
      PSD_SATURATION,
      PSD_SCREEN,
      PSD_NUMBER_BLENDING_MODES, // This Is Not Really a mode, it is the counter of number of modes
      };



PANO13_IMPEX extern char *psdBlendingModesNames[PSD_NUMBER_BLENDING_MODES];
		
PANO13_IMPEX extern char *psdBlendingModesInternalName[PSD_NUMBER_BLENDING_MODES];

PANO13_IMPEX int panoFileOutputNamesCreate(fullPath *ptrOutputFiles, int filesCount, char* outputPrefix);

PANO13_IMPEX char *panoFileExists(fullPath *ptrFiles, int filesCount);

PANO13_IMPEX int panoSingleFileExists(char * filename);

PANO13_IMPEX int panoFileDeleteMultiple(fullPath* files, int filesCount);

#endif
