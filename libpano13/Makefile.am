ACLOCAL_AMFLAGS = -I m4

STD_SRC = filter.c parser.c queryfeature.c correct.c perspect.c \
	  adjust.c  remap.c lmdif.c  file.c \
	  math.c pan.c PTDialogs.c fftn.c fourier.c resample.c \
	  optimize.c morpher.c Triangulate.c \
	  seamer.c hdrfile.c rgbe.c rgbe.h \
	  tiff.c ZComb.c jpeg.c png.c  multilayer.c sys_common.c PTcommon.c ColourBrightness.c\
	  metadata.c jpegicc.c ptstitch.c ptfeather.c PaniniGeneral.c PaniniGeneral.h

if HasJava
JAVA_SRC = ptpicker.c pteditor.c
else
JAVA_SRC = javastub.c
endif

STD_HDR = panorama.h filter.h version.h panotypes.h PTcommon.h pt_stdint.h file.h

INTERNAL_HDR = fftn.h f2c.h ZComb.h adjust.h ColourBrightness.h PTcommon.h pt_stdint.h pttiff.h metadata.h jpegicc.h ptstitch.h ptfeather.h

TOOL_HDR = queryfeature.h pteditor.h ptutils.h 

SYS_HDR = sys_X11.h sys_ansi.h sys_mac.h sys_win.h sys_compat.h

if HAVE_X11
X11_SRC =  sys_X11.c 
else
X11_SRC =
endif

if HAVE_CYGWIN
WIN_SRC =  sys_ansi.c sys_compat_win.c bmp.c 
WIN_DEFS = -D__Win__=1
WIN_LDFS = -mwindows -avoid-version -no-undefined -export-symbols $(srcdir)/libpano13.def 

$(PANO_WIN32_RESOURCE): pano13.rc
	        $(top_srcdir)/build/win32/lt-compile-resource pano13.rc $@

install-exec-hook:
	mv $(DESTDIR)$(bindir)/cygpano13.dll $(DESTDIR)$(bindir)/pano13.dll
endif

if HAVE_MINGW
WIN_SRC =  sys_ansi.c sys_compat_win.c bmp.c
WIN_DEFS = -D__Win__=1
WIN_LDFS = -mwindows -avoid-version -no-undefined -export-symbols $(srcdir)/libpano13.def 

$(PANO_WIN32_RESOURCE): pano13.rc
	        $(top_srcdir)/build/win32/lt-compile-resource pano13.rc $@

TAGS:
	ctags-exuberant -e *.c *.h tools/*.c

install-exec-hook:
	mv $(DESTDIR)$(bindir)/libpano13.dll $(DESTDIR)$(bindir)/pano13.dll
endif

if HAVE_MAC
MAC_SRC =  shell_mac.c sys_mac.c sys_compat_unix.c ppm.c
MAC_HDR =  

install-exec-hook:
	@true

endif

if HAVE_LINUX
LIN_SRC =  sys_ansi.c sys_compat_unix.c ppm.c
LIN_DEFS = -D__Ansi__=1
LIN_LDFS = -version-info 2:0:0

install-exec-hook:
	@true

endif

if HAVE_DARWIN
DAR_SRC =  sys_ansi.c sys_compat_unix.c ppm.c
DAR_DEFS = -D__Ansi__=1
DAR_LDFS = -version-info 2:0:0

install-exec-hook:
	@true

endif

if HAVE_SOLARIS
SOL_SRC =  sys_ansi.c sys_compat_unix.c ppm.c
SOL_DEFS = -D__Ansi__=1
SOL_LDFS = -version-info 2:0:0 -lxnet -lm -lsunmath

install-exec-hook:
	@true

endif

AM_CPPFLAGS = $(JAVA_FLAGS) $(JPEG_FLAGS) $(PNG_FLAGS) $(TIFF_FLAGS) $(ZLIB_FLAGS) $(WIN_DEFS) $(LIN_DEFS) $(DAR_DEFS) $(SOL_DEFS) $(ENDIAN_FLAG)

lib_LTLIBRARIES = libpano13.la

libpano13_la_SOURCES = $(STD_SRC) $(JAVA_SRC) $(X11_SRC) $(WIN_SRC) $(MAC_SRC) $(LIN_SRC) $(DAR_SRC) $(SOL_SRC)
libpano13_la_LIBADD = $(LIB_JPEG) $(LIB_PNG) $(LIB_TIFF) $(LIB_ZLIB) @PANO_WIN32_RESOURCE@
libpano13_la_LDFLAGS = $(LIN_LDFS) $(WIN_LDFS) $(DAR_LDFS) $(SOL_LDFS)
libpano13_la_DEPENDENCIES = @PANO_WIN32_RESOURCE@

pkginclude_HEADERS = $(STD_HDR) $(TOOL_HDR)

noinst_HEADERS = $(INTERNAL_HDR) $(SYS_HDR) $(MAC_HDR)

# Pkg-config script.
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libpano13.pc
DISTCLEANFILES = $(pkgconfig_DATA)

SUBDIRS=m4 . tools doc build tests man

distdir=lib$(PACKAGE)-$(VERSION)

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	rm -rf `find $(distdir) -name .cvsignore`

EXTRA_DIST =  README.linux README.windows \
	      makefile.win32 TODO.org \
	      COPYING INSTALL \
	      CMakeLists.txt \
	      ChangeLog.hg \
	      dump.c dump.h \
	      bootstrap filter.r pano13.rc \
	      seamer_.c sys_X11.c sys_win.c bmp.c shell_mac.c \
	      sys_mac.c ppm.c sys_ansi.c ppm.c ZComb.h adjust.h \
	      libpano.sln LocalDefs.vsprops \
	      libpano.vcproj PanoTools.pbproj.tgz

MAINTAINERCLEANFILES = \
	aclocal.m4			\
	config.h.in			\
	config.h.in~		\
	Makefile.in			\
	configure

maintainer-clean-local:
	rm -rf config

# I know this is wrong!!!
darwin: libpano13.la filter.r
	echo "FIXME: What should this be on osx??"
	/Developer/Tools/Rez -t APPL -o libpano13.dynlib filter.r

changelog:
	hg log --follow --style=changelog > ChangeLog.hg
