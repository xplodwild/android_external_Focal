## cmakelists.txt for libpano13 in a Hugin build environment.
##     03-04 Aug 2009 TKSharpless
##  Updated 16 Jan 2010 to add general Pannini projection
##
## For Windows, download just part of the panotools SVN tree:
## https://panotools.svn.sourceforge.net/svnroot/panotools/trunk/libpano
## into local directory libpano under the hugin SDK root.
## Then build in-place with CMake, and build INSTALL.  That
## will create a directory in the SDK root that the hugin
## CMake build scripts can use.
##    
## Builds libpano13 and selected panotools, by default without 
## support for the Java programs ptpicker and pteditor.  You
## can define HAVE_JAVA on the CMake command line; the Java
## header jni.h and its dependencies must then be in your
## include search path(this script does not find them).
##
## This script uses the Hugin CMake modules to find required
## image file format libraries, so it needs to know the name
## of the Hign source tree root.  By default that must be
## "../hugin" relative to the libpano root(that contains
## this file).  But you can set HUGIN_BASE_DIR to the path
## name of the Hugin root on the CMake command line.
##
## The parent directory of the Hugin root is one place where
## dependencies will be looked for.  On Windows that should
## be the root directory of the Hugin SDK, and the wxWidgets
## package will likely supply the dependencies.  Note:  you
## may need to edit the wxWidgets version number below.
##


# requires at least cmake 2.6, now
cmake_minimum_required(VERSION 2.6)

## Locate the hugin source root and its parent directory
IF(HUGIN_BASE_DIR)
  GET_FILENAME_COMPONENT(SOURCE_BASE_DIR ${HUGIN_BASE_DIR} PATH CACHE)
  set(CMAKE_MODULE_PATH ${HUGIN_BASE_DIR}/CMakeModules)
ELSE(HUGIN_BASE_DIR)
  GET_FILENAME_COMPONENT(SOURCE_BASE_DIR ${CMAKE_SOURCE_DIR} PATH CACHE)
  set(CMAKE_MODULE_PATH ${SOURCE_BASE_DIR}/hugin/CMakeModules)
ENDIF(HUGIN_BASE_DIR)

IF(WIN32)
  set(CMAKE_INSTALL_PREFIX "${SOURCE_BASE_DIR}/libpano13" CACHE PATH "Install prefix for libpano")
ENDIF(WIN32)

