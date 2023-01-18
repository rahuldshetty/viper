# Viper Developer Guide <!-- {docsify-ignore-all} -->

The following guide provides steps to build and run Viper.

## Prerequisite 

* make tool - [Windows](https://gnuwin32.sourceforge.net/packages/make.htm) [Other Platforms](https://www.gnu.org/software/make/)
* [CMAKE](https://cmake.org/download/)
* [GCC](https://gcc.gnu.org/install/binaries.html)

## Building Viper

1) Download Viper repository from Github: `git clone https://github.com/rahuldshetty/viper.git`
2) Open terminal and switch to Viper's cmake directory: `cd viper/cmake`
3) Use cmake tool to build make dependencies: `cmake .`.
4) Use the make tool to compile and generate Viper executables for your platform: `make`

This will create Viper binaries under "cmake/bin" directory.