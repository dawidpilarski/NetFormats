from conans import ConanFile, CMake

class NetFormatsConan(ConanFile):
    name = "NetFormats"
    version = "0.1"
    settings = "os", "compiler", "arch", "build_type"
    exports_sources = "src/*", "CMakeLists.txt"
    no_copy_source = True
    tool_requires = ['catch2/3.3.0', 'benchmark/1.7.1']
    generators=['cmake']

    def build(self):
        cmake = CMake(self)
        cmake.definitions['NF_BUILD_TESTS=ON']
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        self.copy("*.hpp")

    def package_id(self):
        self.info.clear()