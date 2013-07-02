
#include "filter.h"
#include "ptutils.h"
#include <locale.h>

#if _MSC_VER > 1000
#pragma warning(disable: 4100) // disable unreferenced formal parameter warning
#endif

void ReadMLine( char *script, char *m );
int loadProject( fullPath *fspec );
int writeProject( AlignInfo *g, fullPath *pFile);
int jpathTofullPath( const char* jpath, fullPath *fp );
void BackUp();
void Restore();
void SetAlignInfoDefaults( AlignInfo *a);


#define HELPERS "./Helpers/"

#ifdef __Mac__
	#undef HELPERS
	#define HELPERS ":Helpers:"
#endif
 
#ifdef WIN32
  #undef HELPERS
	#define HELPERS "Helpers\\"
#endif


AlignInfo						*gl = NULL;
fullPath 						project;
void							*theBackUp=NULL;
char							mLine[256];


Image im;
int JavaUI 			= FALSE;
static JNIEnv 		*ptenv;
// since gPicker is already defined in ColorPicker.h I change picker to gPicker : Kekus Digital
//static jobject 	picker; //commented by Kekus Digital
static jobject 		gPicker;//added by Kekus Digital

#define SET_JAVA JavaUI = TRUE; ptenv = env; gPicker = obj;

void JPrintError( char* text ){
	jclass cls = (*ptenv)->GetObjectClass(ptenv, gPicker);
  	jmethodID mid = (*ptenv)->GetMethodID(ptenv, cls, "PrintError", "(Ljava/lang/String;)V");
    if (mid == 0)  return;
   	(*ptenv)->CallVoidMethod(ptenv, gPicker, mid, (*ptenv)->NewStringUTF(ptenv, text));
}

#if 0
int JinfoDlg( int command, char* argument ){
	jclass cls = (*ptenv)->GetObjectClass(ptenv, gPicker);
  	jmethodID mid = (*ptenv)->GetMethodID(ptenv, cls, "infoDlg", "(ILjava/lang/String;)I");
    if (mid == 0)  return -1;
   	(*ptenv)->CallIntMethod(ptenv, gPicker, mid, command, (*ptenv)->NewStringUTF(ptenv, argument));
}
	
int JProgress( int command, char* argument ){
	jclass cls = (*ptenv)->GetObjectClass(ptenv, gPicker);
  	jmethodID mid = (*ptenv)->GetMethodID(ptenv, cls, "Progress", "(ILjava/lang/String;)I");
    if (mid == 0)  return -1;
   	(*ptenv)->CallIntMethod(ptenv, gPicker, mid, command, (*ptenv)->NewStringUTF(ptenv, argument));
}
#endif
JNIEXPORT void JNICALL Java_ptutils_CSaveProject
	(JNIEnv *env, jobject obj, jstring path){

	const char *jpath = (*env)->GetStringUTFChars(env, path, 0);

	SET_JAVA

	
	if( strlen(jpath) > 0 ){
		if( jpathTofullPath( jpath, &project ) != 0 ){
			PrintError("Could not create Path from %s", jpath);
			return;
		}
	}
	(*env)->ReleaseStringUTFChars(env, path, jpath);
	writeProject( gl, &project);
}



JNIEXPORT void JNICALL Java_ptutils_CSetControlPointCount
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint nc){
	if( gl && nc != gl->numPts ){
		free(gl->cpt);
		gl->numPts = nc;
		gl->cpt = (controlPoint*) malloc( gl->numPts * sizeof( controlPoint ) );
	}
}

JNIEXPORT void JNICALL Java_ptutils_CSetCP
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint i, jint n0, jint n1, jdouble x0, jdouble x1, jdouble y0, jdouble y1, jint type){
	if( gl && i < gl->numPts ){
		controlPoint *c = &(gl->cpt[i]);
		c->num[0] = n0;
		c->num[1] = n1;
		c->x[0] = x0;
		c->x[1] = x1;
		c->y[0] = y0;
		c->y[1] = y1;
		c->type = type;
	}
}

JNIEXPORT void JNICALL Java_ptutils_CSetTriangleCount
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint nt){
	if( gl && nt != gl->nt ){
		free( gl->t );
		gl->nt = nt;
		gl->t = (triangle*)malloc( gl->nt * sizeof(triangle) );
	}
}

JNIEXPORT void JNICALL Java_ptutils_CSetTR
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint i, jint v0, jint v1, jint v2, jint nIm){
	if( gl && i< gl->nt ){
		triangle *t = &(gl->t[i]);
		t->vert[0] = v0;
		t->vert[1] = v1;
		t->vert[2] = v2;
		t->nIm = nIm;
	}
}


	


JNIEXPORT void JNICALL Java_ptutils_CLoadProject
	(JNIEnv *env, jobject obj, jstring path){

	const char *jpath = (*env)->GetStringUTFChars(env, path, 0);
	
	SET_JAVA

	if( jpathTofullPath( jpath, &project ) != 0 ){
		PrintError("Could not create fullpath from %s", jpath);
		return;
	}
	(*env)->ReleaseStringUTFChars(env, path, jpath);

	if( loadProject( &project ) != 0 ){
		PrintError("Could not load Project");
		return;
	}
}

