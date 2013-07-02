/*
 *  ptstitch.c
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

#include <assert.h>

#include "filter.h"
#include "pttiff.h"
#include "file.h"
#include "ptstitch.h"
#include "PTcommon.h"
#include "ptfeather.h"
#include "metadata.h"

// Get the value of a channel in the  pixel pointed by ptr
unsigned int panoStitchPixelChannelGet(unsigned char *ptr, int bytesPerChannel, int channel)
{
    uint16_t *pixel16;
    assert(ptr != NULL);

    assert(channel >= 0 && channel <=3);
    assert(bytesPerChannel == 1 || bytesPerChannel ==2);

    if (bytesPerChannel == 1) {
        return *(ptr + channel);
    } 
    else if (bytesPerChannel == 2) {
        pixel16  = (uint16_t *) ptr;
        return *(pixel16+channel);
    } 
    else {
        assert(0);
        return 0;// fix warning.
    }

}

// Get the value of a channel in the  pixel pointed by ptr
void panoStitchPixelChannelSet(unsigned char *ptr, int bytesPerChannel, int channel, unsigned int value)
{
    uint16_t *pixel16;
    assert(ptr != NULL);

    assert(channel >= 0 && channel <=3);
    assert(bytesPerChannel == 4 || bytesPerChannel ==8);

    if (bytesPerChannel == 4) {
        *(ptr + channel) = value;
    } 
    else if (bytesPerChannel == 8) {
        pixel16  = (uint16_t *) ptr;
        *(pixel16+channel) = value;
    } 
    else {
        assert(0);
    }

}

static unsigned int panoStitchPixelMapGet(unsigned char *ptr, int bytesPerPixel)
{
    uint16_t *pixel16;
    unsigned char *temp;
    assert(ptr != NULL);

    assert(bytesPerPixel == 4 || bytesPerPixel ==8);

    // We use 2 channel (Green in 16 bit) and (Green and Blue in 8 bit images) to store the map

    // Make sure this happens in char * 
    temp = ptr + bytesPerPixel /2;
    pixel16 = (uint16_t *) temp;

    return *pixel16;
}

// Set the value of a given channel in the pixel pointed by ptr
static void panoStitchPixelMapSet(unsigned char *ptr, int bytesPerPixel, unsigned int value)
{
    uint16_t *ptr16;
    unsigned char *temp;

    // We use 2 channel (Green in 16 bit) and (Green and Blue in 8 bit images) to store the map

    assert(bytesPerPixel == 4 || bytesPerPixel ==8);
    assert(ptr != NULL);

    assert(value >= 0);
    assert(value <= 0xffff);

    temp = ptr + bytesPerPixel /2;
    ptr16 = (uint16_t *) temp;

    *ptr16 = value;
}

// Set the map of a pixel only if it is necessary
static void panoStitchPixelDetermineMap(unsigned char *pixel, int bytesPerPixel,  unsigned int *count)
{

    int alphaChannel;
    unsigned int value = 0;

    assert(bytesPerPixel == 4 || bytesPerPixel ==8);
    assert(pixel != NULL);

    alphaChannel = panoStitchPixelChannelGet(pixel, bytesPerPixel/4, 0);
    
    if (alphaChannel == 0) {
        *count = 0;
    } 
    else {
        (*count)++;
    }
    value = panoStitchPixelMapGet(pixel, bytesPerPixel);
    
    if (value < *count) {
        *count = value;
    }
    else
        panoStitchPixelMapSet(pixel, bytesPerPixel, *count);
    
}



/*
 * This routine creates the stitching mask map
 *
 * Stitching maps contain the same alpha channel than the original image, but instead of pixel data they contiain
 * an "index" of how good that pixel is. Pixels at the center of the image have better indexes than
 * those at the edge
 *
 */
