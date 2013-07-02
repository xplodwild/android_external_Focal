/*
 *  PTfeather.c
 *
 *  Many of the routines are based on the program PTStitcher by Helmut
 *  Dersch.
 * 
 *  Copyright Helmut Dersch and Daniel M. German
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


#include "filter.h"

#include "pttiff.h"
#include "file.h"
#include "PTcommon.h"
#include "ptstitch.h" 
#include "metadata.h"
#include "ptfeather.h"

#include <assert.h>
#include <float.h>

static void panoFeatherSnowPixel8Bit(unsigned char *pixel, int featherSize, unsigned int index)
{
    int newPixel = 0;
    int randomComponent = 0;
    unsigned int level;

    //    printf("Value %d %d\n", *pixel, index);

    // This operation could potentially overflow
    level = (index * 255)/ featherSize;

    // TODO: check this expression. It needs to be evaluated in the order specified by the
    // parenthesis
    
    //Make sure we do the arithmetic in long long to avoid overflows

    randomComponent = ((rand() - RAND_MAX/2) * (0xfeLL /featherSize)) / RAND_MAX;
    
    // we need to split the following expression to guarantee it is computed as integer, not unsigned char

    newPixel = *pixel;
    newPixel = newPixel-  level  + randomComponent;

    //    printf("Value %d newvalue %d Contribution %d Random %d\n", *pixel, newPixel, level, randomComponent);

    if ( newPixel < 0 ) 
	// we can't make it zero. We rely on value 1 to know where the actual edge of an image is
	*pixel = 0;
    else if (newPixel > 0xff)
	*pixel = 0xff;
    else
	*pixel = newPixel;
}

static void panoFeatherSnowPixel16Bit(unsigned char *pixel, int featherSize, int index)
{
    int newPixel = 0;
    int randomComponent = 0;
    unsigned long long int level;

    uint16_t *pixel16;
    

    level = (index * 0xffff)/ featherSize;

    pixel16  = (uint16_t *) pixel;

    // Make sure we do the arithmetic in long long to avoid overflows
    randomComponent = ((rand() - RAND_MAX/2) * (0xfe00LL /featherSize)) / RAND_MAX;
    
    newPixel = (int)(*pixel16  -  level  + randomComponent);

    //    printf("Value %d newvalue %d Contribution %d Random %d\n", *pixel16, newPixel, level, randomComponent);

    if ( newPixel <= 0 ) 
      // we can't make it zero. We rely on value 1 to know where the actual edge of an image is
      *pixel16 = 0;
    else if (newPixel > 0xffff)
      *pixel16 = 0xffff;
    else {
      *pixel16 = newPixel;
    }
}

static void panoFeatherSnowPixel(unsigned char *pixel, int featherSize, int index, int bytesPerSample)
{
    if (bytesPerSample == 1) 
	panoFeatherSnowPixel8Bit(pixel, featherSize, index);
    else if (bytesPerSample == 2) 
	panoFeatherSnowPixel16Bit(pixel, featherSize, index);
    else 
	assert(0);
}


static void panoFeatherSnowingHorizontalLeft(int column, int featherSize, unsigned char *ptrData, Image *image)
{
    int index;
    int currentColumn;

    unsigned char *ptrPixel;
    unsigned int pixel;
    int bytesPerPixel = panoImageBytesPerPixel(image);
    int bytesPerSample = panoImageBytesPerSample(image);

    // ptrData points to the beginning of the line

    // We start to the right, because the current column is the empty one
    for (currentColumn = column+1, index = featherSize;  currentColumn <  column + featherSize+1; currentColumn++, index-- ) {


        // only operate within the image
        // and IF the mask is not zero
        // We do not want to "feather" outside the boundary
        if (currentColumn < 0 || currentColumn >= panoImageWidth(image))
            continue;
	
        ptrPixel = ptrData + currentColumn * bytesPerPixel;
	pixel = panoStitchPixelChannelGet(ptrPixel, bytesPerSample, 0);


	if (pixel == 0) {// stop when we find the edge
	    //	    printf("Breaking %d\n", currentColumn);
	    break;
	}
	panoFeatherSnowPixel(ptrPixel, featherSize, index, bytesPerSample);

    } ///for
                                
    //    printf("End\n");
}

static void panoFeatherSnowingHorizontalRight(int column, int featherSize, unsigned char *ptrData, Image *image)
{
    int index;
    int currentColumn;

    unsigned int pixel;
    unsigned char *ptrPixel;
    int bytesPerPixel = panoImageBytesPerPixel(image);
    int bytesPerSample = panoImageBytesPerSample(image);

    // ptrData points to the beginning of the line

    //    panoFeatherSnowingAreaVerticalFind(ptrData, bytesPerLine, gradient, column, &leftLines, &rightLines);

    // determine where we start snowing to the left

    index = 1;

    for (currentColumn = column, index = featherSize;  currentColumn >  column - featherSize; currentColumn--, index-- ) {


        // only operate within the image
        // and IF the mask is not zero
        // We do not want to "feather" outside the boundary
        if (currentColumn < 0 || currentColumn >= panoImageWidth(image))
            continue;

        ptrPixel = ptrData + currentColumn * bytesPerPixel;
	pixel = panoStitchPixelChannelGet(ptrPixel, bytesPerSample, 0);

	if (pixel == 0) {// stop when we find the edge
	    //	    printf("Breaking %d\n", currentColumn);
	    break;
	}
	panoFeatherSnowPixel(ptrPixel, featherSize, index, bytesPerSample);

    } ///for (currentColumn = column - gradient/2;      currentColumn <= column; currentColumn++, index++ ) {
                                
    //    printf("End\n");
}

static void panoFeatherSnowingVerticalBottom(int row, int featherSize, unsigned char *ptrData, Image *image)
{
    int index;
    int currentRow;
    int pixel;
    unsigned char *ptrPixel;

    int bytesPerLine = panoImageBytesPerLine(image);
    int bytesPerSample = panoImageBytesPerSample(image);

    for (currentRow = row, index=featherSize;  currentRow > row - featherSize; currentRow--, index-- ) {

        // only operate within the image
        // and IF the mask is not zero
        // We do not want to "feather" outside the boundary
        if (currentRow < 0 || currentRow >= panoImageHeight(image)) {
            continue;
        }

        ptrPixel = ptrData + currentRow * bytesPerLine;

	pixel = panoStitchPixelChannelGet(ptrPixel, bytesPerSample, 0);

	if (pixel == 0) {// stop when we find the edge
	    //	    printf("Breaking %d\n", currentRow);
	    break;
	}
	//	printf("Doing...\n");

	panoFeatherSnowPixel(ptrPixel, featherSize, index, bytesPerSample);

    } ///for (currentRow = row - gradient/2;      currentRow <= row; currentRow++, index++ ) {
                                

}

static void panoFeatherSnowingVerticalTop(int row, int featherSize, unsigned char *ptrData, Image *image)
{
    int index;
    int currentRow;
    int pixel;
    unsigned char *ptrPixel;

    int bytesPerLine = panoImageBytesPerLine(image);
    int bytesPerSample = panoImageBytesPerSample(image);

    for (currentRow = row+1, index=featherSize;  currentRow < row + featherSize+1; currentRow++, index-- ) {

        // only operate within the image
        // and IF the mask is not zero
        // We do not want to "feather" outside the boundary

        if (currentRow < 0 || currentRow >= panoImageHeight(image)) {
            continue;
        }

        ptrPixel = ptrData + currentRow * bytesPerLine;

	pixel = panoStitchPixelChannelGet(ptrPixel, bytesPerSample, 0);

	if (pixel == 0) {// stop when we find the edge
	    //	    printf("Breaking %d\n", currentRow);
	    break;
	}
	panoFeatherSnowPixel(ptrPixel, featherSize, index, bytesPerSample);
    } ///for (currentRow = row - gradient/2;      currentRow <= row; currentRow++, index++ ) {
                                
}



void panoFeatherMaskReplace(Image* image, unsigned int from, unsigned int to)
{

    // Replace a given value in the first channel with the desired value

    int row;
    int column;
    uint16_t *pixel16;

    int bitsPerSample = panoImageBitsPerSample(image);

    int bytesPerPixel = panoImageBytesPerPixel(image);

    int bytesPerLine = panoImageBytesPerLine(image);

    int imageHeight = panoImageHeight(image);

    int imageWidth = panoImageWidth(image);

    unsigned char *pixel = panoImageData(image);


    for (row = 0; row < imageHeight; row ++) {

	pixel = panoImageData(image) + row * bytesPerLine;

        for (column = 0; column < imageWidth; column ++, pixel += bytesPerPixel) {
	    if (bitsPerSample == 8) {
		if ( *pixel == from ) {
		    *pixel = to;
		}
	    } 
	    else if (bitsPerSample == 16) {
		pixel16  = (uint16_t *) pixel;
		if (*pixel16 == from) {
		    *pixel16 = to;
		}
	    } else {
		assert(0);
	    }
        } // for column

    } // for row
    
}

void panoFeatherChannelSave(unsigned char *channelBuffer, Image *image, int channel)
{
  // Copy a given channel to a preallocated buffer area
  int i, j,k;
  int bytesPerChannel;
  unsigned char *imageData;
  int bytesPerPixel;

  bytesPerChannel = panoImageBytesPerSample(image);
  imageData = panoImageData(image);
  bytesPerPixel = panoImageBytesPerPixel(image);

  for (i=0;i<panoImageWidth(image);i++) 
    for (j=0;j<panoImageHeight(image);j++) {
      for (k=0;k<bytesPerChannel;k++) {
            *(channelBuffer+k) = *(imageData + bytesPerChannel * channel + k);
      }
      channelBuffer += bytesPerChannel;
      imageData += bytesPerPixel;
    }
}

void panoFeatherChannelMerge(unsigned char *channelBuffer, Image *image, int channel)
{
  // We merge two alpha channels using the  "multiply" operation.

  // Copy a given channel to a preallocated buffer area
  int i, j;
  int bytesPerChannel;
  unsigned char *imageData;
  int bytesPerPixel;
  unsigned int a, b;
  unsigned long long int la, lb;
  
  // FIrst test the rest of the logic before we do this

  bytesPerChannel = panoImageBytesPerSample(image);
  imageData = panoImageData(image);
  bytesPerPixel = panoImageBytesPerPixel(image);

  for (i=0;i<panoImageWidth(image);i++) 
  for (j=0;j<panoImageHeight(image);j++) {
    if (bytesPerChannel == 1) {
      a = *(imageData);
      b = *(channelBuffer);

      if (a < b) 
          *(imageData) = a;
      else 
        *(imageData) = b;
    } else if (bytesPerChannel == 2) {
      la = *((uint16_t*)imageData);
      lb = *((uint16_t*)channelBuffer);
      if (la < lb) 
        *((uint16_t*)imageData) = (uint16_t)(la);
      else 
        *((uint16_t*)imageData) = (uint16_t)(lb);
    } else {
        assert(0);
    }
    channelBuffer += bytesPerChannel;
    imageData += bytesPerPixel;
  }
}

void panoFeatherChannelSwap(unsigned char *channelBuffer, Image *image, int channel)
{
    // Swaps the data from a given channel
    int i, j,k;
    int bytesPerChannel;
    unsigned char temp;
    unsigned char *imageData;
    int bytesPerPixel;

    bytesPerChannel = panoImageBytesPerSample(image);
    imageData = panoImageData(image);
    bytesPerPixel = panoImageBytesPerPixel(image);
    //    printf("Bytes per channel %d\n", bytesPerChannel);
    for (i=0;i<panoImageWidth(image);i++) 
	for (j=0;j<panoImageHeight(image);j++) {
	    for (k=0;k<bytesPerChannel;k++) {
		temp = *(channelBuffer+k);
		*(channelBuffer+k) = *(imageData + bytesPerChannel * channel + k);
		*(imageData + bytesPerChannel * channel + k) = temp;
	    }
	    channelBuffer += bytesPerChannel;
	    imageData += bytesPerPixel;
	}
}




static int panoFeatherImage(Image * image, int featherSize)
{

    int ratio;
    int difference;
    unsigned char *pixelPtr;
    unsigned char *ptrData;
    unsigned char *savedAlphaChannel;
    int column;
    int row;
    int gradient;

    int bytesPerPixel;
    int bytesPerLine;
    int imageWidth;
    int imageHeight;
    int imageIsCropped;
    int imageLeftOffset;
    int imageTopOffset;
    int imageFullWidth;
    int imageFullHeight;
    int bitsPerSample;
    int bytesPerSample;
    unsigned char *imageData;

    if (featherSize == 0) 
	return 1;


    // Use local variables so we don't have to make function calls for each 
    // iteration
    bitsPerSample = panoImageBitsPerSample(image);
    bytesPerSample = bitsPerSample /8;
    bytesPerPixel = panoImageBytesPerPixel(image);
    bytesPerLine  = panoImageBytesPerLine(image);
    imageHeight = panoImageHeight(image);
    imageWidth = panoImageWidth(image);
    imageIsCropped = panoImageIsCropped(image);
    imageData = panoImageData(image);
    imageFullWidth = panoImageFullWidth(image);
    imageFullHeight = panoImageFullHeight(image);

    imageLeftOffset  = panoImageOffsetX(image);
    imageTopOffset = panoImageOffsetY(image);

    // This is sort of a hack. We replace 0's in the mask with 1's 
    // we have to "undo" it at the end

    //    panoFeatherMaskReplace(image, 0, 1);

    ratio = 0xfe00 / featherSize;

    // Horizontal first

    assert(bitsPerSample == 8 || 
	   bitsPerSample == 16);
    
    // This algorithm is not perfect. It does not deal very well with images that have very wavy edges.
    // For that reason I feather in one direction, then in the other, and then I combine the feathers
    // This means we need to allocate space for an extra channel

    savedAlphaChannel = calloc(bytesPerLine * imageHeight, 1);
    if (savedAlphaChannel == NULL) {
	return 0;
    }
    panoFeatherChannelSave(savedAlphaChannel, image, 0);
    ptrData = imageData;
    
    for ( row = 0; row < imageHeight; row++, ptrData += bytesPerLine) {
	int widthToProcess;

	pixelPtr = ptrData;
	
	// The following code deals with images that are cropped. We should feather edges only 
	// if they are not the absolute edge of an image. 

	// by default we start in column zero 
	column = 0;
	widthToProcess = imageWidth;
	if (imageIsCropped) {
	    // we need to deal with edges that are not "real" edges (as in the uncropped image

	    if ( imageLeftOffset > 0) {
		// we have a mask to the left... so we start in column "-1"
		column = -1;
	    }

	    if (imageLeftOffset + widthToProcess < imageFullWidth) {
		// then "add" one pixel to the right */
		widthToProcess ++;
	    }
	}


	for (/*empty, see initialization above */; column < widthToProcess -1; 
						 column ++, pixelPtr+=bytesPerPixel) {
	    
	    // Values of mask in this pixel and next
	    int thisPixel; 
	    int nextPixel;


	    if (column < 0) {
		// this is the imaginary pixel to the left of the edge that should be feathered
		thisPixel = 1;
	    } else  {
		thisPixel = panoStitchPixelChannelGet(pixelPtr, bytesPerSample, 0);
	    }

	    if (column >= imageWidth -1) {
		// this is the imaginary pixel to the right of the edge that should be feathered
		nextPixel = 1;
	    } else {
		nextPixel = panoStitchPixelChannelGet(pixelPtr + bytesPerPixel, bytesPerSample, 0);
	    }

	    difference = thisPixel - nextPixel;
	    
	    // This operation needs to be done here, otherwise 0x100/ratio will underflow
	    if (bitsPerSample == 8) {
		gradient = (abs(difference) * 0x100LL) / ratio;
	    } 
	    else if (bitsPerSample == 16) {
		gradient = abs(difference) / ratio;
	    } else 
		assert(0);

	    if (nextPixel == 0 && thisPixel != 0) {
		// Moving from the mask... proceed if there is not 

		if ( gradient > 1 ) { //
		    panoFeatherSnowingHorizontalRight(column, featherSize, ptrData, image);
		}
	    }

	    if (thisPixel == 0  && nextPixel != 0) {
		if ( gradient > 1 ) { //
		    panoFeatherSnowingHorizontalLeft(column, featherSize, ptrData, image);
		} // 
                
	    } // 
            
	} // for column...
	
    } // for row

    // We need to do the same in the orthogonal direction
    // Sometimes I wished I had  iterators over an image...

    panoFeatherChannelSwap(savedAlphaChannel, image, 0);

    ptrData = imageData;
    
    for (column = 0; column < image->width; column ++, ptrData+=bytesPerPixel) {
	int heightToProcess;

	// The following code deals with images that are cropped. We should feather edges only 
	// if they are not the absolute edge of an image. 

	// by default we start in column zero 
	row = 0;
	heightToProcess = imageHeight;

	if (imageIsCropped) {
	    // we need to deal with edges that are not "real" edges (as in the uncropped image
	    int imageTopOffset;

	    imageTopOffset  = panoImageOffsetY(image);

	    if ( imageTopOffset > 0) {
		// we have a mask to the left... so we start in column "-1"
		row = -1;
	    }

	    if (imageTopOffset + heightToProcess < imageFullHeight) {
		// then "add" one pixel to the right */
		heightToProcess ++;
	    }
	}

	pixelPtr = ptrData;
	for (/*empty, see initialization above */; row < heightToProcess - 1; 
						 row++, pixelPtr += bytesPerLine) {
	    int thisPixel;
	    int nextPixel;

	    // get pixel in current row
	    // with pixel in the next row
	    
	    if (row < 0) {
		// this is the imaginary pixel to the left of the edge that should be feathered
		thisPixel = 1;
	    } else  {
		thisPixel = panoStitchPixelChannelGet(pixelPtr, bytesPerSample, 0);
	    }

	    if (row >= imageHeight -1) {
		// this is the imaginary pixel to the right of the edge that should be feathered
		nextPixel = 1;
	    } else {
		nextPixel = panoStitchPixelChannelGet(pixelPtr + bytesPerLine, bytesPerSample, 0);
	    }

	    difference = thisPixel - nextPixel;

	    // This operation needs to be done here, otherwise 0x100/ratio will underflow
	    if (bitsPerSample == 8) {
		gradient = (abs(difference) * 0x100LL) / ratio;
	    } 
	    else if (bitsPerSample == 16) {
		gradient = abs(difference) / ratio;
	    } else 
		assert(0);

	    if (gradient > 1) {

		if (nextPixel == 0 && thisPixel != 0) {
		    // Moving from the mask... proceed if there is not 
		    panoFeatherSnowingVerticalBottom(row, featherSize, ptrData, image);
		}

		if (nextPixel != 0 && thisPixel == 0) {
		    panoFeatherSnowingVerticalTop(row, featherSize, ptrData, image);
		}
	    }

	} // for column...
	
    } // for row
    
    // Average mask to avoid banding
    
