vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO adriweb/tilibs
    REF 2c4864bf248cd0c8b47908b90fb653bc40b94c4b
    SHA512 5c2205f704d6ee3e1a3d63ebe275a044357914c4212743e00c9ca73033c31ae34049fbe6cf17c16cf0d5c6e3327aa0be6b07fc52f627fd8f3a2a55d85aa899d7
    HEAD_REF experimental2
)

# The application's install-prefix override must not escape vcpkg's staging
# directory when the upstream tilibs CMake project reads the same variable.
unset(ENV{CMAKE_INSTALL_PREFIX_OVERRIDE})

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
        -DBUILD_TIFILEUTIL=OFF
        -DBUILD_ROM_DUMPERS=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
)

if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    file(REMOVE_RECURSE
        "${CURRENT_PACKAGES_DIR}/debug/bin"
        "${CURRENT_PACKAGES_DIR}/bin"
    )
endif()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/libticalcs/trunk/COPYING")
