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


/*
    Modifications by Max Lyons (maxlyons@erols.com):

    March 4, 2002.  Changes made to mymalloc and myfree functions to
    work around a problem allocating more than 256MB of RAM from heap.
    See Microsoft Knowledge Base article Q198959 for more information.
    "PRB: Windows 95/98 Heaps Have A 255.9 MB Allocation Ceiling".

    This problem was causing "out of memory" errors when trying to stitch
    images that require more than 256MB of RAM (i.e. 64 megapixels)
    to be requested using the mymalloc function.

    March 6, 2002.  Added memory monitoring feature...
    This feature reports on memory allocation requests (i.e. chunks of memory
    that Pano Tools request from the operating system), and total memory
    usage (the sum of all currently allocated memory).
    This information is displayed on the progress dialog.


*/

/*------------------------------------------------------------*/

/* modified by Fulvio Senore June.2004

Added a call to disable warnings from the TIFF library: some tiff files (probably created by Photoshop)
caused annoying warnings about unknown tags while loading each tiff source image

*/

// Usefull MSVS tool to track down memory bugs when used with _CrtSetDbgFlag
// Gives increased information in output window
//#define CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

// FS+
// needed for TIFFSetWarningHandler()
#include "tiffio.h"
// FS-

#include "sys_win.h"
#include "direct.h"

#define CG_IDD_PROGRESS                 101
#define CG_IDS_PROGRESS_CAPTION         102
#define CG_IDC_PROGDLG_PROGRESS         1003
#define CG_IDC_PROGDLG_PERCENT          1004
#define CG_IDC_PROGDLG_MEM_REQUEST      1005
#define CG_IDC_PROGDLG_MEM_USAGE        1006


#define ID_PRG 100
#define ID_MM  100
#define ID_SM  101

#ifdef _DLL
HINSTANCE 	hDllInstance 	= NULL;
#endif
HWND 		wndOwner 		= NULL;
int 		dialogDone;
int		infoDone;

//Used to keep track of memory statistics
int         kBytesAlloced       =0;
int         maxKBytesAlloced    =0;
int         maxKBytesUsage      =0;

#if _MSC_VER > 1000
#pragma warning(disable: 4100) // disable unreferenced formal parameter warning
#endif

//------------------ Public functions required by filter.h -------------------------------


void SetWindowOwner( HWND Owner )
{
	wndOwner = Owner;
}

void filter_main( TrformStr *TrPtr, struct size_Prefs *spref)
{
	dispatch	( TrPtr, spref);
	
}

	

// Error reporting

void  PrintErrorIntern(char* fmt, va_list ap){

	char message[257];
	
	vsprintf(message, fmt, ap);
	
#ifdef HasJava
	if( JavaUI )
		JPrintError( message );
	else
#endif
//		MessageBox(GetFocus(), (LPSTR)message, (LPSTR)"", MB_OK | MB_ICONHAND) ;
		MessageBox((HWND)NULL, (LPSTR)message, (LPSTR)"Panorama Tools", MB_OK | MB_ICONHAND) ;
}	


// Progress report; return false if canceled


int ProgressIntern( int command, char* argument ){
	static HWND hwDlg = NULL;
	MSG	msg;
	long percent;	
	switch( command ){
		case _initProgress:
			if( hwDlg != NULL ){ // Update text and reset bar
				SetWindowText(hwDlg, argument);
				SendMessage( hwDlg, WM_COMMAND, ID_PRG, 0 );

				while( PeekMessage(&msg, 0,0,0, PM_REMOVE)){
					if( !IsDialogMessage( hwDlg, &msg ) ){	
						TranslateMessage(&msg); DispatchMessage(&msg);
					}
				}
				return TRUE;
			}
			hwDlg = CreateDialogParam( hDllInstance, 
				MAKEINTRESOURCE( 101 ), 0, DispPrg, (LPARAM)&dialogDone);
			if( hwDlg == NULL )
				PrintError( "Could not create Window" );
			else{
				dialogDone = FALSE;
				SetWindowText(hwDlg, argument);
				CenterDialog(hwDlg);
				ShowWindow(hwDlg, SW_SHOWNORMAL);
			}
			return TRUE;
		case _setProgress:
			if( (hwDlg != NULL)  && !dialogDone){
				ShowWindow(hwDlg, SW_SHOWNORMAL);
				sscanf(argument,"%ld", &percent);
				if(percent>100) percent = 100;
				if(percent<0  ) percent = 0;
				SendMessage( hwDlg, WM_COMMAND, ID_PRG, percent );

				while( PeekMessage(&msg, 0,0,0, PM_REMOVE)){
					if( !IsDialogMessage( hwDlg, &msg ) ){	
						TranslateMessage(&msg); DispatchMessage(&msg);
					}
				}
			}
			if(dialogDone){
				if( hwDlg != NULL ){
					DestroyWindow (hwDlg);
					hwDlg = NULL;
				}
				return FALSE; // User clicked cancel
			}

			return TRUE;
			break;
			
		case _disposeProgress:
			if( hwDlg != NULL )
			{
				DestroyWindow (hwDlg);
				hwDlg = NULL;
			}

			return TRUE;

		case _idleProgress:
			return TRUE;

	}
	return TRUE;
}


