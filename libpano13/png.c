#include "filter.h"
#include "png.h"
#include "file.h"
#include "metadata.h"

int writePNG( Image *im, fullPath *sfile )
{
	FILE * outfile;
   	png_structp png_ptr;
   	png_infop info_ptr;
	char filename[512];
	png_bytep *row_pointers;
	int row;

#ifdef __Mac__
	unsigned char the_pcUnixFilePath[512];//added by Kekus Digital
	Str255 the_cString;
	Boolean the_bReturnValue;
	CFStringRef the_FilePath;
	CFURLRef the_Url;//till here
#endif

	if( GetFullPath (sfile, filename))
		return -1;

#ifdef __Mac__
	CopyCStringToPascal(filename,the_cString);//added by Kekus Digital
	the_FilePath = CFStringCreateWithPascalString(kCFAllocatorDefault, the_cString, kCFStringEncodingUTF8);
	the_Url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, the_FilePath, kCFURLHFSPathStyle, false);
	the_bReturnValue = CFURLGetFileSystemRepresentation(the_Url, true, the_pcUnixFilePath, 512);

	strcpy(filename, the_pcUnixFilePath);//till here
#endif
	
	if ((outfile = fopen(filename, "wb")) == NULL) 
	{
	    PrintError("can't open %s", filename);
	    return -1;
	}
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      (void *)NULL, NULL, NULL);

  	if (!png_ptr)
   	{
      fclose(outfile);
      return -1;
  	 }

   	info_ptr = png_create_info_struct(png_ptr);
   	if (!info_ptr)
   	{
      fclose(outfile);
      png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
      return -1;
   	}

  	/* set error handling */
   	if (setjmp(png_jmpbuf(png_ptr)))
   	{
      /* If we get here, we had a problem reading the file */
      fclose(outfile);
      png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
      return -1;
 	}

   	/* set up the output control if you are using standard C streams */
   	png_init_io(png_ptr, outfile);
	
	FourToThreeBPP( im );
	png_set_IHDR(png_ptr, info_ptr, im->width, im->height,
		     (im->bitsPerPixel > 32 ?  16 : 8), PNG_COLOR_TYPE_RGB,
		     0, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

#if 0
	info_ptr->channels		= (png_byte)(im->bitsPerPixel / info_ptr->bit_depth);
	info_ptr->pixel_depth	= (png_byte)(im->bitsPerPixel);
	info_ptr->rowbytes		= im->bytesPerLine;
#endif

  	png_write_info(png_ptr, info_ptr);

	/* the easiest way to read the image */
	row_pointers = (png_bytep*) malloc( im->height * sizeof(png_bytep) );
	if( row_pointers == NULL ) return -1;
	

   	for (row = 0; row < im->height; row++)
   	{
     	row_pointers[row] = (png_bytep)(*im->data) + row * im->bytesPerLine;
   	}
   

   	png_write_image(png_ptr, row_pointers);
   	png_write_end(png_ptr, info_ptr);

 
   	/* clean up after the write, and free any memory allocated */
   	png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
	
	free( row_pointers );
   	/* close the file */
   	fclose(outfile);

   /* that's it */
   	return 0;
}





	

