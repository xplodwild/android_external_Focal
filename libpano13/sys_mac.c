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

#include "sys_mac.h"



#define		kErrorAlertID			130
#define		kProgressDialog			110
#define		kProgressBar			2
#define		kInfoDialog				115



#define		PREFNAME		"\ppano.prefs"

//pascal OSErr 	__my_initialize(const CFragInitBlock *theInitBlock); //commented by Kekus Digital
//pascal void 	__my_terminate(void); //commented by Kekus Digital

pascal OSErr 	MyInitialize(CFBundleRef bundle); //added by Kekus Digital
pascal void 	MyTerminate(CFBundleRef bundle); //added by Kekus Digital

static void 	pstrcpy(unsigned char* from, unsigned char *to);
static void 	create_event_for_finder(AppleEvent *the_event);
static void 	add_path_name(AppleEvent * the_event_ptr,  const FSSpec* f);
static void 	add_selection(AppleEvent * the_event_ptr,  const FSSpec* f);





// These are set by library initialization routine

FSSpec 			panoLib, prFile;
long 			sleep = 10L;
short 			shlib = -1;


// Main entry point for panorama tools

void filter_main( TrformStr *TrPtr, sPrefs *spref)
{
	Handle			sleepHandle;

	// Check if lib has been opened as resource file	
	/*if( shlib == -1 ) //commented by Kekus Digital
	{
		TrPtr->success = 0;
		return;
	}*/

	// Set sleep value	
	sleepHandle = GetResource('SLEP', 128 );
	if( sleepHandle != nil )
	{
		HLock( sleepHandle );
		sleep = *(long*)*sleepHandle;
		HUnlock( sleepHandle );
		ReleaseResource( sleepHandle );
	//	PrintError( "%ld", sleep);
	}
	
	dispatch	( TrPtr, spref );
}


void setLibToResFile( void )
{
	//shlib		= FSpOpenResFile(&panoLib,   fsRdPerm);//commented by Kekus Digital
	shlib		= FSpOpenResFile(&prFile,   fsRdPerm);//added by Kekus Digital
}

void unsetLibToResFile( void )
{
	CloseResFile( shlib );
}


// Error reporting

void  PrintErrorIntern( char* fmt, va_list ap)
{
	char message[257];
	
	vsprintf(message, fmt, ap);
	
	if( JavaUI ){
		JPrintError( message );
		return;
	}
	
	ParamText( (unsigned char*) c2pstr( message ), '\0', '\0', '\0');
	StopAlert( kErrorAlertID, nil);
}



// Progress report; return false if canceled

