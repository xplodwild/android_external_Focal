/*
 *  pttiff.h
 *
 * 
 *  Copyright Helmut Dersch and Max Lyons
 *  
 *  May 2006
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
 *  Author: Max Lyons
 * 
 */

#ifndef __PTtiff_h__

#define __PTtiff_h__

#include <tiffio.h>
#include <tiff.h>
#include "file.h"

typedef struct
{
    TIFF *tiff;

    pano_ImageMetadata metadata;

} pano_Tiff;


void getCropInformationFromTiff(TIFF *tif, CropInfo *c);
void setCropInformationInTiff(TIFF *tiffFile, CropInfo *crop_info);

//int TiffGetImageParameters(TIFF *tiffFile, pt_tiff_parms *tiffData);
//int TiffSetImageParameters(TIFF *tiffFile, pt_tiff_parms *tiffData);

PANO13_IMPEX int panoTiffUnCrop(char *inputFile, char *outputFile, pano_cropping_parms *croppingParms);
PANO13_IMPEX int panoTiffCrop(char *inputFile, char *outputFile, pano_cropping_parms *croppingParms);

PANO13_IMPEX int panoTiffGetCropInformation(pano_Tiff * file);
PANO13_IMPEX int panoTiffRowInsideROI(pano_Tiff * image, int row);
PANO13_IMPEX int panoTiffIsCropped(pano_Tiff * file);
PANO13_IMPEX int panoTiffBytesPerLine(pano_Tiff * file);
PANO13_IMPEX int panoTiffSamplesPerPixel(pano_Tiff * file);
PANO13_IMPEX int panoTiffBitsPerPixel(pano_Tiff * file);
PANO13_IMPEX int panoTiffBytesPerPixel(pano_Tiff * file);
PANO13_IMPEX int panoTiffImageHeight(pano_Tiff * file);
PANO13_IMPEX int panoTiffImageWidth(pano_Tiff * file);
PANO13_IMPEX int panoTiffXOffset(pano_Tiff * file);
PANO13_IMPEX int panoTiffYOffset(pano_Tiff * file);
PANO13_IMPEX pano_ImageMetadata *panoTiffImageMetadata(pano_Tiff * file);
PANO13_IMPEX int panoTiffFullImageWidth(pano_Tiff * file);
PANO13_IMPEX int panoTiffFullImageHeight(pano_Tiff * file);
PANO13_IMPEX int panoTiffReadScanLineFullSize(pano_Tiff * file, void *buffer, int row);
PANO13_IMPEX int panoTiffWriteScanLineFullSize(pano_Tiff * file, void *buffer, int row);
PANO13_IMPEX int panoTiffSetCropInformation(pano_Tiff * file);
PANO13_IMPEX int panoTiffGetImageProperties(pano_Tiff * tiff);
PANO13_IMPEX int panoTiffSetImageProperties(pano_Tiff * file);
PANO13_IMPEX void panoTiffClose(pano_Tiff * file);
PANO13_IMPEX pano_Tiff *panoTiffCreateUnCropped(char *fileName,
                                   pano_ImageMetadata * metadata);
PANO13_IMPEX pano_Tiff *panoTiffCreate(char *fileName, pano_ImageMetadata * metadata);
PANO13_IMPEX pano_Tiff *panoTiffOpen(char *fileName);
PANO13_IMPEX int panoTiffReadData(Image * im, pano_Tiff * tif);
PANO13_IMPEX int panoTiffWrite(Image * im, char *fileName);
PANO13_IMPEX int panoTiffRead(Image * im, char *fileName);

PANO13_IMPEX void panoTiffSetErrorHandler(void);
PANO13_IMPEX int panoTiffVerifyAreCompatible(fullPath * tiffFiles, int numberImages,
				int optionalCheck);
PANO13_IMPEX int panoTiffDisplayInfo(char *fileName);

void panoImageDispose(Image *im) ;


#endif