JNIEXPORT jint JNICALL Java_ptutils_CGetImageCount
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED){
	if( gl )
		return (jint)gl->numIm;
	else
		return (jint)0;
}

JNIEXPORT jstring JNICALL Java_ptutils_CGetImageName
  (JNIEnv *env, jobject obj, jint nIm){
	SET_JAVA
	if( gl )
  		return (*env)->NewStringUTF(env, gl->im[(int)nIm].name);
  	else
  		return (*env)->NewStringUTF(env, "");
}

JNIEXPORT jint JNICALL Java_ptutils_CGetControlPointCount
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED){
	if( gl )
		return (jint)gl->numPts;
	else
		return (jint)0;
}

JNIEXPORT jint JNICALL Java_ptutils_CGetCP_1n
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint ncpt, jint nim){
	if(gl)
  		return (jint)gl->cpt[(int)ncpt].num[(int)nim];
  	else
  		return (jint) 0;
}

JNIEXPORT jdouble JNICALL Java_ptutils_CGetCP_1x
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint ncpt, jint nim){
	if( gl )
  		return (jdouble)gl->cpt[(int)ncpt].x[(int)nim];
  	else
  		return (jdouble) 0.0;
}

JNIEXPORT jdouble JNICALL Java_ptutils_CGetCP_1y
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint ncpt, jint nim){
	if(gl)
  		return (jdouble)gl->cpt[(int)ncpt].y[(int)nim];
  	else
  		return (jdouble) 0.0;
}

JNIEXPORT jint JNICALL Java_ptutils_CGetCP_1t
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint ncpt){
	if( gl )
 		 return (jint)gl->cpt[(int)ncpt].type;
 	else
 		return (jint ) 0;
}
  
JNIEXPORT jint JNICALL Java_ptutils_CGetTriangleCount
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED){
	if( gl )
  		return (jint)gl->nt;
  	else
  		return (jint ) 0;
}

JNIEXPORT jint JNICALL Java_ptutils_CGetTR_1v
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint ntr, jint nv){
	if( gl )
 		return (jint)gl->t[(int)ntr].vert[(int)nv];
 	else
 		return (jint ) 0;
}
  
JNIEXPORT jint JNICALL Java_ptutils_CGetTR_1i
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint ntr){
  	return (jint)gl->t[(int)ntr].nIm;
}
  
JNIEXPORT void JNICALL Java_ptutils_CSetImageName
  (JNIEnv *env, jobject obj PT_UNUSED, jint n, jstring jname){
    const char *name = (*env)->GetStringUTFChars(env, jname, 0);
    if( n < gl->numIm ){
  		strcpy( gl->im[n].name, name );
	}
	(*env)->ReleaseStringUTFChars(env, jname, name);
}

JNIEXPORT jint JNICALL Java_ptutils_CGetIndex
 (JNIEnv *env, jobject obj PT_UNUSED, jstring jname){
 
 	const char *name =(*env)->GetStringUTFChars(env, jname, 0);
	jint i;
	
	for( i=0; i<gl->numIm; i++){
		if( strcmp( name, gl->im[i].name ) == 0 ){
			(*env)->ReleaseStringUTFChars(env, jname, name);
			return i;
		}
	}
	(*env)->ReleaseStringUTFChars(env, jname, name);
	return -1;
}










