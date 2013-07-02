#include "filter.h"
#include "pteditor.h"

#if _MSC_VER > 1000
#pragma warning(disable: 4100) // disable unreferenced formal parameter warning
#endif

//defined in ptpicker.c
int jpathTofullPath( const char* jpath, fullPath *fp );

static Image *pano = NULL;


JNIEXPORT void JNICALL Java_pteditor_CLoadImage
	(JNIEnv *env, jobject obj PT_UNUSED, jstring path){
	fullPath fp;
	const char *jpath = (*env)->GetStringUTFChars(env, path, 0);

#ifdef __Mac__
	setLibToResFile();
#endif
	if( strlen(jpath) == 0 ) return;	

	if( jpathTofullPath( jpath, &fp ) != 0 ){
		PrintError("Could not create Path from %s", jpath);
		return;
	}

	(*env)->ReleaseStringUTFChars(env, path, jpath);
	
	if( pano != NULL ){
		if( pano->data != NULL ){
			myfree((void**)pano->data);
			pano->data = NULL;
		}
	}else
		pano = (Image*)malloc(sizeof(Image));
	
	SetImageDefaults(pano);

	if( panoImageRead( pano, &fp ) == 0 ){
		PrintError("Could not read image");
		return;
	}
#ifdef __Mac__
	unsetLibToResFile();
#endif

}


JNIEXPORT void JNICALL Java_pteditor_CSaveImage
	(JNIEnv *env, jobject obj PT_UNUSED, jstring path){
	fullPath fp;
	const char *jpath = (*env)->GetStringUTFChars(env, path, 0);

#ifdef __Mac__
	setLibToResFile();
#endif

	if( strlen(jpath) > 0 ){
		if( jpathTofullPath( jpath, &fp ) != 0 ){
			PrintError("Could not create Path from %s", jpath);
			return;
		}
	}
	(*env)->ReleaseStringUTFChars(env, path, jpath);
	
	mycreate( &fp, '8BIM', 'TIFF' );
	if(pano != NULL)
		writeTIFF( pano, &fp );
#ifdef __Mac__
	unsetLibToResFile();
#endif

}


JNIEXPORT jint JNICALL Java_pteditor_CGetImageWidth
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED){
	if(pano != NULL)
		return pano->width;
	else
		return 0;
}

JNIEXPORT jint JNICALL Java_pteditor_CGetImageHeight
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED){
	if(pano != NULL)
		return pano->height;
	else
		return 0;
}

JNIEXPORT void JNICALL Java_pteditor_CGetImageRow
  (JNIEnv *env, jobject obj PT_UNUSED, jintArray jdata, jint nrow){
	if(pano == NULL) return;
  	if(pano->data != NULL){
		jint *pix = (jint*)malloc(pano->width * sizeof(jint));
		if( pix == NULL ) return;
		if( pano->bitsPerPixel == 64 ){
			int x;
			unsigned char *p = *(pano->data) + pano->bytesPerLine * nrow,*q = (unsigned char*)pix;
			for(x=0; x<pano->width; x++, p+=8, q+=4){
#ifdef PT_BIGENDIAN
				q[0] = p[0]; q[1] = p[2]; q[2] = p[4]; q[3] = p[6];
#else
				q[0] = p[7]; q[1] = p[5]; q[2] = p[3]; q[3] = p[1];
#endif
			}
		}else{
#ifdef PT_BIGENDIAN
			memcpy( pix, *(pano->data) + pano->bytesPerLine * nrow, pano->width * sizeof(jint));				
#else
			int x;
			unsigned char *p = *(pano->data) + pano->bytesPerLine * nrow,*q = (unsigned char*)pix;
			for(x=0; x<pano->width; x++, p+=4, q+=4){
				q[0] = p[3];q[1] = p[2];q[2] = p[1];q[3] = p[0];
			}
#endif
		}
		(*env)->SetIntArrayRegion( env, jdata, 0, pano->width , pix ) ;
		free( pix );
	}
}
 
 
JNIEXPORT void JNICALL Java_pteditor_CExtract
  (JNIEnv *env, jobject obj PT_UNUSED, jstring path, jdouble yaw, jdouble pitch, jdouble hfov, jdouble aspect, jint format, jdouble phfov ){
	aPrefs				ap;
	TrformStr  			Tr;
	Image				dest;
	struct size_Prefs 	spref;
	fullPath fp;

	const char *jpath = (*env)->GetStringUTFChars(env, path, 0);
#ifdef __Mac__
	setLibToResFile();
#endif

	if( pano == NULL ) return;

	SetImageDefaults(&dest);
	SetAdjustDefaults( &ap );

	ap.mode = _extract;
	
	memcpy( &ap.pano, pano, sizeof(Image) );
	
	ap.pano.format 	= format;
	ap.pano.hfov	= phfov;

	if( ap.pano.format == _equirectangular ){ // extract image from panorama	
		ap.im.width = (int32_t)((hfov / ap.pano.hfov) * 2 * ap.pano.width);
	}else{ // extract image from fisheye source
		ap.im.width = (int32_t)((hfov / ap.pano.hfov) * ap.pano.width);
	}
	ap.im.height	= (int32_t)((double)ap.im.width / aspect);
	ap.im.format	= _rectilinear;
	ap.im.yaw		= yaw;
	ap.im.pitch		= pitch;
	ap.im.hfov		= hfov;

	if( readPrefs( (char*) &spref, _sizep ) != 0 )
		SetSizeDefaults	( &spref );

	memset(&Tr, 0, sizeof(TrformStr));
	Tr.src 	= pano;
	Tr.dest	= &dest;

	Tr.tool					= _adjust ;
	Tr.mode					= _usedata + _show_progress + _honor_valid;
	Tr.interpolator			= spref.interpolator;
	Tr.gamma				= spref.gamma;
    Tr.fastStep             = spref.fastStep;
	Tr.data					= (void*) &ap;

	Tr.success = 1;
	
	filter_main( &Tr, &spref);
	
	// Save image
	if( Tr.success ){
		if( strlen(jpath) > 0 ){
			if( jpathTofullPath( jpath, &fp ) != 0 ){
				PrintError("Could not create Path from %s", jpath);
				return;
			}
		}
		(*env)->ReleaseStringUTFChars(env, path, jpath);

		mycreate( &fp, '8BIM', 'TIFF' );

		writeTIFF( &dest, &fp );
		myfree((void**)dest.data);
	}
#ifdef __Mac__
	unsetLibToResFile();
#endif

}

