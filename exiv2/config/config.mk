# ***************************************************** -*- Makefile -*-
#
# Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
#
# This Makefile is part of the Exiv2 distribution.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#    3. The name of the author may not be used to endorse or promote
#       products derived from this software without specific prior
#       written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# File:      config.mk.in
# Version:   $Rev: 2681 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   10-Dec-03, ahu: created
#
# Description: 
#  Exiv2 system configuration file.
#

# **********************************************************************
# Exiv2 version for use with libtool (-version-info argument)
EXIV2_LTVERSION = 12:0:0
# Compile for use with a commercial license
COMMERCIAL_VERSION = no

# **********************************************************************
# Libtool
LIBTOOL = $(top_srcdir)/libtool
LIBTOOL_DEPS = $(top_srcdir)/./config/ltmain.sh

# **********************************************************************
# C++ Compiler and precompiler
CXX = g++
GXX = yes

# Common compiler flags (warnings, symbols [-ggdb], optimization [-O2], etc)
CXXFLAGS = -O2 -fvisibility=hidden -fvisibility-inlines-hidden
ifeq ($(GXX),yes)
	CXXFLAGS += -Wall -Wcast-align -Wpointer-arith -Wformat-security -Wmissing-format-attribute -Woverloaded-virtual -W
endif

# Command to run only the preprocessor
CXXCPP = g++ -E

# Preprocessor flags
CPPFLAGS = -I.  -DEXV_LOCALEDIR=\"$(localedir)\"
ifeq ($(COMMERCIAL_VERSION),yes)
	CPPFLAGS += -DEXV_COMMERCIAL_VERSION=1
endif

# Linker flags and libraries
LDFLAGS = 
LIBS =   -lz  -lm

# Suffix of executables
EXEEXT := 

# **********************************************************************
# C Compiler
CC = gcc
GCC = yes

CFLAGS = -O2
ifeq ($(GCC),yes)
	CFLAGS += -Wall
endif

# **********************************************************************
# XMP support
ENABLE_XMP = 

ifdef ENABLE_XMP
        XMPSDK_LIBRARY = xmpsdk
        XMPSDK_DIR = $(top_srcdir)/xmpsdk
        XMPSDK_CPPFLAGS = -I$(XMPSDK_DIR)/include
        XMPSDK_LDFLAGS = -L$(XMPSDK_DIR)/src
        XMPSDK_LIBS = -l$(XMPSDK_LIBRARY)
else
        # Enable additional warnings. XMP Toolkit doesn't compile
        # with these.
        ifeq ($(GXX),yes)
                CXXFLAGS += -Wundef -pedantic
        endif
endif

# Expat library needed to compile the XMP Toolkit
EXPAT_LDFLAGS = 
EXPAT_CPPFLAGS = 
EXPAT_LIBS = 

# **********************************************************************
# Libraries, include files, functions
HAVE_LIBZ = 1
HAVE_STDINT = @HAVE_STDINT@
HAVE_TIMEGM = 1

# **********************************************************************
# Advanced auto-dependency generation
# http://make.paulandlesley.org/autodep.html
DEP_TRACKING = 1

ifdef DEP_TRACKING
        # Directory for dependency files
	DEPDIR = .deps

        # Command to run the compiler or preprocessor to produce 
        # dependencies. If you're not using gcc, you may need to change
        # this to something suitable for your compiler or simply unset
        # the variable. See the link above for suggestions.
	MAKEDEPEND = $(CXX) -MM $(CPPFLAGS) -o $*.d $<

        # Dependency files post-process commands
        POSTDEPEND = if test ! -d $(DEPDIR); then mkdir $(DEPDIR); fi; \
	        if test -e $*.d; then cp $*.d $(DEPDIR)/$*.d; \
	        sed -e 's/^\#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                    -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $(DEPDIR)/$*.d; \
                $(RM) $*.d; fi

        # Compiler flags to generate dependency files at the same time 
        # as object files (for gcc)
        ifeq ($(GXX),yes)
		CXXFLAGS += -MMD
		CFLAGS += -MMD
		MAKEDEPEND =
        endif
endif

# **********************************************************************
# Compilation shortcuts
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) -c
# LINK.cc does not need $(LIBS), libtool's dark magic takes care of that
# when linking a binary with a libtool library.
LINK.cc = $(CXX) $(LDFLAGS)

# **********************************************************************
# Installation programs
INSTALL_EXE = /usr/bin/install -c
INSTALL_PROGRAM = $(INSTALL_EXE)
INSTALL_DATA = $(INSTALL_EXE) -m 644
INSTALL_DIRS = $(top_srcdir)/config/mkinstalldirs

# **********************************************************************
# Other programs
RM = rm -f

# **********************************************************************
# Directories
prefix = /usr/local
exec_prefix = ${prefix}

# Source directory
srcdir = .

# Installation directories
bindir = ${exec_prefix}/bin
datarootdir = ${prefix}/share
datadir = ${datarootdir}
localedir = $(datadir)/locale
incdir = ${prefix}/include/exiv2
libdir = ${exec_prefix}/lib
mandir = ${datarootdir}/man
man1dir = $(mandir)/man1