set(ARCH_TRIPLET)         
if(UNIX)
  FIND_PROGRAM(DPKG_ARCHITECTURE_EXECUTABLE dpkg-architecture)
  if(DPKG_ARCHITECTURE_EXECUTABLE)
    EXECUTE_PROCESS(COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
      OUTPUT_VARIABLE ARCH_TRIPLET
      ERROR_VARIABLE ERROR_ARCH_TRIPLET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()
endif()
if(ARCH_TRIPLET)
  set(SYSTEM_LIB_DIRS /usr/lib /usr/lib/${ARCH_TRIPLET} /usr/local/lib)
else()
  set(SYSTEM_LIB_DIRS /usr/lib /usr/local/lib)
endif()

set(HUGIN_SHARED OFF)

include(CheckIncludeFiles)

##  global setup
project(libpano13)

##
#  This must come before FINDs for tiff, jpeg, png, zlib to enable
#  finding the wxWidgets distributions of those packages on Windows.
IF(WIN32)
  ADD_DEFINITIONS(-D__Win__)
  OPTION(BUILD_DLL "Build dynamic libpano 13" OFF)
  OPTION(BUILD_WINGUI "Build with Windows GUI, otherwise the information is printed to shell/cmd.exe" OFF)
  set(win_c "tools/compat_win32/getopt.c")
  IF(BUILD_WINGUI)
    SET(sys_c "sys_win.c")
    SET(sys_h "sys_win.h")
  ELSE(BUILD_WINGUI)
    SET(sys_c "sys_ansi.c")
    SET(sys_h "sys_ansi.h")
  ENDIF(BUILD_WINGUI)
  SET(sys_compat_c "sys_compat_win.c")
ELSE(WIN32)
  SET(sys_c "sys_ansi.c")
  SET(sys_h "sys_ansi.h")
  SET(sys_compat_c "sys_compat_unix.c")
ENDIF(WIN32)


##
## Graphics libraries
##
# wxWidgets sub-packages will be used if native ones aren't found

FIND_PACKAGE(TIFF REQUIRED)
INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
FIND_PACKAGE(JPEG REQUIRED)
INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

set(_common_libs ${TIFF_LIBRARIES} ${ZLIB_LIBRARIES} ${JPEG_LIBRARIES} ${PNG_LIBRARIES})

#
# Check build type and set options accordingly
#
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
IF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  ADD_DEFINITIONS(-DDEBUG)
  IF(UNIX)
    ADD_DEFINITIONS(-Wall)
  ENDIF(UNIX)
ENDIF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

IF(UNIX)
  ADD_DEFINITIONS(-D__Ansi__)
ELSE(UNIX)
  IF(MSVC)
    # Stop MSVC8 from bitching about the C library
    ADD_DEFINITIONS(/D_CRT_SECURE_NO_DEPRECATE)
    # Stop MSVC9 from bitching about possibly invalid STL usage
    ADD_DEFINITIONS(/D_SCL_SECURE_NO_WARNINGS)
    # Stop MSVC9 from bitching about POSIX names without underscores
    ADD_DEFINITIONS(/D_CRT_NONSTDC_NO_DEPRECATE)
    IF(NOT BUILD_WINGUI)
      ADD_DEFINITIONS(/D__Ansi__)
    ENDIF()
    IF(NOT BUILD_DLL)
      # compile everything for the static C runtime
		FOREACH(TYPE C CXX)
			# makefiles use blank configuration
			FOREACH(CFG "_DEBUG" "_MINSIZEREL" "_RELEASE" "_RELWITHDEBINFO")
				#MESSAGE("Replacing CMAKE_${TYPE}_FLAGS${CFG}: ${CMAKE_${TYPE}_FLAGS${CFG}}")
				SET(NEW_FLAGS "${CMAKE_${TYPE}_FLAGS${CFG}}")
				# fix up static libc flags
				STRING(REPLACE "/MD" "/MT" NEW_FLAGS "${NEW_FLAGS}")
				# *FORCE* to override whats already placed into the cache
				SET(CMAKE_${TYPE}_FLAGS${CFG} "${NEW_FLAGS}" CACHE STRING
					"CMAKE_${TYPE}_FLAGS${CFG} (overwritten to ensure static build)" FORCE)
				#MESSAGE("New       CMAKE_${TYPE}_FLAGS${CFG}: ${CMAKE_${TYPE}_FLAGS${CFG}}")
			ENDFOREACH(CFG)
		ENDFOREACH(TYPE)
	    # link only with the static C runtime !! THIS IS A MUST !!
		SET(NEW_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
		# ensure that the flags are not duplicaated on subsequent runs
		STRING(REPLACE " /NODEFAULTLIB:\"MSVCRT.lib\" /NODEFAULTLIB:\"MSVCRTd.lib\"" "" NEW_FLAGS "${NEW_FLAGS}")
		# note that flag names(NODEFAULTLIB) etc.) are case sensitive
    	SET(CMAKE_EXE_LINKER_FLAGS
        	"${NEW_FLAGS} /NODEFAULTLIB:\"MSVCRT.lib\" /NODEFAULTLIB:\"MSVCRTd.lib\""
	       CACHE STRING "for MSVC" FORCE
		)
	ENDIF(NOT BUILD_DLL)
  ENDIF(MSVC)
  IF(MINGW)
    IF(NOT BUILD_WINGUI)
      ADD_DEFINITIONS(/D__Ansi__)
    ENDIF()
  ENDIF(MINGW)
ENDIF(UNIX)

##
## Here is the part that builds libpano
##
if(HAVE_JAVA)
  set(java_h pteditor.h ptutils.h)
  set(java_c ptpicker.c pteditor.c)
else()
  set(java_c javastub.c)
endif(HAVE_JAVA)

set(pano13_install_hdr
  panorama.h
  filter.h
  version.h
  panotypes.h
  PTcommon.h
  pt_stdint.h
  file.h
  queryfeature.h
  pteditor.h
  ptutils.h)

set(pano13_hdr
	${java_h}
	${pano13_install_hdr}
	ColourBrightness.h
	f2c.h
	fftn.h
	rgbe.h
        sys_compat.h
	${sys_h}
	ZComb.h
	PaniniGeneral.h
)

set(pano13_res
	pano13.rc
)

set(pano13_src
	${java_c}
	${win_c}
	adjust.c
	bmp.c
	ColourBrightness.c
	correct.c
        dump.c
	fftn.c
	file.c
	filter.c
	fourier.c
	hdrfile.c
	jpeg.c
	jpegicc.c
	lmdif.c
	math.c
	metadata.c
	morpher.c
	multilayer.c
	optimize.c
	pan.c
	parser.c
	perspect.c
	png.c
	ppm.c
	PTcommon.c
	PTDialogs.c
	ptfeather.c
	ptstitch.c
	queryfeature.c
	remap.c
	resample.c
	rgbe.c
	seamer.c
	${sys_c}
	${sys_compat_c}
	sys_common.c
	tiff.c
	Triangulate.c
	ZComb.c
	PaniniGeneral.c
)
# We have some header files at top source level now
include_directories(${CMAKE_SOURCE_DIR})

if(NOT WIN32)
  # Create and install shared library on non-windows
  add_library(pano13 SHARED ${pano13_hdr} ${pano13_res} ${pano13_src})
  target_link_libraries(pano13 ${_common_libs})
  set_target_properties(pano13 PROPERTIES VERSION "2.0.0" SOVERSION "2")
  install(TARGETS pano13 LIBRARY DESTINATION lib)
  set(_pano_target pano13)
  #create and install static library
  add_library(pano13a STATIC ${pano13_hdr} ${pano13_res} ${pano13_src})
  set_target_properties(pano13a PROPERTIES OUTPUT_NAME pano13)
  install(TARGETS pano13a ARCHIVE DESTINATION lib)
else()
  IF(BUILD_DLL)
    # Create and install shared library
    ADD_DEFINITIONS(-DPANO13_DLL)
    add_library(pano13 SHARED ${pano13_hdr} ${pano13_res} ${pano13_src})
    target_link_libraries(pano13 ${_common_libs})
    set_target_properties(pano13 PROPERTIES VERSION "2.0.0" SOVERSION "2")
    IF(MINGW)
      set_target_properties(pano13 PROPERTIES PREFIX "")
    ENDIF()
    install(TARGETS pano13 RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
  ELSE(BUILD_DLL)
    #create and install static library
    add_library(pano13 STATIC ${pano13_hdr} ${pano13_res} ${pano13_src})
    install(TARGETS pano13 ARCHIVE DESTINATION lib)
  ENDIF(BUILD_DLL)
  set(_pano_target pano13)
endif()

IF( CMAKE_SIZEOF_VOID_P EQUAL 8 AND MSVC )
	# Error detected with generator Visual Studio 10 Win64
	# > LINK : warning LNK4068: /MACHINE not specified; defaulting to X86
	# > fatal error LNK1112: module machine type 'x64' conflicts with target machine type 'X86'
	#
	# There is no default value for static libraries and cmake isn't setting it either.
	# We fix this by adding the flag manually.
	IF(NOT BUILD_DLL)
		SET_TARGET_PROPERTIES(pano13 PROPERTIES STATIC_LIBRARY_FLAGS \"/machine:x64\")
	ENDIF()
ENDIF()

install(FILES ${pano13_install_hdr} DESTINATION "include/pano13")

# install documentation expected by hugin install script
INSTALL(
  FILES
	AUTHORS
	COPYING
	README
  DESTINATION "include/pano13"
        )
INSTALL(
  FILES
        doc/Optimize.txt
        doc/PTblender.readme
        doc/PTmender.readme
        doc/stitch.txt
  DESTINATION "include/pano13/doc"
        )

set(TOP_SRC_DIR ${CMAKE_SOURCE_DIR})

add_subdirectory(man)
add_dependencies(${_pano_target} man)
add_subdirectory(tools)
add_dependencies(tools ${_pano_target})

macro(setstripped _varname _value)
  string(STRIP ${_value} _v)
  if(USE_POSIX_PACKAGING)
    string(TOLOWER ${_v} ${_varname})
  else()
    set(${_varname} ${_v})
  endif()
endmacro(setstripped)

file(STRINGS "${TOP_SRC_DIR}/configure.ac" _config_lines)
foreach(_c_l ${_config_lines})
  if(_c_l MATCHES "^AC_INIT\\(\\[\([^,]+\)\\], *\\[\([^,]+\)\\], *\([^,]+\)")
    setstripped(PACKAGE_BASE ${CMAKE_MATCH_1})
    setstripped(PACKAGE_VERSION ${CMAKE_MATCH_2})
    setstripped(PACKAGE_BUGREPORT ${CMAKE_MATCH_3})
    if(PACKAGE_VERSION MATCHES "^\([0-9]+\)\\.\([0-9]+\)\\.\([0-9]+\).*$")
      message(STATUS "found version = ${PACKAGE_VERSION}")
      set(PANO_MAJOR_VERSION ${CMAKE_MATCH_1})
      set(PANO_MINOR_VERSION ${CMAKE_MATCH_2})
      set(PANO_PATCH_VERSION ${CMAKE_MATCH_3})
    endif()
  endif()
endforeach(_c_l)

# create TAGS file
ADD_CUSTOM_COMMAND( OUTPUT ctags POST_BUILD
                   COMMAND ctags-exuberant -e *.c *.h tools/*.c
                   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/
                   COMMENT "Build TAGS file"
                 )
ADD_CUSTOM_TARGET( TAGS DEPENDS ctags)


# set some variables which are used in libpano13.pc.in
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX})
set(libdir ${CMAKE_INSTALL_PREFIX}/lib)
set(includedir ${CMAKE_INSTALL_PREFIX}/include)
set(LIB_JPEG ${JPEG_LIBRARY})
set(LIB_TIFF ${TIFF_LIBRARY})
set(LIB_ZLIB ${ZLIB_LIBRARY})
set(LIB_PNG ${PNG_LIBRARY})
set(VERSION ${PACKAGE_VERSION})

configure_file("${TOP_SRC_DIR}/libpano13.pc.in" "${CMAKE_BINARY_DIR}/libpano13.pc" @ONLY)

install(FILES "${CMAKE_BINARY_DIR}/libpano13.pc" DESTINATION "lib/pkgconfig")

set(CPACK_PACKAGE_VERSION_MAJOR "${PANO_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${PANO_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${PANO_PATCH_VERSION}")

# needed by rpm
set(CPACK_SET_DESTDIR "ON")

# the next ones are needed by deb
set(CPACK_PACKAGE_CONTACT "${PACKAGE_BUGREPORT}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "libpano13")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${TOP_SRC_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${TOP_SRC_DIR}/COPYING")

# Find the revision number and use it as the release in rpm-package-build.
# This way we may omit the otherwise needed "--force" parameter when
# installing from that rpm package.
# ... Disabled for the moment, because configure.ac supplies a value
IF(EXISTS ${PROJECT_SOURCE_DIR}/.hg)
  FIND_PROGRAM(_hg hg)
  if(_hg)
    execute_process(COMMAND ${_hg} summary
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE LIBPANO_TIP_OUT
        ERROR_VARIABLE LIBPANO_TIP_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    STRING(REGEX REPLACE "\n" ";" LIBPANO_TIP_LINES ${LIBPANO_TIP_OUT})
    foreach(_v_l ${LIBPANO_TIP_LINES})
      if(_v_l MATCHES ".*:[ ]+\([0-9]+\):[0-9a-f]+ tip$")
        set(CPACK_PACKAGE_VERSION_PATCH "${PANO_PATCH_VERSION}-${CMAKE_MATCH_1}hg")
        break()
      endif()
    endforeach()
  endif()
endif()

set(CPACK_SOURCE_PACKAGE_FILE_NAME "libpano13-${PANO_MAJOR_VERSION}.${PANO_MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")

# Use dpkg-shlibdeps to get the needed dependences
SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)


include(CPack)

enable_testing()
add_subdirectory(tests)