void panoStitchComputeMaskMap(Image * image)
{

    int column;
    int row;
    unsigned char *ptr;
    unsigned char *pixel = NULL;
    unsigned int count;
    int bytesPerPixel = 0;
    unsigned int alphaChannel;


    // determine the type of image
    bytesPerPixel = panoImageBytesPerPixel(image);

    // Use the GreenBlue pixel area is used to keep a counter of the
    // minimum distance (in pixels) away we are from the edges of the
    // mask (horizontal or vertical)

    // The algorithm is fairly simple:

    // For each column
    //   Process each row from top to down
    //     Set each pixel counter to the number of pixels from edge of the mask (from the left)
    //   Process each row from bottom to top
    // Set each pixel counter to the minimum between current counter and number of pixels
    // from edge (from the right)

    // for each row
    //   repeat the same algorithm (done per column)

    for (column = 0; column < panoImageWidth(image); column++) {
        count = 0;
        // Point to the given column in row 0

        ptr = panoImageData(image) + column * bytesPerPixel;

        //    fprintf(stderr, "St1.1 Column[%d]\n", column);
        

        // From top to bottom
        for (row = 0; row < panoImageHeight(image); row++) {

            // Get alpha channel for this point
	    pixel = ptr +  row * panoImageBytesPerLine(image);

            alphaChannel = panoStitchPixelChannelGet(pixel, bytesPerPixel/4, 0);

            if (alphaChannel == 0) {
                count = 0;
            } 
            else {
                count++;
            }
            // In the first pass we ALWAYS set the map because it is uninitialized
            panoStitchPixelMapSet(pixel, bytesPerPixel, count);
        }

        count = 0;
        row = image->height;

        // From bottom to top
        while (--row >= 0) {
            pixel = ptr + row * image->bytesPerLine;

            panoStitchPixelDetermineMap(pixel, bytesPerPixel, &count);

        }                       //while

        //    fprintf(stderr, "St1.5 Column[%d]\n", column);

    }                           //

    ///////////// row by row

    //  fprintf(stderr, "St2\n");

    for (row = 0; row < image->height; row++) {
        count = 0;
        ptr = row * image->bytesPerLine + *(image->data);

        // process from left to right
        for (column = 0; column < image->width; column++) {
            pixel = ptr + panoImageBytesPerPixel(image) * column;

            panoStitchPixelDetermineMap(pixel, bytesPerPixel, &count);
        }                       // for column

        //-----------------------------;;


        //  fprintf(stderr, "St3\n");

        count = 0;
        column = image->width;

        while (--column >= 0) {
            pixel = ptr + panoImageBytesPerPixel(image) * column;

            panoStitchPixelDetermineMap(pixel, bytesPerPixel, &count);
        }
    }                           // end of for row

}


//
// Compute the map of the stitching mask and create a file with it.
// The stitching mask will be contained in the GB channels (this is,
// the 16 bits corresponding to the G and B channel will contain a uint16_t that
// contains, for that particular point, the stitching mask.
//
int panoStitchCreateMaskMapFiles(fullPath * inputFiles, fullPath * maskFiles,
                                    int numberImages)
{
    int index;
    char tempString[512];
    Image image;

    if (ptQuietFlag == 0)
        Progress(_initProgress, "Preparing Stitching Masks");

    // for each image, create merging mask and save to temporal file
    for (index = 0; index < numberImages; index++) {

        sprintf(tempString, "%d", index * 100 / numberImages);

        // Do progress
        if (ptQuietFlag == 0) {
            if (Progress(_setProgress, tempString) == 0) {
                return 0;
            }
        }

        if (panoTiffRead(&image, inputFiles[index].name) == 0) {
            PrintError("Could not read TIFF-file");
            return 0;
        }

        // Compute the stitching mask in-situ
        panoStitchComputeMaskMap(&image);

        strcpy(maskFiles[index].name, inputFiles[0].name);

        if (panoFileMakeTemp(&maskFiles[index]) == 0) {
            PrintError("Could not make Tempfile");
            return -1;
        }

	if (panoTiffWrite(&image, maskFiles[index].name) == 0) {
	    PrintError("Could not write TIFF-file [%s]", maskFiles[index].name);
	    return -1;
	}

        //    fprintf(stderr, "Written to file %s\n", maskFiles[index].name);

	panoImageDispose(&image);

    }                           // for (index...

    // Do progress

    if (!ptQuietFlag)

        Progress(_setProgress, "100");
    Progress(_disposeProgress, tempString);

    return 1;
}

/*
 * This routine takes one given row from numberImages images and tries
 * to set their alpha channel to show the one with the 'best' pixel
 * 
 * TODO: unify this and the 8 bits version
 */