int ProgressIntern( int command, char* argument )
{
	static GrafPtr 		port;			// Must all be static 
	static DialogPtr	dialog = nil;
	static Handle		barHandle;
	static Boolean		inProgress;
	EventRecord			event;
	Rect				itemRect;
	short 				itemHit, itemType;
	long percent;
	char title[256];

	//if( JavaUI ) return JProgress( command, argument );	
   
	switch( command )
	{
		case _initProgress:
            if( dialog != nil)
            {
					strcpy(  title, argument);
                SetWTitle( GetDialogWindow(dialog), (unsigned char*) c2pstr( title )); 
					return TRUE;
					break;
				}
				GetPort(&port);
				dialog = GetNewDialog( kProgressDialog , nil , (WindowPtr)-1L);
				strcpy(  title, argument);
            SetWTitle(GetDialogWindow(dialog), (unsigned char*)c2pstr(title)); 
            //SetThemeWindowBackground(GetDialogWindow(dialog), kThemeTextColorDialogActive, true);//Added by Kekus Digital
				ShowWindow( dialog );
            SetPort(/* dialog*/GetWindowPort(GetDialogWindow(dialog) )); //changed by Kekus Digital
				GetDialogItem( dialog, kProgressBar, &itemType, &barHandle, &itemRect);
				SetControlValue((ControlRef)barHandle, 0);
				inProgress = true;
				SetCursor(*GetCursor(watchCursor));
				return TRUE;
				break;		
		case _setProgress:
			if( inProgress )
			{
				sscanf(argument,"%ld", &percent);
				if(percent >100) 		percent = 100;
				if(percent < 0) 		percent = 0;
					SetControlValue((ControlRef)barHandle, (int) percent );
					if( WaitNextEvent ( everyEvent, &event, sleep, nil) )
					{
						if(IsDialogEvent(&event))
						{
							DialogSelect(&event,&dialog,&itemHit);
							if(itemHit == 1)
							{
								SetPort(port);
								DisposeDialog(dialog);
                                dialog = nil;
								inProgress = false;
								return FALSE;
							}
						}

						switch( event.what )
						{
							case keyDown:
							case autoKey:
								if( ((event.modifiers & cmdKey) != 0)  && ((event.message & charCodeMask) == '.') )
								{
									SetPort(port);
									DisposeDialog(dialog);
                                    dialog = nil;
									inProgress = false;
									return FALSE;
								}
								break;
							default:
								break;
						}
					}
			}
			return TRUE;
			break;
			
		case _disposeProgress:
			if( inProgress )
			{
					SetPort(port);
					DisposeDialog(dialog);
                    dialog = nil;
			}
			return TRUE;
			break;
		case _idleProgress:
			WaitNextEvent ( everyEvent, &event, sleep, nil);
			switch( event.what )
			{
				case keyDown:
				case autoKey:
				if( ((event.modifiers & cmdKey) != 0)  && ((event.message & charCodeMask) == '.') )
				{
					return FALSE;
				}
					break;
				default:
					break;
			}
			return TRUE;
                break;
	}
	return TRUE;
}



int infoDlgIntern ( int command, char* argument )	// Display info: same argumenmts as progress
{
	static GrafPtr 		port;			// Must all be static 
	static DialogPtr	dialog;
	static Handle		title, progress;
	static Boolean		inProgress;
	EventRecord			event;
	Rect				itemRect;
	short 				itemHit, itemType;
	char 				text[256];
	static char			mainMessage[256];						
	
	//if( JavaUI ) return JinfoDlg( command, argument );	

   
	switch( command )
	{
		case _initProgress:
			strcpy( text, argument );
			GetPort(&port);
			dialog = GetNewDialog( kInfoDialog , nil , (WindowPtr)-1L);
			GetDialogItem( dialog, 2, &itemType, &title, &itemRect);
			GetDialogItem( dialog, 3, &itemType, &progress, &itemRect);
			SetDialogItemText (title, 	c2pstr(text) );
			SetDialogItemText (progress, 	"\p");
                        //SetThemeWindowBackground(GetDialogWindow(dialog), kThemeTextColorDialogActive, true);//Added by Kekus Digital
			ShowWindow( dialog );
			SetPort( dialog );
			inProgress = true;
			return TRUE;
		
		case _setProgress:
			if( !inProgress )
				infoDlg ( _initProgress, "" );
			if( inProgress )
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
					SetDialogItemText (progress, 	c2pstr(text) );
				}
				if( WaitNextEvent ( everyEvent, &event, sleep, nil) )
				{
					if(IsDialogEvent(&event))
					{
						DialogSelect(&event,&dialog,&itemHit);
						if(itemHit == 1)
						{
							SetPort(port);
							DisposeDialog(dialog);
							inProgress = false;
							return FALSE;
						}
					}

					switch( event.what )
					{
						case keyDown:
						case autoKey:
						if( ((event.modifiers & cmdKey) != 0)  && ((event.message & charCodeMask) == '.') )
						{
							SetPort(port);
							DisposeDialog(dialog);
							inProgress = false;
							return FALSE;
						}
							break;
						default:
							break;
					}
				}
			}
			return TRUE;
			
		case _disposeProgress:
			if( inProgress )
			{
				SetPort(port);
				DisposeDialog(dialog);
				inProgress = FALSE;
			}
			return TRUE;
		case _idleProgress:
			WaitNextEvent ( everyEvent, &event, sleep, nil);
			switch( event.what )
			{
				case keyDown:
				case autoKey:
				if( ((event.modifiers & cmdKey) != 0)  && ((event.message & charCodeMask) == '.') )
				{
					return FALSE;
				}
					break;
				default:
					break;
			}
			return TRUE;
	}
	return TRUE;
}
			


