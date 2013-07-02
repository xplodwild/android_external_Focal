
// Modify transparency channel to indicate distance from image area
// 255 = in image; distance = 255-alpha

#ifdef SET_TO_EDGE
#undef SET_TO_EDGE
#endif
#ifdef SET_TO_DISTANCE
#undef SET_TO_DISTANCE
#endif

#define SET_TO_EDGE( a )		{ ch = a; if( *(PIXEL_TYPE*)ch == 1 ) *(PIXEL_TYPE*)ch = (PIXEL_TYPE)254;}
#define SET_TO_DISTANCE( a )	{ ch = a; if( *(PIXEL_TYPE*)ch && *(PIXEL_TYPE*)ch < setdist ) *(PIXEL_TYPE*)ch = (PIXEL_TYPE)setdist;}


#ifdef BYTES_PER_CHANNEL
#undef BYTES_PER_CHANNEL
#endif

#define BYTES_PER_CHANNEL	sizeof(PIXEL_TYPE)

void _SetDistance ( Image* im, Image* pano, PTRect *theRect, int showprogress )
{
	int 					x,y,bpp,bppa, bpl, bpla, skip=0,setdist,dist;
	unsigned char			*data = *(im->data), *alpha = *(pano->data), *idata, *adata;
	register unsigned char 	*ch;
	char 					*progressMessage, percent[24];
	PTRect 					OLRect;
	
	bpp 	= im->bitsPerPixel/8;
	bppa	= pano->bitsPerPixel/8;
	bpl 	= im->bytesPerLine;
	bpla	= pano->bytesPerLine;
	
	OLRect.left 	= theRect->right;
	OLRect.right 	= theRect->left;
	OLRect.top 		= theRect->bottom;
	OLRect.bottom 	= theRect->top;

	if( showprogress )
	{
		progressMessage = "Merging Images";
		Progress( _initProgress, progressMessage );
	}

	
	// First, set alpha channel in overlap region to 1, find minimum overlap rectangle
	
	for(y=theRect->top; y<theRect->bottom; y++)
	{
		idata = data  + bpl  * y + theRect->left * bpp; 
		adata = alpha + bpla * y + theRect->left * bppa;
		for(x=theRect->left; x<theRect->right; x++, idata+=bpp, adata+=bppa)
		{
			if( *(PIXEL_TYPE*)idata == PIXEL_MAX && *(PIXEL_TYPE*)adata == PIXEL_MAX ) // Point in  overlap
			{
				*(PIXEL_TYPE*)idata = 1; *(PIXEL_TYPE*)adata = 1;
				if(x>OLRect.right) 	OLRect.right = x;
				if(x<OLRect.left ) 	OLRect.left = x;
				if(y>OLRect.bottom) OLRect.bottom = y;
				if(y<OLRect.top ) 	OLRect.top = y;
			}
		}
	}
	OLRect.right++;  OLRect.bottom++;
	// Set frame
	
	for(y=theRect->top; y<theRect->bottom; y++)
	{
		idata = data  + bpl  * y + theRect->left * bpp; 
		adata = alpha + bpla * y + theRect->left * bppa;
		for(x=theRect->left; x<theRect->right; x++, idata+=bpp, adata+=bppa)
		{
			if( *(PIXEL_TYPE*)idata  && !*(PIXEL_TYPE*)adata )
			{
				if	( x > theRect->left 	&& *(PIXEL_TYPE*)(adata - bppa)) 	SET_TO_EDGE( idata - bpp );
				if	( x < theRect->right-1 	&& *(PIXEL_TYPE*)(adata + bppa)) 	SET_TO_EDGE( idata + bpp );
				if	( y > theRect->top 		&& *(PIXEL_TYPE*)(adata - bpla)) 	SET_TO_EDGE( idata - bpl );
				if	( y < theRect->bottom-1 && *(PIXEL_TYPE*)(adata + bpla)) 	SET_TO_EDGE( idata + bpl );
			}
				
			if(!*(PIXEL_TYPE*)idata  &&  *(PIXEL_TYPE*)adata )
			{	
				if	( x > theRect->left 	&& *(PIXEL_TYPE*)(idata - bpp)) 	SET_TO_EDGE( adata - bppa );
				if	( x < theRect->right-1 	&& *(PIXEL_TYPE*)(idata + bpp)) 	SET_TO_EDGE( adata + bppa );
				if	( y > theRect->top 		&& *(PIXEL_TYPE*)(idata - bpl)) 	SET_TO_EDGE( adata - bpla );
				if	( y < theRect->bottom-1 && *(PIXEL_TYPE*)(idata + bpl)) 	SET_TO_EDGE( adata + bpla );
			}
		}
	}
	

				
	// Set distances	
	
	for( dist = 2; dist<255; dist++)
	{
			// Update Progress report and check for cancel every 5 lines.
		skip++;
		if( showprogress && skip == 5 )
		{
			
			sprintf( percent, "%d", (int) (dist* 100)/255) ;
			if( ! Progress( _setProgress, percent ) )
						return;
			skip = 0;
		}
		setdist = 255-dist;

		for(y=OLRect.top; y<OLRect.bottom; y++)
		{
			idata = data  + bpl  * y + OLRect.left * bpp; 
			adata = alpha + bpla * y + OLRect.left * bppa;
			for(x=OLRect.left; x<OLRect.right; x++, idata+=bpp, adata+=bppa)
			{
				if( *(PIXEL_TYPE*)idata 	== setdist + 1 )
				{
					if	( x > OLRect.left 		&& *(PIXEL_TYPE*)(adata - bppa)) 	SET_TO_DISTANCE( idata - bpp );
					if	( x < OLRect.right-1 	&& *(PIXEL_TYPE*)(adata + bppa)) 	SET_TO_DISTANCE( idata + bpp );
					if	( y > OLRect.top 		&& *(PIXEL_TYPE*)(adata - bpla)) 	SET_TO_DISTANCE( idata - bpl );
					if	( y < OLRect.bottom-1 	&& *(PIXEL_TYPE*)(adata + bpla)) 	SET_TO_DISTANCE( idata + bpl );
				}				
				if( *(PIXEL_TYPE*)adata 	== setdist + 1 )
				{	
					if	( x > OLRect.left 		&& *(PIXEL_TYPE*)(idata - bpp)) 	SET_TO_DISTANCE( adata - bppa );
					if	( x < OLRect.right-1 	&& *(PIXEL_TYPE*)(idata + bpp)) 	SET_TO_DISTANCE( adata + bppa );
					if	( y > OLRect.top 		&& *(PIXEL_TYPE*)(idata - bpl)) 	SET_TO_DISTANCE( adata - bpla );
					if	( y < OLRect.bottom-1 	&& *(PIXEL_TYPE*)(idata + bpl)) 	SET_TO_DISTANCE( adata + bpla );
				}
			}
		}
	}

	if( showprogress )
		Progress( _disposeProgress, percent );
}

