##################################
# CMake support for tilp and gfm #
##################################
#
# This aims to provide a much simpler way to build and install tilp and gfm
# on computers that have decent and recent OS and toolchains.
# As a bonus, it provides better support for CMake-oriented IDE (e.g. CLion).
#
# Features:
#   - builds and installs both executables
#   - creates and installs i18n .mo files (when libs/tools are present)
#
# Caveats:
#   - you'll need to generate the pot files first (intltool-update --pot)
#   - not all the autotools/configure options are available
#   - probably doesn't work very well with ancient toolchains or on uncommon OSes
#
# In the future...:
#   - TODO: look at CPack features for bundling/packaging
#   - WISH: do not hardcode the (auto-generated...) potfiles_* target names
#

cmake_minimum_required(VERSION 3.12)

option(DEPS_RELEASE_ONLY "Build only release versions of vcpkg dependencies" OFF)
if(DEPS_RELEASE_ONLY)
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        message(FATAL_ERROR "Must provide a VCPKG_TARGET_TRIPLET to set as release only")
    endif()
    if(NOT VCPKG_TARGET_TRIPLET MATCHES "-release$")
        set(VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_TRIPLET}-release")
        message("Updated VCPKG_TARGET_TRIPLET to ${VCPKG_TARGET_TRIPLET}")
    endif()
endif()

if(WIN32 AND VCPKG_TARGET_TRIPLET MATCHES "-static(-|$)" AND NOT VCPKG_TARGET_TRIPLET MATCHES "-md(-|$)")
    message("Using static MSVC runtime...")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(tilp_and_gfm C CXX)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# In case there was an env-var override...
if(DEFINED ENV{CMAKE_INSTALL_PREFIX_OVERRIDE})
    set(CMAKE_INSTALL_PREFIX "$ENV{CMAKE_INSTALL_PREFIX_OVERRIDE}")
endif()
# Be sure to have the expected install prefix format
if(CMAKE_INSTALL_PREFIX)
    file(TO_CMAKE_PATH "${CMAKE_INSTALL_PREFIX}" CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "CMAKE_INSTALL_PREFIX" FORCE)
    message("CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
endif()

set(USED_CMAKE_GENERATOR "${CMAKE_GENERATOR}" CACHE STRING "Expose CMAKE_GENERATOR" FORCE)
message(STATUS "Detected system: ${CMAKE_SYSTEM_NAME} - host processor: ${CMAKE_HOST_SYSTEM_PROCESSOR} - CXX_COMPILER: ${CMAKE_CXX_COMPILER_ID}")

# Our modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/.cmake/*.cmake")
foreach(file ${files})
    include(${file})
endforeach()

include(GNUInstallDirs)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /D_CRT_SECURE_NO_WARNINGS")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # sane defaults + hardening
    set(GLOBAL_COMPILE_FLAGS "-W -Wall -Wextra -Wno-unused-parameter -Werror=shadow -Werror=write-strings -Wredundant-decls -Werror=date-time -Werror=return-type -Werror=pointer-arith")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GLOBAL_COMPILE_FLAGS} -Werror=declaration-after-statement -Werror=implicit-function-declaration -Werror=missing-prototypes")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GLOBAL_COMPILE_FLAGS}")
    # useful flags for debugging
    set(GLOBAL_DEBUG_FLAGS "-fno-omit-frame-pointer -fsanitize=address,bounds -fsanitize-undefined-trap-on-error")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${GLOBAL_DEBUG_FLAGS} ")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${GLOBAL_DEBUG_FLAGS} ")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${GLOBAL_DEBUG_FLAGS} ")
    # only for clang/gcc
    add_compile_definitions(HAVE_FVISIBILITY=1)
endif()

if(APPLE)
    add_compile_definitions(__MACOSX__)
elseif(UNIX)
    add_compile_definitions(__LINUX__)
    set(LINUX TRUE)
elseif(MINGW)
    add_compile_definitions(__MINGW32__)
elseif(WIN32)
    add_compile_definitions(__WIN32__)
    add_compile_definitions(HAVE_CONFIG_H)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    include_directories(${CMAKE_SOURCE_DIR}/win32_config)
endif()

# i18n support checking
include(FindGettext)
include(FindIntl)
if(Intl_FOUND AND GETTEXT_FOUND)
    set(ENABLE_NLS 1)
    include_directories(${Intl_INCLUDE_DIR})
    add_compile_definitions(ENABLE_NLS=1)
    link_directories(${Intl_LIBRARY_DIRS})
    link_libraries(${Intl_LIBRARIES})
else()
    message(WARNING "The Intl and GetText libs are needed for translations - Only English will be available")
endif()
set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LOCALEDIR}")

# Global defines
add_compile_definitions(ENABLE_LOGGING=1 LOCALEDIR="${LOCALEDIR}")
if (NOT BUILD_SHARED_LIBS OR VCPKG_TARGET_TRIPLET MATCHES "-static" OR NOT VCPKG_TARGET_TRIPLET MATCHES "-dynamic")
    # When we link statically, we don't want the dllimport stuff, so let's workaround that here
    add_compile_definitions(TICALCS_EXPORTS TIFILES_EXPORTS TICONV_EXPORTS TICABLES_EXPORTS)
endif()

# Don't mess up vcpkg libs finding
if(CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg" AND DEFINED VCPKG_TARGET_TRIPLET)
    set(ENV{PKG_CONFIG_PATH} "")
else()
    # Set manually when installed with Homebrew, see https://github.com/Homebrew/legacy-homebrew/issues/45891
    # Note: we can't do the same for libiconv because it exports libiconv_* symbols and not iconv_*
    if(APPLE)
        set(BREW_LIB_PATHS "/opt/homebrew/lib" "/opt/homebrew/opt" "/usr/local/opt" "/usr/local/lib")
        foreach(LIB_DIR ${BREW_LIB_PATHS})
            if(EXISTS "${LIB_DIR}/libarchive/include/archive.h")
                set(LibArchive_INCLUDE_DIR "${LIB_DIR}/libarchive/include" CACHE INTERNAL "LibArchive_INCLUDE_DIR")
                break()
            endif()
        endforeach()
    endif()
endif()
find_package(PkgConfig)

# Both TILP and GFM need those
find_library(LIBTICONV    NAMES ticonv REQUIRED)
find_library(LIBTIFILES2  NAMES tifiles2 REQUIRED)
find_library(LIBTICABLES2 NAMES ticables2 REQUIRED)
find_library(LIBTICALCS2  NAMES ticalcs2 REQUIRED)
find_path(TILIBS_INCLUDES_DIR NAMES ticalcs.h PATH_SUFFIXES tilp2 REQUIRED)

add_subdirectory(tilp/trunk)

find_package(GTK2 COMPONENTS gtk glade QUIET)
if(GTK2_GLADE_LIBRARY)
    add_subdirectory(gfm/trunk)
else()
    message(WARNING "gtk+2 and/or libglade are not available on your system, gfm will not be built")
endif()