static void panoStitchSetBestAlphaChannel16bits(unsigned char *imagesBuffer,
                                                   int numberImages,
                                                   pano_ImageMetadata * imageParms)
{
    //  fprintf(stderr, "SetBestAlphaChannel16bits not supported yet\n");
    //assert(0); // it should not be here... yet

    unsigned char *pixel;
    uint16_t *ptrCount;
    uint16_t best;
    uint16_t maskValue;
    int column;
    int j;
    int bytesPerLine;

    assert(imageParms->bytesPerPixel == 8);

    bytesPerLine = imageParms->cropInfo.fullWidth * imageParms->bytesPerPixel;

    for (column = 0, pixel = imagesBuffer;
         column < imageParms->cropInfo.fullWidth; column++, pixel += imageParms->bytesPerPixel) {

        best = 0;
        ptrCount = (uint16_t *) (pixel + 2);
        maskValue = *ptrCount;

        // find the image with the highest value

        for (j = 1; j < numberImages; j++) {

            ptrCount = (uint16_t *) (pixel + bytesPerLine * j + 2);

            if (*ptrCount > maskValue) {

                best = j;
                maskValue = *ptrCount;

            }
        }                       // for j

        if (maskValue != 0) {

            // set the mask of the ones above, but not below... interesting...

            for (j = best + 1; j < numberImages; j++) {
                uint16_t *pixel2;

                pixel2 = (uint16_t *) (pixel + bytesPerLine * j);

                if (0 != *pixel2) {
                    *pixel2 = 1;
                }
            }
        }
    }                           // for i


}

/*
 * This routine takes one given row from numberImages images and tries
 * to set their alpha channel to show the one with the 'best' pixel
 * 
 * TODO: unify this and the 16 bits version
 */
static void panoStitchSetBestAlphaChannel8bits(unsigned char *imagesBuffer,
                                                  int numberImages,
                                                  pano_ImageMetadata * imageParms)
{
    unsigned char *pixel;
    uint16_t *ptrCount;
    uint16_t best;
    uint16_t maskValue;
    int column;
    int j;

    int bytesPerLine;

    assert(imageParms->bytesPerPixel == 4);

    bytesPerLine = imageParms->cropInfo.fullWidth * imageParms->bytesPerPixel;

    for (column = 0, pixel = imagesBuffer;
         column < imageParms->cropInfo.fullWidth; 
	 column++, pixel += 4) {

        best = 0;
        ptrCount = (uint16_t *) (pixel + 2);
        maskValue = *ptrCount;

        // find the image with the highest value

        for (j = 1; j < numberImages; j++) {
	    unsigned char *temp;
	  
            temp = (pixel + bytesPerLine * j + 2);

            ptrCount = (uint16_t *) temp;

            if (*ptrCount > maskValue) {

                best = j;
                maskValue = *ptrCount;

            }
        }                       // for j

        if (maskValue != 0) {

            // set the mask of the ones above, but not below... interesting...

            for (j = best + 1; j < numberImages; j++) {
                unsigned char *pixel2;

                pixel2 = pixel + bytesPerLine * j;

                if (0 != *pixel2) {
                    *pixel2 = 1;
                }
            }
        }
    }                           // for i

}




/* 
 * Creates an image with RBG from input but alpha channel from mask
 *
 */
