cmake_minimum_required(VERSION 2.8.12)

project(DISCOUNT C)

get_filename_component(_ROOT "${CMAKE_CURRENT_LIST_DIR}" PATH)

file(READ "${_ROOT}/VERSION" ${PROJECT_NAME}_VERSION)
string(STRIP "${${PROJECT_NAME}_VERSION}" ${PROJECT_NAME}_VERSION)

set(${PROJECT_NAME}_WITH_TABSTOPS "4" CACHE STRING
    "Set tabstops to N characters (default is 4)")
set(TABSTOP "${${PROJECT_NAME}_WITH_TABSTOPS}")

set(${PROJECT_NAME}_MAKE_INSTALL ON CACHE BOOL
    "Set to OFF to disable install rules (default is ON)")

set(${PROJECT_NAME}_INSTALL_SAMPLES OFF CACHE BOOL
    "Set to ON to install sample programs (default is OFF)")

# Check headers
include(CheckIncludeFile)
check_include_file(libgen.h HAVE_LIBGEN_H)
check_include_file(pwd.h HAVE_PWD_H)
check_include_file(alloca.h HAVE_ALLOCA_H)
check_include_file(malloc.h HAVE_MALLOC_H)
check_include_file(sys/stat.h HAVE_STAT)

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

# Check symbols
include(CheckSymbolExists)
foreach(_symbol
        bzero
        strcasecmp _stricmp
        strncasecmp _strnicmp)
    string(TOUPPER ${_symbol} _SYMBOL)
    check_symbol_exists(${_symbol} string.h HAVE_${_SYMBOL})
endforeach()
check_symbol_exists(random stdlib.h HAVE_RANDOM)
check_symbol_exists(srandom stdlib.h HAVE_SRANDOM)
check_symbol_exists(getpwuid pwd.h HAVE_GETPWUID)
check_symbol_exists(basename libgen.h HAVE_BASENAME)
check_symbol_exists(fchdir unistd.h HAVE_FCHDIR)
if(HAVE_STAT)
    check_symbol_exists(S_ISCHR sys/stat.h HAVE_S_ISCHR)
    check_symbol_exists(S_ISFIFO sys/stat.h HAVE_S_ISFIFO)
    check_symbol_exists(S_ISSOCK sys/stat.h HAVE_S_ISSOCK)
endif()

if(NOT HAVE_BZERO)
    set(DEFINE_BZERO "#define bzero(p, n) memset(p, 0, n)")
endif()

if(NOT HAVE_STRCASECMP)
    if(HAVE__STRICMP)
        set(DEFINE_STRCASECMP "#define strcasecmp _stricmp")
    else()
        set(DEFINE_STRCASECMP "#error The symbol strcasecmp is not defined.")
    endif()
endif()

if(NOT HAVE_STRNCASECMP)
    if(HAVE__STRNICMP)
        set(DEFINE_STRNCASECMP "#define strncasecmp _strnicmp")
    else()
        set(DEFINE_STRNCASECMP "#error The symbol strncasecmp is not defined.")
    endif()
endif()

if(NOT HAVE_S_ISCHR OR NOT HAVE_S_ISFIFO OR NOT HAVE_S_ISSOCK)
    set(HAVE_STAT "")
endif()

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

add_custom_command(OUTPUT "${_ROOT}/blocktags"
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

if(${PROJECT_NAME}_MAKE_INSTALL)
    string(TOLOWER ${PROJECT_NAME} _PACKAGE_NAME)
    include(GNUInstallDirs)
    if(NOT DEFINED CMAKE_INSTALL_CMAKEDIR)
        set(CMAKE_INSTALL_CMAKEDIR
            "${CMAKE_INSTALL_LIBDIR}/cmake/${_PACKAGE_NAME}"
            CACHE STRING "CMake packages")
    endif()
    install(FILES "${_ROOT}/mkdio.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    target_include_directories(libmarkdown INTERFACE
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    set(_TARGETS libmarkdown markdown)
    if(${PROJECT_NAME}_INSTALL_SAMPLES)
        list(APPEND _TARGETS mkd2html makepage)
    endif()
    install(TARGETS ${_TARGETS} EXPORT ${_PACKAGE_NAME}-targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
    install(EXPORT ${_PACKAGE_NAME}-targets
        NAMESPACE ${_PACKAGE_NAME}::
        DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
      "${CMAKE_CURRENT_BINARY_DIR}/${_PACKAGE_NAME}-config-version.cmake"
      VERSION ${${PROJECT_NAME}_VERSION}
      COMPATIBILITY AnyNewerVersion
    )
    configure_file("${CMAKE_CURRENT_LIST_DIR}/discount-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${_PACKAGE_NAME}-config.cmake"
        @ONLY)
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${_PACKAGE_NAME}-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${_PACKAGE_NAME}-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
    unset(_TARGETS)
    unset(_PACKAGE_NAME)
endif()

unset(_ROOT)