int readPrefs( char* pref, int selector )
{
	short				prf;
	int					ResNumber, prefSize;
	unsigned char*   	prefName;
	int					result;
	Handle				prefHandle;



	switch( selector)
	{
		case _version:
			ResNumber = 134;
			prefSize  = sizeof( PREF_VERSION );
			prefName  = "\pVersion Info";
			break;
		case _correct:
			ResNumber 	= 128;
			prefSize  	= sizeof(struct correct_Prefs);
			prefName  	= "\pPreferences for correct";
			break;
		case _remap:
			ResNumber 	= 129;
			prefSize  	= sizeof(struct remap_Prefs);
			prefName  	= "\pPreferences for remap";
			break;
		case _perspective:
			ResNumber 	= 130;
			prefSize  	= sizeof(struct perspective_Prefs);
			prefName  	= "\pPreferences for perspective";
			break;
		case _adjust:
			ResNumber 	= 131;
			prefSize  	= sizeof(struct adjust_Prefs);
			prefName  	= "\pPreferences for adjust";
			break;
		case _sizep:
			ResNumber 	= 133;
			prefSize  	= sizeof(struct size_Prefs);
			prefName  	= "\pPreferences for size";
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
			ResNumber 	= 135;
			prefSize  	= sizeof(panControls);
			prefName  	= "\pPreferences for Pan Controls";
			break;
	}



	prf 				= FSpOpenResFile(&prFile,  fsRdPerm);
	
	if( prf == -1  )
		return -1;
		
		
	
	prefHandle 			= GetResource('PREF', ResNumber );
	if( prefHandle 		== nil )
	{
		result 			= -1;
	}
	else if( GetResourceSizeOnDisk( prefHandle )	!= prefSize)
	{
		ReleaseResource( prefHandle );
		result 			= -1;
	}
	else
	{
		HLock( prefHandle );
		memcpy( (char*)(pref), (char*)*prefHandle, prefSize);
		HUnlock( prefHandle );
		ReleaseResource( prefHandle );
		result 			= 0;
	}

	CloseResFile( prf );
	return result;
}


void writePrefs( char* pref, int selector )
{
	int		ResNumber;
	long	prefSize;
	unsigned char*   prefName;
	Handle	prefHandle;

	short	prf;

	switch( selector)
	{
		case _version:
			ResNumber = 134;
			prefSize  = sizeof( PREF_VERSION );
			prefName  = "\pVersion Info";
			break;
		case _correct:
			ResNumber = 128;
			prefSize  = sizeof(struct correct_Prefs);
			prefName  = "\pPreferences for correct";
			break;
		case _remap:
			ResNumber = 129;
			prefSize  = sizeof(struct remap_Prefs);
			prefName  = "\pPreferences for remap";
			break;
		case _perspective:
			ResNumber = 130;
			prefSize  = sizeof(struct perspective_Prefs);
			prefName  = "\pPreferences for perspective";
			break;
		case _adjust:
			ResNumber = 131;
			prefSize  = sizeof(struct adjust_Prefs);
			prefName  = "\pPreferences for adjust";
			break;
		case _sizep:
			ResNumber = 133;
			prefSize  = sizeof(struct size_Prefs);
			prefName  = "\pPreferences for size";
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
			ResNumber 	= 135;
			prefSize  	= sizeof(panControls);
			prefName  	= "\pPreferences for Pan Controls";
			break;
	}



	prf 		= FSpOpenResFile(&prFile,  fsRdWrPerm);

	if( prf == -1 )	// Error
		return;

	prefHandle = GetResource('PREF', ResNumber );
	if( prefHandle == nil ) // add resource
	{
		prefHandle = NewHandle( prefSize );
		AddResource ( prefHandle, 'PREF', ResNumber, prefName);
	}
	else if( GetResourceSizeOnDisk( prefHandle )	!= prefSize)
	{
		SetResourceSize	( prefHandle , prefSize );
		UpdateResFile( prf );
		SetHandleSize( prefHandle , prefSize );
	}

	HLock( prefHandle );
	memcpy(  (char*)*prefHandle, pref, prefSize);
	
	ChangedResource	( prefHandle );
	WriteResource	( prefHandle );
	HUnlock			( prefHandle );
	ReleaseResource	( prefHandle );
	CloseResFile( prf );


}


