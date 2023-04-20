import os
from conans import ConanFile


class Project(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package_multi"

    # def requirements(self):
    #     self.requires("protobuf/3.21.9")

    # def configure(self):
    #     self.options["qt"].shared = True
