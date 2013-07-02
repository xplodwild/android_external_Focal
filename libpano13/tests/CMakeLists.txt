# This file is part of libpano13, the hugin build environment.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

project(test)

macro(prepare_test _testname _prjname _imagemask)
  file(GLOB images  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${_imagemask}")
  file(GLOB references  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/reference/*")
  set(_depends)
  foreach(_i ${images} ${references})
#    if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_i}")
#      add_custom_command(
#        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_i}"
#        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${_i}" "${CMAKE_CURRENT_BINARY_DIR}/${_i}"
#        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_i}")
#      SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_BINARY_DIR}/${_i}" GENERATED)
#      list(APPEND _depends "${CMAKE_CURRENT_BINARY_DIR}/${_i}")
#    endif()
  endforeach(_i)
  add_custom_target(${_prjname} ALL DEPENDS ${_depends})

  add_test(${_testname} ${PERL_EXECUTABLE} "${TOP_SRC_DIR}/tests/panoAutomateTest.pl" "${CMAKE_CURRENT_SOURCE_DIR}/simple.txt")
  add_dependencies(${_prjname} tools)
endmacro()


macro(prepare_test_psd2 _testname _prjname _output _image _image2)
  set(_depends)

  add_custom_target(${_prjname} ALL DEPENDS ${_depends})
  add_dependencies(${_prjname} tools)
endmacro()


find_package(Perl)
if(PERL_FOUND)
    add_subdirectory(simpleStitch)
    add_subdirectory(simpleTiff16)
    add_subdirectory(simpleTiff2psd)
endif()