void _SetDistanceImage ( Image* im, Image* pano, PTRect *theRect, int showprogress, int feather )
{
	int 					x,y,bpp,bppa, bpl, bpla, skip=0,setdist,dist;
	unsigned char			*data = *(im->data), *alpha = *(pano->data), *idata, *adata;
	register unsigned char 	*ch;
	char 					*progressMessage, percent[24];
	PTRect 					OLRect;
	
	bpp 	= im->bitsPerPixel/8;
	bppa	= pano->bitsPerPixel/8;
	bpl 	= im->bytesPerLine;
	bpla	= pano->bytesPerLine;
	
	OLRect.left 	= theRect->right;
	OLRect.right 	= theRect->left;
	OLRect.top 		= theRect->bottom;
	OLRect.bottom 	= theRect->top;

	if( showprogress )
	{
		progressMessage = "Merging Images";
		Progress( _initProgress, progressMessage );
	}

	
	// First, set alpha channel in overlap region to 1, find minimum overlap rectangle
	
	for(y=theRect->top; y<theRect->bottom; y++)
	{
		idata = data  + bpl  * y + theRect->left * bpp; 
		adata = alpha + bpla * y + theRect->left * bppa;
		for(x=theRect->left; x<theRect->right; x++, idata+=bpp, adata+=bppa)
		{
			if( *(PIXEL_TYPE*)idata == PIXEL_MAX && *(PIXEL_TYPE*)adata == PIXEL_MAX ) // Point in  overlap
			{
				*(PIXEL_TYPE*)adata = 1; 
				if(x>OLRect.right) 	OLRect.right = x;
				if(x<OLRect.left ) 	OLRect.left = x;
				if(y>OLRect.bottom) OLRect.bottom = y;
				if(y<OLRect.top ) 	OLRect.top = y;
			}
		}
	}
	OLRect.right++;  OLRect.bottom++;
	// Set frame
	
	for(y=theRect->top; y<theRect->bottom; y++)
	{
		idata = data  + bpl  * y + theRect->left * bpp; 
		adata = alpha + bpla * y + theRect->left * bppa;
		for(x=theRect->left; x<theRect->right; x++, idata+=bpp, adata+=bppa)
		{
			if(!*(PIXEL_TYPE*)idata  &&  *(PIXEL_TYPE*)adata )
			{	
				if	( x > theRect->left 	&& *(PIXEL_TYPE*)(idata - bpp)) 		SET_TO_EDGE( adata - bppa );
				if	( x < theRect->right-1 	&& *(PIXEL_TYPE*)(idata + bpp)) 		SET_TO_EDGE( adata + bppa );
				if	( y > theRect->top 		&& *(PIXEL_TYPE*)(idata - bpl)) 		SET_TO_EDGE( adata - bpla );
				if	( y < theRect->bottom-1 && *(PIXEL_TYPE*)(idata + bpl)) 		SET_TO_EDGE( adata + bpla );
			}
		}
	}
	

				
	// Set distances	
	
	feather+=2;
	if(feather > 255) feather = 255;
	for( dist = 2; dist<feather; dist++)
	{
			// Update Progress report and check for cancel every 5 lines.
		skip++;
		if( showprogress && skip == 5 )
		{
			
			sprintf( percent, "%d", (int) (dist* 100)/255) ;
			if( ! Progress( _setProgress, percent ) )
						return;
			skip = 0;
		}
		setdist = 255-dist;

		for(y=OLRect.top; y<OLRect.bottom; y++)
		{
			idata = data  + bpl  * y + OLRect.left * bpp; 
			adata = alpha + bpla * y + OLRect.left * bppa;
			for(x=OLRect.left; x<OLRect.right; x++, idata+=bpp, adata+=bppa)
			{
				if( *(PIXEL_TYPE*)adata 	== setdist + 1 )
				{	
					if	( x > OLRect.left 		&& *(PIXEL_TYPE*)(idata - bpp)) 		SET_TO_DISTANCE( adata - bppa );
					if	( x < OLRect.right-1 	&& *(PIXEL_TYPE*)(idata + bpp)) 		SET_TO_DISTANCE( adata + bppa );
					if	( y > OLRect.top 		&& *(PIXEL_TYPE*)(idata - bpl)) 		SET_TO_DISTANCE( adata - bpla );
					if	( y < OLRect.bottom-1 	&& *(PIXEL_TYPE*)(idata + bpl)) 		SET_TO_DISTANCE( adata + bpla );
				}
			}
		}
	}

	if( showprogress )
		Progress( _disposeProgress, percent );
}
					