JNIEXPORT void JNICALL Java_ptutils_CLoadImage
  	(JNIEnv *env, jobject obj, jint n){
	fullPath fp;

	SET_JAVA  
	
	memcpy( &fp, &project, sizeof( fullPath ));
	InsertFileName( &fp, gl->im[n].name );
	
	SetImageDefaults(&im);
	
	if( panoImageRead( &im, &fp ) != 0 ){
		PrintError("Could not read image");
		return;
	}
	TwoToOneByte( &im ); 
	
	if( gl->im[n].cP.cutFrame ){
		CropImage( &im, &gl->im[n].selection );
	}

	gl->im[n].width 	= im.width;
	gl->im[n].height 	= im.height;
	
	// If this is a new project, set params now

	if(gl->im[n].hfov < 0.0)
	{
		int i;
		double dYaw = 360.0 / (double)gl->numIm;
		int numParam = 0;
		
		// Calculate Field of view
		
		if( gl->im[n].width < gl->im[n].height ) // portrait
		{
			if( gl->im[n].format == _rectilinear )
			{
				gl->im[n].hfov = 2.0 * atan( 12.0 / (-gl->im[n].hfov) );
			}
			else if ( gl->im[n].format == _fisheye_ff )
			{
				gl->im[n].hfov = 24.0 / (-gl->im[n].hfov);
			}
			else
				gl->im[n].hfov = 4.0 * asin( 5.7 / (-gl->im[n].hfov) );
		}
		else // landscape
		{
			if( gl->im[n].format == _rectilinear )
			{
				gl->im[n].hfov = 2.0 * atan( 18.0 / (-gl->im[n].hfov) );
			}
			else if( gl->im[n].format == _fisheye_ff )
			{
				gl->im[n].hfov = 36.0 / (-gl->im[n].hfov);
			}			
			else
				gl->im[n].hfov = 4.0 * asin( 5.7 / (-gl->im[n].hfov) );
		}
		gl->im[n].hfov = /*RAD_TO_DEG*/(( gl->im[n].hfov) * 360.0 / ( 2.0 * PI ) );
		
		if( gl->im[n].hfov < dYaw )
			PrintError( "Warning: No overlap of images");
		
		for(i=0; i<gl->numIm; i++)
		{
			gl->im[i].format = gl->im[n].format;
			gl->im[i].hfov	= gl->im[n].hfov;
			gl->im[i].yaw 	= (double)i * dYaw;
			gl->im[i].roll	= 0.0;
			gl->im[i].pitch  = 0.0;
			
			if( gl->im[i].format != _fisheye_circ )
			{
				// gl->opt[i].d		= 1; numParam++;
				// gl->opt[i].e		= 1; numParam++;
			}
			
			if(i != 0) // pin first image
			{
				gl->opt[i].hfov 	= 2; // linked to image 1
				gl->opt[i].yaw	= 1; numParam++;
				gl->opt[i].pitch	= 1; numParam++;
				gl->opt[i].roll	= 1; numParam++;
			}
			else
			{
				gl->opt[i].hfov 	= 1; numParam++;
			}
		}
		
		gl->numParam = numParam;
		
		// Set pano dimensionw
		gl->pano.width = (int32_t)(gl->im[n].width * gl->pano.hfov / gl->im[n].hfov);
		gl->pano.width /= 10; gl->pano.width *= 10;
		
			

		if( gl->pano.format == _equirectangular )
		{
			gl->pano.height = gl->pano.width / 2;
		}
		else	// Cylinder
		{
			if( gl->im[n].format == _rectilinear )
				gl->pano.height = (int32_t)(gl->im[n].height * cos( DEG_TO_RAD(dYaw) / 2.0 ));
			else
			{
				double vfov = gl->im[n].hfov * (double)gl->im[n].height / (double)gl->im[n].width;
				
				
				if( vfov < 180.0 )
					gl->pano.height = (int32_t)(gl->im[n].height * tan( DEG_TO_RAD(vfov) / 2.0 ) * cos( DEG_TO_RAD(dYaw) / 2.0 ) / ( DEG_TO_RAD(vfov) / 2.0));
				else
					gl->pano.height = (int32_t)(gl->im[n].height * tan( DEG_TO_RAD(160.0) / 2.0 ) / ( DEG_TO_RAD(160.0) / 2.0));
					
				gl->pano.height /= 10; gl->pano.height *= 10; 
			}
				
			
			if( strcmp( gl->pano.name, "QTVR") == 0 )
			{
				gl->pano.width = gl->pano.width - (gl->pano.width % 96);
				gl->pano.height= gl->pano.height - (gl->pano.height % 4);
			}
		}

		
	}
	

	
}
	


JNIEXPORT void JNICALL Java_ptutils_CGetImageRow
  (JNIEnv *env, jobject obj PT_UNUSED, jintArray jdata, jint nrow){
  	if(im.data != NULL){
#ifdef PT_BIGENDIAN
		(*env)->SetIntArrayRegion( env, jdata, 0, im.width , 
								(jint*)((*im.data) + im.bytesPerLine * nrow) ) ;
#else
		{
			jint *row = (jint*)((*im.data) + im.bytesPerLine * nrow), pix;
			unsigned char *p,*q;
			int x;
			q = (unsigned char*) &pix;
			for(x=0; x<im.width; x++){
				p = (unsigned char*) &(row[x]);
				q[0] = p[3];
				q[1] = p[2];
				q[2] = p[1];
				q[3] = p[0];
				row[x] = pix;
			}
		}
		(*env)->SetIntArrayRegion( env, jdata, 0, im.width , 
								(jint*)((*im.data) + im.bytesPerLine * nrow) ) ;
				
#endif
		if(nrow == im.height - 1){
			myfree((void**)im.data);
			SetImageDefaults(&im);
		}
	}
}
	
JNIEXPORT jint JNICALL Java_ptutils_CGetImageWidth
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint n){
	if( n == -1 )
		return (jint)gl->pano.width;
	if( im.data != NULL )
		return (jint)gl->im[n].width;
	else
		return (jint)0;
	}

JNIEXPORT jint JNICALL Java_ptutils_CGetImageHeight
   (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint n){
	if( n == -1 )
		return (jint)gl->pano.height;
	if( im.data != NULL )
		return (jint)gl->im[n].height;
	else
		return (jint)0;
	}


JNIEXPORT jint JNICALL Java_ptutils_CGetImageFormat
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint n){
	if( n == -1 )
		return (jint)gl->pano.format;
	else if( n < gl->numIm )
		return (jint)gl->im[n].format;
	else
		return -1;
}

