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


#include "sys_compat.h"
#include "filter.h"
#include <assert.h>

// Return the base filename of current executable

char *panoBasenameOfExecutable(void)
{

    // Make it static so we don't have to worry about allocating storage
    // and to make it compatible with what linux does
    // I presume this is reeentrant safe
    static char  AppNamePath[MAX_PATH_LENGTH] = "";
    static char *name = NULL;

    if (name  == NULL) {
        // Only do it if we haven't yet
        // to make it reentrant
        GetModuleFileName( NULL, AppNamePath, MAX_PATH_LENGTH );
    
        // Remove extension
        name = strrchr( AppNamePath, '.' );
        if( name != NULL ) 
            *name = '\0';
        
        // Search for directory separator
        name = strrchr( AppNamePath, PATH_SEP );
        if( name != NULL )
            name++;
        else 
            // No directory, use name
            name = AppNamePath;
    }
    return name;
}

#ifdef _MSC_VER
int panoTimeToStrWithTimeZone(char *sTime, int len, struct tm  *time) 
{
    char sZone[20];
    long        lZone = 0; 

    assert(len > 11); // This function needs at least 11 characters + null
    // %z or %Z  in strftime produces a name of the time zone not a numeric value.
    if (strftime(sTime, len, "%H%M%S", time) != 0) {
        _get_timezone(&lZone);
        sprintf(sZone, "%+03d%02d", -lZone/60/60, lZone/60%60);
        strncat(sTime, sZone, len-1); // Copy at most len -1
        // so we can force the end character to be null
        sTime[len-1] = 0;
        return 1;
    }  else 
        return 0;
}
#else
#ifdef __MINGW32__
int panoTimeToStrWithTimeZone(char *sTime, int len, struct tm  *time) 
{
    assert(len >= 11);
    return strftime(sTime, len, "%H%M%S%z", time);

}
#endif
#endif
