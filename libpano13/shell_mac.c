#include <Controls.h>
#include <TextUtils.h>
#include <Menus.h>
#include <Errors.h>
#include <StandardFile.h>
#include <ToolUtils.h>
#include <Devices.h>
#include <Folders.h>
#include <Events.h>
#include <LowMem.h>
#include <Dialogs.h>
#include <MacMemory.h>
#include <Files.h>
#include <LowMem.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <Files.h>
#include <Fonts.h>
#include "filter.h"
#include <AppleEvents.h>
#include <AERegistry.h>



static  AEEventHandlerUPP	OAPPHandlerUPP, ODOCHandlerUPP, PDOCHandlerUPP, 
							QUITHandlerUPP, DOSCHandlerUPP;



static void			EventInit( void );
static pascal OSErr	DoOpenApp( AppleEvent *theAppleEvent,AppleEvent *reply, long refCon);
static pascal OSErr	DoOpenDoc( AppleEvent *theAppleEvent,AppleEvent *reply, long refCon);
static pascal OSErr	DoScript(  AppleEvent *theAppleEvent,AppleEvent *reply, long refCon);
static void 		SetUpMenus(void);
static void 		DoCommand(long mResult);

static OSErr		GotRequiredParams( AppleEvent *appleEventPtr );
static int 			FullToFolder(StringPtr name, short *volume, long *folder);



// Globals

static Boolean 		gDone = false;

static QDGlobals 	qd;
static int 			argc;
static char 		**argv, appName[64];

int ccommand( char ***argvPtr)
{
	int 					i;
	EventRecord				event;
	OSErr 					myErr = noErr;
	fullPath				app;
	Point					thePoint;
	short 					thePart;
	WindowPtr 				whichWindow;
	unsigned char 			theChar;
	
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs( nil );
	InitCursor();
	MaxApplZone();

	for(i=0; i<10; i++)
		MoreMasters();

	setLibToResFile();
	
	if( makePathToHost ( &app ) == 0 )
	{
		p2cstr( app.name );
		strcpy( appName, (char*)app.name);
	}
	else
		strcpy( appName, "dummy");
		


	EventInit();
	SetUpMenus();
	
	argc = 0; argv = nil;


	while( !gDone )
	{
		if ( WaitNextEvent ( everyEvent, &event, 20L, nil) )
		{
			switch ( event.what )
			{
				case	kHighLevelEvent:
					AEProcessAppleEvent( &event );
					break;
				case mouseDown:
					thePoint = event.where;
					switch (thePart = FindWindow(thePoint, &whichWindow)) 
					{
						case inSysWindow:
	  						SystemClick(&event, whichWindow);
	  						break;

						case inMenuBar:
	  						DoCommand(MenuSelect(thePoint));
	  						break;
					}
					break;
				case keyDown:
      			case autoKey:
					theChar = (unsigned char)BitAnd(event.message, charCodeMask);
					if (BitAnd(event.modifiers, cmdKey) != 0) 
	  						DoCommand(MenuKey(theChar));
					break;
			}
		}					  
	}
	
	*argvPtr = argv;
	return argc;
}
	
	
	
static void	EventInit( void )
{
	OSErr	err;
	OAPPHandlerUPP = NewAEEventHandlerProc(DoOpenApp);
	err = AEInstallEventHandler ( kCoreEventClass, kAEOpenApplication, OAPPHandlerUPP, 0L, false );
	
	ODOCHandlerUPP = NewAEEventHandlerProc(DoOpenDoc);
	if( err == noErr) err = AEInstallEventHandler ( kCoreEventClass, kAEOpenDocuments, ODOCHandlerUPP, 0L, false );

	PDOCHandlerUPP = NewAEEventHandlerProc(DoOpenApp);
	if( err == noErr) err = AEInstallEventHandler ( kCoreEventClass, kAEPrintDocuments, PDOCHandlerUPP, 0L, false );
	
	QUITHandlerUPP = NewAEEventHandlerProc(DoOpenApp);
	if( err == noErr) err = AEInstallEventHandler ( kCoreEventClass, kAEQuitApplication, QUITHandlerUPP, 0L, false );
	
	DOSCHandlerUPP = NewAEEventHandlerProc(DoScript);
	if( err == noErr) err = AEInstallEventHandler ( kAEMiscStandards, kAEDoScript, DOSCHandlerUPP, 0L, false);

	if( err != noErr) PrintError("Could not initialize AE Handler");
	return;
}