JNIEXPORT jdouble JNICALL Java_ptutils_CGetHfov
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint n){
	if( n == -1 )
		return (jdouble)gl->pano.hfov;
	else if( n < gl->numIm && n >= 0 )
		return (jdouble)gl->im[n].hfov;
	else
		return -1.0;
}


JNIEXPORT jdouble JNICALL Java_ptutils_CGetYaw
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint n){
	return (jdouble)gl->im[n].yaw;
}

JNIEXPORT jdouble JNICALL Java_ptutils_CGetPitch
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint n){
		return (jdouble)gl->im[n].pitch;
}

JNIEXPORT jdouble JNICALL Java_ptutils_CGetRoll
  (JNIEnv *env PT_UNUSED, jobject obj PT_UNUSED, jint n){
		return (jdouble)gl->im[n].roll;
}


JNIEXPORT void JNICALL Java_ptutils_CCreateProject
  (JNIEnv *env, jobject obj, jstring path, jint panoMap, jstring panofile, jint imageFormat, jint numIm, jdouble fLength){
	Image kim;
	int i;
	AlignInfo al; 
	
	const char *jpath = (*env)->GetStringUTFChars(env, path, 0), 
			  *jpath2 = (*env)->GetStringUTFChars(env, panofile, 0);

	SET_JAVA
	
	if( jpathTofullPath( jpath, &project ) != 0 ){
		PrintError("Could not create Path from %s", jpath);
		return;
	}
	(*env)->ReleaseStringUTFChars(env, path, jpath);
	
	SetImageDefaults( &kim );
	
	SetAlignInfoDefaults( &al );
	
	SetImageDefaults( &al.pano );
	al.pano.format = _panorama ;
	strcpy( al.pano.name, "PSD_mask" );
	al.im = &kim;
	

	al.numIm = numIm;
	kim.hfov = fLength;
	kim.format = imageFormat;
	al.pano.format = panoMap;
	strcpy(al.pano.name, jpath2 );
	(*env)->ReleaseStringUTFChars(env, panofile, jpath2);
		
	// Check for obvious nonsense
	if( al.numIm <= 0 || kim.hfov <= 0.0)
		return ;
		
	al.pano.hfov = 360.0;
	
	if( kim.format == _fisheye_ff && kim.hfov < 8.5 )
		kim.format = _fisheye_circ;
	
	al.im 		= (Image*)			malloc( al.numIm 	* sizeof(Image ) );
	al.opt		= (optVars*)		malloc( al.numIm 	* sizeof(optVars) );
	al.cim		= (CoordInfo*)		malloc( al.numIm 	* sizeof(CoordInfo) );

	if(al.im == NULL || al.opt == NULL || al.cim == NULL){
		PrintError("Not enough memory");
	}
	SetStitchDefaults(&(al.st));
	strcpy( al.st.srcName, "buf" );

	if( strcmp( al.pano.name, "PSD_mask" ) == 0 ){
		strcpy( al.st.destName, "buf" );
	}else{
		al.st.destName[0] = 0;
	}

	for(i=0; i<al.numIm; i++){
		SetOptDefaults	( &(al.opt[i]));
		SetImageDefaults( &al.im[i] );
		al.im[i].format = kim.format;
		if( al.im[i].format != _fisheye_circ ){
			//al.im[i].cP.horizontal = 1;
			//al.im[i].cP.vertical   = 1;
		}
		al.cim[i].x[0] = (double) i;
		al.cim[i].x[1] = al.cim[i].x[2] = 0.0;
		al.cim[i].set[0] = al.cim[i].set[1] = al.cim[i].set[2] = TRUE;
		al.im[i].hfov	= -kim.hfov; // Calculate later!
	}

	writeProject( &al, &project );
	DisposeAlignInfo( &al );
}
	

JNIEXPORT void JNICALL Java_ptutils_CTriangulate
  (JNIEnv *env, jobject obj, jint n){
	SET_JAVA
	
	TriangulatePoints( gl, n );
}
	
JNIEXPORT void JNICALL Java_ptutils_CReduce
  (JNIEnv *env, jobject obj, jint n){
	SET_JAVA
	
	ReduceTriangles( gl, n );
}

JNIEXPORT void JNICALL Java_ptutils_CCallOptimizer
  (JNIEnv *env, jobject obj){
	OptInfo					opt;
	char *script = NULL;
	SET_JAVA

#ifdef __Mac__
	setLibToResFile();
#endif

	BackUp();
 	script = LoadScript( &project );
		
	if( script == NULL ) {
		PrintError("Error reading script");
#ifdef __Mac__
		unsetLibToResFile();
#endif
		return ;
	}

	gl->fcn	= fcnPano;

	SetGlobalPtr( gl ); 
							
	opt.numVars 		= gl->numParam;
	opt.numData 		= gl->numPts;
	opt.SetVarsToX		= SetLMParams;
	opt.SetXToVars		= SetAlignParams;
	opt.fcn				= gl->fcn;
	*opt.message		= 0;


	RunLMOptimizer( &opt );
	gl->data				= opt.message;

	// Print Results
		
	// First Optimizer data
		
	WriteResults( script, &project, gl, distSquared ,0 );
	if( script ) free( script ); 
#ifdef __Mac__
	unsetLibToResFile();
#endif
	Restore();
}


