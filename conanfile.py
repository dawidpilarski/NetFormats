from conan import ConanFile
from conan.tools.files import copy
from conan.tools.cmake import CMakeDeps, CMake, CMakeToolchain


class NetFormatsConan(ConanFile):
    name = "NetFormats"
    version = "0.1"
    settings = "os", "compiler", "arch", "build_type"
    exports_sources = "src/*", "CMakeLists.txt"
    no_copy_source = True
    test_requires = ['catch2/3.3.0']
    generators=['CMakeDeps']

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["NF_BUILD_TESTS"] = True
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.hpp")

    def package_id(self):
        self.info.clear()