/* Panorama_Tools       -       Generate, Edit and Convert Panoramic Images
   Copyright (C) 1998,1999 - Helmut Dersch  der@fh-furtwangen.de
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*------------------------------------------------------------*/

// Functions to read tiff format


#include <stdio.h>
#include <assert.h>

#include "panorama.h"
#include "filter.h"

#include "tiffio.h"

#include "ZComb.h"


#include "pttiff.h"
#include "metadata.h"
#include "ptstitch.h"


int readplanarTIFF(Image * im, TIFF * tif);
void RGBAtoARGB(uint8_t * buf, int width, int bitsPerPixel);
void ARGBtoRGBA(uint8_t * buf, int width, int bitsPerPixel);
int readtif(Image * im, TIFF * tif);


int readplanarTIFF(Image * im, TIFF * tif)
{
    uint8_t *buf;
    int32_t y;
    short SamplesPerPixel;

    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &SamplesPerPixel);
    if (SamplesPerPixel > 4)
        return -1;
    if (SamplesPerPixel == 3)
    {
        im->bitsPerPixel = im->bitsPerPixel * 3 / 4;
        im->bytesPerLine = im->bytesPerLine * 3 / 4;
    }

    buf = (uint8_t *) malloc((size_t) TIFFScanlineSize(tif));
    if (buf == NULL)
    {
        PrintError("Not enough memory");
        return -1;
    }

    for (y = 0; y < im->height; y++)
    {
        TIFFReadScanline(tif, buf, (uint32) y, 0);
        RGBAtoARGB(buf, im->width, im->bitsPerPixel);
        memcpy(*(im->data) + y * im->bytesPerLine, buf,
               (size_t) (im->bytesPerLine));
    }
    free(buf);
    ThreeToFourBPP(im);
    return 0;

}



// image is allocated, but not image data

int readtif(Image * im, TIFF * tif)
{
    short BitsPerSample, tPhotoMetric, config;
    uint32_t w, h;
    unsigned long **hdl_raster;

    if (tif == NULL || im == NULL)
        return -1;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &tPhotoMetric);
    TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

    SetImageDefaults(im);
    im->width = w;
    im->height = h;
    im->bitsPerPixel = 32 * BitsPerSample / 8;
    im->bytesPerLine = im->width * im->bitsPerPixel / 8;
    im->dataSize = im->bytesPerLine * im->height;


    hdl_raster = (unsigned long **) mymalloc((size_t) im->dataSize);
    if (hdl_raster == NULL)
    {
        PrintError("Not enough memory");
        return -1;
    }

    im->data = (uint8_t **) hdl_raster;

    if (tPhotoMetric == PHOTOMETRIC_RGB && config == PLANARCONFIG_CONTIG)
    {
        return readplanarTIFF(im, tif);
    }

    if (TIFFReadRGBAImage
        (tif, (uint32) w, (uint32) h, (uint32 *) * (im->data), 0))
    {
        // Convert agbr to argb; flip image vertically
        unsigned char *cline, *ct, *cb;
        int h2 = im->height / 2, y;
        // Only do the conversion once
        size_t localBytesPerLine = im->bytesPerLine;

        cline = (unsigned char *) malloc(localBytesPerLine);
        if (cline == NULL)
        {
            PrintError("Not enough memory");
            return -1;
        }

        ct = *im->data;
        cb = *im->data + (im->height - 1) * im->bytesPerLine;

        for (y = 0; y < h2;
             y++, ct += im->bytesPerLine, cb -= im->bytesPerLine)
        {
            RGBAtoARGB(ct, im->width, im->bitsPerPixel);
            RGBAtoARGB(cb, im->width, im->bitsPerPixel);
            memcpy(cline, ct, localBytesPerLine);
            memcpy(ct, cb, localBytesPerLine);
            memcpy(cb, cline, localBytesPerLine);
        }
        if (im->height != 2 * h2)
        {                       // odd number of scanlines
            RGBAtoARGB(*im->data + y * im->bytesPerLine, im->width,
                       im->bitsPerPixel);
        }
        if (cline)
            free(cline);
    }
    else
    {
        PrintError("Could not read tiff-data");
        return -1;
    }
    return 0;
}

/**
 * Populates the CropInfo struct with data about cropping of 
 * the TIFF file specified by filename
 */
void getCropInformationFromTiff(TIFF * tif, CropInfo * c)
{
    float x_position, x_resolution, y_position, y_resolution;

    //these are the actual, physical dimensions of the TIFF file
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &(c->cropped_width));
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &(c->cropped_height));

    //If nothing is stored in these tags, then this must be an "uncropped" TIFF 
    //file, in which case, the "full size" width/height is the same as the 
    //"cropped" width and height
    if (TIFFGetField(tif, TIFFTAG_PIXAR_IMAGEFULLWIDTH, &(c->full_width)) ==
        0)
        (c->full_width = c->cropped_width);
    if (TIFFGetField(tif, TIFFTAG_PIXAR_IMAGEFULLLENGTH, &(c->full_height)) ==
        0)
        (c->full_height = c->cropped_height);

    if (TIFFGetField(tif, TIFFTAG_XPOSITION, &x_position) == 0)
        x_position = 0;
    if (TIFFGetField(tif, TIFFTAG_XRESOLUTION, &x_resolution) == 0)
        x_resolution = 0;
    if (TIFFGetField(tif, TIFFTAG_YPOSITION, &y_position) == 0)
        y_position = 0;
    if (TIFFGetField(tif, TIFFTAG_YRESOLUTION, &y_resolution) == 0)
        y_resolution = 0;

    //offset in pixels of "cropped" image from top left corner of 
    //full image (rounded to nearest integer)
    c->x_offset = (uint32) ((x_position * x_resolution) + 0.49);
    c->y_offset = (uint32) ((y_position * y_resolution) + 0.49);

    //printf("%s: %dx%d  @ %d,%d", filename, c->cropped_width, c->cropped_height, c->x_offset, c->y_offset);
}


void setCropInformationInTiff(TIFF * tiffFile, CropInfo * crop_info)
{
    char *errMsg = "Could not set TIFF tag";
    float pixels_per_resolution_unit = 150.0;

    //If crop_info==NULL then this isn't a "cropped TIFF", so don't include 
    //cropped TIFF tags
    if (crop_info == NULL)
        return;

    //The X offset in ResolutionUnits of the left side of the image, with 
    //respect to the left side of the page.
    if (TIFFSetField
        (tiffFile, TIFFTAG_XPOSITION,
         (float) crop_info->x_offset / pixels_per_resolution_unit) == 0)
        dieWithError(errMsg);
    //The Y offset in ResolutionUnits of the top of the image, with 
    //respect to the top of the page.
    if (TIFFSetField
        (tiffFile, TIFFTAG_YPOSITION,
         (float) crop_info->y_offset / pixels_per_resolution_unit) == 0)
        dieWithError(errMsg);

    //The number of pixels per ResolutionUnit in the ImageWidth
    if (TIFFSetField
        (tiffFile, TIFFTAG_XRESOLUTION,
         (float) pixels_per_resolution_unit) == 0)
        dieWithError(errMsg);
    //The number of pixels per ResolutionUnit in the ImageLength (height)
    if (TIFFSetField
        (tiffFile, TIFFTAG_YRESOLUTION,
         (float) pixels_per_resolution_unit) == 0)
        dieWithError(errMsg);

    //The size of the picture represented by an image.  Note: 2 = Inches.  This
    //is required so that the computation of pixel offset using XPOSITION/YPOSITION and
    //XRESOLUTION/YRESOLUTION is valid (See tag description for XPOSITION/YPOSITION).
    if (TIFFSetField(tiffFile, TIFFTAG_RESOLUTIONUNIT, (uint16_t) 2) == 0)
        dieWithError(errMsg);

    // TIFFTAG_PIXAR_IMAGEFULLWIDTH and TIFFTAG_PIXAR_IMAGEFULLLENGTH
    // are set when an image has been cropped out of a larger image.  
    // They reflect the size of the original uncropped image.
    // The TIFFTAG_XPOSITION and TIFFTAG_YPOSITION can be used
    // to determine the position of the smaller image in the larger one.
    if (TIFFSetField
        (tiffFile, TIFFTAG_PIXAR_IMAGEFULLWIDTH, crop_info->full_width) == 0)
        dieWithError(errMsg);
    if (TIFFSetField
        (tiffFile, TIFFTAG_PIXAR_IMAGEFULLLENGTH,
         crop_info->full_height) == 0)
        dieWithError(errMsg);
}



