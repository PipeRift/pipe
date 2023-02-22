# Pipe

[![release](https://img.shields.io/github/release/PipeRift/pipe?labelColor=394047)](https://github.com/PipeRift/rift/releases) [![build](https://github.com/PipeRift/pipe/actions/workflows/build.yml/badge.svg)](https://github.com/PipeRift/pipe/actions/workflows/build.yml) ![license](https://img.shields.io/github/license/PipeRift/pipe?labelColor=394047) [![Code Quality](https://app.codacy.com/project/badge/Grade/bcaa8bb47ec442b282e56990d668f681)](https://www.codacy.com/gh/PipeRift/pipe/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PipeRift/pipe&amp;utm_campaign=Badge_Grade)

**Pipe** is a cross-platform utility library used as the foundation for [Rift](https://github.com/piperift/rift).


## What is it for?

**Pipe** simplifies and unifies operating system APIs and provides commonly missing tools in C++. However, it is in no way just a merge of random libraries into one. It follows the simple principle of "low-level essentials you wouldn't want to integrate/code every time in your projects".

Pipe also has the secondary objective of unifying many coding standards into one to make code consistent and legible.


## Modules
This library is separated into a set of modules so that you can use only what is relevant to your usecase.
If you feel a feature should be added feel free to propose it [creating an issue](https://github.com/PipeRift/pipe/issues/new).

#### Core
- Containers (Array, Map, Set)
- Strings (String, StringView, FixedString)
- Tags (hashed unique strings)
- Compile-time string operations
- Extensive strings constexpr API
- Search and Sort algorithms
- Concepts and TypeTraits
- Cross-platform types
- Runtime Checks
- Improved Logging (including to file)
- Integrated Profiling
- Functions and Broadcasts (also known as delegates)

#### Reflection
- Runtime and Compile-time reflection
- Automatic registration of reflection types
- Differentiated API for classes and structs<br/>
  (if used, classes and structs are not the same anymore and will follow different uses)

#### Serialization
- Serialization designed for different formats
  - JSON serialization
  - Binary serialization
- Automatic serialization of reflection

#### Files
- Files and directories
- File watching

#### Memory
- OwnPtr: Owned smart pointers
- UniquePtr
- Memory Arenas: Allow controlling allocations (without templates)
  - MonoLinearArena
  - MultiLinearArena
  - BestFitArena: Generic arena for any use
- Memory leak tracking

#### Math
- Colors (RGBA, HSV, Linear, sRGB...)
- Vectors, Rotators, Quaternions
- Time and Duration

#### ECS
 - ECS library using other modules
 - Context: An instance of ECS with its entities, components and statics
 - Components: One instance per entity
 - Statics: Unique instances of a type
 - Integration with reflection


## Software Quality
[![build](https://github.com/PipeRift/pipe/actions/workflows/build.yml/badge.svg)](https://github.com/PipeRift/pipe/actions/workflows/build.yml)

The project's code is validated for the following platforms and compilers:

| Windows      | Ubuntu       | Mac         |
| ------------ | ------------ | ----------- |
| clang ^13.0  | clang ^13.0  | clang ^13.0 |
| msvc         | gcc  ^10     |             |

This library is also extensively tested.