static int panoStitchReplaceAlphaChannel(fullPath * inputImage, fullPath * mask,
                                         fullPath * output)
{
    unsigned char *imageRowBuffer = NULL;
    unsigned char *maskRowBuffer = NULL;
    int row;
    int j;

    int returnValue = 0;
    int numberBytesToCopy;
    unsigned char *source;
    unsigned char *destination;

    pano_Tiff *imageFile  = NULL;
    pano_Tiff *outputFile = NULL;
    pano_Tiff *maskFile   = NULL;

    int jumpBytes;
    int alphaChannelOffset;

    //   For each row
    //     Read row of image
    //     Read row of mask
    //     Replace alpha channel in image with masks alpha channel
    //     Write row
    //
    //Note that all three images involved here (input image, mask image and 
    //resulting image) have the same dimensions, and we don't care what the 
    //dimensions are the for the final output image  

    // FIRST PREPARE THE FILES

    // Open input image   
    if ((imageFile = panoTiffOpen(inputImage->name)) == NULL) {
        PrintError("Could not open TIFF-file");
        returnValue = 0;
        goto end;
    }

    //Allocate line buffers for image and mask
    if ((imageRowBuffer = calloc(panoTiffBytesPerLine(imageFile), 1)) == NULL
        || (maskRowBuffer =
            calloc(panoTiffBytesPerLine(imageFile), 1)) == NULL) {
        PrintError("Not enough memory");
        returnValue = 0;
        goto end;
    }

    // Open mask file
    if ((maskFile = panoTiffOpen(mask->name)) == NULL) {
        PrintError("Could not open mask file");
        returnValue = 0;
        goto end;
    }

    // Create output file
    if ((outputFile =
         panoTiffCreate(output->name, &maskFile->metadata)) == NULL) {
        PrintError("Could not create TIFF-file");
        returnValue = 0;
        goto end;
    }

    // Processing one row at a time
    if (panoTiffBitsPerPixel(imageFile) == 32) {
        jumpBytes = 4;
        alphaChannelOffset = 3;
        numberBytesToCopy = 1;
    }
    else {
        jumpBytes = 8;
        alphaChannelOffset = 6;
        numberBytesToCopy = 2;
    }

    // Process one line at a time.

    for (row = 0; row < panoTiffImageHeight(imageFile); row++) {

        TIFFReadScanline(imageFile->tiff, imageRowBuffer, row, 0);
        TIFFReadScanline(maskFile->tiff, maskRowBuffer, row, 0);

        destination = imageRowBuffer + alphaChannelOffset;
        source = maskRowBuffer + alphaChannelOffset;

        // Copy alpha channel...
        for (j = 0; j < panoTiffImageWidth(imageFile); j++) {
            int k;
            // Copy the mask
            // TODO: use memcpy
            for (k = 0; k < numberBytesToCopy; k++) {
                *(destination + k) = *(source + k);
            }

            destination += jumpBytes;
            source += jumpBytes;
        }

        // Write row to output
        if (TIFFWriteScanline(outputFile->tiff, imageRowBuffer, row, 0) != 1) {
            PrintError
                ("Unable to write data to output file (ReplaceAlphaChannel)\n");
            returnValue = 0;
            goto end;
        }

    }

    returnValue = 1;
  end:

    if(imageFile)
      panoTiffClose(imageFile);

    if(maskFile)
      panoTiffClose(maskFile);

    if(outputFile)
      panoTiffClose(outputFile);

    free(imageRowBuffer);
    free(maskRowBuffer);

    return returnValue;
}


static void panoStitchCalculateAlphaChannel(unsigned char *imagesBuffer,
                                               int numberImages,
                                               pano_ImageMetadata * imageMetadata)
{

    switch (imageMetadata->bitsPerSample) {
    case 8:
        panoStitchSetBestAlphaChannel8bits(imagesBuffer, numberImages, imageMetadata);
        break;
    case 16:
        panoStitchSetBestAlphaChannel16bits(imagesBuffer, numberImages, imageMetadata);
        break;
    default:
        fprintf(stderr,
                "CalculateAlphaChannel not supported for this image type (%d bitsPerPixel)\n",
                imageMetadata->bitsPerPixel);
        exit(1);
    }
}


/*
 * Create the alpha channels for the output images
 *
 */