int readTIFF(Image * im, fullPath * sfile)
{
    char filename[512];
    TIFF *tif;
    int result = 0;
    


#ifdef __Mac__
    unsigned char the_pcUnixFilePath[512];      //added by Kekus Digital
    Str255 the_cString;
    Boolean the_bReturnValue;
    CFStringRef the_FilePath;
    CFURLRef the_Url;           //till here
#endif

    if (FullPathtoString(sfile, filename))
    {
        PrintError("Could not get filename");
        return -1;
    }

#ifdef __Mac__
    CopyCStringToPascal(filename, the_cString); //Added by Kekus Digital
    the_FilePath =
        CFStringCreateWithPascalString(kCFAllocatorDefault, the_cString,
                                       kCFStringEncodingUTF8);
    the_Url =
        CFURLCreateWithFileSystemPath(kCFAllocatorDefault, the_FilePath,
                                      kCFURLHFSPathStyle, false);
    the_bReturnValue =
        CFURLGetFileSystemRepresentation(the_Url, true, the_pcUnixFilePath,
                                         512);

    strcpy(filename, the_pcUnixFilePath);       //till here
#endif

    tif = TIFFOpen(filename, "r");

    if (!tif)
    {
        PrintError("Could not open tiff-file");
        return -1;
    }
    result = readtif(im, tif);

    //Store name of TIFF file
    strncpy(im->name, filename, 255);

    getCropInformationFromTiff(tif, &(im->cropInformation));

    TIFFClose(tif);
    return result;
}

int writeCroppedTIFF(Image * im, fullPath * sfile, CropInfo * crop_info)
{
    char string[512];
    TIFF *tif;
    uint8_t *buf;
    unsigned int y;
    size_t bufsize;

#ifdef __Mac__
    unsigned char the_pcUnixFilePath[512];      //added by Kekus Digital
    Str255 the_cString;
    Boolean the_bReturnValue;
    CFStringRef the_FilePath;
    CFURLRef the_Url;           //till here
#endif

    if (FullPathtoString(sfile, string))
    {
        PrintError("Could not get filename");
        return -1;
    }

#ifdef __Mac__
    CopyCStringToPascal(string, the_cString);   //added by Kekus Digital
    the_FilePath =
        CFStringCreateWithPascalString(kCFAllocatorDefault, the_cString,
                                       kCFStringEncodingUTF8);
    the_Url =
        CFURLCreateWithFileSystemPath(kCFAllocatorDefault, the_FilePath,
                                      kCFURLHFSPathStyle, false);
    the_bReturnValue =
        CFURLGetFileSystemRepresentation(the_Url, true, the_pcUnixFilePath,
                                         512);

    strcpy(string, the_pcUnixFilePath); //till here
#endif

    tif = TIFFOpen(string, "w");
    if (!tif)
    {
        PrintError("Could not create TIFF-file");
        return -1;
    }

    // Rik's mask-from-focus hacking
    if (ZCombSeeImage(im, string))
    {
        PrintError("failed ZCombSeeImage");
    }
    // end Rik's mask-from-focus hacking (for the moment...)

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, im->width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, im->height);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    // Thomas Rauscher, Add for 32 bit (float) support
    //  
    //      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, im->bitsPerPixel < 48 ? 8 : 16 );
    //  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, im->bitsPerPixel == 24 || im->bitsPerPixel == 48 ? 3 : 4);

    switch (im->bitsPerPixel)
    {
    case 24:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
        break;
    case 32:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        break;
    case 48:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
        break;
    case 64:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        break;
    case 96:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 32);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        break;
    case 128:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 32);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        break;
    }
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);


    //"1" indicates that The 0th row represents the visual top of the image, 
    //and the 0th column represents the visual left-hand side.
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);


    //TIFFTAG_ROWSPERSTRIP indicates the number of rows per "strip" of TIFF data.  The original PTStitcher
    //set this value to the panorama height whch meant that the entire image
    //was contained in one strip.  This is not only explicitly discouraged by the 
    //TIFF specification ("Use of a single strip is not recommended. Choose RowsPerStrip 
    //such that each strip is about 8K bytes, even if the data is not compressed, 
    //since it makes buffering simpler for readers. The 8K value is fairly 
    //arbitrary, but seems to work well."), but is also makes it impossible
    //for programs to read the output from Pano Tools to perform random 
    //access on the data which leads to unnecessarily inefficient approaches to 
    //manipulating these images).
    //
    //In practice, most panoramas generated these days (Feb 2006) contain more than 
    //2000 pixels per row (equal to 8KB mentioned above), so it is easiest to
    //hard-code this value to one, which also enables complete random access to 
    //the output files by subsequent blending/processing applications


    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);

    if (crop_info != NULL)
        setCropInformationInTiff(tif, crop_info);

    bufsize = TIFFScanlineSize(tif);
    if ((uint32_t)bufsize < im->bytesPerLine)
        bufsize = im->bytesPerLine;
    buf = (uint8_t *) malloc(bufsize);
    if (buf == NULL)
    {
        PrintError("Not enough memory");
        return -1;
    }

    for (y = 0; (uint32_t) y < im->height; y++)
    {
        memcpy(buf, *(im->data) + y * im->bytesPerLine,
               (size_t) im->bytesPerLine);
        ARGBtoRGBA(buf, im->width, im->bitsPerPixel);
        TIFFWriteScanline(tif, buf, y, 1);
    }
    TIFFClose(tif);
    free(buf);
    return 0;

}


int writeTIFF(Image * im, fullPath * sfile)
{
    return writeCroppedTIFF(im, sfile, &(im->cropInformation));
}



void RGBAtoARGB(uint8_t * buf, int width, int bitsPerPixel)
{
    int x;
    switch (bitsPerPixel)
    {
    case 32:
        {
            uint8_t pix;
            for (x = 0; x < width; x++, buf += 4)
            {
                pix = buf[3];
                buf[3] = buf[2];
                buf[2] = buf[1];
                buf[1] = buf[0];
                buf[0] = pix;
            }
        }
        break;
    case 64:
        {
            uint16_t *bufs = (uint16_t *) buf, pix;
            for (x = 0; x < width; x++, bufs += 4)
            {
                pix = bufs[3];
                bufs[3] = bufs[2];
                bufs[2] = bufs[1];
                bufs[1] = bufs[0];
                bufs[0] = pix;
            }
        }
        break;
    case 128:
        {
            float *bufs = (float *) buf, pix;
            for (x = 0; x < width; x++, bufs += 4)
            {
                pix = bufs[3];
                bufs[3] = bufs[2];
                bufs[2] = bufs[1];
                bufs[1] = bufs[0];
                bufs[0] = pix;
            }
        }
        break;
    }
}

