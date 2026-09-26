not ready for use just yet, do not use this project

## Build with Clang

This parser targets x86-64 with AVX2 and BMI2. It can run under Rosetta on an Apple Silicon Mac, but it is not a native ARM build.

From the repository root on macOS:

```sh
mkdir -p build
clang++ -arch x86_64 -std=c++20 -O0 -g -mavx2 -mbmi -mbmi2 -mssse3 Source.cpp -o build/tatai-debug
lldb build/tatai-debug
```

In LLDB, set a breakpoint and run:

```text
(lldb) breakpoint set --name parse_beatmap_from_memory
(lldb) run
```

For an optimized build, replace `-O0 -g` with `-O3 -DNDEBUG`. On x86-64 Linux, omit `-arch x86_64`.

Run from the repository root because the program reads `within_objects.txt` relative to the current directory. It pauses for Enter before printing the parsed objects.
