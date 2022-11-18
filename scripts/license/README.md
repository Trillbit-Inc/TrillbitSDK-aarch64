## Setup

Create a python virtual environment:

        python -m venv <venv_dir>

To activate the virtual environment:
On Windows:

        <venv_dir>\Scripts\activate.bat

On Ubuntu/Linux:

        source <venv_dir>/bin/activate

Install the following dependencies:

        pip install pyserial requests

## Pre-usage 

Before running the license scripts ,*trillbit_platform_credentials.json* must be downloaded from the Trillbit portal.

##  Usage

The python script *license_device_script.py* will be used to fetch the licesense from the server for the device that the SDK will be integrated into. 

The script requires two arguments to be functional.

Path to *trillbit_platform_credentials.json*, which contains API access keys required for the API call.

The device ID can be obtained by running any of the binary on the device that the SDK will be integrated into. 

For example , to run the binary *decode* present in the *platforms/arch/prebuilt/bin/* folder, enter
        ./decode default

Refer to [README.md](../../platforms/x86_64/prebuilt/bin/README.md) on how to run the binary.

The tool will not proceed without the license key but will print out the device ID that needs to be input into this python script.

After these two arguments are ready run :

        cd scripts/license
        python license_device_script.py <path/to/trillbit_platform_credentials.json> <device_ID>

This fetches and stores the license in a *trillbit.lic* file, which contains the license information that must be used in all Trillbit SDK integrated applications.

# Using the license file

For the current examples, the trillbit.lic (name must be the same), must be placed in the same directory as the binary. 

For example, to run prebuilt binaries on x86_64: Copy the trillbit.lic into *platforms/arch/prebuilt/bin/* and run the generate/decode program with the necessary arguments. Refer : [README.md](../../platforms/aarch64/prebuilt/bin/README.md)

The SDK expects the license as an input in the initialization parameter, b64_license. Provided in the example code is a way to read the trillbit.lic file into a buffer and point the buffer to the b64_license. This can be done in any other way provided the contents of the file be copied into a variable in the application and used in b64_license of the init parameters before calling the trill_init function.