void ARGBtoRGBA(uint8_t * buf, int width, int bitsPerPixel)
{
    int x;
    switch (bitsPerPixel)
    {
    case 32:
        {
            uint8_t pix;
            for (x = 0; x < width; x++, buf += 4)
            {
                pix = buf[0];
                buf[0] = buf[1];
                buf[1] = buf[2];
                buf[2] = buf[3];
                buf[3] = pix;
            }
        }
        break;
    case 64:
        {
            uint16_t *bufs = (uint16_t *) buf, pix;
            for (x = 0; x < width; x++, bufs += 4)
            {
                pix = bufs[0];
                bufs[0] = bufs[1];
                bufs[1] = bufs[2];
                bufs[2] = bufs[3];
                bufs[3] = pix;
            }
        }
        break;
    case 128:
        {
            float *bufs = (float *) buf, pix;
            for (x = 0; x < width; x++, bufs += 4)
            {
                pix = bufs[0];
                bufs[0] = bufs[1];
                bufs[1] = bufs[2];
                bufs[2] = bufs[3];
                bufs[3] = pix;
            }
        }
        break;
    }
}


//////////////////////////////////////////////////////////////////////
// NEW tiff routines
//////////////////////////////////////////////////////////////////////



int panoTiffGetCropInformation(pano_Tiff * file)
{
/*
  Read the crop information of a TIFF file

  Cropped TIFFs have the following properties:

  * Their image width and length is the size of the cropped region
  * The full size of the image is in PIXAR_IMAGEFULLWIDTH and PIXAR_IMAGEFULLHEIGHT
  * If these 2 records do not exist then assume it is uncropped


 */

    float x_position, x_resolution, y_position, y_resolution;
    pano_CropInfo *c;

    assert(file != NULL);
    assert(file->tiff != NULL);

    c = &(file->metadata.cropInfo);
    c->croppedWidth = 0;

    file->metadata.isCropped = FALSE;

    if (TIFFGetField(file->tiff, TIFFTAG_IMAGEWIDTH, &(c->croppedWidth)) == 0
        || TIFFGetField(file->tiff, TIFFTAG_IMAGELENGTH,
                        &(c->croppedHeight)) == 0) {
        PrintError("Error reading file size from TIFF");
        return FALSE;
    }



    if (TIFFGetField(file->tiff, TIFFTAG_XPOSITION, &x_position) != 0) {
	// This means this is a cropped image
	
	file->metadata.isCropped = TRUE;



	// we should get a x resoltion, y position and yresolution, if not, 
	// we will issue an error, as it can be cropped on just "one dimension"
	
	if (TIFFGetField(file->tiff, TIFFTAG_XRESOLUTION, &x_resolution) == 0) {
	    PrintError("Cropped Image contains XPosition but not XResoulion tag. "
		       "Report to developers if you think this is a bug");
	    return FALSE;
	}
	if (TIFFGetField(file->tiff, TIFFTAG_YRESOLUTION, &y_resolution) == 0) {
	    PrintError("Cropped image contains XPosition and YPosition, but it does not contain Y Resultion. "
		       "Report to developers you think this is a bug");
	    return FALSE;
	}

	if (TIFFGetField(file->tiff, TIFFTAG_YPOSITION, &y_position) == 0) {
	    PrintError("Cropped image contains XPosition but not YPosition. "
		       "Report to developers you think this is a bug");
	    return FALSE;
	}

	//offset in pixels of "cropped" image from top left corner of 
	//full image (rounded to nearest integer)
	// The position of the file is given in "real" dimensions, we have to rescale them

	c->xOffset = (uint32) ((x_position * x_resolution) + 0.49);
	c->yOffset = (uint32) ((y_position * y_resolution) + 0.49);

	// One problem is that some images do not contain FULL size. if they don't have
	// then assume it
	
	if (TIFFGetField(file->tiff, TIFFTAG_PIXAR_IMAGEFULLWIDTH, &(c->fullWidth)) == 0)
	    c->fullWidth = c->xOffset  + c->croppedWidth;

	if (TIFFGetField(file->tiff, TIFFTAG_PIXAR_IMAGEFULLLENGTH, &(c->fullHeight)) == 0)
	    c->fullHeight = c->yOffset + c->croppedHeight;
    } else {

	// Not cropped, then initilize fields accordingly

        x_position = 0;
        x_resolution = 0;
        y_position = 0;
        y_resolution = 0;
	c->xOffset = 0;
	c->yOffset = 0;
        c->fullHeight = c->croppedHeight;
	c->fullWidth = c->croppedWidth;

    }


#if 0
    // used for debugging
    printf("%dx%d  @ %d,%d", c->croppedWidth, c->croppedHeight, c->xOffset, c->yOffset);

    printf("get 3 width %d length %d\n", (int) c->croppedWidth,
    (int) c->croppedHeight);
    printf("get 3 full %d length %d\n", (int) c->fullWidth,
               (int) c->fullHeight);
    printf("cropped %d\n", (int) file->metadata.isCropped);
#endif

    return TRUE;

}

// checks if an "absolute" row is inside the ROI
int panoTiffRowInsideROI(pano_Tiff * image, int row)
{
    // We are in the ROI if the row is bigger than the yoffset
    // and the row is less or equal to the offset + height


    assert(image != NULL);
    assert(row >= 0);

    return panoROIRowInside(&(image->metadata.cropInfo), row);


}



int panoTiffIsCropped(pano_Tiff * file)
{
    return file->metadata.isCropped;
}

int panoTiffBytesPerLine(pano_Tiff * file)
{
    return file->metadata.bytesPerLine;
}

int panoTiffSamplesPerPixel(pano_Tiff * file)
{
    return file->metadata.samplesPerPixel;
}



int panoTiffBitsPerPixel(pano_Tiff * file)
{
    return file->metadata.bitsPerPixel;
}

int panoTiffBytesPerPixel(pano_Tiff * file)
{
    return file->metadata.bytesPerPixel;
}

int panoTiffImageHeight(pano_Tiff * file)
{
    return file->metadata.imageHeight;
}

int panoTiffImageWidth(pano_Tiff * file)
{
    return file->metadata.imageWidth;
}

int panoTiffXOffset(pano_Tiff * file)
{
    return file->metadata.cropInfo.xOffset;
}

int panoTiffYOffset(pano_Tiff * file)
{
    return file->metadata.cropInfo.yOffset;
}

pano_ImageMetadata *panoTiffImageMetadata(pano_Tiff * file)
{
    return &file->metadata;
}

int panoTiffFullImageWidth(pano_Tiff * file)
{
    return file->metadata.cropInfo.fullWidth;
}

int panoTiffFullImageHeight(pano_Tiff * file)
{
    return file->metadata.cropInfo.fullHeight;
}





// Read an "absolute" row relative to the cropped area of the TIFF
int panoTiffReadScanLineFullSize(pano_Tiff * file, void *buffer, int row)
{
    // Reads a scan line only if inside ROI, otherwise it only "zeroes" data
    int bytesPerLine;
    int bytesPerPixel;

    assert(file != NULL);

    if (row > panoTiffFullImageHeight(file)) {
        PrintError("Trying to read row %d beyond end of file", row);
        return FALSE;
    }
    bytesPerPixel = panoTiffBytesPerPixel(file);

    bytesPerLine = panoTiffFullImageWidth(file) * bytesPerPixel;

	//printf("Bytes per line %d %d\n", bytesPerLine, panoTiffFullImageWidth(file));

    assert(panoTiffIsCropped(file) ||
           panoTiffFullImageWidth(file) == panoTiffImageWidth(file));


    bzero(buffer, bytesPerLine);

    if (panoTiffRowInsideROI(file, row)) {
        if (TIFFReadScanline
            (file->tiff, (uint8_t *)(buffer) + panoTiffXOffset(file) * bytesPerPixel,
             row - panoTiffYOffset(file), 0) != 1) {
            PrintError("Error reading row %d in tiff file", row);
            return FALSE;
        }
    }
    return TRUE;
}

