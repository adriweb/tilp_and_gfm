cmake_minimum_required(VERSION 3.15)

project(tilp2
        VERSION     1.19
        LANGUAGES   C CXX)

file(GLOB HEADER_FILES src/*.h)
set(SRC_FILES
        ${HEADER_FILES}
        src/about.c
        src/action.c
        src/bookmark.c
        src/clist.c
        src/clist_rbm.c
        src/clock.c
        src/ctree.c
        src/ctree_rbm.c
        src/dboxes.c
        src/device.c
        src/dnd.c
        src/fileprop.c
        src/filesel.c
        src/gtk_gif.c
        src/gtk_update.c
        src/labels.c
        src/main.c
        src/manpage.c
        src/options.c
        src/pbars.c
        src/release.c
        src/screenshot.c
        src/scroptions.c
        src/splash.c
        src/support.c
        src/tilp.c
        src/tilp_calcs.c
        src/tilp_cmdline.c
        src/tilp_config.c
        src/tilp_device.c
        src/tilp_error.c
        src/tilp_files.c
        src/tilp_gif.c
        src/tilp_main.c
        src/tilp_misc.c
        src/tilp_paths.c
        src/tilp_screen.c
        src/tilp_slct.c
        src/tilp_update.c
        src/tilp_vars.c
        src/toolbar.c)

add_executable(tilp2 ${SRC_FILES})
# tilibs contains C++ objects even though this executable has C sources.
set_target_properties(tilp2 PROPERTIES LINKER_LANGUAGE CXX)
tilp_configure_bundled_target(tilp2)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
    set_target_properties(tilp2 PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
            INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
            INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
    )
else()
    message(STATUS "IPO/LTO not supported: <${error}>")
endif()

if(WIN32)
    target_include_directories(tilp2 PRIVATE "build/msvc")
endif()

# Static archives must precede the libraries that satisfy their references.
target_include_directories(tilp2 PRIVATE ${TILIBS_INCLUDES_DIR})
target_link_libraries(tilp2 PRIVATE ${LIBTICALCS2} ${LIBTIFILES2} ${LIBTICABLES2} ${LIBTICONV} ${TILIBS_PLATFORM_LIBRARIES})

# External deps
option(USE_GTK2 "Build with GTK+2 instead of GTK+3" OFF)
if(USE_GTK2)
    pkg_check_modules(GTK2 REQUIRED IMPORTED_TARGET gtk+-2.0>=2.10)
    target_link_libraries(tilp2 PRIVATE PkgConfig::GTK2)
else()
    pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
    target_link_libraries(tilp2 PRIVATE PkgConfig::GTK3)

    # GTK's file chooser uses GSettings at runtime. Static linking does not
    # embed the schema database, so portable installations need a compiled
    # copy instead of relying on GTK being installed globally.
    if(APPLE OR WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(GTK3_SCHEMA_SOURCE_DIR "${GTK3_PREFIX}/share/glib-2.0/schemas")
        file(GLOB GTK3_SCHEMA_FILES CONFIGURE_DEPENDS
                "${GTK3_SCHEMA_SOURCE_DIR}/org.gtk.Settings.*.gschema.xml")

        find_program(GLIB_COMPILE_SCHEMAS_EXECUTABLE
                NAMES glib-compile-schemas
                HINTS "${GTK3_PREFIX}/tools/glib" "${GTK3_PREFIX}/bin")

        if(NOT GTK3_SCHEMA_FILES)
            message(FATAL_ERROR
                    "No GTK GSettings schemas found in ${GTK3_SCHEMA_SOURCE_DIR}")
        endif()
        if(NOT GLIB_COMPILE_SCHEMAS_EXECUTABLE)
            message(FATAL_ERROR "glib-compile-schemas was not found")
        endif()

        set(GTK3_COMPILED_SCHEMA_DIR
                "${CMAKE_CURRENT_BINARY_DIR}/glib-2.0/schemas")
        set(GTK3_COMPILED_SCHEMA
                "${GTK3_COMPILED_SCHEMA_DIR}/gschemas.compiled")

        add_custom_command(
                OUTPUT "${GTK3_COMPILED_SCHEMA}"
                COMMAND "${CMAKE_COMMAND}" -E make_directory
                        "${GTK3_COMPILED_SCHEMA_DIR}"
                COMMAND "${GLIB_COMPILE_SCHEMAS_EXECUTABLE}"
                        "--targetdir=${GTK3_COMPILED_SCHEMA_DIR}"
                        "${GTK3_SCHEMA_SOURCE_DIR}"
                DEPENDS ${GTK3_SCHEMA_FILES}
                COMMENT "Compiling GTK GSettings schemas"
                VERBATIM)
        add_custom_target(tilp2_gsettings_schemas ALL
                DEPENDS "${GTK3_COMPILED_SCHEMA}")
        add_dependencies(tilp2 tilp2_gsettings_schemas)

        set(TILP_GSETTINGS_SCHEMA_SUBDIR
                "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/glib_schemas")
        target_compile_definitions(tilp2 PRIVATE
                GSETTINGS_SCHEMA_SUBDIR="${TILP_GSETTINGS_SCHEMA_SUBDIR}")
        install(FILES ${GTK3_SCHEMA_FILES} "${GTK3_COMPILED_SCHEMA}"
                DESTINATION "${TILP_GSETTINGS_SCHEMA_SUBDIR}")
    endif()
endif()

if(WIN32)
    find_library(LIBUSB0 NAMES libusb0.lib usb0 REQUIRED)
    target_link_libraries(tilp2 PRIVATE ${LIBUSB0})
else()
    pkg_check_modules(LibUSB REQUIRED IMPORTED_TARGET libusb-1.0>=1.0.16)
    target_link_libraries(tilp2 PRIVATE PkgConfig::LibUSB)
endif()

pkg_check_modules(glib REQUIRED IMPORTED_TARGET glib-2.0)
find_package(ZLIB REQUIRED) # for libtifiles
find_package(LibArchive REQUIRED) # for libticables
target_link_libraries(tilp2 PRIVATE ZLIB::ZLIB PkgConfig::glib LibArchive::LibArchive)

# Defines
set(SHAREDIR  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}")
target_compile_definitions(tilp2 PRIVATE PACKAGE="${PROJECT_NAME}" VERSION="${PROJECT_VERSION}" SHARE_DIR="${SHAREDIR}")

if(APPLE)
    find_library(UNIFORMTYPEIDENTIFIERS_FRAMEWORK UniformTypeIdentifiers REQUIRED)
    target_link_libraries(tilp2 PRIVATE "-Wl,-framework,CoreVideo -Wl,-framework,IOSurface -Wl,-framework,QuartzCore")
    target_link_libraries(tilp2 PRIVATE ${UNIFORMTYPEIDENTIFIERS_FRAMEWORK})
endif()

if(APPLE OR UNIX)
    set_target_properties(tilp2 PROPERTIES LINK_FLAGS "-rdynamic")  # rdynamic is needed for the GUI callback functions
endif()

# Takes care of the i18n po/pot/gmo/mo files
if(ENABLE_NLS)
    i18n_mo_from_po_pot()
    if(TARGET potfiles_1)
        add_dependencies(tilp2 potfiles_1)
    else()
        message(WARNING "tilp2 target potfiles_1 (i18n) is not available - did you run intltool-update?")
        remove_definitions(-DENABLE_NLS)
    endif()
endif()

# .app bundle for macOS - disabled for now, we'll have to check how to make packaging better (see CPack?)
#set_target_properties(tilp2 PROPERTIES MACOSX_BUNDLE TRUE)

# The binary should not have the "2" suffix, to match other build systems.
set_target_properties(tilp2 PROPERTIES OUTPUT_NAME "tilp")

# Install rules
install(TARGETS tilp2
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE  DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY   pixmaps help icons builder desktop
        DESTINATION "${SHAREDIR}")
install(FILES       man/tilp.1
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")