// Add image src to dst; use alpha channel
// seam is placed in the middle between edges of valid 
// image portions.
// feather specifies width of soft edge (in pixels)
// showprogress = 1: Progressbar is displayed
// return 0 on success, -1 on failure

int _merge (  Image *dst, Image *src,  int feather, int showprogress, int seam )
{
	register int 			x,y, i;
	register unsigned char *dest, *source;	// Pointer to rgb-data
    double 					sfactor = 1.0;
	register double 		result;
	PTRect					theRect;
	int 					bps,bpd;
	

	// Do all sorts of checks (size, etc)

	if( (dst->bytesPerLine != src->bytesPerLine) ||
		(dst->width        != src->width)		 ||
		(dst->height       != src->height)       ||
	    (dst->dataSize     != src->dataSize)     ||
		(dst->bitsPerPixel != src->bitsPerPixel) ||
		(dst->bitsPerPixel != (BYTES_PER_CHANNEL*32)) 	||
		(dst->data 		   == NULL)				 ||
		(src->data         == NULL))
	{
		return -1;
	}
		
	theRect.left 	= 0;
	theRect.right 	= dst->width;
	theRect.top 	= 0;
	theRect.bottom 	= dst->height;
	
	bps = src->bitsPerPixel/8;
	bpd = dst->bitsPerPixel/8;

	if( seam == _middle ){
		_SetDistance ( src, dst, &theRect, showprogress );

		for(y=0; y<dst->height; y++){	
			dest 	= *(dst->data) + y*dst->bytesPerLine;
			source 	= *(src->data) + y*src->bytesPerLine;
			for(x=0; x<dst->width; x++, dest+=bpd, source+=bps){
				if( *(PIXEL_TYPE*)source ) {// alpha channel set
					if( !*(PIXEL_TYPE*)dest ) {// No data in dest, so copy source to dest
						*(PIXEL_TYPE*)dest = 1; 
						*(PIXEL_TYPE*)(dest+BYTES_PER_CHANNEL) 		= *(PIXEL_TYPE*)(source+BYTES_PER_CHANNEL);
						*(PIXEL_TYPE*)(dest+2*BYTES_PER_CHANNEL) 	= *(PIXEL_TYPE*)(source+2*BYTES_PER_CHANNEL);
						*(PIXEL_TYPE*)(dest+3*BYTES_PER_CHANNEL) 	= *(PIXEL_TYPE*)(source+3*BYTES_PER_CHANNEL);
					}
					else{
						int d = 255 - (int)*(PIXEL_TYPE*)source, s = 255 - (int)*(PIXEL_TYPE*)dest;
						
						if( d==254 || d > s + feather )
						; // Use dest
						else if( s > d + feather ) // Use just source
						{
							*(PIXEL_TYPE*)(dest+BYTES_PER_CHANNEL) 		= *(PIXEL_TYPE*)(source+BYTES_PER_CHANNEL);
							*(PIXEL_TYPE*)(dest+2*BYTES_PER_CHANNEL) 	= *(PIXEL_TYPE*)(source+2*BYTES_PER_CHANNEL);
							*(PIXEL_TYPE*)(dest+3*BYTES_PER_CHANNEL) 	= *(PIXEL_TYPE*)(source+3*BYTES_PER_CHANNEL);
						}
						else
						{
							sfactor = GetBlendfactor( d, s, feather );
							for( i=1; i<=3; i++)
							{
								result = sfactor * *(PIXEL_TYPE*)(source+ i*BYTES_PER_CHANNEL)  + 
										( 1.0 - sfactor ) * *(PIXEL_TYPE*)(dest+i*BYTES_PER_CHANNEL);
								DBL_TO_PIX( *(PIXEL_TYPE*)(dest+i*BYTES_PER_CHANNEL) , result );
							}
						}
					}
				}
			}
		}
	}
	else if( seam == _dest ){
		_SetDistanceImage ( dst, src, &theRect, showprogress, feather );
		for(y=0; y<dst->height; y++){	
			dest 	= *(dst->data) + y*dst->bytesPerLine;
			source 	= *(src->data) + y*src->bytesPerLine;
			for(x=0; x<dst->width; x++, dest+=bpd, source+=bps){
				if( *(PIXEL_TYPE*)source ) {// alpha channel set
					if( !*(PIXEL_TYPE*)dest ) {// No data in dest, so copy source to dest
						*(PIXEL_TYPE*)dest = 1; 
						*(PIXEL_TYPE*)(dest+BYTES_PER_CHANNEL) 	= *(PIXEL_TYPE*)(source+BYTES_PER_CHANNEL);
						*(PIXEL_TYPE*)(dest+2*BYTES_PER_CHANNEL) 	= *(PIXEL_TYPE*)(source+2*BYTES_PER_CHANNEL);
						*(PIXEL_TYPE*)(dest+3*BYTES_PER_CHANNEL) 	= *(PIXEL_TYPE*)(source+3*BYTES_PER_CHANNEL);
					}
					else{
						int d = 255 - (int)*(PIXEL_TYPE*)source; 
						
						if( d > feather ) 
						; // Use dest
						else{ // blend
							sfactor = ((double) d / (double) feather) * ( 1.0 - BLEND_RANDOMIZE * rand() / (double)RAND_MAX );
							for( i=1; i<=3; i++){
								result = sfactor * *(PIXEL_TYPE*)(dest+ i*BYTES_PER_CHANNEL)  + 
										( 1.0 - sfactor ) * *(PIXEL_TYPE*)(source+i*BYTES_PER_CHANNEL);
								DBL_TO_PIX( *(PIXEL_TYPE*)(dest+i*BYTES_PER_CHANNEL) , result );
							}
						}
					}
				}
			}
		}
		
	}
	else
	{
		PrintError("Error in function merge");
		return -1;
	}
		
	// Set alpha channel
	
	LOOP_IMAGE( dst, {if( *(PIXEL_TYPE*)idata ) *(PIXEL_TYPE*)idata = PIXEL_MAX;} );

	return 0;
}