int panoTiffWriteScanLineFullSize(pano_Tiff * file, void *buffer, int row)
{
    // Reads a scan line only if inside ROI, otherwise it only "zeroes" data
    int bytesPerLine;
    int bytesPerPixel;

    assert(file != NULL);

    if (row > panoTiffFullImageHeight(file)) {
        PrintError("Trying to read row %d beyond end of file", row);
        return FALSE;
    }
    bytesPerPixel = panoTiffBytesPerPixel(file);

    bytesPerLine = panoTiffFullImageWidth(file) * bytesPerPixel;

    assert(panoTiffIsCropped(file) ||
           panoTiffFullImageWidth(file) == panoTiffImageWidth(file));


    if (panoTiffRowInsideROI(file, row)) {
        if (TIFFWriteScanline
            (file->tiff, (uint8_t *)(buffer) + panoTiffXOffset(file) * bytesPerPixel,
             row - panoTiffYOffset(file), 0) != 1) {
            PrintError("Error writing row %d in tiff file", row);
            return FALSE;
        }
    }
    return TRUE;
}



int panoTiffSetCropInformation(pano_Tiff * file)
{
    pano_CropInfo *cropInfo;
    pano_ImageMetadata *metadata;
    TIFF *tiffFile;
    int result;

    assert(file != NULL);

    tiffFile = file->tiff;
    assert(tiffFile != NULL);
    metadata = &(file->metadata);
    cropInfo = &(metadata->cropInfo);


    if (!panoTiffIsCropped(file))
        return TRUE;
	//MRDL: Photoshop sometimes writes out files with a TIFFTAG_XRESOLUTION of 0.
	//If input files are from Photoshop, this values propogates from input
	//file to metadata, and can mess up setting of XPOSITION here...
	if (metadata->xPixelsPerResolution == 0 || metadata->xPixelsPerResolution == 0)
	{
		metadata->xPixelsPerResolution = PANO_DEFAULT_PIXELS_PER_RESOLUTION;
		metadata->yPixelsPerResolution = PANO_DEFAULT_PIXELS_PER_RESOLUTION;
	}

    //The X offset in ResolutionUnits of the left side of the image, with 
    //respect to the left side of the page.
    //The Y offset in ResolutionUnits of the top of the image, with 
    //respect to the top of the page.

    result =
        TIFFSetField(tiffFile, TIFFTAG_XPOSITION,
                     (float) cropInfo->xOffset /
                     metadata->xPixelsPerResolution)
        && TIFFSetField(tiffFile, TIFFTAG_YPOSITION,
                        (float) cropInfo->yOffset /
                        metadata->yPixelsPerResolution);

    //The number of pixels per ResolutionUnit in the ImageWidth
    //The number of pixels per ResolutionUnit in the ImageLength (height)
    result = result &&
        TIFFSetField(tiffFile, TIFFTAG_XRESOLUTION,
                     metadata->xPixelsPerResolution)
        && TIFFSetField(tiffFile, TIFFTAG_YRESOLUTION,
                        metadata->yPixelsPerResolution);

    //The size of the picture represented by an image.  This
    //is required so that the computation of pixel offset using XPOSITION/YPOSITION and
    //XRESOLUTION/YRESOLUTION is valid (See tag description for XPOSITION/YPOSITION).
    result = result &&
        TIFFSetField(tiffFile, TIFFTAG_RESOLUTIONUNIT,
                     metadata->resolutionUnits);

    // TIFFTAG_PIXAR_IMAGEFULLWIDTH and TIFFTAG_PIXAR_IMAGEFULLLENGTH
    // are set when an image has been cropped out of a larger image.  
    // They reflect the size of the original uncropped image.
    // The TIFFTAG_XPOSITION and TIFFTAG_YPOSITION can be used
    // to determine the position of the smaller image in the larger one.
    result = result &&
        TIFFSetField(tiffFile, TIFFTAG_PIXAR_IMAGEFULLWIDTH,
                     cropInfo->fullWidth)
        && TIFFSetField(tiffFile, TIFFTAG_PIXAR_IMAGEFULLLENGTH,
                        cropInfo->fullHeight);
    if (!result) {
        PrintError("Unable to set metadata of output tiff file");
        return FALSE;
    }
    return result;
}

char *panoTiffGetString(pano_Tiff *tiffFile, ttag_t tiffTag)
{
    char *temp;
    char *returnValue;
    if (TIFFGetField(tiffFile->tiff, tiffTag, &temp) == 0) {
    // If the tag does not exist just return
        return NULL;
    } 
    else {
    // Allocate its memory
    returnValue = calloc(strlen(temp) + 1, 1);

    if (returnValue == NULL) 
        return NULL;
    // copy to the new location and return
    strcpy(returnValue, temp);
    return returnValue;
    }
}

int panoTiffGetImageProperties(pano_Tiff * tiff)
{
/*
  Retrieve the properties of the image that we need to keep
 */


    TIFF *tiffFile;
    pano_ImageMetadata *metadata;
    int result;
    void *ptr;

    assert(tiff != NULL);

    tiffFile = tiff->tiff;

    metadata = &tiff->metadata;

    assert(tiffFile != NULL);

    //printf("get\n");

    if (!panoTiffGetCropInformation(tiff)) {
        goto error;
    }


    // These are tags that are expected to be present

    result = TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &metadata->imageWidth)
        && TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &metadata->imageHeight)
        && TIFFGetField(tiffFile, TIFFTAG_BITSPERSAMPLE,
                        &metadata->bitsPerSample)
        && TIFFGetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL,
                        &metadata->samplesPerPixel)
        && TIFFGetField(tiffFile, TIFFTAG_COMPRESSION,
                        &metadata->compression.type)
        && TIFFGetField(tiffFile, TIFFTAG_ROWSPERSTRIP,
                        &metadata->rowsPerStrip);


    if (!result)
        goto error;

    if (metadata->compression.type == COMPRESSION_LZW) {

	// set default compression predictor
	metadata->compression.predictor = 2; //horizontal differencing

	// unleess it comes in the file
	TIFFGetField(tiffFile, TIFFTAG_PREDICTOR,
		     &(metadata->compression.predictor));
    }

    metadata->bytesPerLine = TIFFScanlineSize(tiffFile);
    if (metadata->bytesPerLine <= 0) {
        PrintError("File did not include proper bytes per line information.");
        return 0;
    }

    // These are optional tags
    
    

    if (TIFFGetField(tiffFile, TIFFTAG_ICCPROFILE, &(metadata->iccProfile.size),
             &ptr)) {

        if ((metadata->iccProfile.data = calloc(metadata->iccProfile.size, 1)) == NULL) {
            PrintError("Not enough memory");
            return 0;
        }
        memcpy(metadata->iccProfile.data, ptr, metadata->iccProfile.size);
    }

    tiff->metadata.copyright = panoTiffGetString(tiff, TIFFTAG_COPYRIGHT);
    tiff->metadata.datetime = panoTiffGetString(tiff, TIFFTAG_DATETIME);
    tiff->metadata.imageDescription = panoTiffGetString(tiff, TIFFTAG_IMAGEDESCRIPTION);
    tiff->metadata.artist = panoTiffGetString(tiff, TIFFTAG_ARTIST);

    TIFFGetField(tiffFile, TIFFTAG_PAGENUMBER, &metadata->imageNumber, &metadata->imageTotalNumber);

    //printf("...........................REad and allocate ICC Profile %d %x\n",
    //(int)(metadata->iccProfile.size), (int)(metadata->iccProfile.data));

    if (TIFFGetField
        (tiffFile, TIFFTAG_RESOLUTIONUNIT, &(metadata->resolutionUnits)) == 0)
        metadata->resolutionUnits = PANO_DEFAULT_TIFF_RESOLUTION_UNITS; 

    if (TIFFGetField
        (tiffFile, TIFFTAG_XRESOLUTION,
         &(metadata->xPixelsPerResolution)) == 0)
        metadata->xPixelsPerResolution =
            PANO_DEFAULT_PIXELS_PER_RESOLUTION;

    if (TIFFGetField
        (tiffFile, TIFFTAG_YRESOLUTION,
         &(metadata->yPixelsPerResolution)) == 0)
        metadata->yPixelsPerResolution =
            PANO_DEFAULT_PIXELS_PER_RESOLUTION;

    // Compute rest of the fields 

    // let us truly hope the size of a byte never changes :)
    metadata->bytesPerPixel =
        (metadata->samplesPerPixel * metadata->bitsPerSample) / 8;
    metadata->bitsPerPixel = metadata->bytesPerPixel * 8;

    //printf("get2 bits per sample %d\n", metadata->bitsPerSample);
    //    printf("get2 bits per pixel  %d\n", metadata->bitsPerPixel);
    //printf("get2 bytes per pixel %d\n", metadata->bytesPerPixel);

    return 1;

  error:
    PrintError("Error retrieving metadata from TIFF file");
    return 0;

}