int panoStitchCreateAlphaChannels(fullPath * masksNames,
                                  fullPath * alphaChannelNames, int numberImages)
{
    pano_Tiff **tiffMasks;
    pano_Tiff **tiffAlphaChannels;
    unsigned char *imagesBuffer = NULL;
    unsigned char *ptrBuffer;
    int index;
    char tempString[24];

    int returnValue = 0;
    int fullSizeRowIndex;

    int fullImageWidth;
    int fullImageHeight;
    int bytesPerLine;
    int bitsPerPixel;

    assert(numberImages > 0);
    assert(masksNames != NULL);
    assert(alphaChannelNames != NULL);

    //printf("CreateAlpha %d\n", numberImages);

    // Allocate arrays of TIFF* for the input and output
    // images. process is one row at a time, with all images
    // processed at the same time
    tiffMasks = calloc(numberImages, sizeof(pano_Tiff));
    tiffAlphaChannels = calloc(numberImages, sizeof(pano_Tiff));

    if (tiffAlphaChannels == NULL || tiffMasks == NULL) {
        PrintError("Not enough memory");
        return 0;
    }

    if (ptQuietFlag == 0)
        Progress(_initProgress, "Calculating Alpha Channel");

    // Alpha Channel calculation    
    // Open for read
    //       mask files
    //  and  input files
    // Open for write  alpha channel files

    // Open up an input image, then create a corresponding output image...repeat for all images
    for (index = 0; index < numberImages; index++) {

        if ((tiffMasks[index] = panoTiffOpen(masksNames[index].name)) == 0) {
            PrintError("Could not open TIFF-file");
            return 0;
        }

        strcpy(alphaChannelNames[index].name, masksNames[0].name);

        if (panoFileMakeTemp(&alphaChannelNames[index]) == 0) {
            PrintError("Could not make Tempfile");
            goto end;
        }

        tiffAlphaChannels[index] =
            panoTiffCreate(alphaChannelNames[index].name,
                           panoTiffImageMetadata(tiffMasks[index]));

        if (tiffAlphaChannels[index] == NULL) {
            PrintError("Could not create TIFF-file");
            goto end;
        }

    }                           // finished opening up output files

    // Get sizes of the entire image
    fullImageWidth = panoTiffFullImageWidth(tiffMasks[0]);
    fullImageHeight = panoTiffFullImageHeight(tiffMasks[0]);
    bitsPerPixel = panoTiffBitsPerPixel(tiffMasks[0]);
    bytesPerLine = fullImageWidth * panoTiffBytesPerPixel(tiffMasks[0]);

    for (index = 0; index < numberImages; index++) {
	assert(fullImageWidth == panoTiffFullImageWidth(tiffMasks[index]));
	assert(fullImageHeight == panoTiffFullImageHeight(tiffMasks[index]));
	assert(bitsPerPixel == panoTiffBitsPerPixel(tiffMasks[index]));
	assert(bytesPerLine == fullImageWidth * panoTiffBytesPerPixel(tiffMasks[index]));
    }

    // just for the sake of it

    // The imagesBuffer contains as many rows as we have input images, and 
    // each row is as wide as the final output image

        //      printf("Fulls ize %d %d bytesPerLine %d bitsPerPixel %d\n", numberImages,
        //                 bytesPerLine,
        //                 bytesPerLine, bitsPerPixel);

    imagesBuffer = calloc(numberImages, bytesPerLine);
    if (imagesBuffer == NULL) {
        PrintError("Not enough memory");
        goto end;
    }

    assert(fullImageWidth > 0 && fullImageHeight > 0 && bytesPerLine > 0
           && bitsPerPixel > 0);
    //  fprintf(stderr, "Files have been created, process each row\n");

    //iterate one row at a time, and for each row process all images

    for (fullSizeRowIndex = 0; fullSizeRowIndex < fullImageHeight;
         fullSizeRowIndex++) {
	
        // Update progress
        if (ptQuietFlag == 0) {
            if (fullSizeRowIndex == (fullSizeRowIndex / 20) * 20) {
                sprintf(tempString, "%lu",
                        (long unsigned) fullSizeRowIndex * 100 /
                        fullImageHeight);
                if (Progress(_setProgress, tempString) == 0) {
                    // If user aborts, end
                    returnValue = 0;
                    goto end;
                }
            }
        }


        // process the current row for all images
        for (ptrBuffer = imagesBuffer, index = 0; index < numberImages;
             index++, ptrBuffer += bytesPerLine) {

            if (!panoTiffReadScanLineFullSize
                (tiffMasks[index], ptrBuffer, fullSizeRowIndex)) {
                PrintError("Error reading temporary TIFF data");
                returnValue = 0;
                goto end;
            }
	    RGBAtoARGB(ptrBuffer, fullImageWidth, bitsPerPixel);

        }
	


        //calculate the alpha channel for this row in all images

	panoStitchCalculateAlphaChannel(imagesBuffer, numberImages,
					panoTiffImageMetadata(tiffMasks[0]));



        //write out the alpha channel data for this row to all output images
        for (index = 0, ptrBuffer = imagesBuffer; index < numberImages;
             index++, ptrBuffer += bytesPerLine) {


	    ARGBtoRGBA(ptrBuffer, fullImageWidth, bitsPerPixel);
            if (!panoTiffWriteScanLineFullSize
                (tiffAlphaChannels[index], ptrBuffer, fullSizeRowIndex)) {
                PrintError
                    ("Unable to write data to output file (CreateAlphaChannel)\n");
                returnValue = 0;
                goto end;
            }
        }


    }                           //for fullSizeRowIndex
    returnValue = 1;

  end:

    if (!ptQuietFlag) {
        Progress(_setProgress, "100");
        Progress(_disposeProgress, "");
    }

    for (index = 0; index < numberImages; index++) {
        if (tiffMasks[index] != NULL)
            panoTiffClose(tiffMasks[index]);
        if (tiffAlphaChannels[index] != NULL)
            panoTiffClose(tiffAlphaChannels[index]);
    }                           // for index.

    free(imagesBuffer);
    free(tiffAlphaChannels);
    free(tiffMasks);

    return returnValue;
}



