# Chapter14_WeatherApiClient

This repository contains a C++ sample Weather API client used in Chapter 14.

Overview
--------
A small C++ project demonstrating how to build a simple client that queries a weather API. The repository uses CMake for building.

Prerequisites
-------------
- CMake (recommended 3.10+)
- A C++ compiler that supports C++17 (GCC, Clang, MSVC)
- Internet access if the client calls an external API

If the project depends on third-party libraries (for example libcurl or a JSON library), check the CMakeLists.txt in the repository for exact requirements and how those dependencies are configured.

Build
-----
From the repository root:

```sh
git clone https://github.com/liewvk/Chapter14_WeatherApiClient.git
cd Chapter14_WeatherApiClient
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

This will configure and build the project. The produced binary will be placed in the build directory (or according to the CMake configuration). See CMakeLists.txt for target and output details.

Run
---
After building, run the produced executable. The exact executable name depends on the CMake target — inspect CMakeLists.txt or the build output to find it. Example:

```sh
./your-weather-client-executable [options]
```

Contributing
------------
Suggestions, bug reports, and pull requests are welcome. Please open an issue or a PR with a clear description of the change.

License
-------
If you have a preferred license, add it to the repository. Otherwise please add a LICENSE file to clarify usage rights.