int panoTiffSetImageProperties(pano_Tiff * file)
{
    int returnValue = 1;
    TIFF *tiffFile;
    pano_ImageMetadata *metadata;

    assert(file != NULL);

    tiffFile = file->tiff;
    assert(tiffFile != NULL);

    metadata = &(file->metadata);

    assert(metadata != NULL);

    // Each of the invocations below returns 1 if ok, 0 if error. 

    //printf("samples per pixel %d\n", (int) metadata->samplesPerPixel);
    //printf("samples width %d\n", (int) metadata->imageWidth);
    //printf("compression %d\n", (int) metadata->compression.type);
    returnValue =
        TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, metadata->imageWidth)
        && TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, metadata->imageHeight)
        && TIFFSetField(tiffFile, TIFFTAG_BITSPERSAMPLE,
                        metadata->bitsPerSample)
        && TIFFSetField(tiffFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)
        && TIFFSetField(tiffFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG)
        && TIFFSetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL,
                        metadata->samplesPerPixel)
        && TIFFSetField(tiffFile, TIFFTAG_COMPRESSION,
                        metadata->compression.type)
        && TIFFSetField(tiffFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT)
        && TIFFSetField(tiffFile, TIFFTAG_ROWSPERSTRIP,
                        metadata->rowsPerStrip)
        && TIFFSetField(tiffFile, TIFFTAG_RESOLUTIONUNIT,
                        metadata->resolutionUnits)
        && TIFFSetField(tiffFile, TIFFTAG_XRESOLUTION,
                        metadata->xPixelsPerResolution)
        && TIFFSetField(tiffFile, TIFFTAG_YRESOLUTION,
                        metadata->yPixelsPerResolution)
	&& TIFFSetField(tiffFile, TIFFTAG_PAGENUMBER,  metadata->imageNumber, 
			metadata->imageTotalNumber);




    if (returnValue && metadata->bitsPerSample == 32)
    {
    // If it is 96 or 128 it is  floatint point
        returnValue = TIFFSetField(tiffFile, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    }
    // Take care of special cases

    if (returnValue) {


    }

    // Only set ICCprofile if size > 0

    if (returnValue && metadata->iccProfile.size > 0) {
        returnValue =
            TIFFSetField(tiffFile, TIFFTAG_ICCPROFILE,
             (uint32)metadata->iccProfile.size,
             (void*)(metadata->iccProfile.data));
             //100, data);
    }

    if (returnValue && metadata->compression.type == COMPRESSION_LZW) {
        returnValue =
            TIFFSetField(tiffFile, TIFFTAG_PREDICTOR,
                         metadata->compression.predictor);
    }

    // String fields
    //printf("TO set tricky fields\n");

    if (returnValue && metadata->copyright != NULL)
        returnValue = TIFFSetField(tiffFile, TIFFTAG_COPYRIGHT, metadata->copyright);

    if (returnValue && metadata->artist != NULL)
        returnValue = TIFFSetField(tiffFile, TIFFTAG_ARTIST, metadata->artist);

    if (returnValue && metadata->datetime!=NULL)
        returnValue = TIFFSetField(tiffFile, TIFFTAG_DATETIME, metadata->datetime);

    if (returnValue && metadata->imageDescription != NULL)
        returnValue = TIFFSetField(tiffFile, TIFFTAG_IMAGEDESCRIPTION, metadata->imageDescription);
    
    
    returnValue = returnValue &&
        TIFFSetField(tiffFile, TIFFTAG_SOFTWARE, "Created by Panotools version " VERSION);
    
    //    printf("TO set crop\n");
    if (returnValue && panoTiffIsCropped(file)) {
      //printf("TO set crop 2\n");
        returnValue = panoTiffSetCropInformation(file);
    }

    return returnValue;

}



int panoTiffReadPlannar(Image * im, pano_Tiff * tif)
{
    uint8_t *buf;
    uint32 row;
    short samplesPerPixel;
    int bytesRead;
    int bitsPerPixel;

    samplesPerPixel = panoTiffSamplesPerPixel(tif);


    // We can't read more than 4 samples per pixel
    if (samplesPerPixel > 4 || samplesPerPixel < 3) {
        PrintError("We only support 3 or 4 samples per pixel in TIFF");
        return 0;
    }
    // REmember, we need the values of the TIFF,
    // not the values in the image struct
    // (which will might not be the same
    bytesRead = panoTiffBytesPerLine(tif);
    bitsPerPixel = panoTiffBitsPerPixel(tif);

    //    printf("9 Bytes per line %d (im %d) %d %d\n", panoTiffBytesPerLine(tif),
    //     im->bytesPerLine, im->bitsPerPixel, panoTiffBitsPerPixel(tif));
    buf = calloc(bytesRead, 1);
    if (buf == NULL) {
        PrintError("Not enough memory");
        return 0;
    }

    for (row = 0; row < im->height; row++) {
        if (TIFFReadScanline(tif->tiff, buf, row, 0) != 1) {
            PrintError("Error reading TIFF file");
            goto error;
        }

        RGBAtoARGB(buf, im->width, bitsPerPixel);

        memcpy(*(im->data) + row * im->bytesPerLine, buf,
               (size_t) bytesRead);
    }

    // If we don't have an alpha channel we need to rebuild it
    if (samplesPerPixel == 3) {
        ThreeToFourBPP(im);
    }
    return 1;

 error:
    free(buf);
    return 0;
}







void panoTiffClose(pano_Tiff * file)
{
    panoMetadataFree(&file->metadata);
    TIFFClose(file->tiff);
    free(file);
}


// create the tiff according to most of the needed data
pano_Tiff *panoTiffCreateGeneral(char *fileName,
                                 pano_ImageMetadata * metadata, int uncropped)
{
    pano_Tiff *panoTiff;

    // Allocate the struct's memory
    if ((panoTiff = calloc(sizeof(pano_Tiff), 1)) == NULL) {
        PrintError("Not enough memory");
        return NULL;
    }

    // Open file and retrieve metadata
    panoTiff->tiff = TIFFOpen(fileName, "w");
    if (panoTiff->tiff == NULL) {
        PrintError("Unable to create output file [%s]", fileName);
        free(panoTiff);
        return NULL;
    }

    //printf("Copy metadata from %d\n", (int) metadata->cropInfo.fullWidth);
    if (!panoMetadataCopy(&panoTiff->metadata, metadata)) {
        panoTiffClose(panoTiff);
        return NULL;
    }

    if (uncropped) {
        panoUnCropMetadata(&panoTiff->metadata);
    }

    //printf("Copy metadata %d\n", (int) panoTiff->metadata.cropInfo.fullWidth);
    if (!panoTiffSetImageProperties(panoTiff)) {
        panoTiffClose(panoTiff);
        return NULL;
    }
    //printf("After set image properties\n");

    // return value
    return panoTiff;
}

