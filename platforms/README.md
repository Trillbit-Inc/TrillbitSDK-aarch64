# Platform specific files

This folder contains platform specific files.

The platforms currently supported are :
1. x86_64
2. aarch64

In each folder, *external* and *prebuilt* folders can be found.

*external* folder contains external dependencies required for the functionality of the examples provided.

*prebuilt* folder contains 
1. *bin* folder which has the prebuilt binaries for that particular platform. Before running this, a license file with the name *trillbit.lic* must be placed in this folder. For more details on how to obtain the license file refer to [README.md](../scripts/license/README.md). For mode details on using the binary refer to [README.md](platforms/x86_64/prebuilt/bin/README.md).
2. *lib/GCC* folder which contains the prebuilt Trillbit library for that particular architecture. This library contains the function calls required to run the Trillbit algorithm and can be used to integrate with any application.