static pascal OSErr	DoOpenApp( AppleEvent *theAppleEvent,
						AppleEvent *reply, long refCon)
{
	OSErr		err = noErr;
	return err;
}


static pascal OSErr	DoScript( AppleEvent *theAppleEvent,
						AppleEvent *reply, long refCon)
{
	DescType	returned;
	OSErr		err = noErr;
	long		length;
	char 		*theScript, *ch, *c;
	int			i;
	
	theScript = (char*) NewPtr(10);
	if(theScript == nil) 
	{
		PrintError("Not enough memory");
		goto _DoScript_Exit;
	}
	err = AEGetParamPtr( theAppleEvent, keyDirectObject,
						typeChar, &returned,  theScript , 10, &length);

	DisposePtr(theScript);
	theScript = (char*) NewPtr(length + 1);
	if(theScript == nil) 		
	{
		PrintError("Not enough memory");
		goto _DoScript_Exit;
	}

	err = AEGetParamPtr( theAppleEvent, keyDirectObject,
						typeChar, &returned,  theScript , length, &length);

	if( err!= noErr) 
	{
		PrintError( "Could not receive DoScript Command");
		goto _DoScript_Exit;
	}
	theScript[length] = 0;
	// printf("%s\n", theScript); fflush(stdout);
	
	// get number of args, calculate argc
	
	argc = 1; ch = theScript;
	
	while( *ch != 0 )
	{
		switch( *ch )
		{
			case ' ': 
			case '\t': 	ch++; 
						break;
			case '"': 	argc++; ch++;
						while( *ch!='"' && *ch!=0 ) ch++;
						if( *ch == 0 )
						{
							PrintError("Syntax Error in Script");
							goto _DoScript_Exit;
						}
						ch++;
						break;
			default:	argc++; while( *ch!=' ' && *ch!='\t' && *ch!=0 ) ch++;
			 			break;
		}
	}
	
	// Fill argv array
	
	argv = (char**)NewPtr( argc * sizeof( char* ) );
	if( argv == nil )
	{
		PrintError("Not enough memory");
		goto _DoScript_Exit;
	}
	
	argv[0] = (char*) NewPtr( strlen( appName ) + 1 );
	if( argv[0] == nil )
	{
		PrintError("Not enough memory");
		goto _DoScript_Exit;
	}
	strcpy( argv[0], appName );
	
	ch = theScript; i=0;
	
	while( *ch != 0 && i<argc )
	{
		switch( *ch )
		{
			case ' ': 
			case '\t': 	ch++; 
						break;
			case '"': 	i++; ch++; c = ch;
						while( *c!='"' && *c!=0 ) c++;
						argv[i] = (char*) NewPtr( c-ch+1 );
						if( argv[i] == nil )
						{
							PrintError("Not enough memory");
							goto _DoScript_Exit;
						}
						c=argv[i];
						while(*ch!='"') *c++=*ch++;
						*c=0; ch++;
						break;
			default:	i++; c = ch;
						while( *c!=' ' && *c!='\t' && *c!=0 ) c++;
						argv[i] = (char*) NewPtr( c-ch+1 );
						if( argv[i] == nil )
						{
							PrintError("Not enough memory");
							goto _DoScript_Exit;
						}
						c=argv[i];
						while(*ch!=' ' && *ch!='\t' && *ch!=0 ) *c++=*ch++;
						*c=0;
						break;
		}
	}

_DoScript_Exit:

	gDone = true;
	DisposePtr( theScript );
	
	return err;
}



