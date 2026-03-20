from conan import ConanFile
from conan.tools.files import copy
import os


class CrashCatchConan(ConanFile):
    name = "crashcatch"
    version = "1.4.0"
    description = "A cross-platform, single-header C++ crash-reporting library for modern C++ applications."
    license = "MIT"
    url = "https://github.com/keithpotz/CrashCatch"
    homepage = "https://github.com/keithpotz/CrashCatch"
    topics = ("crash-reporting", "crash-handler", "minidump", "header-only", "single-header")
    package_type = "header-library"
    no_copy_source = True

    # Bundles headers and license into the Conan cache for local builds.
    # For ConanCenter submission: remove exports_sources and replace with a
    # source() method that calls get() to fetch the release tarball from GitHub.
    exports_sources = "include/*.hpp", "LICENSE"

    def package_id(self):
        # Header-only: binary is the same regardless of compiler/settings
        self.info.clear()

    def package(self):
        copy(self, "LICENSE",
             src=self.source_folder,
             dst=os.path.join(self.package_folder, "licenses"))
        copy(self, "*.hpp",
             src=os.path.join(self.source_folder, "include"),
             dst=os.path.join(self.package_folder, "include"))

    def package_info(self):
        # No compiled lib — only headers
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []

        # Use get_safe() because package_id() clears settings for the binary ID.
        # Expose the platform system libraries consumers must link against.
        os_name = self.settings.get_safe("os")
        if os_name == "Windows":
            self.cpp_info.system_libs = ["DbgHelp", "User32"]
        elif os_name == "Linux":
            # backtrace() and dladdr() live in libdl on some distros
            self.cpp_info.system_libs = ["dl"]