JNIEXPORT void JNICALL Java_ptutils_CShowScript
  (JNIEnv *env, jobject obj){
	SET_JAVA
	showScript( &project );
}

JNIEXPORT void JNICALL Java_ptutils_CLaunchAndSendScript
  (JNIEnv *env, jobject obj,  jstring japp, jstring joutput){
	char *script = malloc( 512 * 2 + 100);
	char fname[512];
	char appPath[32];
	fullPath fspec;
	const char *output = (*env)->GetStringUTFChars(env, joutput, 0);
	const char *app = (*env)->GetStringUTFChars(env, japp, 0);

	SET_JAVA

	if( script == NULL ) return;

	if( output == NULL || strlen(output) == 0 ){
		script[0] = 0;
	}else{
		jpathTofullPath( output, &fspec );

		FullPathtoString( &fspec, fname );
		sprintf(script,"-o \"%s\" ", fname);
	}
	(*env)->ReleaseStringUTFChars(env, joutput, output);
						
	memcpy( &fspec, &project, sizeof( fullPath ));
						
	FullPathtoString( &fspec, fname );
	strcat(script, "\"");
	strcat(script,fname);
	strcat(script, "\" ");

	sprintf(appPath, "%s%s", HELPERS, app);
	LaunchAndSendScript( appPath, script);
	(*env)->ReleaseStringUTFChars(env, japp, app);
	if( script ) free( script );
}