pano_Tiff *panoTiffCreateUnCropped(char *fileName,
                                   pano_ImageMetadata * metadata)
{
    // If the file is uncropped it creates a cropped version
    return panoTiffCreateGeneral(fileName, metadata, TRUE);
}


pano_Tiff *panoTiffCreate(char *fileName, pano_ImageMetadata * metadata)
{
    return panoTiffCreateGeneral(fileName, metadata, FALSE);
}

pano_Tiff *panoTiffOpen(char *fileName)
{
    pano_Tiff *panoTiff;


    // Allocate the struct's memory
    if ((panoTiff = calloc(sizeof(*panoTiff), 1)) == NULL) {
        PrintError("Not enough memory");
        return NULL;
    }

    // Open file and retrieve metadata
    panoTiff->tiff = TIFFOpen(fileName, "r");

    if (panoTiff->tiff == NULL) {
	PrintError("Unable to open file %s", fileName);
	goto error;
    }

    if (!panoTiffGetImageProperties(panoTiff)) {
	TIFFClose(panoTiff->tiff);
	PrintError("Unable to get properties of tiff file %s", fileName);	
	goto error;
    }
    // return value
    return panoTiff;

 error:
    free(panoTiff);
    return NULL;

}


// image is allocated, but not image data

int panoTiffReadData(Image * im, pano_Tiff * tif)
{
    short tPhotoMetric, config;

    assert(im != NULL);
    // Assume that it is unallocated
    assert(im->data == NULL);

    assert(tif != NULL);

    TIFFGetField(tif->tiff, TIFFTAG_PHOTOMETRIC, &tPhotoMetric);
    TIFFGetField(tif->tiff, TIFFTAG_PLANARCONFIG, &config);


    // Set general parameters. The width and height are of the data

    if ((im->data = (unsigned char **) mymalloc( im->dataSize) ) == NULL) {
        PrintError("Not enough memory");
        return 0;
    }
        
    if (tPhotoMetric == PHOTOMETRIC_RGB && config == PLANARCONFIG_CONTIG) {
        if (!panoTiffReadPlannar(im, tif))
            goto error;
    return TRUE;
    }

    // I changed the stopOnError to 1 so the function reports an error
    // as soon as it happens

    if (TIFFReadRGBAImage(tif->tiff, (uint32) panoTiffImageWidth(tif), 
                          (uint32) panoTiffImageHeight(tif), 
                          (uint32 *) * (im->data), 1)) {
        // Convert agbr to argb; flip image vertically

        unsigned char *cline, *ct, *cb;
        int h2 = im->height / 2, y;
        // Only do the conversion once
        size_t localBytesPerLine = im->bytesPerLine;

        cline = (unsigned char *) calloc(localBytesPerLine, 1);
        if (cline == NULL)
        {
            PrintError("Not enough memory");
            goto error;
        }

        ct = *im->data;
        cb = *im->data + (im->height - 1) * im->bytesPerLine;

        for (y = 0; y < h2;
             y++, ct += im->bytesPerLine, cb -= im->bytesPerLine) {
            RGBAtoARGB(ct, im->width, im->bitsPerPixel);
            RGBAtoARGB(cb, im->width, im->bitsPerPixel);
            memcpy(cline, ct, localBytesPerLine);
            memcpy(ct, cb, localBytesPerLine);
            memcpy(cb, cline, localBytesPerLine);
        }
        if (im->height != 2 * h2) {                       // odd number of scanlines
            RGBAtoARGB(*im->data + y * im->bytesPerLine, im->width,
                       im->bitsPerPixel);
        }
        free(cline);
    }
    else {
        PrintError("Could not read tiff-data");
        goto error;
    }
    return 1;

 error:
    myfree((void**)im->data);
    im->data = NULL;
    return 0;
}




// Output an image to a file
int panoTiffWrite(Image * im, char *fileName)
{
    pano_Tiff *tif = NULL;
    void *buf = 0;
    unsigned int y;
    size_t bufsize;

    // Make sure that the metadata is there...
    assert(im->metadata.imageWidth != 0 &&
       im->metadata.imageHeight != 0);
    

    // first verify the value of some of the metadata fields
    
    assert(im->bitsPerPixel != 0);

    switch (im->bitsPerPixel) {
    case 96:
    case 24:
    case 48:
        im->metadata.samplesPerPixel = 3;
        break;
    case 32:
    case 64:
    case 128:
        im->metadata.samplesPerPixel = 4;
        break;
    default:
        PrintError("Illegal value for bits per pixel in TIFF image to write %s", fileName);
        return FALSE;
    }
    im->metadata.bitsPerSample = (uint16_t)im->bitsPerPixel/im->metadata.samplesPerPixel;


    tif = panoTiffCreate(fileName, &im->metadata);


    if (!tif) {
        PrintError("Could not create TIFF-file");
        return 0;
    }

    // Rik's mask-from-focus hacking
    if (ZCombSeeImage(im, fileName)) {
        PrintError("failed ZCombSeeImage");
    }
    // end Rik's mask-from-focus hacking (for the moment...)

    bufsize = TIFFScanlineSize(tif->tiff);

    if ((uint32_t)bufsize < im->bytesPerLine)
        bufsize = im->bytesPerLine;

    buf = calloc(bufsize, 1);
    if (buf == NULL) {
        PrintError("Not enough memory");
        goto error;
    }

    for (y = 0; (uint32_t) y < im->height; y++) {
	//	printf("Here 1 buffsize %d bytesperline %d width %d\n", bufsize, im->bytesPerLine, im->width);
        memcpy(buf, *(im->data) + y * im->bytesPerLine,
               (size_t) im->bytesPerLine);
        ARGBtoRGBA(buf, im->width, im->bitsPerPixel);
        if (TIFFWriteScanline(tif->tiff, buf, y, 0) != 1) {
            PrintError("Unable to write to TIFF");
            goto error;
        }
    }
    panoTiffClose(tif);
    free(buf);
    return 1;
    
 error:
    if (buf != NULL) 
        free(buf);
    
    if (tif != NULL)
        panoTiffClose(tif);
    
    return 0;
}


int panoUpdateMetadataFromTiff(Image *im, pano_Tiff *tiff) 
{
    int bytesPerLine;
    
    if (!panoMetadataCopy(&im->metadata, &tiff->metadata)) {
        return FALSE;
    }
    //    printf("IMage width %d %d\n",im->width, panoTiffImageWidth(tiff));
    //printf("Bites per pixel %d\n",(int)im->bitsPerPixel);

    im->width = panoTiffImageWidth(tiff);
    im->height = panoTiffImageHeight(tiff);
    
    // We will allocate enough memory for the 3 samples + Alpha Channel
    // Regardless of the actual number of samples in the image

    im->bytesPerLine = panoTiffBytesPerLine(tiff);
    im->bitsPerPixel = panoTiffBitsPerPixel(tiff);

    // Even if we only find 3 samples we will end with 4 
    switch (panoTiffSamplesPerPixel(tiff))
    {
    case 3:
        bytesPerLine = panoTiffBytesPerLine(tiff) * 4 / 3;
        
        im->metadata.bytesPerLine = bytesPerLine;
        
        im->metadata.bitsPerPixel = im->bitsPerPixel * 4/ 3;
        im->metadata.samplesPerPixel = 4;
        im->metadata.bytesPerPixel =
            (4 * im->metadata.bitsPerSample) / 8;
        break;
    case 4:
        bytesPerLine = panoTiffBytesPerLine(tiff);
        break;
    default:
        PrintError("We only support 3 or 4 samples per pixel");
        return 0;
    }
    
    im->dataSize = bytesPerLine * im->height;
    
    // compute how much space we need
    //printf("Data size %d bytesperline %d width %d height %d\n",  
    //(int)im->dataSize,
    //       (int)im->bytesPerLine, (int)im->width,(int)im->height
    //);
    
    return TRUE;
}


