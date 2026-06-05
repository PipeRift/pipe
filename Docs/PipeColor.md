---
title: Pipe Color
---
**Header:** [`<PipeColor.h>`](https://github.com/PipeRift/pipe/blob/main/Include/PipeColor.h)
**Namespace:** `p`
**Uses:** [`PipeMath`](./PipeMath.md) [`PipeVectors`](./PipeVectors.md)

## Overview

`PipeColor` provides a color system supporting four color space modes: **RGBA**, **Linear**, **sRGB**, and **HSV**. The color struct `TColor<Mode>` is parameterized with a `ColorMode` ensuring zero-cost abstraction and out of the box conversions & compatibility.

## Color Mode
```cpp
enum class ColorMode : u8 { RGBA, Linear, sRGB, HSV };
```

| Mode     | Component Type        | Range                              | Description                                     |
| -------- | --------------------- | ---------------------------------- | ----------------------------------------------- |
| `RGBA`   | `u8` per component    | `[0, 255]`                         | Standard 8-bit integer color with alpha         |
| `Linear` | `float` per component | `[0.0, 1.0]`                       | Linear (gamma-corrected) floating-point RGBA    |
| `sRGB`   | `float` per component | `[0.0, 1.0]`                       | Standard sRGB gamma-encoded floating-point RGBA |
| `HSV`    | `float` per component | H: `[0, 360)`, S/V/A: `[0.0, 1.0]` | Hue-Saturation-Value with Alpha                 |

## TColor<>
```cpp
template<ColorMode Mode>
struct TColor { ... };
```
### Aliases

```cpp
using LinearColor = TColor<ColorMode::Linear>;
using sRGBColor   = TColor<ColorMode::sRGB>;
using HSVColor    = TColor<ColorMode::HSV>;
using Color       = TColor<ColorMode::RGBA>;
```
### Constructors

| Signature                                           | Color Modes         | Description                                                      |
| --------------------------------------------------- | ------------------- | ---------------------------------------------------------------- |
| `TColor()`                                          |                     | Default: zero-initialized (RGBA) or `{0, 0, 0, 1}` (float modes) |
| `TColor(u8 r, u8 g, u8 b, u8 a = 255)`              | `RGBA`              | Construct from 8-bit components                                  |
| `TColor(float r, float g, float b, float a = 1.0f)` | `Linear, sRGB`      | Construct from normalized floats                                 |
| `TColor(float h, float s, float v, float a = 1.0f)` | `HSV`               | Construct from HSV components                                    |
| `TColor(const v3& vector)`                          | `Linear, sRGB, HSV` | Construct from `v3` (alpha defaults to 1.0)                      |
| `TColor(const v4& vector)`                          | `Linear, sRGB, HSV` | Construct from `v4`                                              |
| `TColor(const TColor<mode>& other)`                 | `Same Mode`         | Copy constructor                                                 |
| `TColor(const TColor<otherMode>& other)`            | `Other Mode`        | Conversion constructor<br>Calls `other.Convert<Mode>()`          |
All constructors are `constexpr`.

### Operators
#### Assignment 
```cpp
constexpr TColor& operator=(const TColor& other);           // same mode
template<ColorMode otherMode>
constexpr TColor& operator=(const TColor<otherMode>& other); // cross-mode
```
#### Arithmetic (component-wise)
All arithmetic operators are `constexpr`. For `RGBA`, results are clamped to `[0, 255]`; for float modes (`Linear`, `sRGB`), no clamping is applied.

| Operator                        | Behavior                                                  |
| ------------------------------- | --------------------------------------------------------- |
| `+`   `-`   `*`   `/` color     | Add, subtract, multiply & divide each component.          |
| `+=`   `-=`   `*=`   `/=` color | Add, subtract, multiply & divide each component in-place. |
| `*`   `/`   scalar              | Multiply & divide each component by an scalar.            |
| `*=`   `/=`   scalar            | Multiply & divide each component by an scalar in-place.   |
> [!warning] HSV colors disallow arithmetic with other **colors**
> However arithmetic with scalars is allowed, where `value` and `alpha` are the only components multiplied or divided.
#### Comparison
```cpp
constexpr bool operator==(const TColor& other) const;
constexpr bool operator!=(const TColor& other) const;
```

- **RGBA:** Compares via raw `u32` bitwise equality (`DWColor()`).
- **Float modes:** Compares each component.

> [!Note] Float mode comparison uses exact bit-wise equality.
> For approximate comparison, use [`Equals()`](#utility-methods).
#### Indexing
```cpp
constexpr auto& operator[](u32 i);
constexpr const auto& operator[](u32 i) const;
```

Returns the component at index `i` starting from `r` (or `h` for HSV). Equivalent to `Data()[i]`.
### Conversion
```cpp
template<ColorMode to>
constexpr TColor<to> Convert() const;
```

All conversions are resolved at compile-time. The following table summarizes supported paths:

| From → To     | Path           |
| ------------- | -------------- |
| RGBA → Linear | Direct         |
| Linear → RGBA | Direct         |
| RGBA → sRGB   | Direct         |
| sRGB → RGBA   | Direct         |
| Linear → HSV  | Direct         |
| HSV → Linear  | Direct         |
| RGBA ↔ HSV    | Through Linear |
| sRGB ↔ HSV    | Through Linear |

> [!Note]
> Conversions between non-adjacent spaces (e.g., RGBA↔HSV) route through Linear. This is intentional for numerical accuracy and simplicity but results in two conversion steps.

### Static Methods

| Method                     | Signature                                                                  | Description                                                                                                                          |
| -------------------------- | -------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| `FromRGB`                  | `static constexpr TColor FromRGB(u8 r, u8 g, u8 b, u8 a = 255)`            | Create from 8-bit RGBA and converts to target mode                                                                                   |
| `FromHex`                  | `static constexpr TColor FromHex(u32 value)`                               | Create from a hex value. \#FF00AA → `FromHex(0xFF00AA)`                                                                              |
| `FromHexAlpha`             | `static constexpr TColor FromHexAlpha(u32 value)`                          | FromHEX with Alpha.<br>\#FF00AA55 → `FromHexAlpha(0xFF00AA55)`                                                                       |
| `MakeFromHSV8`             | `static TColor MakeFromHSV8(u8 hue, u8 saturation, u8 value)`              | Create from 8-bit per component HSV.                                                                                                 |
| `MakeRandomColor`          | `static TColor MakeRandomColor(float saturation = 0.f, float value = 1.f)` | Create color with random hue and fixed saturation/value.                                                                             |
| `MakeFromColorTemperature` | `static TColor MakeFromColorTemperature(float temp)`                       | Converts black body temperature (Kelvin, clamped to `[1000, 15000]`) → sRGB via Planckian locus approximation → BT.709 RGB. Runtime. |

### Packed Format Accessors

Available only in `RGBA` mode:

| Method           | Returns | Byte Order                                |
| ---------------- | ------- | ----------------------------------------- |
| `DWColor()`      | `u32`   | `(A << 24) \| (B << 16) \| (G << 8) \| R` |
| `ToPackedARGB()` | `u32`   | `(A << 24) \| (R << 16) \| (G << 8) \| B` |
| `ToPackedABGR()` | `u32`   | `(A << 24) \| (B << 16) \| (G << 8) \| R` |
| `ToPackedRGBA()` | `u32`   | `(R << 24) \| (G << 16) \| (B << 8) \| A` |
| `ToPackedBGRA()` | `u32`   | `(B << 24) \| (G << 16) \| (R << 8) \| A` |
### Utility Methods

| Method              | Signature                                                                                                                                                                        | Description                                                                            |
| ------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| `Clamp`             | `constexpr TColor Clamp(float min = 0.f, float max = 1.f) const`                                                                                                                 | Clamps all components.<br>Only non-RGBA modes.                                         |
| `Equals`            | `constexpr bool Equals(const TColor& other, float tolerance = smallNumber) const`                                                                                                | Error-tolerant comparison using absolute difference. Only for non-RGBA modes.          |
| `ToV3()` / `ToV4()` | `constexpr v3_u8 ToV3() const` (RGBA)<br>`constexpr v4_u8 ToV4() const` (RGBA)<br>`constexpr v3 ToV3() const` (Linear/sRGB/HSV)<br>`constexpr v4 ToV4() const` (Linear/sRGB/HSV) | Converts to vector types. Returns `v3_u8`/`v4_u8` for RGBA, `v3`/`v4` for float modes. |
| `Desaturate`        | `constexpr TColor Desaturate(float desaturation) const`                                                                                                                          | Lerp between color and its luminance (gray). Range `[0..1]`. Only Linear/sRGB modes.   |
| `Shade`             | `constexpr TColor Shade(float delta) const`                                                                                                                                      | Lerp toward black.                                                                     |
| `Tint`              | `constexpr TColor Tint(float delta) const`                                                                                                                                       | Lerp toward white.                                                                     |
| `Translucency`      | `constexpr TColor Translucency(u8 alpha) const` (RGBA)<br>`constexpr TColor Translucency(float alpha) const` (Linear/sRGB/HSV)                                                   | Sets alpha, returns new color with original RGB preserved.                             |
| `GetLuminance`      | `constexpr float GetLuminance() const`                                                                                                                                           | Perceptual luminance: `r × 0.3 + g × 0.59 + b × 0.11`.                                 |
| `GetMax` / `GetMin` | `constexpr float GetMax() const`<br>`constexpr float GetMin() const`                                                                                                             | Maximum/minimum of all four components. Runtime (not constexpr).                       |
| `IsAlmostBlack`     | `constexpr bool IsAlmostBlack() const`                                                                                                                                           | Useful for light contribution culling.                                                 |
| `ToString()`        | `String ToString() const`                                                                                                                                                        | Format: `(r={val},g={val},b={val},a={val})` or HSV equivalent. Runtime.                |
| `ToHex`             | `String ToHex(bool includeAlpha = true) const` (RGBA only)                                                                                                                       | Hexadecimal string, e.g., `"FF00AA"` or `"FF00AABB"`. Uppercase.                       |
### Common Colors

All are `constexpr static` methods returning the target mode converted from RGBA:

| Method | RGBA Value |
|--------|-----------|
| `White()` | `(255, 255, 255)` |
| `Gray()` | `(127, 127, 127)` |
| `Black()` | `(0, 0, 0)` |
| `Transparent()` | `(0, 0, 0, 0)` |
| `Red()` | `(255, 0, 0)` |
| `Green()` | `(0, 255, 0)` |
| `Blue()` | `(0, 0, 255)` |
| `Yellow()` | `(255, 255, 0)` |
| `Cyan()` | `(0, 255, 255)` |
| `Magenta()` | `(255, 0, 255)` |
| `Orange()` | `(243, 156, 18)` |
| `Purple()` | `(169, 7, 228)` |
| `Turquoise()` | `(26, 188, 156)` |
| `Silver()` | `(189, 195, 199)` |
| `Emerald()` | `(46, 204, 113)` |
### Lerp Using HSV

```cpp
static TColor<mode> LerpUsingHSV(const TColor<mode>& from, const TColor<mode>& to, float delta);
```

Performs linear interpolation in HSV color space, taking the shortest path around the hue wheel (wrapping at 180°). For non-HSV modes, converts both inputs to HSV, lerps, then converts back.

- **Hue:** Interpolated with wrap-around logic — if `|from.h - to.h| > 180`, the shorter arc is chosen by adding/subtracting 360°.
- **S / V / A:** Standard linear interpolation via `p::Lerp`.
- **Result:** Same mode as input.

> [!Warning]
> More expensive than `Lerp()` when conversions to HSV are required. Use when hue continuity matters (e.g. color cycling).

## Free Functions

### ComputeAndFixedColorAndIntensity

```cpp
extern constexpr void ComputeAndFixedColorAndIntensity(
    LinearColor& linearColor, Color& outColor, float& outIntensity);
```

Normalizes a linear color to `[0, 1]` range and produces a fixed-point (RGBA) representation. Sets `outIntensity = max(r, g, b)` (clamped away from zero). Used for brightness-aware color quantization.

### GetHash

```cpp
template<ColorMode mode>
sizet GetHash(const TColor<mode>& color);
```

- **RGBA:** Returns the raw `u32` value (`DWColor()`) — no hashing needed since it's already a 32-bit integer.
- **Other modes:** Calls `HashBytes(&color, sizeof(TColor<mode>))`.

## Usage & Examples

```cpp
#include "Pipe/PipeColor.h"

// Construction
p::Color red(255, 0, 0);                    // From individual components
auto blue = p::Color::FromHex(0x0000FF);    // From hexadecimal
p::LinearColor white = p::Color::White();   // From constants

// Conversion (implicit)
p::sRGBColor srgb = red;                     // RGBA → sRGB
p::HSVColor hsv = white;                     // Linear → HSV

// Arithmetic (RGBA clamped, float unclamped)
p::Color bright = p::Color::Red() + p::Color::Gray();
p::LinearColor dim = white * 0.5f;

// Packed formats
u32 argb = red.ToPackedARGB();   // 0xFFFF0000
u32 bgra = red.ToPackedBGRA();   // 0x0000FFFF

// HSV lerp with shortest hue path
p::LinearColor from = p::LinearColor::Red();
p::LinearColor to   = p::LinearColor::Blue();
p::LinearColor mid  = p::LinearColor::LerpUsingHSV(from, to, 0.5f);

// Color temperature → sRGB
auto warmLight = p::sRGBColor::MakeFromColorTemperature(3200.0f); // 3200K tungsten
```