int readPNG	( Image *im, fullPath *sfile )
{
	FILE * infile;
	char filename[256];
   	png_structp png_ptr;
   	png_infop info_ptr;
	png_bytep *row_pointers;
	int row;
	unsigned long  dataSize;
	int color_type;

#ifdef __Mac__
	unsigned char the_pcUnixFilePath[256];//added by Kekus Digital
	Str255 the_cString;
	Boolean the_bReturnValue;
	CFStringRef the_FilePath;
	CFURLRef the_Url;//till here
#endif

	if( GetFullPath (sfile, filename))
		return -1;

#ifdef __Mac__
	CopyCStringToPascal(filename,the_cString);//added by Kekus Digital
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

   	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      	NULL, NULL, NULL);

   	if (!png_ptr)
   	{
      	fclose(infile);
     	return-1;
   	}
   
   	info_ptr = png_create_info_struct(png_ptr);
   	if (!info_ptr)
   	{
      	fclose(infile);
      	png_destroy_read_struct(&png_ptr,  (png_infopp)NULL, (png_infopp)NULL);
     	return -1;
   	}

   	/* set error handling if you are using the setjmp/longjmp method */
   	if (setjmp(png_jmpbuf(png_ptr)))
   	{
      	/* Free all of the memory associated with the png_ptr and info_ptr */
      	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      	fclose(infile);
		PrintError("Could not read png file");
      	/* If we get here, we had a problem reading the file */
      	return -1;
   	}

   	/* set up the input control if you are using standard C streams */
   	png_init_io(png_ptr, infile);


   	/* read the file information */
  	 png_read_info(png_ptr, info_ptr);

	color_type = png_get_color_type(png_ptr, info_ptr);
	if( color_type != PNG_COLOR_TYPE_RGB &&
		color_type != PNG_COLOR_TYPE_PALETTE &&
		color_type != PNG_COLOR_TYPE_RGB_ALPHA)
	{
		PrintError(" Only rgb images  supported");
      	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose( infile );
		return -1;
	}

	 
	    /* expand paletted colors into true RGB triplets */
   	if (color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_expand(png_ptr);


	SetImageDefaults( im );
	
	im->width 		= png_get_image_width(png_ptr, info_ptr);
	im->height 		= png_get_image_height(png_ptr, info_ptr);
	im->bytesPerLine	= png_get_rowbytes(png_ptr, info_ptr);
	im->dataSize		= im->height * im->bytesPerLine;
	im->bitsPerPixel	= 8 * im->bytesPerLine / im->width;

	im->data = (unsigned char**)mymalloc( (dataSize > im->dataSize ? dataSize : im->dataSize) );
	if( im->data == NULL ){
		PrintError("Not enough memory");
      		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose( infile );
		return -1;
	}
	
	   /* the easiest way to read the image */
	row_pointers = (png_bytep*) malloc( im->height * sizeof(png_bytep) );
	if( row_pointers == NULL ) return -1;
	

   	for (row = 0; row < im->height; row++){
     		row_pointers[row] = (png_bytep)(*im->data + row * im->bytesPerLine);
   	}

   	png_read_image(png_ptr, row_pointers);
	
	if(im->bitsPerPixel == 32){ // rgba; switch to argb 
		uint8_t pix, *p;		
	 	LOOP_IMAGE( im, p = (uint8_t*)idata; pix=p[0]; p[0]=p[3];p[3]=p[2];p[2]=p[1];p[1]=pix; )		
	}
	if(im->bitsPerPixel == 64){ // rgba; switch to argb 
		uint16_t pix, *p;		
	 	LOOP_IMAGE( im, p = (uint16_t*)idata; pix=p[0]; p[0]=p[3];p[3]=p[2];p[2]=p[1];p[1]=pix; )		
	}

#ifndef PT_BIGENDIAN
	// Swap bytes in shorts
	if(im->bitsPerPixel == 48){ 
		uint8_t  b,*id;

// the original construct of *id = *(++id) results in:
//.L80:
//	movb    (%edx), %al
// 	incl    %ecx
//	movb    %al, 1(%edx)
//	movb    2(%edx), %al
//	movb    %al, 3(%edx)
//	movb    4(%edx), %al
//	movb    %al, 5(%edx)
//	movb    6(%edx), %al
//	movb    %al, 7(%edx)
//	addl    %edi, %edx
//	movl    8(%ebp), %eax
//	cmpl    %ecx, (%eax)
//	jg  .L80
//
// when compiled with -O2 which is incorrect since it does not swap the bytes. 
// Force explicit ordering with *id=*(id+1); id++ for correct output

	 	LOOP_IMAGE( im, id = idata; \
				b = *id; *id = *(id + 1); id++; *(id++)=b; \
				b = *id; *id = *(id + 1); id++; *(id++)=b; \
				b = *id; *id = *(id + 1); id++; *(id)=b; )
		}
	if(im->bitsPerPixel == 64){ 
		uint8_t  b,*id;	
	 	LOOP_IMAGE( im, id = idata; \
				b = *id; *id = *(id + 1); id++; *(id++)=b; \
				b = *id; *id = *(id + 1); id++; *(id++)=b; \
				b = *id; *id = *(id + 1); id++; *(id++)=b; \
				b = *id; *id = *(id + 1); id++; *(id)=b; )
	}
	
#endif	
	ThreeToFourBPP( im );

	
	   /* read the rest of the file, getting any additional chunks in info_ptr */
   	png_read_end(png_ptr, info_ptr);

   	/* clean up after the read, and free any memory allocated */
   	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	
	free(row_pointers);
   	/* close the file */
   	fclose(infile);
	

   	/* that's it */
   	return 0;
}



int panoPNGRead(Image *im, fullPath *sfile )
{
    if (readPNG(im, sfile) == 0) {
	return panoMetadataUpdateFromImage(im);
    } else
	return FALSE;
}
