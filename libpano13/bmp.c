#include "filter.h"

#ifdef __Ansi__
  #include "sys_ansi.h"
#elif defined (__Win__)
  #include "sys_win.h"
#endif

#include "metadata.h"
#include "file.h"

static int readBMPFileHeader(Image *im, file_spec input);


#pragma pack(push, 1) 
// Bitmap file header
typedef struct tagwin3xHead{
	short ImageFileType;						// Always 4D42h ("BM")
	long  FileSize;								// Physical size of file in bytes
	short Reserved1;							// Always 0
	short Reserved2;							// Always 0
	long  ImageDataOffset;						// Start of image data offset in bytes
} win3xHead;

typedef struct tagwin3xBitmapInfoHeader{
	long  HeaderSize;							// Size of this header
	long  ImageWidth;							// Image width in pixels
	long  ImageHeight;							// Image height in pixels
	short NumberOfImagePlanes;					// Number of planes (Always 1)
	short BitsPerPixel;							// Bits per pixel (1,4, 8 or 24)
	long  CompressionMethod;					// Compression method (0, 1 or 2)

	long  SizeOfBitmap;							// Size of bitmap in bytes
	long  HorzResolution;						// Horizontal resolution in pixels / meter
	long  VertResolution;						// Vertical resolution in pixels / meter
	long  NumColorsUsed;						// Number of colors in image
	long  NumSignificantColors;					// Number of important colors in palette
} win3xBitmapInfoHeader;
#pragma pack(pop)



int writeBMP( Image *im, fullPath *sfile )
{
	file_spec  output;
	unsigned char *buf, *data;
	long count;
	int scanLength;
	int y;
	win3xHead head;							// First part of bitmap header
	win3xBitmapInfoHeader ihead;			// Second part of bitmap header

	//In BMP files, each line MUST have n bytes, such that n is divisible by 4.
	//If the image doesn't have n divisible by 4, it will contain non-valid
	//information. For example if an image has 1 pixel per line in 24-bit color
	//depth, each line will have 1 additional byte that must be ignored when
	//reading the bitmap.

	//scanlength must be divisble by four (adding 3, dividing by 4 and then multiplying does this).
	scanLength = ((im->width * 3) + 3)/4;
	scanLength *= 4;

	// Build header structures

	head.ImageFileType=0x4D42;				// Always 4D42h ("BM")
	head.FileSize = scanLength * im->height+54;		// Physical size of file in bytes
	head.Reserved1=0;						// Always 0
	head.Reserved2=0;						// Always 0
	head.ImageDataOffset= 54;				// Start of image data offset in bytes

	ihead.HeaderSize=40;					// Size of this header
	ihead.ImageWidth=im->width;				// Image width in pixels
	ihead.ImageHeight=im->height;			// Image height in pixels
	ihead.NumberOfImagePlanes=1;			// Number of planes (Always 1)
	ihead.BitsPerPixel=24;					// Bits per pixel (24 bit color)
	ihead.CompressionMethod=0;				// Compression method (0=uncompressed)

	ihead.SizeOfBitmap = head.FileSize-54;	// Size of bitmap in bytes
	ihead.HorzResolution=7085;				// Horizontal resolution in pixels / meter
	ihead.VertResolution=7085;				// Vertical resolution in pixels / meter
	ihead.NumColorsUsed =0;					// Number of colors in image
	ihead.NumSignificantColors=0;			// Number of important colors in palette

	if( myopen( sfile, write_bin, output )	)
	{
		PrintError("writeBMP, could not open file"); 
		return -1;
	}
	
	count = sizeof( head );
	mywrite( output, count, &head );
	if( count != sizeof( head ) )
	{
		PrintError("writeBMP, could not write header"); 
		return -1;
	}
	

	count = sizeof( ihead );
	mywrite( output, count, &ihead );
	if( count != sizeof( ihead ) )
	{
		PrintError("writeBMP, could not write header"); 
		return -1;
	}

	buf = (unsigned char*)malloc( im->bytesPerLine + 1); // Worst case
	if( buf == NULL )
	{
		PrintError( "Not enough memory" );
		return -1;
	}

	// Write image in reverse order, also change rgb->bgr	
	
	data = *(im->data) + ((im->height-1) * im->bytesPerLine);
	for(y=0; y<im->height; y++)
	{
		if( im->bitsPerPixel == 32 )	// Convert 4->3 samples
		{
			int x;
			unsigned char *c1=buf, *c2=data;
			for(x=0; x < im->width; x++)
			{
				*c1++ = c2[3];
				*c1++ = c2[2];
				*c1++ = c2[1];
				c2 += 4;
			}
		}
		else
		{
			int x;
			unsigned char *c1=buf, *c2=data;
			for(x=0; x < im->width; x++)
			{
				*c1++ = c2[2];
				*c1++ = c2[1];
				*c1++ = c2[0];
				c2 += 3;
			}
		}
		


		count = scanLength;
		mywrite( output, count, buf );
		if( count != scanLength )
		{
			PrintError("writeBMP, could not write image data");
			free( buf );
			return -1;
		}
		data -= im->bytesPerLine;
	}
	myclose( output );
	free( buf );
	
	return 0;
}