/*
  Read a TIFF file and place it in a Image data structure
  Read also the metadata including crop information
*/

int panoTiffRead(Image * im, char *fileName)
{
    pano_Tiff *tiff = NULL;
    int result = FALSE;
    
    SetImageDefaults(im);
    
    //printf("Reading tiff\n");
    if ((tiff = panoTiffOpen(fileName)) == NULL) {
        PrintError("Could not open tiff-file %s", fileName);
        goto end;
    }
    //printf("to update metadata tiff\n");

    // Update metadata in the image
    if (!panoUpdateMetadataFromTiff(im, tiff)) {
        goto end;
    }
    
    //printf("to read data\n");
    
    if (!panoTiffReadData(im, tiff)) {
        PrintError("Unable to read data from TIFF file %s", fileName);
        goto end;
    }

    //Store name of TIFF file
    snprintf(im->name, MAX_PATH_LENGTH, "%s", fileName);

    //printf("after update metadata tiff\n");
    result = TRUE;
   
 end:

    //printf("ENd of Reading tiff\n");

    //panoDumpMetadata(&im->metadata,"Read metadata");

    if (tiff != NULL)
	panoTiffClose(tiff);
    return result;
}


// THis functions clens any memory currently used by the Image 
// data structure
void panoImageDispose(Image *im) 
{
    if (im != NULL) {

	// Release metadata
	panoMetadataFree(&(im->metadata));

	// Release image data
	if (im->data != NULL) {
	    myfree((void **) im->data);
	    im->data = NULL;
	}
    }
}


void panoTiffErrorHandler(const char *module, const char *fmt, va_list ap)
{
    PrintError("Error in TIFF file (%s) ", module);
    PrintError((char *) fmt, ap);
}

void panoTiffSetErrorHandler(void)
{
  // TODO
  // This routines need to be properly implemented. Currently it does nothing

    //MRDL: Reluctantly commented these out...the calls to TIFFSetWarningHandler and 
    //TIFFSetErrorHandler cause to GCC to abort, with a series of errors like this:
    //../../../LibTiff/tiff-v3.6.1/libtiff/libtiff.a(tif_unix.o)(.text+0x11a): In function `TIFFOpen':
    //../../../libtiff/tiff-v3.6.1/libtiff/../libtiff/tif_unix.c:144: multiple definition of `TIFFOpen'
    //../libpano12.a(dyces00121.o)(.text+0x0): first defined here
    // Make sure we have a tiff error handler

  // Disable warnings in TIFF library

    TIFFSetWarningHandler(NULL);

#ifdef TOBEIMPLEMENTED
    TIFFSetWarningHandler(panoTiffErrorHandler);
    TIFFSetErrorHandler(panoTiffErrorHandler);

#endif
}


/* panotools is only able to operate on images that have the same size and same depth.
   if the colour profiles exist they should be the same too

   Some checksk are optional

*/
int panoTiffVerifyAreCompatible(fullPath * tiffFiles, int numberImages,
                                 int optionalCheck)
{
    int currentImage;
    pano_Tiff *firstFile;
    pano_Tiff *otherFile;

    pano_CropInfo *firstCropInfo = NULL;
    pano_CropInfo *otherCropInfo = NULL;

    assert(tiffFiles != NULL);

    assert(numberImages > 1);


    panoTiffSetErrorHandler();

    // Open TIFFs

    firstFile = panoTiffOpen(tiffFiles[0].name);

    if (firstFile == NULL) {
        PrintError("Unable to read tiff file %s", tiffFiles[0].name);
        return FALSE;
    }

    firstCropInfo = &firstFile->metadata.cropInfo;

    // Compare the metadata of the current file with each of the other ones
    for (currentImage = 1; currentImage < numberImages; currentImage++) {

        otherFile = panoTiffOpen(tiffFiles[currentImage].name);
	otherCropInfo = &otherFile->metadata.cropInfo;

        if (otherFile == NULL) {
            PrintError("Unable to read tiff file %s",
                       tiffFiles[currentImage].name);
            return FALSE;
        }


        // THey should have the same width
        if (panoTiffFullImageWidth(firstFile) !=
            panoTiffFullImageWidth(otherFile)) {
            PrintError
                ("Image 0 and %d do not have the same width: %d vs %d\n",
                 currentImage, (int) firstCropInfo->fullWidth,
                 (int) otherCropInfo->fullWidth);
            return FALSE;
        }

        // THey should have the same height
        if (panoTiffFullImageHeight(firstFile) !=
            panoTiffFullImageHeight(otherFile)) {
            PrintError
                ("Image 0 and %d do not have the same length: %d vs %d\n",
                 currentImage, (int) firstCropInfo->fullHeight,
                 (int) otherCropInfo->fullHeight);
            return FALSE;
        }

        // THey should have the same colour depth
        if (panoTiffBytesPerPixel(firstFile) !=
            panoTiffBytesPerPixel(otherFile)) {
            PrintError("Image 0 and %d do not have the same colour depth\n",
                       currentImage);
            return FALSE;
        }
        //printf("compatible 1\n");
        // THey should have the same number of channels per pixel
        if (panoTiffSamplesPerPixel(firstFile) !=
            panoTiffSamplesPerPixel(otherFile)) {
            PrintError
                ("Image 0 and %d do not have the same number of channels per pixel\n",
                 currentImage);
            return FALSE;
        }

        if (optionalCheck) {

            // Compare profiles

            if (firstFile->metadata.iccProfile.size > 0) {

                //  They should be the same size and have the same contents
                if (firstFile->metadata.iccProfile.size !=
                    otherFile->metadata.iccProfile.size
                    || memcmp(firstFile->metadata.iccProfile.data,
                              otherFile->metadata.iccProfile.data,
                              firstFile->metadata.iccProfile.size) != 0) {
                    PrintError
                        ("Image 0 and %d have different colour profiles\n",
                         currentImage);
                    return FALSE;
                }
            }
        }
        panoTiffClose(otherFile);

    }                           // for loop

    panoTiffClose(firstFile);
    //printf("THe files are compatible\n");

    return TRUE;

}

/**
 * Reads inputFile and "uncrops" the image by adding black space to pad
 * image to its full size, saving the result as outputFile.  If an error
 * is encountered messageBuffer is filled with the message, and a non-zero
 * value is returned.  If success, zero is returned
 */
