# PixelMap

Mapper for Minecraft Java Edition and Minecraft Bedrock Edition. Will read any Minecraft world and output an image representing it.

![build.yml](/../../badges/worlkflows/build.yml/badge.svg "Current build status")
![Releases](/../../badges/release.svg "Latest release")

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

Required:
- [CMake](https://cmake.org/)
- C/C++ compiler

For GUI:
- OpenGL
- GTK (for linux)

### Installing

The command and output changes slightly depends on platform, but default compiler would use the following.

```bash
mkdir build
cd build
cmake ..
```

Additional variables could be used to change the build system.

## Running the tests

```bash
ctest
```
## Deployment

Remove from below depending on what is wanted.

### Linux

```bash
cpack -G "DEB;RPM;ZIP" -D CPACK_COMPONENTS_ALL="cli;gui" -D CPACK_OUTPUT_FILE_PREFIX=publish
```

### Windows

```bash
cpack -G "NSIS;ZIP" -D CPACK_COMPONENTS_ALL="cli;gui" -D CPACK_OUTPUT_FILE_PREFIX=publish
```

## Authors

- **McTwist** - *Owner*

## License

This project is licensed under the GPLv3 License - see the [LICENSE](LICENSE) file for details
