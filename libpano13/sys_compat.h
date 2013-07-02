/*
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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  
*/

/* these are functions that have very different implementions in different systems, so we
   define them here and implemement them in sys_ansi.c or sys_win.c */

#ifndef SYS_COMPAT_H
#define SYS_COMPAT_H

#include <time.h>

char *panoBasenameOfExecutable(void);

int panoTimeToStrWithTimeZone(char *sTime, int len, struct tm  *time) ;



#endif 