int panoTiffUnCrop(char *inputFile, char *outputFile, pano_cropping_parms *croppingParms)
{

    pano_CropInfo *inputCropInfo = NULL;
    char *buffer = NULL;
    char *offsetInBuffer;
    int inputRow, outputRow;
    pano_Tiff *tiffInput = NULL;
    pano_Tiff *tiffOutput = NULL;
    pano_ImageMetadata *metadata = NULL;

    if ((tiffInput = panoTiffOpen(inputFile)) == NULL) {
        PrintError("Unable to open input file");
        goto error;
    }

    if (!panoTiffIsCropped(tiffInput)) {
        PrintError("Source image is not a cropped tiff");
	if (!croppingParms->forceProcessing)
	    goto error;
        PrintError("Forced processing... continuing");	
    }

    inputCropInfo = &tiffInput->metadata.cropInfo;

    if ((tiffOutput =
         panoTiffCreateUnCropped(outputFile, &tiffInput->metadata)) == NULL) {
        PrintError("Unable to create output file [%s]", outputFile);
        goto error;
    }

    metadata = &tiffOutput->metadata;
    //printf("***Size of line %d\n", metadata->bytesPerLine);

    // Allocate buffer for line
    buffer = calloc(metadata->bytesPerLine, 1);

    if (buffer == NULL) {
        PrintError("Unable to allocate memory for IO buffer");
        goto error;
    }

    inputRow = 0;
    // The crop data has to be placed inside the buffer according to the
    // cropinfo offset

    offsetInBuffer =
        buffer + inputCropInfo->xOffset * metadata->bytesPerPixel;

    assert(metadata->imageHeight > 0);
    // Read one line at a time and transfer to output file
    for (outputRow = 0; outputRow < (int) metadata->imageHeight; outputRow++) {

        //fill empty buffer with empty space (zeros)
        bzero(buffer, metadata->bytesPerLine);

        //if inside ROI then read from input file
        if (panoROIRowInside(inputCropInfo, outputRow)) {

            if (TIFFReadScanline(tiffInput->tiff, offsetInBuffer, inputRow, 0)
                != 1) {
                PrintError("Unable to read scanline %d", inputRow);
                goto error;
            }
            inputRow++;
        }

        //write buffer to outputfile
        if (TIFFWriteScanline(tiffOutput->tiff, buffer, outputRow, 0) != 1) {
            PrintError("Unable to write scanline %d", outputRow);
            goto error;
        }

    }

    //printf("Finished\n");

    free(buffer);
    panoTiffClose(tiffInput);
    panoTiffClose(tiffOutput);

    return 1;

  error:
    // Error handler
    // Make sure we release any resources we have

    if (buffer != NULL)
        free(buffer);

    if (tiffOutput != NULL)
        panoTiffClose(tiffOutput);

    if (tiffInput != NULL)
        panoTiffClose(tiffInput);


    return 0;
}



int panoImageBoundingRectangleCompute(unsigned char *data, int width, int height, int bytesPerPixel, pano_CropInfo *cropInfo)
{
    unsigned char *pixel;
    int xLeft, xRight, yTop, yBottom;
    int row; int column;
    int alphaChannel;

    xLeft = width;
    yTop = 0;

    xRight = 0;
    yBottom = 0;

    // We can do it all in one pass over the data

    pixel = data;
    for (row = 0; row < height; row++) {
	
	for (column = 0; column < width; column++) {

	    alphaChannel = panoStitchPixelChannelGet(pixel, bytesPerPixel/4, 0);

	    if (alphaChannel != 0) {
		// Only set the row the first time
		if (yTop == 0) 
		    yTop = row;
		// Keep setting it until we find no more data
		yBottom = row;

		// Columns are trickier...
		// We are scanning row by row, so we need to 
		if (xLeft > column) {
		    xLeft = column;
		}
		if (xRight < column) {
		    xRight = column;
		}
	    }
	    pixel += bytesPerPixel;
	}
    }

    assert(xRight > xLeft);
    assert(yBottom > yTop);


    // Fill return struct

    cropInfo->fullWidth = width;
    cropInfo->fullHeight = height;
    cropInfo->xOffset = xLeft;
    cropInfo->yOffset = yTop;
    cropInfo->croppedWidth = 1 + xRight - xLeft ; 
    cropInfo->croppedHeight = 1+ yBottom - yTop;
    
    // it should be at most equal to the original size
    assert(width >= cropInfo->croppedWidth);
    assert(height >= cropInfo->croppedHeight);



    //    fprintf(stderr, "Finding boudinging box: x %d y %d width %d height %d\n", (int)cropInfo->xOffset, (int)cropInfo->yOffset,
    //	    (int)cropInfo->croppedWidth, (int)cropInfo->croppedHeight);
	    
    return 1;
}

/**
 * Reads inputFile and crops image
 */
int panoTiffCrop(char *inputFile, char *outputFile, pano_cropping_parms *croppingParms)
{

    pano_Tiff *tiffOutput = NULL;
    pano_ImageMetadata metadata;
    pano_CropInfo cropInfo;
    Image im;
    unsigned char *data = NULL;
    int i;
    fullPath tempFile;

    // Let us do the processing in a different file
    if (panoFileMakeTemp(&tempFile) == 0) {
	PrintError("Could not make Tempfile");
	return -1;
    }
    
    if (panoTiffRead(&im, inputFile) ==0 ) {
        PrintError("Unable to open input file %s", inputFile);
        goto error;
    }

    // Compute inner rectangle

    panoImageBoundingRectangleCompute(*im.data, im.width, im.height, im.bitsPerPixel/8, &cropInfo);

    // Cropinfo is with respect to the data of the read image, not with respet to the "uncropped" image
    if (cropInfo.croppedWidth == 0 || cropInfo.croppedHeight == 0) {
	PrintError("Image is empty, unable to crop. ");
	goto error;
    }

    if (!panoMetadataCopy(&metadata, &(im.metadata))) {
	goto error;
    }

    panoMetadataCropSizeUpdate(&metadata, &cropInfo);

    if ((tiffOutput =
         panoTiffCreate(tempFile.name, &metadata)) == NULL) {
        PrintError("Unable to create output file [%s]", outputFile);
        goto error;
    }

    // Now we need to copy the data.
    
    data = *(im.data);

    // We need to advance data the number of lines that this file has more of ofset
    data += im.bytesPerLine * cropInfo.yOffset;
    for (i =0;i < (int) metadata.imageHeight; i++) {
	unsigned char *ptr;

	// skip the necessary bytes

	ptr = data + im.metadata.bytesPerPixel * cropInfo.xOffset;

	// write
	ARGBtoRGBA(ptr, metadata.imageWidth, metadata.bitsPerPixel);
        if (TIFFWriteScanline(tiffOutput->tiff, ptr, i, 1) != 1) {
	    PrintError("Error writing to output file");
	    goto error;
	}
	
	data += im.bytesPerLine;

    }

    //printf("Finished\n");

    panoTiffClose(tiffOutput);
    remove(outputFile);
    if (rename(tempFile.name, outputFile) != 0) {
	PrintError("Unable to create output file %s", outputFile);
	goto error;
    }


    return 1;

  error:
    // Error handler
    // Make sure we release any resources we have

    if (tiffOutput != NULL) {
        panoTiffClose(tiffOutput);
	remove(tempFile.name);
    }

    return 0;
}


int panoTiffDisplayInfo(char *fileName)
{
    pano_Tiff *imageFile;
    pano_ImageMetadata *meta;

    char *line = NULL;

    if ((imageFile = panoTiffOpen(fileName)) == NULL) {
        PrintError("Could not open TIFF-file %s", fileName);
        return 0;
    }
    meta = &(imageFile->metadata);
    printf("Dimensions: %d,%d\n", meta->imageWidth, meta->imageHeight);
    if (meta->isCropped) {
	printf("Cropped tiff. Full size: %d,%d Offset: %d,%d\n", 
	       (int)meta->cropInfo.fullWidth, (int)meta->cropInfo.fullHeight,
	       (int)meta->cropInfo.xOffset, (int)meta->cropInfo.yOffset);
    }
    printf("Samples per pixel: %d\n", meta->samplesPerPixel);
    printf("Bits per sample: %d\n", meta->bitsPerSample);

    if (meta->iccProfile.size == 0) {
	printf("Contains ICC profile\n");
    }
    if (meta->copyright != NULL){
	printf("Copyright: %s\n", meta->copyright);
    }
    if (meta->datetime != NULL){
	printf("Date created: %s\n", meta->datetime);
    }
    if (meta->artist != NULL){
	printf("Photographer: %s\n", meta->artist);
    }
    printf("Image: %d out of %d\n", meta->imageNumber, meta->imageTotalNumber);

    line = panoParserFindOLine(meta->imageDescription, meta->imageNumber);
    if (line != NULL) {
	printf("Image Spec: %s\n", line);
	free(line);
	if (meta->imageDescription) {
	    printf("Script that created it:\n%s\n", meta->imageDescription);
	}
    }

    return 1;
}
