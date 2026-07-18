# ilib_c

**ilib_c** is a modular collection of C99 libraries for systems programming, data structures, algorithms, memory management, and low-level runtime facilities.

> [!NOTE]
> **ilib_c** is under active development. Stable modules aim to preserve API compatibility, while experimental components may change between releases.

## Features

- ISO C99
- Modular architecture
- Memory allocators
- Data structures
- Graph algorithms
- User-space execution contexts (`icontext`)
- Architecture-specific implementations
- Minimal external dependencies

## Repository Layout

```text
include/    Public headers
src/        Portable implementations
arch/       Architecture-specific code
tests/      Test suite
buildtools/ Internal build utilities
docs/       Project documentation
```

## Design Philosophy

ilib_c is built around a few core principles:

- Keep modules independent whenever possible.
- Keep dependencies to an absolute minimum.
- Use portable code whenever practical.
- Optimize only where it provides measurable benefits.

> [!TIP]
> Most modules are completely independent. You only need to include and compile the parts of the library that your project actually uses.

## Building

> [!IMPORTANT]
> The current Makefile is intended for development and testing of the project itself. It is **not** yet a public build or installation system.

## Project Status

The library is actively evolving. New modules are added regularly, and some APIs may still change before the first stable release.

## License

This project is licensed under the MIT License.