int infoDlgIntern ( int command, char* argument )	// Display info: same argumenmts as progress
{
	char 				text[256];
	static char			mainMessage[256];
	static HWND hwDlg;

	MSG	msg;


	switch( command )
	{
		case _initProgress:

			hwDlg = CreateDialogParam( hDllInstance, 
				MAKEINTRESOURCE( 105 ), 0, InfoPrg, (LPARAM)&infoDone);
			if( hwDlg == NULL )
				PrintError( "Could not create Window" );
			else
			{
				infoDone = FALSE;
				SetWindowText(hwDlg, argument);
				CenterDialog(hwDlg);
				ShowWindow(hwDlg, SW_SHOWNORMAL);
				SendMessage( hwDlg, WM_COMMAND, ID_MM, (LPARAM)argument );
			}
			return TRUE;
		case _setProgress:
			if( (hwDlg != NULL)  && !infoDone)
			{
				if( *argument != 0 )
				{
					if( *argument != '+' )
					{
						strcpy( mainMessage, argument );
						strcpy( text, argument );
					}
					else
					{
						sprintf( text,"%s%s", mainMessage, &(argument[1]) );
					}
					SendMessage( hwDlg, WM_COMMAND, ID_SM, (LPARAM)text );
					while( PeekMessage(&msg, 0,0,0, PM_REMOVE))
					{
						if( !IsDialogMessage( hwDlg, &msg ) )
						{	
							TranslateMessage(&msg); DispatchMessage(&msg);
						}
					}
				}
			}

			if(infoDone)
			{
				if( hwDlg != NULL )
				{
					DestroyWindow (hwDlg);
				}
				return FALSE; // User clicked cancel
			}

			return TRUE;
			break;
			
		case _disposeProgress:
			if( hwDlg != NULL )
			{
				DestroyWindow (hwDlg);
			}

			return TRUE;

		case _idleProgress:
			return TRUE;

	}
	return TRUE;
}

// Applications will use this path and name to store and retrieve preferences
BOOL GetPrefsFileName( char *prefname )
{
  char Modfname[_MAX_PATH], *name, *appData;
  char OldDir[_MAX_PATH];

  GetModuleFileName( NULL, Modfname, _MAX_PATH );
  name = strrchr( Modfname, '.' );
  if( name!= NULL )
  {
    *name = '\0';
  }
  name = strrchr( Modfname, '\\' );
  if( name!= NULL )
  {
    name++;
  }
  else
  {
    name = Modfname;
  }

  appData = getenv ("APPDATA");
  if (!appData )
  {
    appData = "c:";
  }

  sprintf( prefname, "%s\\PanoTools", appData, name );

  /* Get the current working directory: */
  if( getcwd( OldDir, _MAX_PATH ) != NULL )
  {
    // Test to see if the panotools folder exist
    if(-1 == chdir(prefname) )
    {
      // If not create the folder
      mkdir(prefname);
    }
    // go back from where we came
    chdir(OldDir);
  }

  // Add file name to path
  sprintf( prefname, "%s\\PanoTools\\%s.prf", appData, name );

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
	char  prefname[256];
	long  size;
	FILE *prfile;
	int   result = 0;

	GetPrefsFileName( prefname );

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
		struct size_Prefs			s;
		panControls					pc;
	} prf;

	FILE *prfile;
	char  prefname[256];

	GetPrefsFileName( prefname );

	if( (prfile = fopen( prefname, "rb" )) != NULL )
	{
		fread( &prf, sizeof(prf), 1 , prfile);
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
		fwrite( &prf, sizeof(prf), 1 , prfile);
		fclose(prfile);
	}
  else
  {
    PrintError( "Could not save settings to file %s", prefname);
  }
}


