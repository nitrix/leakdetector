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

## Output

Misuses are printed to stderr during excution:

```
$ ./example
Freeing unallocated memory: 00000254A18166A0
```

Leaks are printed at exit and breakpoints within a debugger:

```
$ ./example
Memory leak detected! (2 still alive)
        -> 0000024925F26030 allocated at /home/nitrix/example/foo.c:42 (8 bytes)
        -> 0000024923C86A80 allocated at /home/nitrix/example/bar.c:159 (1000 bytes) 
```

## License

This is free and unencumbered software released into the public domain. See the [UNLICENSE](UNLICENSE) file for more details.