void**  mymalloc( long numBytes )
{
	Handle	mem;
	OSErr resultCode;

	mem = TempNewHandle(  numBytes, &resultCode);	
	if( mem == nil )
		return NULL;

	//MoveHHi( mem );	
	HLock( mem );
	return (void**) mem;
}

void myfree( void** Hdl )
{
	HUnlock( (Handle)Hdl );
	DisposeHandle( (Handle)Hdl );
}
		

// Display Scriptfile using plain text editor

void showScript( fullPath* scriptFile )
{
		open_selection( scriptFile );
}




//------------------ Some Private functions ---------------------------------------------
	
	

static void pstrcpy(unsigned char* from, unsigned char *to)
{
	register unsigned char *f, *t;
	register int i ;
	
	f = from; t = to;
	
	
	for(i=0; i <= (int) *from; i++)
		*t++ = *f++;
}


//-----------------------  Launcher ---------------------------------------------------



void open_selection( FSSpec *f)
{
	AppleEvent the_event, reply;
	

	create_event_for_finder( &the_event );
	add_path_name(&the_event,f);
	add_selection(&the_event,f);
	
	AESend(&the_event, &reply, kAENoReply+kAENeverInteract, 
				 kAENormalPriority, kAEDefaultTimeout, nil, nil) ;

	AEDisposeDesc(&the_event) ;
	AEDisposeDesc(&reply) ;
}


							// Create an 'open selection' event to be sent to Finder
static void create_event_for_finder(AppleEvent *the_event)
{
	const AEEventClass 	kAEFinderEvents = 'FNDR';		// Finder event
	const AEEventID 	kOpenSelection = 'sope';		// OpenSelection event
	const OSType 		kFinderSignature = 'MACS';		// Our destination
	AEAddressDesc 		finder_id;
	
	AECreateDesc(typeApplSignature,(void *)&kFinderSignature,sizeof(kFinderSignature),
					   &finder_id) ;

	AECreateAppleEvent(kAEFinderEvents,kOpenSelection,&finder_id,
							kAutoGenerateReturnID,
							kAnyTransactionID,
						    the_event) ;

	AEDisposeDesc(&finder_id) ;					// the_event is created, finder_id can
	  														// be disposed of now
}

									// Add the alias of file or path name to the event
									// Only the path field is considered
static void add_path_name(AppleEvent * the_event_ptr,  const FSSpec* f)
{
	AliasHandle alias;
	
	FSSpec dir_spec;
	FSMakeFSSpec(f->vRefNum,f->parID,"\p",&dir_spec) ;
	NewAliasMinimal(&dir_spec,&alias) ;
	HLock((char **)alias);
	//AEPutParamPtr(the_event_ptr, keyDirectObject, typeAlias, StripAddress(*alias), (**alias).aliasSize) ;//commented by Kekus Digital
	AEPutParamPtr(the_event_ptr, keyDirectObject, typeAlias, *alias, (**alias).aliasSize) ;//added by Kekus Digital

	HUnlock((char **)alias);
	DisposeHandle((char **)alias);
}

									// Create a "selection" item in the event from the
									// full file name specified 
