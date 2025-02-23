# PixelMap

Mapper for Minecraft Java Edition and Minecraft Bedrock Edition. Will read any Minecraft world and output an image representing it.

![build.yml](https://git.aposoc.net/McTwist/PixelMap/badges/workflows/build.yml/badge.svg "Current build status")
![Releases](https://git.aposoc.net/McTwist/PixelMap/badges/release.svg "Latest release")

## Examples

![default.png](screenshots/default.png) ![night.png](screenshots/night.png) ![cave.png](screenshots/cave.png)
![gradient.png](screenshots/gradient.png) ![heightline.png](screenshots/heightline.png) ![opaque.png](screenshots/opaque.png)
![slice.png](screenshots/slice.png) ![gray.png](screenshots/gray.png) ![color.png](screenshots/color.png)

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
cmake --preset all
cmake --build --preset all
```

Additional variables could be used to change the build system.

## Running the tests

```bash
ctest --preset all
```
## Deployment

Remove from below depending on what is wanted.

### Linux

```bash
cpack --preset linux
```

### Windows

```bash
cpack --preset windows
```

## Authors

- **McTwist** - *Owner*

## License

This project is licensed under the GPLv3 License - see the [LICENSE](LICENSE) file for details.
