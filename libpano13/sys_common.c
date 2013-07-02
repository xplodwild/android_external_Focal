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

#include "panotypes.h"
#include "filter.h"
#include <signal.h>
#include <stdlib.h>


//------------------ Public functions required by filter.h -------------------------------



static int (*g_progressFcn)(int, char *) = NULL;

static int (*g_infoDlgFcn)(int, char *) = NULL;

static void  (*g_printErrorFcn)(char* , va_list va) = NULL;

int ProgressIntern( int command, char* argument );
void PrintErrorIntern(char*fmt, va_list va);
int infoDlgIntern( int command, char* argument );

void PT_setErrorFcn(void (*ptr)(char *, va_list va))
{
    g_printErrorFcn = ptr;
}

void PT_setProgressFcn(int (*ptr)(int, char *))
{
    g_progressFcn = ptr;
}

void PT_setInfoDlgFcn(int (*ptr)(int, char *))
{
    g_infoDlgFcn = ptr;
}

/**
 * Print an error message and then exit program
 */
void dieWithError(char*fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    if (g_printErrorFcn == NULL) {
        PrintErrorIntern(fmt, ap);
    } else {
        (*g_printErrorFcn)(fmt, ap);
    }

    va_end(ap);

    exit(1);
}

void PrintError(char*fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    if (g_printErrorFcn == NULL) {
        PrintErrorIntern(fmt, ap);
    } else {
        (*g_printErrorFcn)(fmt, ap);
    }

    va_end(ap);
}

// Progress report; return false if canceled
int Progress( int command, char* argument )
{
    if (g_progressFcn == NULL)
        return ProgressIntern(command, argument);
    else
        return (*g_progressFcn)(command, argument);
}

int infoDlg( int command, char* argument )
{
    if (g_infoDlgFcn== NULL)
        return infoDlgIntern(command, argument);
    else
        return (*g_infoDlgFcn)(command, argument);
}
