# leakdetector

A memory leak detector.

## Setup

### CMake
Ideally vendor as a Git submodule into your project.

Add it as a dependency to your targets:
```cmake
add_subdirectory(vendor/leakdetector)
target_link_libraries(foo PRIVATE leakdetector)
```

### Manual

You're mostly on your own. Build `leakdetector.c` and include `leakdetector.h` into all of your translation units.

## Usage

For CMake you can simply turn the option:
```cmake
set(LEAKDETECTOR ON)
```
otherwise, you may have to resort to a C definition:
```c
-DLEAKDETECTOR
```
or at runtime if you live dangerously:
```c
leakdetector = true;
```

## License

This is free and unencumbered software released into the public domain. See the [UNLICENSE](UNLICENSE) file for more details.