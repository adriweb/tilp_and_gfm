# Enabled by the Linux shared presets; normal system installations stay unchanged.
function(tilp_configure_bundled_target target)
    if(BUNDLE_LINUX_SHARED_DEPS)
        set_target_properties(${target} PROPERTIES
                INSTALL_RPATH "$ORIGIN/../${TILP_RUNTIME_LIBDIR}"
                INSTALL_RPATH_USE_LINK_PATH FALSE)
        target_compile_definitions(${target} PRIVATE RELOCATABLE_INSTALL
                SHARE_SUBDIR="${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}"
                LOCALE_SUBDIR="${CMAKE_INSTALL_LOCALEDIR}")
        set_property(GLOBAL APPEND PROPERTY TILP_BUNDLED_TARGETS ${target})
    endif()
endfunction()

function(tilp_install_linux_runtime)
    if(NOT BUNDLE_LINUX_SHARED_DEPS)
        return()
    endif()

    get_property(targets GLOBAL PROPERTY TILP_BUNDLED_TARGETS)
    set(executables "")
    foreach(target IN LISTS targets)
        list(APPEND executables "$<TARGET_FILE:${target}>")
    endforeach()
    set(search_directories "")
    foreach(library IN ITEMS "${LIBTICALCS2}" "${LIBTIFILES2}" "${LIBTICABLES2}" "${LIBTICONV}")
        get_filename_component(directory "${library}" DIRECTORY)
        list(APPEND search_directories "${directory}")
    endforeach()
    list(REMOVE_DUPLICATES search_directories)

    string(CONFIGURE [[
        set(executables "@executables@")
        set(search_directories "@search_directories@")
        file(GET_RUNTIME_DEPENDENCIES
                EXECUTABLES ${executables}
                DIRECTORIES ${search_directories}
                RESOLVED_DEPENDENCIES_VAR libraries
                UNRESOLVED_DEPENDENCIES_VAR missing
                CONFLICTING_DEPENDENCIES_PREFIX conflicts
                PRE_EXCLUDE_REGEXES
                    "^linux-vdso.*" "^ld-linux.*"
                    "^lib(c|m|dl|pthread|rt|resolv|util|anl|nss_.*|thread_db)[.]so")
        if(missing OR conflicts_FILENAMES)
            message(FATAL_ERROR "Cannot bundle runtime libraries: missing=${missing}; conflicting=${conflicts_FILENAMES}")
        endif()
        set(destination "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/@TILP_RUNTIME_LIBDIR@")
        file(INSTALL DESTINATION "${destination}" TYPE SHARED_LIBRARY
                FOLLOW_SYMLINK_CHAIN FILES ${libraries})
        # RUNPATH on an executable does not cover indirect dependencies. Give
        # every copied library a relative path to the rest of the bundle.
        file(GLOB installed_libraries "${destination}/*.so*")
        foreach(library IN LISTS installed_libraries)
            if(NOT IS_SYMLINK "${library}")
                execute_process(COMMAND "@PATCHELF_EXECUTABLE@" --set-rpath "$ORIGIN" "${library}"
                        RESULT_VARIABLE result ERROR_VARIABLE error)
                if(NOT result EQUAL 0)
                    message(FATAL_ERROR "Cannot set runtime path on ${library}: ${error}")
                endif()
            endif()
        endforeach()
    ]] install_code @ONLY)
    # Generate per configuration so target paths are resolved even with the
    # project's older install(CODE) generator-expression policy.
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/bundle-runtime-$<CONFIG>.cmake"
            CONTENT "${install_code}")
    install(CODE "include(\"${CMAKE_CURRENT_BINARY_DIR}/bundle-runtime-\${CMAKE_INSTALL_CONFIG_NAME}.cmake\")")
endfunction()