JNIEXPORT void JNICALL Java_pteditor_CInsert
  (JNIEnv *env, jobject obj PT_UNUSED, jstring path, jdouble yaw, jdouble pitch, jdouble rot, jdouble hfov, jint format){
	aPrefs				ap;
	TrformStr  			Tr;
	struct size_Prefs 	spref;
	fullPath fp;
	Image	src, dest;

	const char *jpath = (*env)->GetStringUTFChars(env, path, 0);
#ifdef __Mac__
	setLibToResFile();
#endif

	if(pano == NULL) return;
	SetAdjustDefaults( &ap );
	SetImageDefaults(&src);
	SetImageDefaults(&dest);

	ap.mode = _insert;
	
	memcpy( &ap.pano, pano, sizeof(Image) );
	ap.pano.format 	= _equirectangular;
	ap.pano.hfov	= 360.0;
	
	// Set destination image here
	
	memcpy( &dest, pano, sizeof(Image) );
	dest.data = (unsigned char**)mymalloc(dest.dataSize);
	if(dest.data == NULL){
		PrintError("Not enough memory");
		return;
	}


	if( strlen(jpath) > 0 ){
		if( jpathTofullPath( jpath, &fp ) != 0 ){
			PrintError("Could not create Path from %s", jpath);
			return;
		}
	}
	(*env)->ReleaseStringUTFChars(env, path, jpath);

	if( panoImageRead( &src, &fp ) == 0 ){
		PrintError("Could not read image");
		return;
	}

	ap.im.format		= format;
	ap.im.yaw		= yaw;
	ap.im.pitch		= pitch;
	ap.im.hfov		= hfov;
	ap.im.roll		= rot;

	if( readPrefs( (char*) &spref, _sizep ) != 0 )
		SetSizeDefaults	( &spref );
	
  memset(&Tr, 0, sizeof(TrformStr));
	Tr.src 	= &src;
	Tr.dest	= &dest;

	Tr.tool					= _adjust ;
	Tr.mode					= _usedata + _show_progress + _destSupplied;
	Tr.interpolator			= spref.interpolator;
	Tr.gamma				= spref.gamma;
    Tr.fastStep             = spref.fastStep;
	Tr.data					= (void*) &ap;

	Tr.success = 1;
	
	filter_main( &Tr, &spref);
	
	myfree((void**)src.data);
	
	// Merge images
	if( Tr.success ){
		if( merge( &dest , pano, ap.sBuf.feather, Tr.mode & _show_progress, _dest ) != 0 ){
			PrintError( "Error merging images" );
			myfree((void**)dest.data);
		}else{
			myfree((void**)pano->data);
			pano->data = dest.data;
		}
	}
#ifdef __Mac__
	unsetLibToResFile();
#endif

}


JNIEXPORT void JNICALL Java_pteditor_CSetImageWidth
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint width){
	if( pano != NULL ){
		if( pano->data != NULL ){
			myfree((void**)pano->data);
			pano->data = NULL;
		}
	}else{
		pano = (Image*)malloc(sizeof(Image));
		SetImageDefaults(pano);
	}
	pano->width = width;
}

JNIEXPORT void JNICALL Java_pteditor_CSetImageHeight
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint height){
	if( pano != NULL ){
		if( pano->data != NULL ){
			myfree((void**)pano->data);
			pano->data = NULL;
		}
	}else{
		pano = (Image*)malloc(sizeof(Image));
		SetImageDefaults(pano);
	}
	
	pano->height = height;
}

JNIEXPORT void JNICALL Java_pteditor_CSetImageRow
  (JNIEnv *env, jobject obj PT_UNUSED, jintArray jdata, jint nrow){
	if( pano == NULL ) return;
	if( pano->width == 0 || pano->height == 0 ) return;
	
	if( pano->data == NULL){
		pano->bitsPerPixel = 32;
		pano->bytesPerLine = pano->width  * pano->bitsPerPixel/8;
		pano->dataSize = pano->height * pano->bytesPerLine;
		pano->data = (unsigned char**) mymalloc( pano->dataSize * sizeof(unsigned char) );
	}

	if( pano->data == NULL) return;

	(*env)->GetIntArrayRegion( env, jdata, 0, pano->width , 
				(jint*)((*pano->data) + pano->bytesPerLine * nrow) ) ;
#ifdef PT_BIGENDIAN
#else
	{
		jint *row = (jint*)((*pano->data) + pano->bytesPerLine * nrow), pix;
		unsigned char *p,*q;
		int x;
		q = (unsigned char*) &pix;
		for(x=0; x<pano->width; x++){
			p = (unsigned char*) &(row[x]);
			q[0] = p[3];
			q[1] = p[2];
			q[2] = p[1];
			q[3] = p[0];
			row[x] = pix;
		}
	}
				
#endif
}