/**
 * Replaces the alpha channel in each image in inputFiles with a generated
 * mask.  The mask is calculated so as to route the seam between overlapping
 * images through the center of the overlap region...
 */
int panoStitchReplaceMasks(fullPath * inputFiles, fullPath * outputFiles,
                                  int numberImages, int featherSize)
{
    int returnValue = -1; // default to fail
    fullPath *alphaChannelFiles = NULL;
    fullPath *maskFiles         = NULL;
    int i;
    Image image;
    char tempString[512];

    if (numberImages == 0) {
        return 0;
    }


    SetImageDefaults(&image);

    maskFiles = calloc(numberImages, sizeof(fullPath));
    alphaChannelFiles = calloc(numberImages, sizeof(fullPath));


    if (maskFiles == NULL || alphaChannelFiles == NULL) {
        PrintError("Not enough memory");
        goto end;
    }

    // CREATE stitching maps
    if (!panoStitchCreateMaskMapFiles(inputFiles, maskFiles, numberImages)) {
        PrintError("Could not create the stitching masks");
        goto end;
    }

    if (!panoStitchCreateAlphaChannels(maskFiles, alphaChannelFiles, numberImages)) {
        PrintError("Could not create alpha channels");
        goto end;
    }

    // From this point on we do not need to process all files at once. This will save temporary disk space

    for (i = 0; i < numberImages; i++) {
        fullPath withAlphaChannel;

        sprintf(tempString, "%d", 100 * i / numberImages);

        if (ptQuietFlag == 0) {
            if (Progress(_setProgress, tempString) == 0) {
                // We have to delete any temp file
                goto end;
            }
        }

        // We no longer need the mask files
        remove(maskFiles[i].name);

        // Reuse the temporary name
        memcpy(&withAlphaChannel, &maskFiles[i], sizeof(fullPath));

        // Replace the alpha channel of the input image
        if (!panoStitchReplaceAlphaChannel
            (&inputFiles[i], &alphaChannelFiles[i], &withAlphaChannel)) {
            PrintError("Unable to replace alpha channel in image %d", i);
            goto end;
        }
        // we no longer need the alpha channel
        remove(alphaChannelFiles[i].name);


        // Do feathering
        if (featherSize > 0) {

            fullPath feathered;

            memcpy(&feathered, &maskFiles[i], sizeof(fullPath));

            if (!panoFeatherFile(&withAlphaChannel, &feathered, featherSize)) {
                PrintError("Unable to apply feather to image %d", i);
                goto end;
            }

	    if (strcmp(withAlphaChannel.name, feathered.name) != 0) {
	      remove(withAlphaChannel.name);
	    }
            rename(feathered.name, outputFiles[i].name);
        }
        else {
	  rename(withAlphaChannel.name, outputFiles[i].name);

        }
    }
    returnValue = 0; //success

end:
    free(maskFiles);
    free(alphaChannelFiles);

    return returnValue;

}

/// BLENDING ROUTINES

