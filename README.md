[![CodeQL for C++](https://github.com/dolbyio-samples/comms-cpp-injection-demo/actions/workflows/codeql-analysis-cpp.yml/badge.svg)](https://github.com/dolbyio-samples/comms-cpp-injection-demo/actions/workflows/codeql-analysis-cpp.yml)[![CodeQL for Python](https://github.com/dolbyio-samples/comms-cpp-injection-demo/actions/workflows/codeql-analysis-python.yml/badge.svg)](https://github.com/dolbyio-samples/comms-cpp-injection-demo/actions/workflows/codeql-analysis-python.yml)

# Communications API C++ SDK Media Injection Demo

## Prerequisites
All sytems:
- CMake 3.23
- Python 3
- A Dolby.io account and access to the Dolby.io dashboard

macOS:
- macOS 10.15+
- Clang

Windows:
- Windows 10+
- [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) 10.0.20348.0+
- [Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist) 2019+

Linux:
- Ubuntu 20.04
- gcc9+
- PulseAudio Sound Server [runtime depedency](https://api-references.dolby.io/comms-sdk-cpp/other/run_time_deps.html#linux-systems)

## Supported platforms
This project relies on the Dolby.io Communications API C++ SDK, see this [link](https://api-references.dolby.io/comms-sdk-cpp/other/supported_platforms.html) for supported platforms.

## Building
If building on Windows make sure you have installed msys2 shell and unzip inside of it.

```bash
git clone git@github.com:dolbyio-samples/comms-cpp-injection-demo.git 
cd cpp-injection-demo
```

Building on MacOS or Linux in terminal navigate to the comms-cpp-injection-demo/ directory and run:
```bash
bash setup/unix.sh
```

Building on Windows in Command Prompt navigate to the comms-cpp-injection-demo\ directory and run:
```bash
setup\window.bat
```

## Getting Started
The `demo.py` script will scan the `injection-input.json` file to determine which conversations it will inject into the conference. Based on the content of the `injection-input.json`, the `demo.py` script will take conversations in m4a or mp4 format from the respective folder and inject them at locations/rotations specified in the `def.json` files in the conversation folders.

The environment set by the cpp application is as follows:
```cpp
dolbyio::comms::spatial_position right{1, 0, 0};
dolbyio::comms::spatial_position up{0, 1, 0};
dolbyio::comms::spatial_position forward{0, 0, -1};
dolbyio::comms::spatial_scale scale{1, 1, 1};
```

Run the demo.py script and then follow the prompts: 
```
cd build/
python3 demo.py 
```
On MacOS/Windows the application will just run in terminal so entering **q** on command line will exit. Remember that each injector is running in its own process so you will have to **q+enter** for each spawned injector
instance.

When running on Ubuntu the process will run as daemon and is to be stopped using the python script:
```bash
python3 demo.py -stop yes
```

## Access Token
A [Client Access Token](https://api-references.dolby.io/comms-sdk-cpp/other/getting_started.html#getting-the-access-token) is required to connect to the Dolby.io platform. The `demo.py` script will scan the `injection-input.json` file and look for either the `token_server_url` field to find a url where it can fetch the token from; or the `client_access_token` field to find a token which is hardcoded into the file. The former takes precedent. The python script then passes the token as a command line parameter when running the `cpp-injection-demo` binary.

## Open Project IDE
### QtCreator (macOS/Ubuntu)
 - After building with setup/unix.sh
 - Open QtCreator
 - Go to top bar settings and select `QtCreator -> Open File or Project...` and select the top level CMakeLists.txt file (`cpp-injection-demo/CMakeLists.txt`)
 - Create the project for x86_64 (or arm64 on macOS)

### Visual Studio (Windows)
 - After building with `setup\windows.bat` navigate to `build/` directory
 - Double-click on `cpp_injection_demo.sln` in the `build/` directory
