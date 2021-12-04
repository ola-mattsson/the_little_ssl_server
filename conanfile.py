from conans import ConanFile, CMake

class TheApp(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"
    default_options = {"boost:header_only": True, "libcurl:with_nghttp2": True}

    def requirements(self):
        self.requires("fmt/7.1.3")
        self.requires("boost/1.69.0")
        self.requires("openssl/1.1.1k")
        self.requires("zlib/1.2.11")

        # self.requires("libpq/11.11")
        # self.requires("libuuid/1.0.3")
        # self.requires("libcurl/7.75.0")

    def build_requirements(self):
        self.requires("cmake/3.21.4")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