int writeProject( AlignInfo *g, fullPath *pFile)
{
	fullPath tmp;
	file_spec fnum;
	int i;
	char line[512];
	long count;
	Image *im;
	controlPoint *c;
	triangle *t;
	optVars *o;
	CoordInfo *ci;
	int pf;
	char ch[256], cv[64];


	if( g == NULL ) return 0;

	setlocale(LC_ALL, "C");
	
	
	memcpy(&tmp, pFile, sizeof( fullPath ));

	p2cstr( tmp.name );
	strcat((char*)tmp.name, "_temp_");
	c2pstr((char*)tmp.name);

	mycreate( &tmp, 'ttxt', 'TEXT' );
	if( myopen( &tmp, write_text, fnum ) )
		return -1;
	
	if( g->pano.format == _equirectangular ) 
		pf = 2;
	else 
		pf = g->pano.format;	

	// Add command for stitcher, depending on g->st
	// If '-' option has been set (destName != 0), do nothing
	// else add stitch commands
	
	if(g->st.destName[0] != 0)
		sprintf(ch, "-%s", g->st.destName);
	else
		*ch=0;
		
	if(g->pano.cP.radial)
		sprintf(cv, "a%lg b%lg c%lg", 	g->pano.cP.radial_params[0][3],
										g->pano.cP.radial_params[0][2],
										g->pano.cP.radial_params[0][1] );
	else
		*cv=0;
		
		
	
	sprintf(line, "p f%d w%ld h%ld v%lg u%d %s n\"%s\" %s\n\n", pf, (long int) g->pano.width, (long int) g->pano.height, g->pano.hfov, g->st.feather, cv, g->pano.name, ch );
	count = strlen( line ); mywrite( fnum, count, line );
	
	for(i=0; i<g->numIm; i++)
	{
		im = &g->im[i];
		
		if( g->opt[i].hfov > 1 )
			sprintf(ch, "v=%d", g->opt[i].hfov - 2);
		else
			sprintf(ch, "v%lg", g->im[i].hfov);
			
		sprintf(line, "i f%ld w%ld h%ld y%lg p%lg r%lg %s %s n\"%s\" ", (long int) im->format, (long int) im->width, (long int) im->height, 
											im->yaw, im->pitch, im->roll, 
											(im->cP.correction_mode & correction_mode_morph ? "o" : "" ),
											ch, im->name );
		if(g->im[i].cP.radial)
		{
			if(g->opt[i].a > 1 )
				sprintf(ch, " a=%d", g->opt[i].a - 2);
			else
				sprintf(ch, " a%lg", g->im[i].cP.radial_params[0][3]);
			strcat(line, ch);
			
			if(g->opt[i].b > 1 )
				sprintf(ch, " b=%d", g->opt[i].b - 2);
			else
				sprintf(ch, " b%lg", g->im[i].cP.radial_params[0][2]);
			strcat(line, ch);
			
			if(g->opt[i].c > 1 )
				sprintf(ch, " c=%d", g->opt[i].c - 2);
			else
				sprintf(ch, " c%lg", g->im[i].cP.radial_params[0][1]);
			strcat(line, ch);
		}
		if(g->im[i].cP.horizontal)
		{
			sprintf(ch, " d%lg", g->im[i].cP.horizontal_params[0]);
			strcat(line, ch);
		}
		if(g->im[i].cP.vertical)
		{
			sprintf(ch, " e%lg", g->im[i].cP.vertical_params[0]);
			strcat(line, ch);
		}

		if( g->im[i].cP.correction_mode & correction_mode_morph )
		{
			strcat( line, "o " );
		}

		if( g->im[i].selection.bottom != 0 || g->im[i].selection.right != 0 ){
			sprintf( ch, " S%ld,%ld,%ld,%ld ",(long int) g->im[i].selection.left, (long int) g->im[i].selection.right,
						       (long int) g->im[i].selection.top, (long int) g->im[i].selection.bottom );
			strcat(line, ch);
		}

		if(g->cim[i].set[0])
		{
			sprintf(ch, " X%lg", g->cim[i].x[0]);
			strcat(line, ch);
		}
		if(g->cim[i].set[1])
		{
			sprintf(ch, " Y%lg", g->cim[i].x[1]);
			strcat(line, ch);
		}
		if(g->cim[i].set[2])
		{
			sprintf(ch, " Z%lg", g->cim[i].x[2]);
			strcat(line, ch);
		}
		
		strcat(line, "\n");
		count = strlen( line ); mywrite( fnum, count, line );
	}

	// Print v-lines
	for(i=0; i<g->numIm; i++)
	{
		char ch[8];

		strcpy(line, "v ");
		
		o = &g->opt[i];
		ci = &g->cim[i];
		
		if( o->yaw == 1 )
		{
			sprintf(ch, "y%d ", i );  strcat(line, ch);
		}
		if( o->pitch == 1 )
		{
			sprintf(ch, "p%d ", i );  strcat(line, ch);
		}
		if( o->roll == 1 )
		{
			sprintf(ch, "r%d ", i );  strcat(line, ch);
		}
		if( o->hfov == 1 )
		{
			sprintf(ch, "v%d ", i );  strcat(line, ch);
		}
		if( o->a == 1 )
		{
			sprintf(ch, "a%d ", i );  strcat(line, ch);
		}
		if( o->b == 1 )
		{
			sprintf(ch, "b%d ", i );  strcat(line, ch);
		}
		if( o->c == 1 )
		{
			sprintf(ch, "c%d ", i );  strcat(line, ch);
		}
		if( o->d == 1 )
		{
			sprintf(ch, "d%d ", i );  strcat(line, ch);
		}
		if( o->e == 1 )
		{
			sprintf(ch, "e%d ", i );  strcat(line, ch);
		}
		
		if( !ci->set[0] )
		{
			sprintf(ch, "X%d ", i );  strcat(line, ch);
		}
		if( !ci->set[1] )
		{
			sprintf(ch, "Y%d ", i );  strcat(line, ch);
		}
		if( !ci->set[2] )
		{
			sprintf(ch, "Z%d ", i );  strcat(line, ch);
		}
		
		strcat(line,"\n");
		count = strlen( line ); mywrite( fnum, count, line );
	}
					
	for(i=0; i<g->numPts; i++)
	{
		c = &g->cpt[i];
		sprintf(line, "c n%d N%d x%lg y%lg X%lg Y%lg \n", c->num[0], c->num[1], c->x[0], c->y[0], c->x[1], c->y[1]);
		count = strlen( line ); mywrite( fnum, count, line );
	}
	for(i=0; i<g->nt; i++)
	{
		t = &g->t[i];
		sprintf(line, "t %d %d %d i%d\n", t->vert[0], t->vert[1], t->vert[2], t->nIm );
		count = strlen( line ); mywrite( fnum, count, line );
	}

	strcat(mLine, "\n"); 
	count = strlen( mLine ); mywrite( fnum, count, mLine );
			
	
	
	myclose( fnum );
	mydelete( pFile );
	myrename( &tmp, pFile );
	return 0;
}


int loadProject( fullPath *fspec ){
	char *script=NULL;
		
	script = LoadScript( fspec );
	
	if( script == NULL ){
		PrintError("Could not read project file");
		return -1;
	}
	
	if( gl != NULL ){
		DisposeAlignInfo( gl );
		free( gl );
	}

	gl = (AlignInfo*)malloc( sizeof( AlignInfo ) );
	if( gl == NULL ) return -1;
	
	SetAlignInfoDefaults( gl );
		
	if( ParseScript( script, gl ) != 0 ){
		PrintError("Could not parse project file");
		return -1;
	}

	ReadMLine( script, mLine );

	if( script ) free( script );
	return 0;
}	

void ReadMLine( char *script, char *m )
{
	char *c = script;
	int i=0;
	
	while(*c == '\n' && *c != 0) c++;
	
	while( *c != 0 )
	{
		c++;
		if( *c == 'm')
		{
			while(*c!= '\n' && *c!=0 && i<250)
						m[i++] = *c++;
			 m[i] = 0;
			return;
		}
		while(*c!=0 && *c!='\n')c++;
	}
}


int jpathTofullPath( const char* jpath, fullPath *fp ){
	int length = strlen(jpath);
	char *cpath = malloc( length + 1), *c;
	int i, result = 0;
	strcpy( cpath, jpath );
  	for(i=0; i<length; i++){
		if(cpath[i] == '/')
			cpath[i] = PATH_SEP ;
	}
#ifdef __Mac__
	c = cpath + 1;
	if( /*StringtoFullPath*/GetFullPath( fp, c) != 0 )//Kekus Digital
#else
	c = cpath;
	if( StringtoFullPath( fp, c) != 0 )
#endif
	{
		result = -1;
	}
	free( cpath );
	return result;
}

