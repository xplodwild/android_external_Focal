/*
 *  PTstitch.h
 *
 *  Routines related to stitching and creation of alpha channels
 * 
 *  Copyright Helmut Dersch and Daniel M. German
 *  
 *  Aug 2006
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

#ifndef __PTSTITCH_H__

#define __PTSTITCH_H__

PANO13_IMPEX void panoStitchBlendLayers(unsigned char **imageDataBuffers,
                           unsigned int counterImageFiles,
                           unsigned char *resultBuffer, 
                           unsigned int linesToRead,
                           int imageWidth,
                           unsigned int bitsPerPixel,
                           unsigned int scanLineSize);


PANO13_IMPEX int panoStitchReplaceMasks(fullPath * inputFiles, fullPath * outputFiles,
                           int numberImages, int featherSize);

PANO13_IMPEX unsigned int panoStitchPixelChannelGet(unsigned char *ptr, int bytesPerPixel, int channel);
PANO13_IMPEX void panoStitchPixelChannelSet(unsigned char *ptr, int bytesPerPixel, int channel, unsigned int value);



#endif