static pascal OSErr	DoOpenDoc( AppleEvent *theAppleEvent,
						AppleEvent *reply, long refCon)
{
	AEDescList			fileSpecList;
	OSErr				err = noErr;
	DescType			type;
	Size				actual;
	AEKeyword			keyword;
	int					i;
	long				numFiles=0;
	FSSpec f;


	err = AEGetParamDesc( theAppleEvent, keyDirectObject,
						typeAEList, &fileSpecList );
	if(err == noErr)
	{
		err = GotRequiredParams( theAppleEvent );
		if(err == noErr)
			{
				err = AECountItems( &fileSpecList, &numFiles );
			}
	}
	if(err != noErr)
	{
		PrintError("Error receiving ODOC AppleEvent");
		goto _ODOC_exit;
	} 
	
	argc = numFiles + 1;

	// Fill argv array
	
	argv = (char**)NewPtr( argc * sizeof( char* ) );
	if( argv == nil )
	{
		PrintError("Not enough memory");
		goto _ODOC_exit;
	}


	argv[0] = (char*) NewPtr( strlen( appName ) + 1 );
	if( argv[0] == nil )
	{
		PrintError("Not enough memory");
		goto _ODOC_exit;
	}
	strcpy( argv[0], appName );

	for( i=1; i<argc; i++)
	{
		argv[i] = (char*) NewPtr( 256 );
		if( argv[i] == nil )
		{
			PrintError("Not enough memory");
			goto _ODOC_exit;
		}
		err = AEGetNthPtr( &fileSpecList, i,
				typeFSS, &keyword, &type, (Ptr)&f,
				sizeof(FSSpec), &actual);

		if( err != noErr )
		{
			PrintError("Error processing file list");
			goto _ODOC_exit;
		}
		
		if( FullPathtoString ( &f, argv[i]) )
		{
			PrintError("Error processing file list");
			goto _ODOC_exit;
		}
	}


_ODOC_exit:
	gDone = true;
	return err;
	
}						


static OSErr	GotRequiredParams( AppleEvent *appleEventPtr )
{
	DescType		returnedType;
	Size			actualSize;
	OSErr			err;
	
	err = AEGetAttributePtr( appleEventPtr,
					keyMissedKeywordAttr, typeWildCard,
					&returnedType, nil, 0, &actualSize );
	if ( err == errAEDescNotFound )
			return(noErr);
	else if (err == noErr)
		return(errAEEventNotHandled);
	else
		return(err);
}

	

static int FullToFolder(StringPtr name, short *volume, long *folder)
{	CInfoPBRec dirInfo; HVolumeParam hvp; short i;
	dirInfo.dirInfo.ioNamePtr = name;
	dirInfo.dirInfo.ioDrDirID = 0;
	dirInfo.dirInfo.ioVRefNum = 0;
	dirInfo.dirInfo.ioFDirIndex = 0;
	if (PBGetCatInfo (&dirInfo, 0) == noErr) {
		if (dirInfo.dirInfo.ioFlAttrib & 0x10) {
			*folder = dirInfo.dirInfo.ioDrDirID;
			hvp.ioNamePtr = name;
			for (i = 0; i < name[0]; i++)
				if (name[i+1] == ':')
					{ name[0] = i + 1; break; }
			hvp.ioVolIndex = -1;
			hvp.ioVRefNum = 0;
			PBHGetVInfo ((HParmBlkPtr)&hvp, 0);
			*volume = hvp.ioVRefNum;
		}
		else {	*volume = 0, *folder = 0; return -1; }
	}
	else if (dirInfo.dirInfo.ioResult == fnfErr
		 || dirInfo.dirInfo.ioResult == dirNFErr)
	{	dirInfo.dirInfo.ioNamePtr = (StringPtr)name;
		dirInfo.dirInfo.ioDrDirID = 0;
		dirInfo.dirInfo.ioVRefNum = 0;
		if (PBDirCreate((HParmBlkPtr)&dirInfo, false) == noErr) {
			*volume = dirInfo.dirInfo.ioVRefNum;
			*folder = dirInfo.dirInfo.ioDrDirID;
		}
		else {	*volume = 0, *folder = 0;
			return dirInfo.dirInfo.ioResult;
		}
	}
	else	{ *volume = 0, *folder = 0; return dirInfo.dirInfo.ioResult; }
	return noErr;
}


int StringtoFullPath(fullPath *file, char *path)
{
	unsigned char ppath[255];
	int i,k;
	
	/* Make p string */
	
	strcpy((char*)&(ppath[1]), path);
	*ppath = (unsigned char) strlen(path);
	
	/* Get basename */
	k = 0;
	for(i = 0; i<= ppath[0]; i++)
		if(ppath[i] == ':') k=i;
	for(i=k; i  <= ppath[0]; i++)
		file->name[i-k] = ppath[i];
	file->name[0] = ppath[0] - k;
	
	/* and dirname */ 
	ppath[0] = k;
		
	return( FullToFolder((StringPtr) ppath, &(file->vRefNum), &(file->parID)));
	
}