/**
 *
 * Function counts the amount of memory that has been virtualAlloc'd by this process
 */
DWORD countMemUsage()
{
    MEMORY_BASIC_INFORMATION mbi;
    DWORD      dwMemUsed = 0;
    PVOID      pvAddress = 0;
    CHAR      szBuf[512];

    memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));

    while(pvAddress < (PVOID)0x80000000 && VirtualQuery(pvAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
        {
        if(mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE && (!GetModuleFileNameA((HINSTANCE)mbi.AllocationBase, szBuf, 512)))
        dwMemUsed += mbi.RegionSize;
        pvAddress = ((BYTE*)mbi.BaseAddress) + mbi.RegionSize;
        }

    return dwMemUsed/1024;
}



#define signatureSize	4

void**  mymalloc( size_t numBytes )					// Memory allocation, use Handles
{
	LPSTR *mHand;
	char *p;
	static char cSig[signatureSize] = {'O', 'T', 'O', 'F'};

	mHand =	(LPSTR *) GlobalAllocPtr (GHND, (sizeof (LPSTR *) + signatureSize));

	if (mHand)
        //*mHand = (LPSTR) GlobalAllocPtr (GHND, numBytes);

        //Use VirtualAlloc to work around Win 95/98/ME memory allocation problems
        //See Microsoft KB article Q198959
		*mHand = (LPSTR) VirtualAlloc (NULL, numBytes, MEM_COMMIT, PAGE_READWRITE);

	if (!mHand || !(*mHand))
		{
		PrintError("Error allocating %ld KB of memory", numBytes/1024);
		return NULL;
		}

	kBytesAlloced = numBytes/1024;
    if (kBytesAlloced>maxKBytesAlloced) maxKBytesAlloced = kBytesAlloced;

	// put the signature after the pointer
	p = (char *) mHand;
	p += sizeof (Handle);
	memcpy (p,cSig, signatureSize);

	GlobalLock(mHand);

	return (void**)mHand;
		
}

void 	myfree( void** Hdl )						// free Memory, use Handles
{
	if (Hdl)
	{
		LPSTR p;
		
		GlobalUnlock((LPSTR *) Hdl);

		p = *((LPSTR*) Hdl);

        //Use VirtualFree because of Win 95/98/ME memory allocation problems
        //See Microsfot KB article Q198959
		if (p)
            VirtualFree(p, 0, MEM_RELEASE);

		GlobalFreePtr ((LPSTR) Hdl);
	}
}







// Display Scriptfile using plain text editor

void showScript( fullPath* scriptFile )
{
	char cmd[ MAX_PATH_LENGTH + 32 ];
	sprintf( cmd, "Notepad \"%s\"", scriptFile->name );
	WinExec( cmd , SW_SHOWNORMAL );
}

	

void CenterDialog(HWND hDlg)
{
    HWND hParent;
	int  nHeight;
    int  nWidth;
    int  nTitleBits;
    RECT rcDialog;
    RECT rcParent;
    int  xOrigin;
    int  yOrigin;
    int  xScreen;
    int  yScreen;

	hParent = GetParent(hDlg);
    if  (hParent == NULL)
        hParent = GetDesktopWindow();

    GetClientRect(hParent, &rcParent);
    ClientToScreen(hParent, (LPPOINT)&rcParent.left);  // point(left,  top)
    ClientToScreen(hParent, (LPPOINT)&rcParent.right); // point(right, bottom)

    // Center on Title: title bar has system menu, minimize,  maximize bitmaps
    // Width of title bar bitmaps - assumes 3 of them and dialog has a sysmenu
    nTitleBits = GetSystemMetrics(SM_CXSIZE);

    // If dialog has no sys menu compensate for odd# bitmaps by sub 1 bitwidth
    if  ( ! (GetWindowLong(hDlg, GWL_STYLE) & WS_SYSMENU))
        nTitleBits -= nTitleBits / 3;

    GetWindowRect(hDlg, &rcDialog);
    nWidth  = rcDialog.right  - rcDialog.left;
    nHeight = rcDialog.bottom - rcDialog.top;

    xOrigin = max(rcParent.right - rcParent.left - nWidth, 0) / 2
            + rcParent.left - nTitleBits;
    xScreen = GetSystemMetrics(SM_CXSCREEN);
    if  (xOrigin + nWidth > xScreen)
        xOrigin = max (0, xScreen - nWidth);

	yOrigin = max(rcParent.bottom - rcParent.top - nHeight, 0) / 3
            + rcParent.top;
    yScreen = GetSystemMetrics(SM_CYSCREEN);
    if  (yOrigin + nHeight > yScreen)
        yOrigin = max(0 , yScreen - nHeight);

   SetWindowPos(hDlg, NULL, xOrigin, yOrigin, nWidth, nHeight, SWP_NOZORDER);
}

int getConfirmation(HWND hWnd, char *message)
{
	return (MessageBox(hWnd, (LPSTR)message, (LPSTR)"Panorama Tools", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_APPLMODAL));
}


INT_PTR CALLBACK DispPrg(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)       // Win32 Change
{
	int			item, cmd;
	// short			numberErr = 0;
	char			message[128];
	char			bar[128];
	long			percent;
	int i;
    int             memUsage;

	static int *dDone;

	switch  (wMsg)
	{

		case  WM_INITDIALOG:
			dDone = (int*)lParam;

			/* drop into PAINT */
		case WM_PAINT:
			return FALSE;
			break;

		case  WM_COMMAND:
	  		item = LOWORD(wParam);              // WIN32 Change
			cmd = HIWORD (wParam);

			switch  (item)
			{
				case IDCANCEL:
					if (cmd == BN_CLICKED)
					{
						if (getConfirmation(hDlg, "Are you sure you want to cancel?")==IDYES)
							{
						*dDone = TRUE;
						return TRUE;
					}
					}
					break;
							 

				case ID_PRG:
					sprintf(message,"Progress: %ld %%",(long)lParam);
					SetDlgItemText( hDlg, CG_IDC_PROGDLG_PERCENT, message); // stuff string

					//Show some statistics on memory allocation requests
					sprintf(message,"Memory Requests (Last/Max): %.1lf / %.1lf MB", (double)kBytesAlloced/1024.0, (double)maxKBytesAlloced/1024.0);
                    SetDlgItemText( hDlg, CG_IDC_PROGDLG_MEM_REQUEST, message); // stuff string

					//Show some statistics on current/peak memory usage
                    memUsage = countMemUsage();
                    if (memUsage > maxKBytesUsage) maxKBytesUsage = memUsage;

					sprintf(message,"Memory Usage (Current/Max): %.1lf / %.1lf MB", (double)memUsage/1024.0, (double)maxKBytesUsage/1024.0);
					SetDlgItemText( hDlg, CG_IDC_PROGDLG_MEM_USAGE, message); // stuff string

					percent = (long)lParam; 
					if( percent > 100 ) 
						percent = 100;
					
					for(i=0; i<percent; i++)
						bar[i] = '|';
					bar[percent] = 0;
					SetDlgItemText( hDlg, CG_IDC_PROGDLG_PROGRESS, bar); // stuff string

					break;
				default:
					break;
			}

			break; // case WM_COMMAND
		default:
			return FALSE;
			break;
		} // switch

	return  TRUE;
}

INT_PTR CALLBACK InfoPrg(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)       // Win32 Change
{
	int				item, cmd;
	// short			numberErr = 0;
	static int *dDone;

	switch  (wMsg)
	{

		case  WM_INITDIALOG:
			dDone = (int*)lParam;

			/* drop into PAINT */
		case WM_PAINT:
			return FALSE;
			break;

		case  WM_COMMAND:
	  		item = LOWORD(wParam);              // WIN32 Change
			cmd = HIWORD (wParam);

			switch  (item)
			{
				case IDCANCEL:
					if (cmd == BN_CLICKED)
					{
						if (getConfirmation(hDlg, "Are you sure you want to cancel?")==IDYES)
							{
						*dDone = TRUE;
						return TRUE;
					}
					}
					break;
				case ID_MM:
					SetDlgItemText( hDlg, 10, (char*)lParam); // stuff string
					break;
				case ID_SM:
					SetDlgItemText( hDlg, 11, (char*)lParam); // stuff string
					break;
				default:
					break;
			}

			break; // case WM_COMMAND
		default:
			return FALSE;
			break;
		} // switch

	return  TRUE;
}


// Every 32-Bit DLL has an entry point DLLInit
#ifdef _DLL
BOOL APIENTRY DLLInit(HANDLE hInstance, DWORD fdwReason, LPVOID lpReserved)
{

	if (fdwReason == DLL_PROCESS_ATTACH)
		hDllInstance = (HINSTANCE)hInstance;

	return TRUE;   // Indicate that the DLL was initialized successfully.
}
#endif



int 	FindFile( fullPath *fname )
{
	OPENFILENAME 	f;
	char			fName[256];
	char 			zTitle[64];
	int 			nresult;

	memset(&f, 0, sizeof(f)); // initialize structure to 0/NULL
	fName[0] = '\0';
	zTitle[0] = '\0';

	f.lStructSize 		= sizeof(f);
	f.lpstrFile 		= fName;
	f.nMaxFile 		= 256;
	f.lpstrDefExt 		= "";
	f.lpstrFileTitle 	= (LPTSTR)zTitle;
	f.nMaxFileTitle 	= 256;//64;
	f.hInstance 		= hDllInstance;
	f.hwndOwner 		= wndOwner;

	nresult = GetOpenFileName(&f);
	if( nresult )
	{
		strcpy( fname->name, fName );
		return 0;
	}
	else
		return -1;
}



int 	SaveFileAs			( fullPath *fname, char *prompt, char *name )
{
	OPENFILENAME 	f;
	char			fName[sizeof(fname->name)];
	int 			nresult;

	memset(&f, 0, sizeof(f)); // initialize structure to 0/NULL
	strcpy(fName, name );

	f.lStructSize 		= sizeof(f);
	f.lpstrFile 		= fName;
	f.nMaxFile 		= 256;
	f.lpstrDefExt 		= "";//prompt;
	f.lpstrTitle		= prompt;
	f.hInstance 		= hDllInstance;
	f.hwndOwner 		= wndOwner;

	nresult = GetSaveFileName(&f);
	if( nresult )
	{
		strcpy( fname->name, fName );
		return 0;
	}
	else
		return -1;
}


void makePathForResult	( fullPath *path )
{
	strcpy( path->name, "C:\\ptool_result" );
}

int makePathToHost ( fullPath *path )
{
	GetModuleFileName( NULL, path->name, MAX_PATH_LENGTH );
	return 0;
}




// Fname is appended to host-directory path

void MakeTempName( fullPath *destPath, char *fname )
{
	char  sname[256];
	char *temp_dir;

	sprintf( sname, "pano13.%s", fname );

  temp_dir = getenv ("TEMP");
	if (!temp_dir )
  {
    temp_dir = getenv ("TMP");
	  if (!temp_dir )
    {
      temp_dir = "c:\\";
    }
  }

  //GetModuleFileName( NULL, path, 256 );
	GetShortPathName( temp_dir, destPath->name, 256 );

	if( strrchr( destPath->name, '\\' ) != NULL )
		sprintf( strrchr( destPath->name, '\\' )+1, "%s", sname );
	else
		sprintf( destPath->name, "C:\\%s", sname );
}

void ConvFileName( fullPath *fspec,char *string)
{
	strcpy( string, fspec->name );
}

int FullPathtoString (fullPath *path, char *filename)
{
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
//	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	if( strrchr( fname, '.' ) != NULL && 
			(strcmp( strrchr( fname, '.' ), ".txt") == 0 ||
			 strcmp( strrchr( fname, '.' ), ".TXT") == 0)	)
	return TRUE;
	

	return FALSE;
}

