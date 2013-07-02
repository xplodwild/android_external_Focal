/*
 *  metadata.c
 *
 *  Many of the routines are based on the program PTStitcher by Helmut
 *  Dersch.
 * 
 *  Copyright Helmut Dersch,  Daniel M. German
 *  
 *  Dec 2006
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

// update metadata from an image. IT does not update cropped information

#include <stdio.h>

#include <assert.h>

#include "panorama.h"
#include "metadata.h"


int panoMetadataUpdateFromImage(Image *im) 
{
    im->metadata.imageWidth = im->width;
    im->metadata.imageHeight = im->height;
    im->metadata.bytesPerLine = im->bytesPerLine;
    im->metadata.bitsPerSample = (uint16_t)(im->bitsPerPixel / 4);
    im->metadata.samplesPerPixel = 4;
    im->metadata.bytesPerPixel = im->bitsPerPixel/8;
    im->metadata.bitsPerPixel = im->bitsPerPixel;
    return 1;
}

// Sometimes we need to convert an image from cropped to uncropped. This function 
// takes care of setting the metadata accordingly
void panoUnCropMetadata(pano_ImageMetadata * metadata)
{
    metadata->imageWidth = metadata->cropInfo.fullWidth;
    metadata->imageHeight = metadata->cropInfo.fullHeight;
    metadata->isCropped = 0;
    metadata->bytesPerLine = metadata->imageWidth * metadata->bytesPerPixel;
}

void panoMetadataCropSizeUpdate(pano_ImageMetadata * metadata, pano_CropInfo *cropInfo)
{
    // Update metadata with cropped info.

    metadata->imageWidth = cropInfo->croppedWidth;
    metadata->imageHeight = cropInfo->croppedHeight;
    metadata->bytesPerLine = metadata->imageWidth * metadata->bytesPerPixel;

    // now the crop info in the metadata
   
    metadata->cropInfo.croppedWidth = cropInfo->croppedWidth;
    metadata->cropInfo.croppedHeight = cropInfo->croppedHeight;
    metadata->cropInfo.xOffset += cropInfo->xOffset;
    metadata->cropInfo.yOffset += cropInfo->yOffset;
    metadata->cropInfo.fullWidth = cropInfo->fullWidth;
    metadata->cropInfo.fullHeight = cropInfo->fullHeight;

    metadata->isCropped = (cropInfo->croppedWidth != cropInfo->fullWidth) ||
      (cropInfo->croppedHeight != cropInfo->fullHeight);

    // The full size remains the same, 
    // The rest of the metadata should be the same
}

int panoImageIsCropped(Image *image)
{
    assert(image!= NULL);
    return (image->metadata.isCropped);
}

int panoImageBytesPerPixel(Image *image)
{
    return (image->metadata.bitsPerSample * image->metadata.samplesPerPixel )/ 8;
}

int panoImageBytesPerLine(Image *image)
{
    return (image->metadata.bytesPerLine);
}

int panoImageBitsPerSample(Image *image)
{
    return (image->metadata.bitsPerSample);
}

int panoImageBytesPerSample(Image *image)
{
    return (image->metadata.bitsPerSample/8);
}

int panoImageFullWidth(Image *image)
{
    assert(image!= NULL);
    if (panoImageIsCropped(image))
	return image->metadata.cropInfo.fullWidth;
    else
	return image->width;
}

int panoImageWidth(Image *image)
{
    assert(image!= NULL);
    return image->width;
}

int panoImageHeight(Image *image)
{
    assert(image!= NULL);
    return image->height;
}



int panoImageFullHeight(Image *image)
{
    assert(image!= NULL);
    if (panoImageIsCropped(image))
	return image->metadata.cropInfo.fullHeight;
    else
	return image->height;
}


int panoImageOffsetX(Image *image)
{
    assert(image!= NULL);
    if (panoImageIsCropped(image))
	return image->metadata.cropInfo.xOffset;
    else
	return 0;
}


int panoImageOffsetY(Image *image)
{
    assert(image!= NULL);
    if (panoImageIsCropped(image))
	return image->metadata.cropInfo.yOffset;
    else
	return 0;
}

unsigned char *panoImageData(Image *image)
{
    return *(image->data);
}
