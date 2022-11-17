## Setup

Create a python virtual environment:

        python -m venv <venv_dir>

Activate the virtual environment on Windows

        <venv_dir>\Scripts\activate.bat

Activate the virtual environment on Ubuntu/Linux

        source <venv_dir>/bin/activate

Install the following dependencies:

        pip install pyserial requests

## Pre-usage 

Before running the license scripts ,*client_secret_json_file* must be got downloaded from the Trillbit portal.

##  Usage

The python script *license_device_script.py* will be used to fetch the licesense from the server for the device that the SDK will be integrated into. 

The script requires two arguments to be functional.

1. Path to *client_secret_json_file*, which contains API access keys required for the API call.
2. The device ID, can be got by running any of the tools program on the device that the SDK will be integrated into. The tool will not proceed without the license key but will print out the device ID that needs to be input into this python script.

After these two arguments are ready run :

        cd scripts/license
        python license_device_script.py <client_json_file> <device_ID>

This fetches and stores the license in a trillbit.lic file, which must be used as a license file for all Trillbit SDK integrated applications. 

# Using the license file

For the current examples, the trillbit.lic (name must be the same), must be placed in the same directory as to where the executable resides. 

For example, to run prebuilt binaries on x86_64: Copy the trillbit.lic into platforms/x86_64/prebuilt/bin/ and run the generate/decode program with the necessary arguments.

The SDK expects the license as an input in the initialization parameter, b64_license. Provided in the example code is a way to read the trillbit.lic file into a buffer and point the buffer to the b64_license. This can be done in any other way too, just the contents of the file must be copied into any application and used in init_params.b64_license before calling the trill_init function.