#if 0

    // THIS WAS AN ATTEMPT TO REMOVE BANDING, BUT IT IS TOO SIMPLE... IT WOULD REQUIRE A LARGER AREA,
    // OR EVEN BETTER, A KERNEL BASED BLUR
    
    {
	int row, column;
	unsigned int above, below,left, right, thisPixel;

	pixelPtr = imageData;

	for ( row = 0; row < imageHeight; row++) {
	    for ( column = 0; column < imageWidth; column++, pixelPtr += bytesPerPixel) {
		// average pixel to its pixels around
		thisPixel = panoStitchPixelChannelGet(pixelPtr, bytesPerSample, 0);
		
		
		if (thisPixel == 0) 
		    continue;
		
		if ((bitsPerSample == 8 && thisPixel == 0xff) ||
		    (bitsPerSample == 16 && thisPixel == 0xffff)) {
		    continue;
		}
		
		// average to its neighbors 
		
		above = below = left = right = thisPixel;
		if (row > 0) 
		    above = panoStitchPixelChannelGet(pixelPtr - bytesPerLine, bytesPerSample, 0);
		
		if (row < imageHeight) 
		    below = panoStitchPixelChannelGet(pixelPtr + bytesPerLine, bytesPerSample, 0);
		
		if (column > 0) 
		    left = panoStitchPixelChannelGet(pixelPtr - bytesPerLine, bytesPerSample, 0);
		
		if (column < imageWidth) 
		    right = panoStitchPixelChannelGet(pixelPtr + bytesPerLine, bytesPerSample, 0);
		
		thisPixel = (thisPixel + above + below + left + right)/ 5;
					     
		panoStitchPixelChannelSet(pixelPtr, bytesPerSample, 0, thisPixel);

	    }	
	}
    }
#endif

    panoFeatherChannelMerge(savedAlphaChannel, image, 0);
    free(savedAlphaChannel);

    return 1;
}


int panoFeatherFile(fullPath * inputFile, fullPath * outputFile,
		     int featherSize)
{
    Image image;
    if (panoTiffRead(&image, inputFile->name) == 0) {
        PrintError("Could not open TIFF-file [%s]", inputFile->name);
        return 0;
    }

    if (panoImageBitsPerSample(&image) == 8 ||  
	panoImageBitsPerSample(&image) == 16) {
        panoFeatherImage(&image, featherSize);
    }
    else {
        fprintf(stderr,
                "Apply feather not supported for this image type (%d bitsPerPixel)\n",
                (int) image.bitsPerPixel);
        exit(1);
    }

    if (panoTiffWrite(&image, outputFile->name) == 0) {
        PrintError("Could not write TIFF-file [%s]", outputFile->name);
        return 0;
    }

    panoImageDispose(&image);

    return 1;

}

