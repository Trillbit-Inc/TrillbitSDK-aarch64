## Prebuilt binaries

This folder includes two binaries:

### Player
**generator** : 
Used to modulate data. Takes the following arguments:
- payload.txt - A file which has the payload that needs to be sent.
- Output - Can either be a wav file/ a system output hardware(in ALSA format). Refer to ALSA documentation to understand about the hardware naming format. For example purpose *default* should be a good place to start.
- Range incidicator (optional) - Three ranges are currently supported : Near(1), Mid(2) and Far(3). Default selected is near.    

Example:
    
    echo "Sample payload here" > payload.txt
    ./generator payload.txt default 2
    

### Decoder
**decoder** :

Used to decode data. Takes the following arguments:
- input - Can either be a recorded wav file/ a system input hardware(in ALSA format).Refer to ALSA documentation to understand about the hardware naming format. For example purpose *default* should be a good place to start.

Example:

    ./decoder default

### Note

- The following binaries can be run to get the device ID of the device it is run on.
- The trill_init process will proceed only when the licensing info is complete. i.e. The *trillbit.lic* file is placed in this folder. Note that the license file must be fetched for the device that the binaries will be executed on. Refer to the [Licensing README.md](../../../../scripts/license/README.md) for further info about licensing.