void BackUp()
{
	int i;
	
	if( theBackUp != NULL )
		free( theBackUp );
		
	theBackUp = malloc( gl->numIm * sizeof( Image ));
	if( theBackUp == NULL )
		return;
		
	for(i=0; i<gl->numIm; i++)
	{
		memcpy( &((Image*)theBackUp)[i], &gl->im[i], sizeof( Image ));
	}
	return;
}

void Restore(){
	int i;
	
	if( theBackUp == NULL )
		return;
		
		
	for(i=0; i<gl->numIm; i++){
		memcpy( &gl->im[i], &((Image*)theBackUp)[i], sizeof( Image ));
	}
	return;
}

	
void SetAlignInfoDefaults( AlignInfo *al){
	al->numIm 	= 0;
	al->numPts 	= 0;
	al->nt 		= 0;
	al->numParam= 0;
	al->im		= NULL;
	al->opt		= NULL;
	al->cpt		= NULL;
	al->t		= NULL;
	al->cim		= NULL;
}

static TrformStr *pc_Tr;
static Image *pc_reg;
static fDesc *pc_fDesc;
static aPrefs *pc_adj;
static struct size_Prefs *pc_sp;

int	pc_SetXtoVars( double *x ){
	pc_adj->im.cP.horizontal_params[2] =pc_adj->im.cP.horizontal_params[1] =
	pc_adj->im.cP.horizontal_params[0] =x[2];//-params[0];
	pc_adj->im.cP.vertical_params[2] =pc_adj->im.cP.vertical_params[1] =
	pc_adj->im.cP.vertical_params[0] =x[3];//-params[1];
	pc_adj->im.hfov=x[0]/100.0;
	pc_adj->im.roll=x[1];

#if 0	
	double *p = (double*) pc_fDesc->param;

	p[0] = x[3];// shift_x
	p[1] = x[2];// shift_y
	p[2] = x[1] / 100.0; // scale
	p[5] = x[0] * PI / 180.0; // phi
	p[3] = cos(p[5]); // cos_phi
	p[4] = sin(p[5]); // sin_phi
#endif	
	//PrintError("shift_x = %lg, shift_y = %lg, scale = %lg, Phi = %lg",
	//			p[0] ,p[1],p[2],p[5] *180.0/PI );  
	return 0;
}

int	pc_SetVarsToX( double *x ){
	x[2] = pc_adj->im.cP.horizontal_params[0];//-params[0];
	x[3] = pc_adj->im.cP.vertical_params[0];//-params[1];
	x[0] = pc_adj->im.hfov * 100.0;
	x[1] = pc_adj->im.roll;
#if 0
	double *p = (double*) pc_fDesc->param;

	x[3] = p[0];// shift_x
	x[2] = p[1];// shift_y
	x[1] = p[2] * 100.0; // scale
	x[0] = p[5] * 180.0 / PI; // phi
#endif	
	return 0;
}


int fcnAlign(int m PT_UNUSED, int n PT_UNUSED, double x[], double fvec[],int *iflag)
{
	// int i;
	double r = 0.0;
	static int numIt,a=0;
	
	if( *iflag == -100 ){ // reset
		numIt = 0;
		// infoDlg ( _initProgress, "Optimizing Variables" );
		return 0;
	}
	if( *iflag == -99 ){ // 
		//infoDlg ( _disposeProgress, "" );
		return 0;
	}


	if( *iflag == 0 ){
		char message[256];
		
		sprintf( message, "Average Difference between Pixels \nafter %d iteration(s): %g ", numIt,sqrt(fvec[0]/(3*16*16)));
		numIt ++;a=0;
		//if( !infoDlg ( _setProgress,message ) ) *iflag = -1;
		return 0;
	}
		
	
	// Set Parameters
	pc_SetXtoVars(x);
	
	filter_main( pc_Tr, pc_sp );
	//transForm( pc_Tr, pc_fDesc, 0);

	{
		Image *a = pc_Tr->dest, *b = pc_reg;
		unsigned char *adata, *bdata;
		int x,y,cy;
		int c1,c2,c3;
		
		for(y=0; y<a->height; y++){
			cy = y * a->bytesPerLine;
			for(x=0, adata = *a->data+cy, bdata = *b->data+cy;
			    x<a->width; x++, adata+=4, bdata+=4){
				if( *adata != 0 && *bdata != 0 ){
					c1 = ((int)(adata[1])) - ((int)(bdata[1]));
					c2 = ((int)(adata[2])) - ((int)(bdata[2]));
					c3 = ((int)(adata[3])) - ((int)(bdata[3]));
					r += c1*c1 + c2*c2 + c3*c3;
				}else{
					r += 3.0 * 255.0 * 255.0;
				}
			}
		}
	}
	fvec[0]= r;
	fvec[1]= r;
	fvec[2]= r;
	fvec[3]= r;

	return 0;
}