static void add_selection(AppleEvent * the_event_ptr,  const FSSpec* f)
{
	AliasHandle alias;
	AEDescList selection_list;
	
	NewAliasMinimal( f,&alias) ;

	AECreateList(nil, 0, FALSE, &selection_list) ;
	HLock((char **)alias);
	//AEPutPtr(&selection_list, 1, typeAlias, StripAddress(*alias), (**alias).aliasSize) ;//commented by Kekus Digital
	AEPutPtr(&selection_list, 1, typeAlias, *alias, (**alias).aliasSize) ; //added by Kekus Digital
	HUnlock((char **)alias);
	DisposeHandle((char **)alias);

	AEPutParamDesc(the_event_ptr, 'fsel', &selection_list) ;
	AEDeleteItem(&selection_list, 1) ;
	AEDisposeDesc(&selection_list) ;
}


void makePathForResult( fullPath *path )
{
	FSSpec          fsspec_IMG ;
	long 			lgDeskTopDirID 	= 0L;
	short  			shDeskTopVol 	= 0;
	Str255			imName;
				
				
	sprintf( (char*)imName, "ptools_result");
	c2pstr((char*)imName);
				
	// Create FSSpec
				
	FindFolder( kOnSystemDisk, kDesktopFolderType, kDontCreateFolder,
                                                     &shDeskTopVol, &lgDeskTopDirID );

	FSMakeFSSpec( shDeskTopVol, lgDeskTopDirID, imName, &fsspec_IMG );
				
	memcpy( path, &fsspec_IMG, sizeof( fullPath ));
}


int makePathToHost ( fullPath *path )
{
	ProcessSerialNumber PSN;
	ProcessInfoRec		info;
	OSErr				myErr;
	fullPath			f;
	
	info.processName		=	nil;
	info.processAppSpec		=	&f;
	PSN.highLongOfPSN 		= 	0;
	PSN.lowLongOfPSN 		= 	kNoProcess;
   	info.processInfoLength  = sizeof( ProcessInfoRec );

	
	myErr =  GetCurrentProcess(& PSN);
	
	if(myErr == noErr)
		myErr = GetProcessInformation(&PSN,&info);
	
	if(myErr == noErr)
	{
		memcpy( path, &f, sizeof( fullPath ));
		return 0;
	}
	else
	{
		return -1;
	}
}


// Thanks to Brian Fitzgerald Future Point
/*pascal OSErr __my_initialize(const CFragInitBlock *theInitBlock)//commented by Kekus Digital
{
   	OSErr 	err;

	// Get FSSpec of library file
	
   	err = __init_lib(theInitBlock);
   	if (err != noErr)
      return err;
        
	memcpy( &panoLib, theInitBlock->fragLocator.u.onDisk.fileSpec, sizeof( FSSpec ) );
	
	// and of preferences file

	
	err = FindFolder( kOnSystemDisk, kPreferencesFolderType, 
				   kDontCreateFolder, &prFile.vRefNum, &prFile.parID) ;
	
	if( err == noErr )
	{
		pstrcpy( PREFNAME , prFile.name );
		FSpCreateResFile( &prFile, 'GKON', '????', 0 );
	}
	return err;
}
	

pascal void __my_terminate(void)
{
        
   __term_lib();
}
*/ //till here

short gResNum = -1;//added by Kekus Digital
pascal OSErr MyInitialize(CFBundleRef bundle)
{
   	OSErr 	err;
 
         if(bundle != NULL)
            gResNum = CFBundleOpenBundleResourceMap(bundle);
        
	err = FindFolder( kOnSystemDisk, kPreferencesFolderType, 
				   kDontCreateFolder, &prFile.vRefNum, &prFile.parID) ;
	
	if( err == noErr )
	{
		pstrcpy( PREFNAME , prFile.name );
                FSpCreateResFile( &prFile, 'GKON', '????', 0 );
                err = ResError();
	}
	return err;
}
	

