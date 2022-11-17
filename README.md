# Trillbit SDK integrated with Linux GCC based platforms. 

This repository demostrates the integration of Trillbit SDK with a general linux based system. There are two example codes provided : generator and decoder, and there are two modes of function in both of them : read from/write to a wav file or play/decode from a speaker/microhphone. More details about the code can be understood by going through the implementation

## Platforms supported

Currently two platforms are supported
1. x86_64 - Uses the conventional GNU toolchain provided for the x86_64 architecture. 
2. aarch64 - Uses the aarch64-linux-gnu-* toolchain provided for aarch64 architecture.

## Dependencies

This example assumes sndfile, ALSA are available in the x86_64 platform. The same are provided for aarch64 platform

For cross-compiling purposes gcc-aarch64-linux-gnu is assumed to be present in the system.

To install all the dependencies for x86_64:

    sudo apt-get install gcc-aarch64-linux-gnu libsndfile1-dev libasound2-dev

All dependencies required to cross-compile for aarch64 are provided in the platforms/aarch64/external folder.

## Build process

First create the build directory

    mkdir build && cd build

Next invoke the cmake command. 
The cmake command expects a -DPLATFORM_OPTION=<x86_64/aarch64> tag , if nothing is given x86_64 is assumed.

To cross compile for aarch64 from a x86_64, an additional -DCMAKE_TOOLCHAIN_FILE="../toolchains/aarch64_gcc_linux.cmake" must be provided. 

To have a local install path -DCMAKE_INSTALL_PREFIX="../install" can be given

Normal build:

    cmake .. -DPLATFORM_OPTION=<x86_64/aarch64> -DCMAKE_INSTALL_PREFIX="../install"

Cross-compiling:

    cmake .. -DPLATFORM_OPTION=<x86_64/aarch64> -DCMAKE_INSTALL_PREFIX="../install" -DCMAKE_TOOLCHAIN_FILE="../toolchains/aarch64_gcc_linux.cmake"

## Prebuilt files

Platform specific prebuilt binaries can also found in the *platforms/\<arch\>/prebuilt/bin* folder, where arch can be x86_64/aarch64. 

The library for integration can also be found in the *platforms/\<arch\>/prebuilt/lib/GCC* folder as a static library that can be compiled with the application in which it will be integrated in.

For mode details refer to [README.md](platforms/README.md).

## Licensing
    
Trillbit SDK will be functional only after the device on which the SDK will run on is licensed on the Trillbit backend. For this a one time API call with the UID of the device, on which the SDK will be integrated into, will be requiered.

Please refer to the [README.md](scripts/license/README.md) in the scripts/license folder for exact steps of licensing. 

The output of the licensing procedure trillbit.lic must be placed in the same directory of the built examples. For custom integrated applications, only the license data can be used directly before compilation.