JNIEXPORT void JNICALL Java_ptutils_CAlignPoint
  (JNIEnv *env, jobject obj, jdoubleArray jX, jintArray jreg, jintArray jseek){
	OptInfo				opt;
	int					w_reg, h_reg, w_seek, h_seek;
	Image				src, dst, reg;
	jdouble *X; 
	jint  *pix_reg, *pix_seek;
	fDesc pc_fD;
	double params[6];
	struct size_Prefs spref;
	
	pc_sp = &spref;

	SET_JAVA
	JavaUI = FALSE;

#ifdef __Mac__
	setLibToResFile();   // MacOS: Get resources from shared lib
#endif	

	X 			= (*env)->GetDoubleArrayElements(env, jX, 0);
	pix_reg 	= (*env)->GetIntArrayElements(env, jreg, 0);
	pix_seek 	= (*env)->GetIntArrayElements(env, jseek, 0);

	w_reg 	= h_reg 	= (int)(sqrt((double) (*env)->GetArrayLength(env, jreg))+0.5);	
	w_seek 	= h_seek 	= (int)(sqrt((double) (*env)->GetArrayLength(env, jseek))+0.5);	
	
	pc_Tr 	= (TrformStr *) malloc(sizeof(TrformStr));
  memset(pc_Tr, 0, sizeof(TrformStr));
	pc_reg 	= &reg;
	pc_adj	= (aPrefs*) malloc(sizeof(aPrefs));
	SetAdjustDefaults(pc_adj);
	
	SetImageDefaults(&src);
	src.width 	= w_seek;
	src.height 	= h_seek;
	src.bitsPerPixel = 32;
	src.bytesPerLine = 4 * src.width;
	src.dataSize = src.bytesPerLine * src.height;
	src.data = (unsigned char**)(&pix_seek);
	
	SetImageDefaults(&dst);
	dst.width 	= w_reg;
	dst.height 	= h_reg;
	dst.bitsPerPixel = 32;
	dst.bytesPerLine = 4 * dst.width;
	dst.dataSize = dst.bytesPerLine * dst.height;
	dst.data = (unsigned char**)mymalloc(dst.dataSize);
	
	SetImageDefaults(&reg);
	reg.width 	= w_reg;
	reg.height 	= h_reg;
	reg.bitsPerPixel = 32;
	reg.bytesPerLine = 4 * reg.width;
	reg.dataSize = reg.bytesPerLine * reg.height;
	reg.data = (unsigned char**)(&pix_reg);


	pc_fDesc = &pc_fD;
	params[0] = 0.0; // shift_x
	params[1] = 0.0; // shift_y
	params[2] = 1.0; // scale
	params[5] = 0.0; // phi
	params[3] = cos(params[5]); // cos_phi
	params[4] = sin(params[5]); // sin_phi
	
	SetDesc(pc_fD, shift_scale_rotate, params);
	

	pc_adj->mode = _insert;
	
	memcpy( &pc_adj->im, &src, sizeof(Image) );
	pc_adj->im.format = _rectilinear;
	pc_adj->im.hfov	  = ((double)w_seek)/100.0;
	pc_adj->im.cP.horizontal = 1;
	pc_adj->im.cP.vertical = 1;
	
	memcpy( &pc_adj->pano, &dst, sizeof(Image));
	pc_adj->pano.format = _rectilinear;
	pc_adj->pano.hfov = ((double)w_reg)/100.0;

	if( readPrefs( (char*) pc_sp, _sizep ) != 0 )
		SetSizeDefaults	( pc_sp );
	
	
	pc_Tr->src 	= &src;
	pc_Tr->dest	= &dst;
	
	pc_Tr->tool					= _adjust;
	pc_Tr->mode					= _usedata + _destSupplied;
	pc_Tr->interpolator			= _spline36;
	pc_Tr->gamma				= 1.0;
    pc_Tr->fastStep             = FAST_TRANSFORM_STEP_NONE;
	pc_Tr->data					= (void*) pc_adj;

	opt.numVars 		= 4;
	opt.numData 		= 4;
	opt.SetVarsToX		= pc_SetVarsToX;
	opt.SetXToVars		= pc_SetXtoVars;
	opt.fcn				= fcnAlign;
	*opt.message		= 0;
	//RunLMOptimizer( &opt );
	
	//PrintError("Start");

	RunBROptimizer ( &opt, 1e-3);
	
	X[0] = pc_adj->im.cP.horizontal_params[0];//-params[0];
	X[1] = pc_adj->im.cP.vertical_params[0];//-params[1];

	if(pc_Tr)  free(pc_Tr);
	if(pc_adj) free(pc_adj);
	myfree((void**)dst.data);
	
	(*env)->ReleaseIntArrayElements(env, jseek, pix_seek, 0);
	(*env)->ReleaseIntArrayElements(env, jreg, pix_reg, 0);
	(*env)->ReleaseDoubleArrayElements(env, jX, X, 0);
	
#ifdef __Mac__
	unsetLibToResFile( );	
#endif	
	

}