pascal void MyTerminate(CFBundleRef bundle)
{
    if(gResNum != -1 && bundle != NULL)
        CFBundleCloseBundleResourceMap(bundle, gResNum);     
}// till here


// Create FSSpec for temporary buffer file 

void MakeTempName( fullPath *fspec, char *fname )
{
   	OSErr 			err;
	FInfo			fndrInfo;

	err = FindFolder( kOnSystemDisk, kPreferencesFolderType, 
				   kDontCreateFolder, &fspec->vRefNum, &fspec->parID) ;
	if( err != noErr )
	{
		fspec->vRefNum 	= 0;
		fspec->parID	= 0;
	}
	sprintf( (char*)fspec->name, "pano13.%s", fname );
	c2pstr( (char*)fspec->name );

	if( ( FSpGetFInfo	(fspec,&fndrInfo) == noErr )	&&		// Don't accidentaly overwrite important stuff
		  fndrInfo.fdCreator 	!= '8BIM'				&&
		  fndrInfo.fdType 		!= '8BPS' )
	{
		fspec->vRefNum 	= 0;
		fspec->parID	= 0;
	}
}


// Present 'Save As...' dialog

/*int SaveFileAs( fullPath *path, char *prompt, char *name ) // commented by Kekus Digital
{
	StandardFileReply	reply;

	c2pstr( prompt ); c2pstr( name );
	StandardPutFile((unsigned char* )prompt, (unsigned char*) name, &reply);
	p2cstr( (unsigned char* )prompt ); p2cstr( (unsigned char*) name );
	if( reply.sfGood )
	{
		memcpy( path, &reply.sfFile, sizeof( fullPath ));
		return 0;
	}
	else
		return -1;
}*/ //till here
	
int SaveFileAs( fullPath *path, char *prompt, char *name ) //added by Kekus Digital
{
    NavReplyRecord reply;
    NavDialogOptions the_dialogOptions;
	
    OSErr the_sErr = NavGetDefaultDialogOptions(&the_dialogOptions);
    CopyCStringToPascal(prompt,the_dialogOptions.windowTitle); 
    CopyCStringToPascal(name,the_dialogOptions.savedFileName); 
	
    the_sErr = NavPutFile(NULL, &reply, &the_dialogOptions, NULL, '????', '????', NULL);
    if( reply.validRecord )
    {
	
        AEDesc resultDesc;
        OSErr the_sErr = noErr;

        //grab information about file for opening:	
        if ((the_sErr = AEGetNthDesc( &(reply.selection),1, typeFSS, NULL, &resultDesc )) ==noErr)
        {	
            the_sErr = AEGetDescData(&resultDesc, path,sizeof(*path));
            if(the_sErr == noErr)
                AEDisposeDesc( &resultDesc ); 
        }    
            
        return 0;
    }
    else
        return -1;
} // till here

// Present "Find File" dialog

/*int FindFile( fullPath *fspec ) //commented by Kekus Digital
{
	StandardFileReply	reply;
	SFTypeList	typeList;

	typeList[0] = 'TEXT';
	StandardGetFile(nil, -1, typeList, &reply);
	if( reply.sfGood )
	{
		memcpy( fspec,  &reply.sfFile, sizeof (FSSpec) );
		return 0;
	}
	else
		return -1;
}*/ //till here

int FindFile( fullPath *fspec ) //added by Kekus Digital
{
    NavReplyRecord reply;
    NavTypeList  typeList = {0};
    NavTypeListPtr theTypeListPtr;
        
    typeList.componentSignature = kNavGenericSignature;
    typeList.reserved = 1;
    typeList.osTypeCount = 1;
    typeList.osType[0]='TEXT';
        
    theTypeListPtr = &typeList;
    NavGetFile(NULL, &reply, NULL, NULL, NULL, NULL, &theTypeListPtr, NULL);
    if( reply.validRecord )
    {
        AEDesc resultDesc;
        OSErr the_sErr = noErr;
			
        //grab information about file for opening:	
        if ((the_sErr = AEGetNthDesc( &(reply.selection),1, typeFSS, NULL, &resultDesc )) ==noErr)
        {	
            the_sErr = AEGetDescData(&resultDesc, fspec,sizeof(*fspec));
            if(the_sErr == noErr)
                AEDisposeDesc( &resultDesc ); 
        }    
        return 0;
}
    else
        return -1;
}// till here


