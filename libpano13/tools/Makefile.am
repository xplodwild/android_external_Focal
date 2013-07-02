# Over-ride default includes so that we have finer control in the makefiles
DEFAULT_INCLUDES="-I."
INCLUDES    = -I${top_srcdir}

PANOLIB=pano13

if HAVE_CYGWIN
WIN_SRC =  panoinfo.c
else
WIN_SRC =
endif

if HAVE_MINGW
WIN_SRC =  panoinfo.c
else
WIN_SRC =
endif

if HAVE_LINUX
LIN_SRC =  panoinfo_unix.c
LIN_DEFS = -D__Ansi__=1
else
LIN_SRC =
LIN_DEFS = 
endif

if HAVE_DARWIN
DAR_SRC =  panoinfo_unix.c
DAR_DEFS = -D__Ansi__=1
else
DAR_SRC =
DAR_DEFS = 
endif

if HAVE_SOLARIS
SOL_SRC =  panoinfo_unix.c
SOL_DEFS = -D__Ansi__=1
else
SOL_SRC =
SOL_DEFS = 
endif

AM_CPPFLAGS = $(JAVA_FLAGS) $(LIN_DEFS) $(DAR_DEFS) $(SOL_DEFS) $(TIFF_FLAGS) $(JPEG_FLAGS)
AM_LDFLAGS  = -L${top_builddir}

# PTAInterpolate can only be compiled with ptpicker.h, which in turn depends on java
if HasJava
bin_PROGRAMS = panoinfo PTAInterpolate PToptimizer PTmender PTblender PTtiff2psd PTuncrop PTtiffdump PTroller PTcrop PTmasker PTinfo
else
bin_PROGRAMS = panoinfo PToptimizer PTmender PTblender PTtiff2psd PTuncrop PTtiffdump PTroller PTcrop PTmasker PTinfo
endif

PTinfo_SOURCES = PTinfo.c 
PTinfo_LDADD = -l${PANOLIB} -ltiff

PTAInterpolate_SOURCES = PTAInterpolate.c 
PTAInterpolate_LDADD = -l${PANOLIB} -ltiff

PTmasker_SOURCES = PTmasker.c 
PTmasker_LDADD = -l${PANOLIB} -ltiff

PTblender_SOURCES = PTblender.c 
PTblender_LDADD = -l${PANOLIB} -ltiff

PTroller_SOURCES = PTroller.c 
PTroller_LDADD = -l${PANOLIB} -ltiff

PTmender_SOURCES = PTmender.c PTmender.h 
PTmender_LDADD   = -l${PANOLIB} -ljpeg -ltiff

PTtiff2psd_SOURCES = PTtiff2psd.c 
PTtiff2psd_LDADD   = -l${PANOLIB} -ljpeg -ltiff

PTuncrop_SOURCES = PTuncrop.c
PTuncrop_LDADD   = -l${PANOLIB} -ljpeg -ltiff

PTcrop_SOURCES = PTcrop.c
PTcrop_LDADD   = -l${PANOLIB} -ljpeg -ltiff


PTtiffdump_SOURCES = PTtiffdump.c
PTtiffdump_LDADD   = -l${PANOLIB} -ljpeg -ltiff


PToptimizer_SOURCES = PToptimizer.c
PToptimizer_LDADD   = -l${PANOLIB}

panoinfo_SOURCES    = $(WIN_SRC) $(LIN_SRC) $(DAR_SRC) $(SOL_SRC)
panoinfo_LDADD      = -l${PANOLIB}

EXTRA_DIST = README.PTmender \
        compat_win32/getopt.h compat_win32/getopt.c compat_win32/dirent.h \
	PTainterpolate.rc PTblender.rc PTcrop.rc PTinfo.rc PTmasker.rc PTmender.rc \
	PToptimizer.rc PTroller.rc PTtiff2psd.rc PTtiffdump.rc PTuncrop.rc \
	PTAInterpolate.vcproj PTblender.vcproj PTcrop.vcproj PTinfo.vcproj PTmasker.vcproj \
	PTmender.vcproj PTOptimizer.vcproj PTroller.vcproj PTtiff2psd.vcproj PTtiffdump.vcproj PTuncrop.vcproj \
	CMakeLists.txt

               

MAINTAINERCLEANFILES = \
	Makefile.in
