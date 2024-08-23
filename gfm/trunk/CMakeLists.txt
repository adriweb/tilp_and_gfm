cmake_minimum_required(VERSION 3.12)

project(gfm
        VERSION     1.09
        LANGUAGES   C CXX)

file(GLOB HEADER_FILES src/*.h)
set(SRC_FILES
        ${HEADER_FILES}
        src/cmdline.c
        src/ctree.c
        src/dialog.c
        src/file.c
        src/filesel.c
        src/gui.c
        src/labels.c
        src/main.c
        src/paths.c
        src/rwgroup.c
        src/splashscreen.c
        src/support.c
        src/tilibs.c
        src/ungroup.c)

add_executable(gfm ${SRC_FILES})

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
    set_target_properties(gfm PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
            INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
            INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
    )
else()
    message(STATUS "IPO/LTO not supported: <${error}>")
endif()

if(WIN32)
    target_include_directories(gfm PRIVATE "build/msvc")
endif()

# External deps
find_package(GTK2 2.10 REQUIRED gtk glade)
find_package(TIFF REQUIRED) # from gtk deps
find_package(PNG REQUIRED) # from gtk deps
find_package(ZLIB REQUIRED) # for libtifiles
find_package(LibArchive REQUIRED)
pkg_check_modules(DEPS REQUIRED IMPORTED_TARGET glib-2.0 libffi)

if(WIN32)
    find_library(LIBUSB0 NAMES libusb0.lib usb0 REQUIRED)
    target_link_libraries(gfm PRIVATE ${LIBUSB0})
else()
    pkg_check_modules(LibUSB REQUIRED IMPORTED_TARGET libusb-1.0>=1.0.16)
    target_link_libraries(gfm PRIVATE PkgConfig::LibUSB)
endif()

target_include_directories(gfm PRIVATE ${GTK2_INCLUDE_DIRS} ${TILIBS_INCLUDES_DIR})
target_link_libraries(gfm PRIVATE ${GTK2_LIBRARIES} PNG::PNG TIFF::TIFF ZLIB::ZLIB LibArchive::LibArchive PkgConfig::DEPS)
target_link_libraries(gfm PRIVATE ${LIBTICONV} ${LIBTIFILES2} ${LIBTICABLES2} ${LIBTICALCS2})

if (APPLE)
    target_link_libraries(gfm PRIVATE "resolv") # for gio for gtk...
endif()

# Defines
set(SHAREDIR  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}")
target_compile_definitions(gfm PRIVATE PACKAGE="${PROJECT_NAME}" VERSION="${PROJECT_VERSION}" SHARE_DIR="${SHAREDIR}")

if(APPLE OR UNIX)
    set_target_properties(gfm PROPERTIES LINK_FLAGS "-rdynamic")  # rdynamic is needed for the GUI callback functions
endif()

# Takes care of the i18n po/pot/gmo/mo files
if(ENABLE_NLS)
    i18n_mo_from_po_pot()
    if(TARGET potfiles_2)
        add_dependencies(gfm potfiles_2)
    else()
        message(WARNING "gfm target potfiles_2 (i18n) is not available - did you run intltool-update?")
        remove_definitions(-DENABLE_NLS)
    endif()
endif()

# .app bundle for macOS - disabled for now, we'll have to check how to make packaging better (see CPack?)
#set_target_properties(gfm PROPERTIES MACOSX_BUNDLE TRUE)

# Install rules
install(TARGETS gfm
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE  DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY   pixmaps help icons glade desktop
        DESTINATION "${SHAREDIR}")
install(FILES       man/gfm.1
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")
