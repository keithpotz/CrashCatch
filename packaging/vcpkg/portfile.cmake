# CrashCatch is a header-only library — no compilation required.
set(VCPKG_BUILD_TYPE release)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO keithpotz/CrashCatch
    REF "v${VERSION}"
    # Run the following after tagging the release to get the correct SHA512:
    #   vcpkg install crashcatch --overlay-ports=packaging/vcpkg
    # vcpkg will print the expected SHA512 in the error output if this is wrong.
    SHA512 0
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME CrashCatch
    CONFIG_PATH lib/cmake/CrashCatch
)

# Header-only: no debug artifacts
file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
    "${CURRENT_PACKAGES_DIR}/debug"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/usage"
    "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage"
    COPYONLY
)
