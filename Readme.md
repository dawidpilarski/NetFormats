# NetFormats (alpha)

![linux clang 15 build](https://github.com/dawidpilarski/NetFormats/actions/workflows/build-linux-clang.yml/badge.svg)
![linux gcc 12 build](https://github.com/dawidpilarski/NetFormats/actions/workflows/build-linux-gcc.yml/badge.svg)

A header only, C++ library designed to help people with parsing popular
networking-related formats. Some of those include:

- json (PoC implemented)
- yaml (not started)
- url (not started)
- graphql (not started)


We want to provide as C++ native as possible API for interaction with
those formats.

We also are performance and memory efficiency concentrated

This may lead to APIs in multiple layers (different for general use-case )
and different for very specific ones).

Updates will be reflected in readme on the regular basis

## Preparing for development

```bash 
conan install . --output-folder=build --build=missing
conan install . --output-folder=build_debug -s build_type=Debug --build=missing

cmake --preset conan-debug
cmake --build --preset conan-debug
```

Clion will automatically understand presets