static void panoStitchBlendLayers8Bit(unsigned char **imageDataBuffers, int counterImageFiles,
                                      unsigned char *resultBuffer, int lines, int imageWidth,
                                      int scanLineSize)
{

    // 0x8(%ebp)    imageDataBuffers
    // 0xc(%ebp)    counterImageFiles
    // 0x10(%ebp)   resultBuffer
    // 0x14(%ebp)   lines
    // 0x18(%ebp)   imageWidth
    // 0x1c(%ebp)   scanLineSize

    // 0xffffffdc(%ebp)  imageIndex
    // 0xffffffe0(%ebp)  alphaChannel
    // 0xffffffe4(%ebp)  blue
    // 0xffffffe8(%ebp)  green
    // 0xffffffec(%ebp)  red
    // 0xfffffff0(%ebp)  rowOffset
    // 0xfffffff4(%ebp)  pixelOffset
    // 0xfffffff8(%ebp)  currentLine
    // 0xfffffffc(%ebp)  currentColumn

    int imageIndex = 0;
    unsigned int colours[3];
    unsigned int alphaChannel;
    unsigned int currentLine;
    unsigned int currentColumn;
    unsigned int rowOffset;

    currentLine = 0;

    for (currentLine = 0; (int)currentLine < lines; currentLine++) {

        //printf("Currnet line %d\n", currentLine);

        rowOffset = scanLineSize * currentLine;

        for (currentColumn = 0; (int) currentColumn < imageWidth; currentColumn++) {

            unsigned int pixelOffset;
            unsigned int i;

            //      printf("Currnet column %d\n", currentColumn);

            pixelOffset = rowOffset + currentColumn * 4;


            // Initialize colours for this pixel
            alphaChannel = 0;
            for (i = 0; i < 3; i++)
                colours[i] = 0;


            // Do alpha blending, from top to bottom. Bail out when alpha channel is equal to maximum

            for (imageIndex = counterImageFiles - 1; imageIndex >= 0;
                 imageIndex--) {

                unsigned int alphaContribution;
                unsigned char *ptrPixel;
                unsigned int bottomAlpha;
                unsigned int index;


                // printf("Currnet image %d\n", imageIndex);


                // The alpha blending algorithm is (for premultiplied values)

                // C_result = C_above + C_below * (1 - alpha_above)
                // A_result = Alpha_above + alpha_below * (1 - alpha_above)

                // Find pixel in this layer
                ptrPixel = imageDataBuffers[imageIndex] + pixelOffset;


                // printf("TO read pixel\n");

                bottomAlpha = *(ptrPixel + 3);  // this should be the value of the mask for this particular pixel

                // printf("After read pixel\n");

                alphaContribution =
                    ((0xff - alphaChannel) * bottomAlpha) / 0xff;

                // I don't really think this step is necessary, but due to innestability of the calculation
                // alphaContribution it might overflow the byte valuex

                if (alphaChannel + alphaContribution > 0xff) {
                    alphaContribution = 0xff - alphaChannel;
                }

                alphaChannel += alphaContribution;

                // Makek sure the alpha channel is within range
                assert(alphaChannel >= 0 && alphaChannel <= 0xff);

                // now do the colours

                // printf("TO set pixel\n");

                for (index = 0; index < 3; index++) {
                    colours[index] += (*(ptrPixel + index) * alphaContribution) / 0xff; // 

                    if (!(colours[index] >= 0 && colours[index] <= 0xff)) {
                        printf("PPPPPPPPPPPPPPPPPanic %d index [%d]\n",
                               colours[index], index);
                    }
                    assert(colours[index] >= 0 && colours[index] <= 0xff);
                }

                // We don't need to continue if the alpha channel is at the max
                if (alphaChannel >= 0xff)
                    break;

            }                   // for (imageIndex =counterImageFiles-1; imageIndex >= 0; imageIndex--) {

            // Is it really necessary to check the values of the colours and alphachannel to make
            // sure they are not overflowing a byte?

            // Set the value of the pixel
            for (i = 0; i < 3; i++) {
                assert(colours[i] <= 0xff && colours[i] >= 0);
                *(resultBuffer + pixelOffset + i) = colours[i];
            }

            *(resultBuffer + pixelOffset + 3) = alphaChannel;


        }                       //(currentColumn < imageWidth)

    }                           //for currentLine < lines

}