void ConvFileName( fullPath *fspec,char *string)
{
	pstrcpy( fspec->name , (unsigned char *)string);
	p2cstr((unsigned char *)string);
}


/* concatonate 2 Pascal strings:  a = CONCAT(b, c).  Assumes Str255.  Truncates if necessary. */
#define CONCAT(a, b, c)	do {	int	cchars;														\
								cchars = ((int)b[0] + (int)c[0]  > 255 ? 255 - b[0] : c[0]);	\
								if (&a[0] == &b[0]) {											\
									BlockMove(&c[1], &a[a[0]+1], cchars);						\
									a[0] += cchars;												\
								} else {														\
									BlockMove(&c[1], &a[1 + b[0]], cchars);						\
									BlockMove(&b[1], &a[1], b[0]);								\
									a[0] = b[0] + cchars;										\
								}																\
							} while (0)


// get the full path as string, garantee filename fits in 256 bytes
int GetFullPath (fullPath *path, char *filename)
{
	Str255			pfilename;
	Str63			dirName;	/* allow room for a 31 char file/dir name + a ':' */
	CInfoPBRec		di;
	OSErr			err;

	*filename = 0;
	
	pfilename[0] = 0;

	dirName[0] = 0;
	di.dirInfo.ioNamePtr = dirName;
	di.dirInfo.ioDrParID = path->parID;
	
	if( path->name[1] != ':' )
		BlockMove(path->name, pfilename, path->name[0]+1);
	else{
		BlockMove(path->name+1, pfilename, path->name[0]);
		pfilename[0] = path->name[0]-1;
	}
		
	
	do 
	{
		di.dirInfo.ioVRefNum = path->vRefNum;
		di.dirInfo.ioFDirIndex = -1;
		di.dirInfo.ioDrDirID = di.dirInfo.ioDrParID;
		
		err = PBGetCatInfo(&di, FALSE);
		if (err == noErr) 
		{
			dirName[++dirName[0]] = ':';
			if ((int)dirName[0] + (int)pfilename[0] > 255)
			{
				PrintError("Filename too long");
				return -1;
			}
			else
				CONCAT(pfilename, dirName, pfilename);
		} 
		else
		{
			//PrintError("Error getting PBGetCatInfo");
			//return -1;
		}
	}
	while (di.dirInfo.ioDrDirID != fsRtDirID && err == noErr);
	
	p2cstr( pfilename );
	strcpy( filename, (char*)pfilename );

	return 0;
}


int IsTextFile( char* fname )
{
	FileInfo fInfo;
	fullPath f;

	if( strrchr( fname, '.' ) != NULL && 
			(strcmp( strrchr( fname, '.' ), ".txt") == 0 ||
			 strcmp( strrchr( fname, '.' ), ".TXT") == 0)	)
	return TRUE;
	
	//StringtoFullPath(&f, fname); //commented by Kekus Digital
	GetFullPath(&f, fname);//addded by Kekus Digital
				
	FSpGetFInfo	(&f, (FInfo*)&fInfo );

	if( fInfo.fileType == 'TEXT' )
		return TRUE;

	return FALSE;
}


