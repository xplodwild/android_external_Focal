#include <stdio.h>
#include "rgbe.h"
#include "filter.h"
#include "metadata.h"



int writeHDR( Image *im, fullPath *sfile)
{
	rgbe_header_info rgbe_h;
	FILE * outfile;
	char filename[512];
	unsigned char *data;
	float *fdata = NULL;
	int i;


#ifdef __Mac__
	unsigned char the_pcUnixFilePath[512];// added by Kekus Digital
	Str255 the_cString;
	Boolean the_bReturnValue;
	CFStringRef the_FilePath;
	CFURLRef the_Url;//till here
#endif
	
	if( GetFullPath (sfile, filename))
		return -1;

#ifdef __Mac__
	CopyCStringToPascal(filename,the_cString);//Added by Kekus Digital
	the_FilePath = CFStringCreateWithPascalString(kCFAllocatorDefault, the_cString, kCFStringEncodingUTF8);
	the_Url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, the_FilePath, kCFURLHFSPathStyle, false);
	the_bReturnValue = CFURLGetFileSystemRepresentation(the_Url, true, the_pcUnixFilePath, 512);

	strcpy(filename, the_pcUnixFilePath);//till here
#endif
	
	data=malloc(im->width * im->height * 4 * 3);

	if (im->bitsPerPixel==128) {
		float *srcdata;
		fdata=(float *)data;
		srcdata=(float *)*im->data;
		for(i=0;i<(im->width * im->height); i++) {
			srcdata++; // skip alpha
			*fdata++=*srcdata++;
			*fdata++=*srcdata++;
			*fdata++=*srcdata++;
		}
		fdata=(float *)data;
	}
	if (im->bitsPerPixel==96) {
		fdata=(float *)*im->data;
	}
	if ((im->bitsPerPixel==64) || (im->bitsPerPixel==48)) {
   		double gnorm;
		unsigned short *srcdata;
		fdata=(float *)data;
		srcdata=(unsigned short *)*im->data;
	
		gnorm = (1 / pow( 65535.0 , 2.2 )); 
		for(i=0;i<(im->width * im->height); i++) {
			if (im->bitsPerPixel==64) srcdata++; // skip alpha
			*fdata++=(float)(pow((double)*srcdata++,2.2) * gnorm);
			*fdata++=(float)(pow((double)*srcdata++,2.2) * gnorm);
			*fdata++=(float)(pow((double)*srcdata++,2.2) * gnorm);
//			*fdata++=pow((double)*sdata++ * (1.0/65535.0);
		}
		fdata=(float *)data;
	}
	if ((im->bitsPerPixel==32) || (im->bitsPerPixel==24)) {
   		double gnorm;
		unsigned char *srcdata;
		fdata=(float *)data;
		srcdata=(unsigned char *)*im->data;
		gnorm = (1 / pow( 255.0 , 2.2 )); 
		for(i=0;i<(im->width * im->height); i++) {
			if (im->bitsPerPixel==32) srcdata++; // skip alpha
			*fdata++=(float)(pow((double)*srcdata++,2.2) * gnorm);
			*fdata++=(float)(pow((double)*srcdata++,2.2) * gnorm);
			*fdata++=(float)(pow((double)*srcdata++,2.2) * gnorm);
//			*fdata++=(*cdata++) * (1.0/255.0);
		}
		fdata=(float *)data;
	}
	if ((outfile = fopen(filename, "wb")) == NULL) 
	{
	    PrintError("can't open %s", filename);
      free( data );
	    return -1;
	}
	
	rgbe_h.valid=-1;
	strcpy(rgbe_h.programtype,"RADIANCE");
	rgbe_h.gamma=1.0;   
	rgbe_h.exposure=1.0;   

	RGBE_WriteHeader(outfile, im->width,im->height, &rgbe_h);
	RGBE_WritePixels(outfile,fdata, im->width * im->height );

	fclose( outfile );
	free( data );
	return 0;
}


int readHDR ( Image *im, fullPath *sfile )
{
	rgbe_header_info rgbe_h;
	FILE * infile;
	float *srcdata, *fdata;
	char filename[256];
	int i;

#ifdef __Mac__
	unsigned char the_pcUnixFilePath[256];// added by Kekus Digital
	Str255 the_cString;
	Boolean the_bReturnValue;
	CFStringRef the_FilePath;
	CFURLRef the_Url;//till here
#endif

	if( GetFullPath (sfile, filename))
		return -1;

#ifdef __Mac__
	CopyCStringToPascal(filename,the_cString);//Added by Kekus Digital
	the_FilePath = CFStringCreateWithPascalString(kCFAllocatorDefault, the_cString, kCFStringEncodingUTF8);
	the_Url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, the_FilePath, kCFURLHFSPathStyle, false);
	the_bReturnValue = CFURLGetFileSystemRepresentation(the_Url, true, the_pcUnixFilePath, 256);

	strcpy(filename, the_pcUnixFilePath);//till here
#endif

	if ((infile = fopen(filename, "rb")) == NULL) 
	{
	    PrintError("can't open %s", filename);
	    return -1;
	}
	SetImageDefaults( im );
	RGBE_ReadHeader(infile,&im->width,&im->height,&rgbe_h);

	im->bitsPerPixel = 96;
	im->bytesPerLine = im->width * 4 * 4;
	im->dataSize = im->bytesPerLine * im->height;
	im->data = (unsigned char**)mymalloc( (size_t)im->dataSize );
	if( im->data == NULL )
	{
		PrintError("Not enough memory");
		fclose( infile );
		return -1;
	}
	RGBE_ReadPixels_RLE(infile,(float *)*im->data, im->width, im->height);

	fdata=(float *)*im->data;
	srcdata=(float *)*im->data;
	fdata+=(im->width * im->height)*4;
	srcdata+=(im->width * im->height)*3;
	for(i=0;i<(im->width * im->height); i++) {
		*(--fdata)=*(--srcdata);
		*(--fdata)=*(--srcdata);
		*(--fdata)=*(--srcdata);

		*(--fdata)=1.0; // alpha
	}
	im->bitsPerPixel = 128;
	
	fclose( infile );

	return 0;
}

int panoHDRRead(Image *im, fullPath *sfile )
{
    if (readHDR(im, sfile) == 0) {
	return panoMetadataUpdateFromImage(im);
    } else
	return FALSE;
}
