# DP::DUMP

## Introduction

A creation of the proposed debug tool from [`P2879R0`: Proposal of `std::dump`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2879r0.pdf) - a tool to quickly dump a set of data. In summary,

```cpp
dp::dump(a,b,c,d);
```
is equivalent to
```cpp
std::print("{} {} {} {}", a, b, c, d);
```
including the necessary overloads for `std::FILE*` and `std::ostream` output, as well as `dp::dumpln` to insert a newline after the dump of variables.

This tool supports both C++20 and C++23 with no additional dependencies, as it is powered by `std::print` on implementations which have it and `std::format` otherwise. It is also possible to use this tool on C++14 and C++17, using the [fmt](https://github.com/fmtlib/fmt) library, and passing `DP_DUMP_USE_FMTLIB` as a preprocessor define. Taking this approach obviously requires fmt as a dependency.


## Installation

This tool is a single header. Just download and `#include` it, and away you go.

## Sample Code

For a main file
```cpp
#include "dump.h"
#include <fstream>

int main(){
	std::ofstream os{"Some_output_file.txt"};
	dp::dumpln(os, 1, 2, 3);
};
```

When compiled to the C++23 standard, this will call `std::print` to place `1 2 3` into the output file specified. If compiled with `-DDP_DUMP_USE_FMTLIB` (or your platform's equivalent) and linked with fmtlib, you will get the same observible behaviour as if `fmt::print(os, "{} {} {}", 1, 2, 3);` had been called.

## FAQ

* **Can I use libfmt over the standard functions in C++20 and C++23?** Yes, defining `DP_DUMP_USE_FMTLIB` will mean fmt is used regardless of your C++ version and what tools are supported.

* **Does using `std::format` mean that this function allocates? Why not use a different approach?** This was considered during development, but ultimately `std::format` was chosen. The reasons were simple - requiring an `operator<<` overload in implementations without `std::print` and a `std::formatter` specialisation in implementations with it adds a high degree of inconsistency which was deemed unacceptable. It is far preferable for the user to need only define a `formatter` for their desired type and call it a day. If this still leaves a bad taste in your mouth, you can use the fmtlib version of this library.