int LaunchAndSendScript(char* application, char* script){
	char *cmd;
	
	cmd = (char*) malloc( strlen(application) + strlen(script) + 16 );
	
	if( cmd == NULL )
	{
		PrintError("Not enough memory");
		return -1;
	}
	sprintf( cmd , "%s %s", application, script );
	if(strlen(cmd)<256){
		WinExec( cmd , SW_SHOWNORMAL );
		return 0;
	}else{
		PrintError("Command too long for WinExec");
		free( cmd );
		return -1;
	}
}		

int 	StringtoFullPath	(fullPath *path, char *filename){
	if( strlen( filename ) < 256 ){
		strcpy( path->name, filename );
		return 0;
	}else{
		return -1;
	}
}

#if defined _USRDLL
// Only include the DLLMain with the dll build of pano13
BOOL WINAPI
DllMain (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
	    hDllInstance = (HINSTANCE)hDll;
           // Code to run when the DLL is loaded

			// FS+
			// disable warnings for unknown tags
			TIFFSetWarningHandler( 0 );
			// FS-

			break;

        case DLL_PROCESS_DETACH:
           // Code to run when the DLL is freed
            break;

        case DLL_THREAD_ATTACH:
            // Code to run when a thread is created during the DLL's lifetime
            break;

        case DLL_THREAD_DETACH:
            // Code to run when a thread ends normally.
            break;
    }
    return TRUE;
}
#endif
