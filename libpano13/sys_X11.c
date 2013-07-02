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


#include "sys_x11.h"
#include <unistd.h>



int ptools_isOK;

//---------------- Callback functions


void pt_widget_destroy( GtkWidget *widget )
{
	ptools_isOK = FALSE;
	gtk_widget_destroy( widget );
}

void pt_main_destroy	(GtkWidget *widget)
{
	ptools_isOK = FALSE;
	gtk_main_quit();
}



void  pt_set_size_source(GtkWidget *widget, gpointer data)
{
	GtkWidget **dp;
    char text[32];
	
	dp = (GtkWidget**) data;

   sprintf( text, "%ld", gTrPtr->src->width );
   gtk_entry_set_text( GTK_ENTRY(dp[kSetPerspectivePrefs_Width]), text );
   sprintf( text, "%ld", gTrPtr->src->height );
   gtk_entry_set_text( GTK_ENTRY(dp[kSetPerspectivePrefs_Height]), text );
}







void filter_main( TrformStr *TrPtr, struct size_Prefs *spref)
{
	dispatch	( TrPtr, spref);
	
}



// Error reporting

void  PrintErrorIntern( char* fmt, va_list ap)
{
	char message[255];
	
	vsprintf(message, fmt, ap);
	
	gimp_message (message);
}

// Progress report; return false if canceled

int ProgressIntern( int command, char* argument )
{
	double percentage;

	switch( command )
	{
		case _initProgress:
			gimp_progress_init( argument );
			return TRUE;
			break;
		case _setProgress:
			sscanf(argument,"%lf", &percentage);
			gimp_progress_update ((gdouble) percentage/100.0);
			return TRUE;
			break;
		case _disposeProgress:
			percentage = 1.0;
			gimp_progress_update ((gdouble) percentage);
			return TRUE;
			break;
		case _idleProgress:
			return TRUE;
	}
	return TRUE;
}

int infoDlgIntern ( int command, char* argument )	// Display info: same argumenmts as progress
{
	if( command != _setProgress)
		return Progress( command, argument );
	else
		return Progress( command, "0.5" );
}



int readPrefs( char* pref, int selector )
{

	struct {
		char						v[sizeof(PREF_VERSION)];
		struct correct_Prefs		c;
		struct remap_Prefs			r;
		struct perspective_Prefs	p;
		struct adjust_Prefs			a;
		struct interp_Prefs			i;
		struct size_Prefs			s;
		panControls					pc;
	} prf;
	char *home_dir;
	char prefname[512];
	long size;

	FILE 	*prfile;
	int result = 0;

	home_dir = getenv ("HOME");
	if (!home_dir)
		return -1;
	sprintf (prefname, "%s/.gimp/pano13.prf", home_dir);


	if( (prfile = fopen( prefname, "rb" )) != NULL )
	{
		size = fread( &prf, 1, sizeof(prf),  prfile);
		fclose( prfile );
		
		if( size != sizeof(prf) )
		{
			result = -1;
		}
		else
		{
			switch( selector)
			{
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
				case _interpolate:
					if( prf.i.magic != 60 ) 
						result = -1;
					else
						memcpy( pref, &prf.i , sizeof(struct interp_Prefs)); 
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





void writePrefs( char* prefs, int selector )
{

	struct {
		char						v[sizeof(PREF_VERSION)];
		struct correct_Prefs		c;
		struct remap_Prefs			r;
		struct perspective_Prefs	p;
		struct adjust_Prefs			a;
		struct interp_Prefs			i;
		struct size_Prefs			s;
		panControls					pc;
	} prf;

	FILE 	*prfile;
	char *home_dir;
	char prefname[512];


	home_dir = getenv ("HOME");
	if (!home_dir)
		return ;
	sprintf (prefname, "%s/.gimp/pano13.prf", home_dir);

	if( (prfile = fopen( prefname, "rb" )) != NULL )
	{
		fread( &prf, 1, sizeof(prf),  prfile);
		fclose( prfile );
	}

	switch( selector)
	{
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
		case _interpolate:
			memcpy( &prf.i , prefs, sizeof(struct interp_Prefs)); 
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
	
	if( (prfile = fopen( prefname, "wb" )) != NULL )
	{
		fwrite( &prf, 1, sizeof(prf), prfile);
		fclose(prfile);
	}
}


void**  mymalloc( long numBytes )					// Memory allocation, use Handles
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


void 	showScript			( fullPath* scriptFile )
{
	char cmd[sizeof(fullPath) + 16];
	
	sprintf( cmd, "xedit \"%s\"", scriptFile->name );
	system( cmd );
}
	


void 	makePathForResult	( fullPath *path )
{
	char *home_dir;

	home_dir = getenv ("HOME");
	if (!home_dir || strlen( home_dir ) > sizeof( fullPath ) - 15 )
		home_dir = ".";
	sprintf (path->name, "%s/ptool_result", home_dir);
}	

void 	makePathToHost 		( fullPath *path )
{
	sprintf ( path->name, "gimp");
}



// Fname is appended to host-directory path

void MakeTempName( fullPath *destPath, char *fname )
{
	char *home_dir;

	home_dir = getenv ("HOME");
	if (!home_dir || strlen( home_dir ) > sizeof( fullPath ) - 64 )
		home_dir = ".";
	sprintf (destPath->name, "%s/.gimp/%s", home_dir,fname);
}




void ptool_save_callback		(GtkWidget *widget, gpointer data)
{
    GtkWidget *filew;
    static wdata wd;
	
    filew = gtk_file_selection_new ("Save corrections as...");
  	gtk_window_position (GTK_WINDOW (filew), GTK_WIN_POS_MOUSE);

	wd.data = data;
	wd.widg = filew;

     gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
			"clicked", (GtkSignalFunc) pt_save_corr, (gpointer) &wd );
    
    gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(filew)->cancel_button),
			       "clicked", (GtkSignalFunc) gtk_widget_destroy,
			       GTK_OBJECT (GTK_WINDOW (filew)));
    
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), "Corrections");

  	gtk_widget_show (filew);
}

