# Viper Developer Guide <!-- {docsify-ignore-all} -->

The following guide provides steps to build and run Viper.

## Prerequisite 

* [make](https://www.gnu.org/software/make/)
* [CMAKE](https://cmake.org/download/)
* [GCC](https://gcc.gnu.org/install/binaries.html)

## Build Steps

You can follow either of the below two methods to build & generate Viper executables that can run Viper interpreter & scripts.

### Method 1: Build & Install Viper (Easy)

1) Download Viper repository from Github: `git clone https://github.com/rahuldshetty/viper.git`
2) Open terminal and switch to Viper's root directory: `cd viper`
2) Run *install.sh* script to install Viper executable in your system: `sudo sh install.sh`

You can use the terminal and directly run Viper with the command: **viper**

### Method 2: Build & Generate Viper Executable

1) Download Viper repository from Github: `git clone https://github.com/rahuldshetty/viper.git`
2) Open terminal and switch to Viper's cmake directory: `cd viper/cmake`
3) Use cmake tool to build make dependencies: `cmake .`.
4) Use the make tool to compile and generate Viper executables for your platform: `make`

This will create Viper binaries under "cmake/bin" directory.