void _mergeAlpha ( Image *im, unsigned char *alpha, int feather, PTRect *theRect )
{
	register int 			x,y;
	double 				sfactor;
	unsigned char			*data = *(im->data), *idata, *adata;
	int				channels, BitsPerChannel,bpp;
	Image				aImage;	 // Dummy for transfering alpha data
	
	
	GetChannels( im, channels );
	GetBitsPerChannel( im, BitsPerChannel );
	bpp = im->bitsPerPixel/8;

	memcpy( &aImage, im, sizeof( Image ));
	aImage.bitsPerPixel = BitsPerChannel;
	aImage.bytesPerLine = im->width*BYTES_PER_CHANNEL;
	aImage.data = &alpha;
	
	
	_SetDistance ( im, &aImage, theRect, 1 );

	for(y=theRect->top; y<theRect->bottom; y++){
		idata = data + im->bytesPerLine * y + theRect->left * bpp;
		adata = alpha + im->width * y * BYTES_PER_CHANNEL + theRect->left * BYTES_PER_CHANNEL;
		for(x=theRect->left; x<theRect->right; x++, idata+=bpp, adata+=BYTES_PER_CHANNEL){
			if( *(PIXEL_TYPE*)idata ){ // alpha channel of image set
				if( !*(PIXEL_TYPE*)adata ) {// No data in pano, so copy source to dest
					*(PIXEL_TYPE*)idata = PIXEL_MAX;
					}
				else{
					int d = 255 - (int)*(PIXEL_TYPE*)idata, s = 255 - (int)*(PIXEL_TYPE*)adata;
						
					if( d==254 || d > s + feather ){
							*(PIXEL_TYPE*)idata = 0;
					}
					else if( s > d + feather ){ // Use just source
							*(PIXEL_TYPE*)idata = PIXEL_MAX;
					}
					else{
						sfactor = 255.0 * GetBlendfactor( d, s, feather );
						DBL_TO_PIX( *(PIXEL_TYPE*)idata , sfactor );
					}
				}
			}
		}
	}
}

					



