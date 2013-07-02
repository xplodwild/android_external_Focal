/*
 * The so called TIFF types conflict with definitions from inttypes.h 
 * included from sys/types.h on AIX (at least using VisualAge compiler). 
 * We try to work around this by detecting this case.  Defining 
 * _TIFF_DATA_TYPEDEFS_ short circuits the later definitions in tiff.h, and
 * we will in the holes not provided for by inttypes.h. 
 *
 * See http://bugzilla.remotesensing.org/show_bug.cgi?id=39
 */

#ifndef PANOTYPES_H
#define PANOTYPES_H

// First make sure that we have the int8_t, int16_t (32, and 64) and uint8_t equivalents
#include "pt_stdint.h"



/* The macro PT_UNUSED indicates that a function, function argument or
 * variable may potentially be unused.
 * Examples:
 *   1) static int PT_UNUSED unused_function (char arg);
 *   2) int foo (char unused_argument PT_UNUSED);
 *   3) int unused_variable PT_UNUSED;
 */

#ifdef __GNUC__
  #define PT_UNUSED __attribute__ ((__unused__))
#else
  #define PT_UNUSED
#endif

/* Simple define to reduce warnings in printfs */
#if  __WORDSIZE == 64 /* 64 bit system */
   #define FMT_INT32 "%ld"
#else
   #define FMT_INT32 "%d"
#endif

#endif