// Read bitmap file
static int readBMP( Image *im, fullPath *sfile )
{
	file_spec input;
	unsigned char *data, *buf;
	int y;
	int scanLength;
	long count;
	int reverse = 0;
	
	// Bitmap file open
	if( myopen( sfile, read_bin, input )	)
	{
		PrintError("readBMP, could not open file"); 
		return -1;
	}


	// Read bitmap file header

	if( readBMPFileHeader(im, input)  )
	{
		PrintError("readBMP, error reading bitmap file header");
		return -1;
	}
	if(0 > im->height) //JMW  BMP has rows from bottom to top
	{
		im->height = -im->height;
		reverse = 1;
	}

	scanLength = (im->width * 3 + 1)/2;
	scanLength *= 2;

	
	im->data = (unsigned char**) mymalloc( im->dataSize );
	if( im->data == NULL )
	{
		PrintError("Not enough memory");
		return -1;
	}

	buf = (unsigned char*)malloc( im->bytesPerLine + 1); // Worst case
	if( buf == NULL )
	{
		PrintError( "Not enough memory" );
		return -1;
	}
	

	if( 0 == reverse)
		data = *(im->data) + ((im->height-1) * im->bytesPerLine);
	else // 1 == reverse
		data = *(im->data);

	for(y=0; y<im->height; y++)
	{
		count = scanLength;
		myread(  input, count, buf );
		if( count != scanLength )
		{
			PrintError("Error reading image data");
			return -1;
		}
		// Convert 3->4 samples; bgr->rgb
		{
			int x;
			unsigned char *c1=buf, *c2=data;
			for(x=0; x < im->width; x++)
			{
				*c2++ = UCHAR_MAX;
				*c2++ = c1[2];
				*c2++ = c1[1];
				*c2++ = c1[0];
				c1+=3;
			}
		}
		if( 0 == reverse)
			data -= im->bytesPerLine;
		else //1 == reverse
			data += im->bytesPerLine;
	}
	
	myclose(input);
	free( buf );
	return 0;
}


// Read bitmap file header
static int readBMPFileHeader(Image *im, file_spec input)
{
	win3xHead header;						// First part of bitmap header
	win3xBitmapInfoHeader iheader;			// Second part of bitmap header
	long count;
	
	count = sizeof(header);
	myread(  input, count, &header );
	if( count != sizeof(header) )
	{
		PrintError("Error reading first BMP header segment");
		return -1;
	}

	if(header.ImageFileType != 0x4D42)
	{
		PrintError("readBMPFileHeader, BMP bad magic No");
		return -1;
	}


	count = sizeof(iheader);
	myread(  input, count, &iheader );
	if( count != sizeof(iheader) )
	{
		PrintError("Error reading second BMP header segment");
		return -1;
	}

	if(iheader.HeaderSize != 40)
	{			// Size of this header
		PrintError("readBMPFileHeader, secondary header length wrong -- Not MS version 3 compatible");
		return -1;
	}

	if(iheader.NumberOfImagePlanes != 1){	// Number of planes (Always 1)
		PrintError("readBMPFileHeader, should be 1 image plane");
		return -1;
	}

	if(iheader.BitsPerPixel != 24){			// Bits per pixel (1,4, 8 or 24)
		PrintError("readBMPFileHeader, only 24 bit color supported");
		return -1;
	}
	
	if(iheader.CompressionMethod != 0)
	{		// Compression method (0, 1 or 2)
		PrintError("readBMPFileHeader, only uncompressed BMP supported");
		return -1;
	}
	
	SetImageDefaults( im );
	
	// Setup height and width
	im->height = iheader.ImageHeight;
	im->width  = iheader.ImageWidth;
	
	im->bitsPerPixel = 32;
	im->bytesPerLine = im->width * 4;
	// JMW Negative height BMP have rows bottom to top
	im->dataSize = im->bytesPerLine * abs(im->height);
	
	// Advance file pointer to start of image data
	fseek( input, header.ImageDataOffset, SEEK_SET );
	
	return 0;							// Header ok -- no error
}


int panoBMPRead( Image *im, fullPath *sfile )
{
  if (readBMP(im, sfile) == 0) {
    return panoMetadataUpdateFromImage(im);
  } 
  else 
    return FALSE;
}