char *getFileName( fullPath *path )
{
	p2cstr( path->name );
	return (char*) path->name;
}

static void SetUpMenus(void)
{
	MenuHandle menu;
	
	DeleteMenu(1);	
	InsertMenu(menu = NewMenu(1, "\p\024"), 0);
	AppendMenu(menu, "\pAbout...;(-");
	AppendResMenu(menu, 'DRVR');
	DeleteMenu(2);
	InsertMenu(menu = NewMenu(2, "\pFile"), 0);
	AppendMenu(menu, "\p(New/N;Open/O;Run/R;Save/S;Save As...;(-;Quit/Q");
	DeleteMenu(3);
	InsertMenu(menu = NewMenu(3, "\pEdit"), 0);
	AppendMenu(menu, "\p(Undo/Z;(-;(Cut/X;(Copy/C;(Paste/V;(Clear");
	
	DrawMenuBar();
}


static void 		DoCommand(long mResult)
{
  short theItem, theMenu;
  MenuHandle appleMenu;
  Str255 name;

  theItem = mResult & 65535L;
  theMenu = ((unsigned long)mResult) >> 16;

  switch (theMenu) 
  {
		case 1:		/*  Apple  */
			switch(theItem)
			{
				case 1: PrintError( "%s\n %s", appName, LONGVERSION );
						break;
				default:appleMenu = GetMenuHandle( 1);
						GetMenuItemText(appleMenu, theItem, name);
						OpenDeskAcc(name);
						break;
			}
			break;
		case 2:		/*  File  */
			switch(theItem)
			{
				case 1: break;
				case 2: {	// Open file
							fullPath f;
							
							if( FindFile( &f ) == 0 )
							{
								char** temp;
								int i;
								if( argc==0 )
								{
									argc=1;
									argv=(char**)NewPtr(sizeof(char*));
									if(argv==nil)
									{
										PrintError("Not enough memory");
										exit(0);
									}
									argv[0] = (char*)NewPtr(strlen(appName)+1);
									strcpy(argv[0], appName );
								}
								argc+=1;
								temp = (char**)NewPtr(argc * sizeof(char*));
								for(i=0; i<argc-1; i++)
									temp[i]=argv[i];
								temp[argc-1] = (char*) NewPtr( 256 );
								if( temp[argc-1] == nil )
								{
									PrintError("Not enough memory");
									exit(0);
								}
								if( FullPathtoString ( &f, temp[argc-1]) )
								{
									PrintError("Error processing file list");
									exit(0);
								}
								DisposePtr( (char*)argv );
								argv = temp;
							}
						}
						break;
				case 3:	// Run
						gDone = TRUE;
						break;
				case 4: // Save
				case 5: {// Save As...
							fullPath f;
							
							if( SaveFileAs( &f, "Save Result As...", "Result" ) == 0 )
							{
								char** temp;
								int i;
								if( argc==0 )
								{
									argc=1;
									argv=(char**)NewPtr(sizeof(char*));
									if(argv==nil)
									{
										PrintError("Not enough memory");
										exit(0);
									}
									argv[0] = (char*)NewPtr(strlen(appName)+1);
									strcpy(argv[0], appName );
								}
								argc+=2;
								temp = (char**)NewPtr(argc * sizeof(char*));
								for(i=0; i<argc-2; i++)
									temp[i]=argv[i];
								temp[argc-2] = (char*) NewPtr( 3 );
								strcpy(temp[argc-2], "-o");
								temp[argc-1] = (char*) NewPtr( 256 );
								if( temp[argc-1] == nil )
								{
									PrintError("Not enough memory");
									exit(0);
								}
								if( FullPathtoString ( &f, temp[argc-1]) )
								{
									PrintError("Error processing file list");
									exit(0);
								}
								DisposePtr( (char*)argv );
								argv = temp;
							}
						}
						break;
						
				case 7: exit(0);	
						break;
			}
	}
	HiliteMenu(0);
}
	