static void panoStitchBlendLayers16Bit(unsigned char **imageDataBuffers, int counterImageFiles,
                                unsigned char *resultBuffer, int lines, int imageWidth,
                                int scanLineSize)
{

    int imageIndex = 0;
    unsigned long long colours[3];
    unsigned long long alphaChannel;
    unsigned int currentLine;
    unsigned int currentColumn;
    unsigned int rowOffset;

    uint16_t *u16ResultBuffer = (uint16_t *) resultBuffer;
    uint16_t **u16ImageDataBuffers = (uint16 **) imageDataBuffers;

    currentLine = 0;


    for (currentLine = 0; (int) currentLine < lines; currentLine++) {

        //  printf("Lines %d\n", lines);
        //  printf("Width %d\n", imageWidth);
        //  printf("length %d\n", scanLineSize);


        //printf("Currnet line %d\n", currentLine);

        // scanLineSize is in bytes, but we need the length in 16bit units
        rowOffset = (scanLineSize / 2) * currentLine;

        for (currentColumn = 0; (int) currentColumn < imageWidth; currentColumn++) {

            unsigned int pixelOffset;
            unsigned int i;

            //      printf("Currnet column %d\n", currentColumn);

            pixelOffset = rowOffset + currentColumn * 4;

            //      printf("Currnet offset %d\n", pixelOffset);

            // Initialize colours for this pixel
            alphaChannel = 0;
            for (i = 0; i < 3; i++)
                colours[i] = 0;


            // Do alpha blending, from top to bottom. Bail out when alpha channel is equal to maximum

            for (imageIndex = counterImageFiles - 1; imageIndex >= 0;
                 imageIndex--) {

                unsigned long long alphaContribution;
                uint16_t *ptrPixel;
                unsigned long long bottomAlpha;
                unsigned int index;


                // printf("Currnet image %d\n", imageIndex);


                // The alpha blending algorithm is (for premultiplied values)

                // C_result = C_above + C_below * (1 - alpha_above)
                // A_result = Alpha_above + alpha_below * (1 - alpha_above)

                // Find pixel in this layer
                ptrPixel = u16ImageDataBuffers[imageIndex] + pixelOffset;


                // printf("TO read pixel\n");

                bottomAlpha = *(ptrPixel + 3);  // this should be the value of the mask for this particular pixel

                //printf("After read pixel\n");

                alphaContribution =
                    ((0xffff - alphaChannel) * bottomAlpha) / 0xffff;

                // I don't really think this step is necessary, but due to innestability of the calculation
                // alphaContribution it might overflow the byte valuex

                if (alphaChannel + alphaContribution > 0xffff) {
                    alphaContribution = 0xffff - alphaChannel;
                }

                alphaChannel += alphaContribution;

                // Makek sure the alpha channel is within range
                assert(alphaChannel >= 0 && alphaChannel <= 0xffff);

                // now do the colours

                //printf("TO set pixel\n");

                for (index = 0; index < 3; index++) {
                    colours[index] += (*(ptrPixel + index) * alphaContribution) / 0xffff;       // 
                    if (!(colours[index] >= 0 && colours[index] <= 0xffff)) {
                        printf("PPPPPPPPPPPPPPPPPanic %lld index [%d]\n",
                               colours[index], index);
                    }
                    assert(colours[index] >= 0 && colours[index] <= 0xffff);
                }

                // We don't need to continue if the alpha channel is at the max
                if (alphaChannel >= 0xffff)
                    break;

            }                   // for (imageIndex =counterImageFiles-1; imageIndex >= 0; imageIndex--) {

            // Is it really necessary to check the values of the colours and alphachannel to make
            // sure they are not overflowing a byte?
            //      printf("Done loop\n");      
            // Set the value of the pixel
            for (i = 0; i < 3; i++) {
                assert(colours[i] <= 0xffff && colours[i] >= 0);
                *(u16ResultBuffer + pixelOffset + i) = (uint16_t)(colours[i]);
            }
            //      printf("Done loop 2\n");      
            *(u16ResultBuffer + pixelOffset + 3) = (uint16_t)(alphaChannel);


        }                       //(currentColumn < imageWidth)

    }                           //for currentLine < lines

}

/*
 * TODO: BLend 8 and 16 versions into one
 * Blend all the images into one
 */
void panoStitchBlendLayers(unsigned char **imageDataBuffers,
                           unsigned int counterImageFiles,
                           unsigned char *resultBuffer, 
                           unsigned int linesToRead,
                           int imageWidth,
                           unsigned int bitsPerPixel,
                           unsigned int scanLineSize)
{

    if (bitsPerPixel == 32) {
        panoStitchBlendLayers8Bit(imageDataBuffers, counterImageFiles, resultBuffer,
                        linesToRead, imageWidth, scanLineSize);
    }
    else if (bitsPerPixel == 64) {
        panoStitchBlendLayers16Bit(imageDataBuffers, counterImageFiles, resultBuffer,
                                   linesToRead, imageWidth, scanLineSize);
    }
}
