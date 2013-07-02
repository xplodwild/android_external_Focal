#include <stdio.h>
//#include <jpeglib.h>
//#define __NO_SYSTEM__
#include "filter.h"
#include "metadata.h"
#include "file.h"

#include "jpegicc.h"


int writeJPEG(Image * im, fullPath * sfile, int quality, int progressive)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *outfile;
    char filename[512];
    int scanlines_written;
    unsigned char *data, *buf;

#ifdef __Mac__
    unsigned char the_pcUnixFilePath[512];      // added by Kekus Digital
    Str255 the_cString;
    Boolean the_bReturnValue;
    CFStringRef the_FilePath;
    CFURLRef the_Url;           //till here
#endif

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if (GetFullPath(sfile, filename))
        return -1;

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

    if ((outfile = fopen(filename, "wb")) == NULL)
    {
        PrintError("can't open %s", filename);
        return -1;
    }
    TwoToOneByte(im);

    jpeg_stdio_dest(&cinfo, outfile);



    cinfo.image_width = im->width;      /* image width and height, in pixels */
    cinfo.image_height = im->height;
    cinfo.input_components = 3; /* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;     /* colorspace of input image */

    jpeg_set_defaults(&cinfo);

    if (2 != (progressive & 2))
        cinfo.optimize_coding = TRUE;

    jpeg_set_quality(&cinfo, quality, TRUE);

    if (1 == (progressive & 1))
        jpeg_simple_progression(&cinfo);


    jpeg_start_compress(&cinfo, TRUE);

    // Write ICC Profile if it exists

    if (im->metadata.iccProfile.size > 0) {
        jpegICCWriteProfile(&cinfo, 
                           (JOCTET *)im->metadata.iccProfile.data,
                           im->metadata.iccProfile.size);
    }

    scanlines_written = 0;
    data = *(im->data);
    buf = (unsigned char *) malloc((size_t) im->bytesPerLine);
    if (buf == NULL)
    {

        PrintError("Not enough memory");
        fclose(outfile);
        return -1;
    }


    while (scanlines_written < im->height)
    {
        memcpy(buf, data, (size_t) im->bytesPerLine);
        if (im->bitsPerPixel == 32)     // Convert 4->3 samples
        {
            int x;
            unsigned char *c1 = buf, *c2 = buf;
            for (x = 0; x < im->width; x++)
            {
                c2++;
                *c1++ = *c2++;
                *c1++ = *c2++;
                *c1++ = *c2++;
            }

        }

        if (jpeg_write_scanlines(&cinfo, (JSAMPARRAY) & buf, 1))
        {
            scanlines_written++;
            data += im->bytesPerLine;
        }
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    free(buf);
    return 0;


}


int readJPEG(Image * im, fullPath * sfile)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile;
    char filename[256];
    int scan_lines_to_be_read, scan_lines_read;
    unsigned int i, scanheight;
    unsigned char *data;
    JSAMPARRAY sarray;
    JOCTET *ptr = NULL;
    unsigned int size  = 0; 



#ifdef __Mac__
    unsigned char the_pcUnixFilePath[256];      // added by Kekus Digital
    Str255 the_cString;
    Boolean the_bReturnValue;
    CFStringRef the_FilePath;
    CFURLRef the_Url;           //till here
#endif

    //PrintError("%s", sfile->name);        

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // Prepare the library for reading the ICC profile, see its source code
    // for further information
    jpegICCSetupReadICCProfile(&cinfo);

    if (GetFullPath(sfile, filename))
        return -1;

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
                                         256);

    strcpy(filename, the_pcUnixFilePath);       //till here
#endif

    if ((infile = fopen(filename, "rb")) == NULL) {
        PrintError("can't open %s", filename);
        return -1;
    }

    jpeg_stdio_src(&cinfo, infile);

    jpeg_read_header(&cinfo, TRUE);

    jpeg_start_decompress(&cinfo);

    SetImageDefaults(im);
    im->width = cinfo.output_width;
    im->height = cinfo.output_height;
    if (cinfo.output_components != 3)
    {
        PrintError("Image must be rgb");
        fclose(infile);
        return -1;
    }


    im->bitsPerPixel = 24;
    im->bytesPerLine = im->width * 3;
    im->dataSize = im->width * 4 * im->height;
    im->data = (unsigned char **) mymalloc((size_t) im->dataSize);
    if (im->data == NULL)
    {
        PrintError("Not enough memory");
        fclose(infile);
        return -1;
    }

    scanheight = cinfo.rec_outbuf_height;
    sarray = (JSAMPARRAY) malloc(scanheight * sizeof(JSAMPROW));

    scan_lines_to_be_read = im->height;
    data = *(im->data);

    while (scan_lines_to_be_read)
    {
        for (i = 0; i < scanheight; i++)
        {
            sarray[i] = (JSAMPROW) (data + i * im->bytesPerLine);
        }

        scan_lines_read = jpeg_read_scanlines(&cinfo, sarray, scanheight);

        scan_lines_to_be_read -= scan_lines_read;
        data += scan_lines_read * im->bytesPerLine;
    }

    // read ICC profile 
    
    if (jpegICCReadProfile(&cinfo, &ptr, &size)) {
        
        im->metadata.iccProfile.size = size;        
        im->metadata.iccProfile.data = (char*) ptr;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    ThreeToFourBPP(im);
    free(sarray);


    fclose(infile);

    return 0;


}

// this function is very likely to change
// so it should be considered a hack until 
// we are able to read all the JPEG metadata

int panoJPEGRead(Image * im, fullPath * sfile)
{
  if ( readJPEG(im, sfile) == 0) {
      return panoMetadataUpdateFromImage(im);
  } else
      return FALSE;


}