int	MakeDoScriptEvent(char* script, AppleEvent *appleEvent,
						 AEAddressDesc *target, AEEventID theAEEventID)
{
	OSErr			err;
	AEDesc			source;
			
			
	err = AECreateDesc(typeChar, script, strlen(script) + 1, &source);
	if( err == noErr )
	{
	err = 	AECreateAppleEvent('misc', theAEEventID, target, 
			kAutoGenerateReturnID, kAnyTransactionID, appleEvent);
	AEPutParamDesc(appleEvent, keyDirectObject, &source);
	AEDisposeDesc(&source);
	}
	if (err == noErr)
		return 0;
	else
		return -1;
}

int LaunchWithAE(AppleEvent *appleEvent, FSSpec *target)
{
	
	LaunchParamBlockRec		launchParams;
	AEDesc 					ioLaunchDesc;
	int 					result;
	
	if( appleEvent == nil)
			launchParams.launchAppParameters = nil;
	else
	{
		AECoerceDesc(appleEvent, typeAppParameters, &ioLaunchDesc);
		HLock((Handle) appleEvent->dataHandle);
		launchParams.launchAppParameters = (AppParametersPtr) *(ioLaunchDesc.dataHandle); 
	}
	launchParams.launchBlockID = extendedBlock;
	launchParams.launchEPBLength = extendedBlockLen;
	launchParams.launchFileFlags = 0;
	launchParams.launchControlFlags = launchContinue;
	launchParams.launchAppSpec = target;
	
	if(LaunchApplication( &launchParams))
	{
		result	= -1;
	}
	else
		result	=  0;
	return(result);
}

/* Are two FSSpecs equal ? */

Boolean EqFSSpec(FSSpec *a, FSSpec *b)
{
	char as[512],bs[512];
	FullPathtoString( a, as );
	FullPathtoString( b, bs );
	//printf("%s\n", as); printf("%s\n\n", bs);
	if( strcmp(as, bs) == 0 )
		return true;
	else
		return false;
}

Boolean IsAppRunning(FSSpec *myApp)
{
	ProcessSerialNumber PSN;
	ProcessInfoRec		info;
	FSSpec				app;
	
	info.processName		=	nil;
	info.processAppSpec		=	&app;
	PSN.highLongOfPSN 		= 	0;
	PSN.lowLongOfPSN 		= 	kNoProcess;


	
	while (GetNextProcess(&PSN) == noErr)
	{
	GetProcessInformation(&PSN,&info);
	if(EqFSSpec( &app, myApp))
		return (true);
	}
	return (false);
}


#include <time.h>

int LaunchAndSendScript(char* application, char* script){
	AEAddressDesc	target;
	OSErr			myErr;
	AppleEvent		appleEvent,reply;
	fullPath		theApp;
	FInfo			finfo;
	EventRecord				event;
	int tt;

	if( makePathToHost( &theApp ) != 0 ){
		PrintError("Could not find host application");
		return -1;
	}
	
	strcpy((char*)theApp.name, application);
	c2pstr((char*)theApp.name);

	open_selection( &theApp );

	tt = time(NULL);
	while( time(NULL) < tt + 10  && !IsAppRunning(&theApp) )
			WaitNextEvent ( everyEvent, &event, 100L, nil);
	

     // Create Descriptor 

	FSpGetFInfo (&theApp,&finfo) ;
    myErr = AECreateDesc( typeApplSignature, (Ptr)(&finfo.fdCreator),
                                (Size)sizeof( finfo.fdCreator ), &target);
	
	if(myErr != noErr){
		PrintError("Error creating AE descriptor");
		return -1;
	}
	

	if( MakeDoScriptEvent(script, &appleEvent, &target, 'dosc')){
		PrintError("Error creating DOSC Apple Event");
		return -1;
	}

	AESend(&appleEvent, &reply, kAENoReply+kAENeverInteract, 
				 kAENormalPriority, kAEDefaultTimeout, nil, nil) ;

	AEDisposeDesc(&appleEvent);
	AEDisposeDesc(&reply) ;
	
	return 0;
}		
	
unsigned char *MyCtoPStr(char *x) //added by Kekus Digital
{ 
    CopyCStringToPascal(x,x); 
    return x;
}//till here
