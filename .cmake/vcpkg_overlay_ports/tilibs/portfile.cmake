vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO adriweb/tilibs
    REF 30293faf4081c63273d88572fb9ddaa427126a88
    SHA512 e774d04b9adb4a03fbf0f9e1390ad80de37f4a6ef65383f19c1864c9449f3f009dd7b1516be21bb8f5eb127ae955936c927b5136ea0a12a1a9e69fbc6adda844
    HEAD_REF experimental2
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
        -DBUILD_TIFILEUTIL=OFF
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
