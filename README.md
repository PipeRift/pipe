# Rift Core

[![release](https://img.shields.io/github/release/PipeRift/rift-core?labelColor=394047)](https://github.com/PipeRift/rift/releases) [![build](https://github.com/PipeRift/rift-core/actions/workflows/build.yml/badge.svg)](https://github.com/PipeRift/rift-core/actions/workflows/build.yml) ![license](https://img.shields.io/github/license/PipeRift/rift-core?labelColor=394047) [![Code Quality](https://app.codacy.com/project/badge/Grade/bcaa8bb47ec442b282e56990d668f681)](https://www.codacy.com/gh/PipeRift/rift-core/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PipeRift/rift-core&amp;utm_campaign=Badge_Grade)

**Rift-Core** is a cross-platform utility library used as the foundation for [Rift](https://github.com/piperift/rift).

It simplifies and unifies operating system APIs and provides commonly missing tools in C++. However, Rift-Core is in no way just a merge of random libraries into one. It follows the simple principle of "low-level essentials you wouldn't want to integrate/code in your project".

Rift-Core also has the secondary objective of unifying many coding standards into one to make code consistent and legible.

## What is it for

Rift-Core has the main objective of providing this tools to Rift (hence the name), but it can be used in all sorts of applications where the tools provided are relevant.

Some of this features are:
- Runtime and Static reflection
  - Automatic registration of reflection
- Serialization to different formats
  - Serialization from reflection
  - Included json serialization
- Files and directories
  - File watching
- Memory management
  - Owned smart pointers
  - Allocators and Arenas
- Containers
- Functions and Broadcasts (also known as delegates)
- Math
  - Colors
  - Vectors, Rotators, Quaternions
  - Search and Sort algorithms
- Time and Duration
- Runtime Checks
- Concepts and TypeTraits
- Cross-platform types
- Strings
  - Extensive String and StringView constexpr API
  - Fixed Strings
- Differentiated API for classes and structs<br/>
  (if used, classes and structs are not the same anymore and will follow different uses)
- Multithreading tools
- Integrated Profiling
- Improved Logging (including to file)

## Software Quality
[![build](https://github.com/PipeRift/rift-core/actions/workflows/build.yml/badge.svg)](https://github.com/PipeRift/rift-core/actions/workflows/build.yml)

The project's code is validated for the following platforms and compilers:

| Windows      | Ubuntu       | Mac         |
| ------------ | ------------ | ----------- |
| clang ^12.0  | clang ^12.0  | clang ^12.0 |
| msvc         | gcc          |             |
