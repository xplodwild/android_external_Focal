/* Panorama_Tools	-	Generate, Edit and Convert Panoramic Images
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

#include "sys_ansi.h"
#include "panotypes.h"
#include <signal.h>

//------------------ Public functions required by filter.h -------------------------------


// Required by filter.h but not used in the sys_ansi build of panotools.
// just return and do nothing
void SetWindowOwner(void * Owner) {return;}
void CenterDialog(void * hDlg) {return;}


void filter_main( TrformStr *TrPtr, struct size_Prefs *spref)
{
	dispatch	( TrPtr, spref);
	
}

	

// Error reporting

void  PrintErrorIntern(char* fmt, va_list ap)
{
	char message[512];
	char *toPrint;

	if (strlen(fmt) < 512) {
 	  vsprintf(message, fmt, ap);	  
	  toPrint = message;
	}   else {
	  // we don't have enough space, so just 
	  // print original string
	  toPrint = fmt;
	}

#ifdef HasJava
if( JavaUI ){
	  JPrintError( toPrint );
	}else
#endif
  {
	  printf("%s", toPrint);

	  // Add an end of line if none is provide
	  if (strlen(toPrint) > 0 &&
	      toPrint[strlen(toPrint)-1] != '\n') {
	      putchar('\n');
	  }
	  fflush(stdout);
	}
}


// Progress report; return false if canceled


int ProgressIntern( int command, char* argument )
{
	long percent;	

	switch( command ){
		case _initProgress:
			printf( "\n%s          ", argument );
			return TRUE;
		
		case _setProgress:
			sscanf(argument,"%ld", &percent);
			printf("\b\b\b\b%3ld%%", (long) percent);
			fflush (stdout);
			return TRUE;
			
		case _disposeProgress:
			printf("\n");
			return TRUE;
		case _idleProgress:
			return TRUE;
	}
	return TRUE;
}

volatile sig_atomic_t sigFlag;

void sigHandler(int sig PT_UNUSED){
	signal( SIGINT, sigHandler );
	sigFlag = 1;
}

int infoDlgIntern ( int command, char* argument ){
	static char	mainMessage[256];	
	// int reply;
	
	*mainMessage = 0;

	switch( command ){
		case _initProgress:
			signal( SIGINT, sigHandler );
			sigFlag = 0;
			printf( "%s\n", argument );
			return TRUE;
		case _setProgress:
			if( *argument != 0 ){
				if( *argument != '+' ){
					strcpy( mainMessage, argument );
					printf( "%s\n", argument );
				}else{
					printf( "%s%s", mainMessage, &(argument[1]) );
				}
				fflush (stdout);
			}
			//printf("\nContinue (c) or stop (s) ?\n");
			//reply = getchar();
			//if( reply == 's' )
			//	return FALSE;
			//return TRUE;
			if( sigFlag )
				return FALSE;
			return TRUE;
		case _disposeProgress:
			printf("\n");
			return TRUE;
		case _idleProgress:
			return TRUE;
	}
	return TRUE;
}


int readPrefs( char* pref, int selector )
{

	struct {
		char						v[sizeof(PREF_VERSION)];
		struct correct_Prefs		c;
		struct remap_Prefs			r;
		struct perspective_Prefs	p;
		struct adjust_Prefs			a;
		struct size_Prefs			s;
		panControls					pc;
	} prf;
	char* prefname = "pano13.prf";
	long size;

	FILE 	*prfile;
	int result = 0;


	if( (prfile = fopen( prefname, "rb" )) != NULL ){
		size = fread( &prf, 1, sizeof(prf),  prfile);
		fclose( prfile );
		
		if( size != sizeof(prf) ){
			result = -1;
		}else{
			switch( selector){
				case _version:
					memcpy( pref, &prf.v, sizeof( PREF_VERSION ) );
					break;
				case _correct:
					if( prf.c.magic != 20 ) 
						result = -1;
					else
						memcpy( pref, &prf.c, sizeof(struct correct_Prefs)); 
					break;
				case _remap:
					if( prf.r.magic != 30 ) 
						result = -1;
					else
						memcpy( pref, &prf.r , sizeof(struct remap_Prefs)); 
					break;
				case _perspective:
					if( prf.p.magic != 40 ) 
						result = -1;
					else
						memcpy( pref, &prf.p , sizeof(struct perspective_Prefs)); 
					break;
				case _adjust:
					if( prf.a.magic != 50 ) 
						result = -1;
					else
						memcpy( pref, &prf.a , sizeof(struct adjust_Prefs)); 
					break;
				case _sizep:
					if( prf.s.magic != 70 ) 
						result = -1;
					else
						memcpy( pref, &prf.s , sizeof(struct size_Prefs)); 
					break;
				case _panright:
				case _panleft:
				case _panup:
				case _pandown:
				case _zoomin:
				case _zoomout:
				case _apply:
				case _getPano:
				case _increment:
					memcpy( pref, &prf.pc , sizeof(panControls)); 
					break;
			}// switch
		} // sizes match
	}
	else
		result = -1;

	return result;
}





void writePrefs( char* prefs, int selector ){

	struct {
		char						v[sizeof(PREF_VERSION)];
		struct correct_Prefs		c;
		struct remap_Prefs			r;
		struct perspective_Prefs	p;
		struct adjust_Prefs			a;
		struct size_Prefs			s;
		panControls					pc;
	} prf;

	FILE 	*prfile;
	char* prefname = "pano13.prf";



	if( (prfile = fopen( prefname, "rb" )) != NULL ){
            int i;
            i = fread( &prf, sizeof(prf), 1 , prfile);
            if (i != sizeof(prf)) {
                PrintError("Unable to write to preference file [%s]\n", prefname);
            }
            fclose( prfile );
	}

	switch( selector){
		case _version:
			memcpy( &prf.v,  prefs, sizeof( PREF_VERSION ) );
			break;
		case _correct:
			memcpy( &prf.c , prefs, sizeof(struct correct_Prefs)); 
			break;
		case _remap:
			memcpy( &prf.r , prefs, sizeof(struct remap_Prefs)); 
			break;
		case _perspective:
			memcpy( &prf.p , prefs, sizeof(struct perspective_Prefs)); 
			break;
		case _adjust:
			memcpy( &prf.a , prefs, sizeof(struct adjust_Prefs)); 
			break;
		case _sizep:
			memcpy( &prf.s , prefs, sizeof(struct size_Prefs)); 
			break;
		case _panright:
		case _panleft:
		case _panup:
		case _pandown:
		case _zoomin:
		case _zoomout:
		case _apply:
		case _getPano:
		case _increment:
			memcpy( &prf.pc , prefs, sizeof(panControls)); 
			break;
	}
	
	if( (prfile = fopen( prefname, "wb" )) != NULL ){
		fwrite( &prf, sizeof(prf), 1 , prfile);
		fclose(prfile);
	}
}


#define signatureSize	4

void**  mymalloc( size_t numBytes )					// Memory allocation, use Handles
{
	char **mem;
	
	mem = (char**)malloc( sizeof(char*) );			// Allocate memory for pointer
	if(mem == NULL)
		return (void**)NULL;
	else
	{
		(*mem) = (char*) malloc( numBytes );		// Allocate numBytes
		if( *mem == NULL )
		{
			free( mem );
			return (void**)NULL;
		}
		else
			return (void**)mem;
	}
}

void 	myfree( void** Hdl )						// free Memory, use Handles
{
	free( (char*) *Hdl );
	free( (char**) Hdl );
}		

// Display Scriptfile using plain text editor

void 	showScript			( fullPath* scriptFile )
{
	char cmd[sizeof(fullPath) + 16];
	
	sprintf( cmd, "vi \"%s\"", scriptFile->name );
	if (system( cmd ) == -1) {
            PrintError("Unable to execute script editor");
        }
}
	

	

int 	FindFile( fullPath *fname ){
	printf("\n");
	printf("Load File:\n");
	if (scanf("%s", fname->name) != 1) {
            return -1;
        }
	
	if(strlen(fname->name) > 0)
		return 0;
	else
		return -1;
}

int 	SaveFileAs			( fullPath *fname, char *prompt PT_UNUSED, char *name PT_UNUSED){
	printf("\n");
	printf("Save File As:\n");
	if (scanf("%s", fname->name) != 1) {
            return -1;
        }
	
	if(strlen(fname->name) > 0)
		return 0;
	else
		return -1;
}


void makePathForResult	( fullPath *path ){
	strcpy( path->name, "//ptool_result" );
}

int makePathToHost ( fullPath *path ){
	strcpy(path->name, "./");
	return 0;
}




// Fname is appended to host-directory path

void MakeTempName( fullPath *destPath, char *fname ){
	sprintf( destPath->name, "pano13.%s", fname );
}

void ConvFileName( fullPath *fspec,char *string){
	strcpy( string, fspec->name );
}

int FullPathtoString (fullPath *path, char *filename){
	if( strlen( path->name ) < 256 )
	{
		strcpy( filename, path->name );
		return 0;
	}
	else
	{
		return -1;
	}
}

int IsTextFile( char* fname )
{

	if( strrchr( fname, '.' ) != NULL && 
			(strcmp( strrchr( fname, '.' ), ".txt") == 0 ||
			 strcmp( strrchr( fname, '.' ), ".TXT") == 0)	)
	return TRUE;
	

	return FALSE;
}

int LaunchAndSendScript(char *application, char *script){
	char *cmd = (char*)malloc( strlen(application) + strlen(script) + 16);
	if( cmd == NULL){
		PrintError("Not enough memory");
		return -1;
	}
	sprintf(cmd, "%s %s", application, script );
	if (system( cmd ) == -1) {
            PrintError("Unable to launch script");
        }
	free(cmd);
	return 0;
}

int StringtoFullPath(fullPath *path, char *filename){
	if(strlen( filename ) < 256 ){
		strcpy( path->name, filename);
		return 0;
	}else{
		return -1;
	}
}
