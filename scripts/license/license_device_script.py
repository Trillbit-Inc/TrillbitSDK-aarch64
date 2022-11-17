import sys
import json
import requests

print()
print("Trillbit License Provisioning Application v1.0")
print()
print("Args(1): <path to client_secret_json_file> <device id> ")
print()


def get_device_license(device_id, client_secret_json):
    # reading client credentials from json

    try:
        credentials = client_secret_json["credentials"]
        platform_key = credentials["platform_key"]
        license_uri = credentials["license_uri"]
    except Exception as e:
        print(e)
        print("Error :: Invalid credentials file. Please download again.")
        sys.exit(-1)

    # device_platform, platform_version, device_model are optional parameters that may or may not be present
    # if not present, just use empty string
    payload = {
        "platform_key": platform_key,
        "device_id": device_id,
        "device_model": "",
        "device_platform": "",
        "platform_version": "",
    }

    response = requests.request("POST", license_uri, data=payload)
    response_json = json.loads(response.text)
    if response.status_code != 200:
        error_message = response_json.get("message", "")
        print(f"Error :: licensing device failed. {error_message}")
        sys.exit(-1)

    user_lic = response_json["payload"]["license"]
    # user_lic_bytes = user_lic.encode('utf-8')
    return user_lic

if len(sys.argv) == 3:
    # Args: <path to client_secret_json_file> <device id>
    device_id = sys.argv[2]
    with open(sys.argv[1], 'r') as f:
        client_secret_json = json.load(f)
    lic_bytes = get_device_license(device_id, client_secret_json)
    print("User lic:", lic_bytes)
    with open('trillbit.lic', 'w+') as lic_file:
        lic_file.write(lic_bytes)

    sys.exit(0)
else:
    print("Error: invalid arguments")
    sys.exit(-1)
