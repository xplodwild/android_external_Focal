# This file is part of libpano13, the hugin build environment.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

project(man)

file(GLOB_RECURSE manuals RELATIVE "${TOP_SRC_DIR}/man" "${TOP_SRC_DIR}/man/*.1")

macro(add_manual _mansrc)
  get_filename_component(_mandest ${_mansrc} NAME_WE)
  SET(_created_manual "${CMAKE_CURRENT_BINARY_DIR}/${_mandest}.1")
  configure_file("${TOP_SRC_DIR}/man/${_mansrc}" ${_created_manual} @ONLY)
  SET_SOURCE_FILES_PROPERTIES(${_created_manual} GENERATED)
  LIST(APPEND _manuals ${_created_manual})
endmacro(add_manual)

set(_manuals)
foreach(_m ${manuals})
  # this is not needed now, but maybe we will later
  # adapt data like version or date into the manuals
  add_manual(${_m})
endforeach(_m)

add_custom_target(man ALL DEPENDS ${_manuals})

install(FILES ${_manuals} DESTINATION "share/man/man1")
