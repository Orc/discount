cmake_minimum_required(VERSION 2.8.12)

project(DISCOUNT C)

get_filename_component(_ROOT "${CMAKE_CURRENT_LIST_DIR}" PATH)

file(READ "${_ROOT}/VERSION" ${PROJECT_NAME}_VERSION)
string(STRIP "${${PROJECT_NAME}_VERSION}" ${PROJECT_NAME}_VERSION)

set(${PROJECT_NAME}_WITH_TABSTOPS "4" CACHE STRING
    "Set tabstops to N characters (default is 4)")
set(TABSTOP "${${PROJECT_NAME}_WITH_TABSTOPS}")

# Types detection (from configure.inc: AC_SCALAR_TYPES ())
include(CheckTypeSize)
check_type_size("unsigned long" SIZEOF_ULONG BUILTIN_TYPES_ONLY)
check_type_size("unsigned int" SIZEOF_UINT BUILTIN_TYPES_ONLY)
check_type_size("unsigned short" SIZEOF_USHORT BUILTIN_TYPES_ONLY)

if(SIZEOF_ULONG EQUAL 4)
    set(DWORD "unsigned long")
elseif(SIZEOF_UINT EQUAL 4)
    set(DWORD "unsigned int")
else()
    message(FATAL_ERROR "Could not detect DWORD type")
endif()

if(SIZEOF_UINT EQUAL 2)
    set(WORD "unsigned int")
elseif(SIZEOF_USHORT EQUAL 2)
    set(WORD "unsigned short")
else()
    message(FATAL_ERROR "Could not detect WORD type")
endif()

set(BYTE "unsigned char")

configure_file(config.h.in
    "${_ROOT}/config.h"
    @ONLY)

configure_file("${_ROOT}/version.c.in"
    "${_ROOT}/version.c"
    @ONLY)
set_property(SOURCE "${_ROOT}/version.c" APPEND PROPERTY COMPILE_DEFINITIONS
    BRANCH=""
    VERSION="${${PROJECT_NAME}_VERSION}")

configure_file("${_ROOT}/mkdio.h.in"
    "${_ROOT}/mkdio.h"
    @ONLY)

include_directories("${_ROOT}")

add_executable(mktags
    "${_ROOT}/mktags.c")

add_custom_command(OUTPUT blocktags
    COMMAND mktags > blocktags
    WORKING_DIRECTORY "${_ROOT}")

add_library(libmarkdown
    "${_ROOT}/mkdio.c"
    "${_ROOT}/markdown.c"
    "${_ROOT}/dumptree.c"
    "${_ROOT}/generate.c"
    "${_ROOT}/resource.c"
    "${_ROOT}/docheader.c"
    "${_ROOT}/version.c"
    "${_ROOT}/toc.c"
    "${_ROOT}/css.c"
    "${_ROOT}/xml.c"
    "${_ROOT}/Csio.c"
    "${_ROOT}/xmlpage.c"
    "${_ROOT}/basename.c"
    "${_ROOT}/emmatch.c"
    "${_ROOT}/github_flavoured.c"
    "${_ROOT}/setup.c"
    "${_ROOT}/blocktags" "${_ROOT}/tags.c"
    "${_ROOT}/html5.c"
    "${_ROOT}/flags.c")

set_target_properties(libmarkdown PROPERTIES
    OUTPUT_NAME markdown)

add_library(common OBJECT
    "${_ROOT}/pgm_options.c"
    "${_ROOT}/gethopt.c")

add_executable(markdown
    "${_ROOT}/main.c"
    $<TARGET_OBJECTS:common>)

target_link_libraries(markdown PRIVATE libmarkdown)

add_executable(mkd2html
    "${_ROOT}/mkd2html.c"
    $<TARGET_OBJECTS:common>
    "${_ROOT}/notspecial.c")

target_link_libraries(mkd2html PRIVATE libmarkdown)

add_executable(makepage
    "${_ROOT}/makepage.c"
    $<TARGET_OBJECTS:common>)

target_link_libraries(makepage PRIVATE libmarkdown)

unset(_ROOT)