void pt_save_corr				(GtkWidget *widget, gpointer data)
{
	FILE *cf;
    cPrefs *c;
    char *filename;
    
    c = (cPrefs*) ((wdata*)data)->data;
 	filename = gtk_file_selection_get_filename ((GtkFileSelection *)  ((wdata*)data)->widg );
  	if (! filename)
    	return;
	
	if( (cf = fopen( filename, "wb" )) != NULL )
	{
		fwrite( c, sizeof(cPrefs), 1 , cf);
		fclose( cf );
	}
	gtk_widget_destroy( ((wdata*)data)->widg ) ;
}    



void ptool_load_callback		(GtkWidget *widget, gpointer data)
{
    GtkWidget *filew;
   	static wdata wd;
		
    filew = gtk_file_selection_new ("Please find corrections file...");
  	gtk_window_position (GTK_WINDOW (filew), GTK_WIN_POS_MOUSE);

	wd.data = data;
	wd.widg = filew;

    /* Connect the ok_button to file_ok_sel function */
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
			"clicked", (GtkSignalFunc) pt_load_corr, (gpointer) &wd );

    
    /* Connect the cancel_button to destroy the widget */
    gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(filew)->cancel_button),
			       "clicked", (GtkSignalFunc) gtk_widget_destroy,
			       GTK_OBJECT (GTK_WINDOW (filew)));
    
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), "");

  	gtk_widget_show (filew);
}


void pt_load_corr				(GtkWidget *widget, gpointer data)
{
	FILE *cf;
    cPrefs *c, cP;
    char *filename;
	long size;
    
    c = (cPrefs*) ((wdata*)data)->data;
	filename = gtk_file_selection_get_filename ((GtkFileSelection *)  ((wdata*)data)->widg );
   	if (! filename)
    	return;
	
	if( (cf = fopen( filename, "rb" )) != NULL )
	{
		size = fread( &cP, 1, sizeof(cPrefs),  cf);
		fclose( cf );
		if( size == sizeof(cPrefs) )
		{
			memcpy((char*) c, (char*)&cP, sizeof(struct correct_Prefs));
		}
	}
	gtk_widget_destroy( ((wdata*)data)->widg );
}    


// Set file as specified by dialog
// data is *fullPath
void pt_find_file( GtkWidget *widget, gpointer data )
{
    GtkWidget *filew;
	static wdata wd;
 		
    filew = gtk_file_selection_new ("Find/Set file...");
  	gtk_window_position (GTK_WINDOW (filew), GTK_WIN_POS_MOUSE);

	wd.data = data;
	wd.widg = filew;

    /* Connect the ok_button to file_ok_sel function */
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
			"clicked", (GtkSignalFunc) pt_set_file, (gpointer)&wd );

    
    /* Connect the cancel_button to destroy the widget */
    gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(filew)->cancel_button),
			       "clicked", (GtkSignalFunc) gtk_widget_destroy,
			       GTK_OBJECT (GTK_WINDOW (filew)));
    
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), "");

  	gtk_widget_show (filew);
}

void pt_set_file				(GtkWidget *widget, gpointer data)
{
    fullPath *a;
	char* filename;
    
    a = (fullPath*) ((wdata*)data)->data;
	filename = gtk_file_selection_get_filename ((GtkFileSelection *)  ((wdata*)data)->widg );
   	if (! filename )
    	return;
	if( strlen(filename) > sizeof( fullPath )-1)
	{
		PrintError("Path too long");
		gtk_widget_destroy( ((wdata*)data)->widg );
		return;
	}
	
	strcpy( a->name, filename );

	gtk_widget_destroy( ((wdata*)data)->widg );
}    

	

void ConvFileName( fullPath *fspec,char *string)
{
	strcpy( string, fspec->name );
}
			
//--------------- Unused Functions -------------------------------

int FindFile( fullPath *fspec )	{ return 0;};
int SaveFileAs( fullPath *path, char *prompt, char *name )	{ return 0;};

