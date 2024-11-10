# PixelMap

Mapper for Minecraft Java Edition and Minecraft Bedrock Edition.

## Compilation

1. Clone the repository.
2. Create `build` folder and enter it.
3. `cmake ..`
4. Build with compilator on designated platform.

Build files support building either statically or dynamically.

To explicitly build specific executables or just the library, disable the following variables depending on your requirements:
```
PIXELMAP_BUILD_CLI
PIXELMAP_BUILD_GUI
PIXELMAP_BUILD_TESTS
```

For the library there exist additional flags:
- `PIXELMAP_DEBUG_MODE` Enable to debug the library.
- `PIXELMAP_USE_LIBDEFLATE` Use libdeflate depedency instead of zlib.
- `PIXELMAP_ENABLE_AFFINITY` Enable thread affinity.
- `PIXELMAP_PROFILE` Enable profiling of program.

## Testing

Prepare the `build` directory with `cmake ..`. Then test with `ctest`. It will ensure that everything is built and then run all tests provided.
