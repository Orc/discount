cmake_minimum_required(VERSION 2.8.12)

project(DISCOUNT C)

file(READ "VERSION" ${PROJECT_NAME}_VERSION)
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
    "${CMAKE_CURRENT_LIST_DIR}/config.h"
    @ONLY)

configure_file(version.c.in
    "${CMAKE_CURRENT_LIST_DIR}/version.c"
    @ONLY)
set_property(SOURCE version.c APPEND PROPERTY COMPILE_DEFINITIONS
    BRANCH=""
    VERSION="${${PROJECT_NAME}_VERSION}")

configure_file(mkdio.h.in
    "${CMAKE_CURRENT_LIST_DIR}/mkdio.h"
    @ONLY)

include_directories("${CMAKE_CURRENT_LIST_DIR}")

add_executable(mktags mktags.c)

add_custom_command(OUTPUT blocktags
    COMMAND mktags > blocktags
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

add_library(libmarkdown
    mkdio.c
    markdown.c
    dumptree.c
    generate.c
    resource.c
    docheader.c
    version.c
    toc.c
    css.c
    xml.c
    Csio.c
    xmlpage.c
    basename.c
    emmatch.c
    github_flavoured.c
    setup.c
    blocktags tags.c
    html5.c
    flags.c)

set_target_properties(libmarkdown PROPERTIES
    OUTPUT_NAME markdown)

add_library(common OBJECT
    pgm_options.c
    gethopt.c)

add_executable(markdown
    main.c
    $<TARGET_OBJECTS:common>)

target_link_libraries(markdown PRIVATE libmarkdown)

add_executable(mkd2html
    mkd2html.c
    $<TARGET_OBJECTS:common>
    notspecial.c)

target_link_libraries(mkd2html PRIVATE libmarkdown)

add_executable(makepage
    makepage.c
    $<TARGET_OBJECTS:common>)

target_link_libraries(makepage PRIVATE libmarkdown)
