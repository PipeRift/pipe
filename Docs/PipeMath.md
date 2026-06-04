# Pipe Math
Defined in header [<PipeMath.h>](https://github.com/PipeRift/pipe/blob/main/Include/PipeMath.h)

## Overview

`PipeMath` provides a collection of constexpr and runtime math utilities in the `p` namespace. Functions are overloaded by type via C++20 concepts (`FloatingPoint<T>`, `SignedIntegral<T>`, `UnsignedIntegral<T>`, `Number<T>`) to dispatch between constexpr compile-time implementations (when `std::is_constant_evaluated()` is true) and standard library fallbacks at runtime.

All functions marked with `P_API` are exported from the Pipe shared library; others are inline or template-only.

## Constants

| Constant | Type | Value |
|----------|------|-------|
| `pi` | `float` | π (3.141592...) |
| `halfPi` | `float` | π / 2 |
| `invPi` | `float` | 1 / π |
| `radToDeg` | `float` | Radians → degrees factor |
| `degToRad` | `float` | Degrees → radians factor |
| `smallNumber` | `float` | 1.e-4f |
| `verySmallNumber` | `float` | 1.e-8f |
| `bigNumber` | `float` | 3.4e+38f |
| `euler` | `float` | e (2.718281...) |

## Comparison

### Max / Min (binary)

```cpp
template<typename Type>
constexpr Type Max(Type a, Type b);   // returns max(a, b)

template<typename Type>
constexpr Type Min(Type a, Type b);   // returns min(a, b)
```

- **Behavior:** `Max` uses `a >= b`, `Min` uses `a <= b`. Both are `constexpr`.
- **Type constraint:** None — works for any comparable type.

### Max / Min (variadic)

```cpp
template<typename Type, typename... Args>
constexpr Type Max(Type a, Type b, Args... args);

template<typename Type, typename... Args>
constexpr Type Min(Type a, Type b, Args... args);
```

- Recursively chains binary `Max`/`Min`. All arguments must be implicitly convertible to `Type`.

### Clamp

```cpp
template<typename Type>
constexpr Type Clamp(Type a, Type min, Type max);
```

Equivalent to `Max(min, Min(a, max))`.

---

## Arithmetic

### Abs

```cpp
template<typename Type>
constexpr Type Abs(const Type a);
```

Returns absolute value. Uses `(a >= 0) ? a : -a` — does not call `std::abs`.

### Sign

```cpp
template<typename Type>
constexpr Type Sign(const Type a);
```

Returns `1`, `0`, or `-1` based on whether `a` is positive, zero, or negative.

### Pow (integer)
```cpp
// Signed integral overload
template<Integral T>
constexpr T Pow(T value, u32 power);
```
- Iterative multiplication. `power == 0` returns `1` (or `-1` for negative base with signed types).

> **Warning:** Does not check overflow.

### Pow (float & double)
```cpp
template<FloatingPoint V, Number P>
constexpr V Pow(V value, P power);
```
- **Compile-time:** Iterative multiplication when `power` is integral.
- **Runtime:** Delegates to `std::pow`.

### Square

```cpp
template<typename T>
constexpr T Square(T val);
```

Returns `val * val`.

### Sqrt

```cpp
template<typename T>
T Sqrt(T val);
```

Delegates to `std::sqrt`.
Not `constexpr` (no compile-time specialization).

### InvSqrt

```cpp
float  InvSqrt(float x);
double InvSqrt(double x);
float  InvSqrt(i32 x);
double InvSqrt(i64 x);
```

Returns `1.f / Sqrt(x)` (or `1. / Sqrt(x)` for double).

## Rounding

### Floor / Ceil (floating point)

```cpp
template<FloatingPoint T>
constexpr T Floor(T v);

template<FloatingPoint T>
constexpr T Ceil(T v);
```

- **Compile-time:** Manual implementation handling NaN, infinity, and near-zero values.
- **Runtime:** Delegates to `std::floor` / `std::ceil`.

### FloorToI32 / FloorToI64

```cpp
constexpr i32 FloorToI32(float f);
constexpr i64 FloorToI64(double f);
```

Converts float/double to integer via  [Floor](#Floor).

### CeilToI32 / CeilToI64

```cpp
constexpr i32 CeilToI32(float f);
constexpr i64 CeilToI64(double f);
```

Converts float/double to integer via [Ceil](#Ceil).

### Round

```cpp
constexpr float  Round(float f);
constexpr double Round(double f);
```

- **Compile-time:** Manual implementation
- **Runtime:** Delegates to `std::round`.

### RoundToI32/RoundToI64

```cpp
constexpr i32 RoundToI32(float f);
constexpr i64 RoundToI64(double f);
```

Converts float/double to integer via [Round](#Round).

### RoundFromZero

```cpp
float  RoundFromZero(float f);
double RoundFromZero(double d);
```

Rounds away from zero: `0.1 → 1`, `-0.1 → -1`. Equivalent to `(f > 0) ? Ceil(f) : Floor(f)`.

### RoundToZero

```cpp
float  RoundToZero(float f);
double RoundToZero(double d);
```

Rounds toward zero: `0.1 → 0`, `-0.1 → 0`. Equivalent to `(f < 0) ? Ceil(f) : Floor(f)`.

### RoundToNegativeInfinity / RoundToPositiveInfinity

```cpp
float  RoundToNegativeInfinity(float f);   // = Floor(f)
double RoundToNegativeInfinity(double d);  // = Floor(d)
float  RoundToPositiveInfinity(float f);   // = Ceil(f)
double RoundToPositiveInfinity(double d);  // = Ceil(d)
```

Aliases for `Floor` and `Ceil`.

### Frac

```cpp
float  Frac(float f);
double Frac(double d);
```

Returns the fractional part of a number: `3.25 → 0,25`. Equivalent to `f - Floor(f)`. Result is always in range `[0, 1)`.

---

## Random

```cpp
i32    Rand();           // [0, RAND_MAX] inclusive
float  Rand01();         // [0, 1] inclusive
float  Random(float min, float max);   // [min, max] inclusive
i32    Random(i32 min, i32 max);       // [min, max] inclusive
```

## Floating-point Checks

```cpp
template<typename T>
constexpr bool IsPosInf(const T x);   // x == Limits<T>::Infinity()

template<typename T>
constexpr bool IsNegInf(const T x);   // x == -Limits<T>::Infinity()

template<typename T>
constexpr bool IsInf(const T x);      // IsNegInf(x) || IsPosInf(x)

template<typename T>
constexpr bool IsNAN(const T x) noexcept;  // x != x
```

- `IsNAN` uses the NaN self-comparison property (`x != x`). Works for both float and double.
- Infinity checks compare against `Limits<T>::Infinity()` from `Pipe/Core/Limits.h`.

## Trigonometry

### Sin/Cos
```cpp
float Sin(float value);
float Cos(float value);
void SinCos(float value, float& outSin, float& outCos); // Simultaneous Sin and Cos
```

### Atan2
```cpp
float Atan2(float Y, float X);
```

### FastAsin

```cpp
float FastAsin(float Value);
```

7-degree minimax approximation of arcsine. Clamps input to `[-1, 1]`.

> **Warning:** This is an approximation — not suitable for high-precision applications. For full precision, use `std::asin`.

### Angles

```cpp
float ClampAngle(float a);                    // clamp to [0, 2π)
float NormalizeAngle(float a);                // normalize to [-π, π]
float ClampAngle(float a, float min, float max); // custom range
```

Runtime-only functions (defined in `.cpp`). `NormalizeAngle` maps angles into the principal range.

## Modulo

```cpp
// Floating point: a - b * Floor(a / b)
template<FloatingPoint Type>
constexpr Type Mod(Type a, Type b);

// Signed integral: ((a % b) + b) % b  (always non-negative result)
template<SignedIntegral Type>
constexpr Type Mod(Type a, Type b);

// Unsigned integral: a - b * (a / b)
template<UnsignedIntegral Type>
constexpr Type Mod(Type a, Type b);
```

- Floating-point version implements mathematical modulo (always non-negative for positive `b`).
- Signed-integral version normalizes C++ `%` to always return `[0, b)` regardless of sign.
- Unsigned version is equivalent to `a % b`.

## Logarithmic & Exponential

### Log

```cpp
float  Log(float k);               // ln(k)
float  Log(float k, float base);   // log_base(k) = ln(k)/ln(base)
double Log(double k);              // ln(k)
double Log(double k, double base); // log_base(k)
template<Integral T>
double Log(T k);            // converts to double, calls std::log
```

### Log2

```cpp
float  Log2(float k);    // std::log2f(k)
double Log2(double k);   // std::log2(k)
template<Integral T>
double Log2(T k); // converts to double, calls std::log2
```

### Exp2

```cpp
const float  Exp2(const float k);  // std::exp2f(k)
const double Exp2(const double k); // std::exp2(k)
template<Integral T>
const double Exp2(T k);     // converts to double, calls std::exp2
```

## Bit Manipulation

### IsPowerOfTwo

```cpp
template<typename T>
bool IsPowerOfTwo(T value);
```

Returns `(value & (value - 1)) == 0`. Note: `IsPowerOfTwo(0)` returns `true` since `0 & (-1) == 0`. For strict positive power-of-two, add a zero check.

### CountBits

```cpp
constexpr i32 CountBits(u64 value);
```

Counts the number of bits set 1 in a 64 bits integer.

#### Implementation
Population count (Hamming weight) of a 64-bit integer.
- **Linux/macOS:** Uses `__builtin_popcountll(value)` (compiler intrinsic).
- **Windows/other:** Software implementation using the SWAR bit-counting algorithm:
  ```cpp
  value -= (value >> 1) & 0x5555555555555555ull;
  value = (value & 0x3333333333333333ull) + ((value >> 2) & 0x3333333333333333ull);
  value = (value + (value >> 4)) & 0x0f0f0f0f0f0f0f0full;
  return (value * 0x0101010101010101) >> 56;
  ```

## Utility

### NearlyEqual

```cpp
bool NearlyEqual(float a, float b, float tolerance = smallNumber);
```

Returns `true` if `Abs(b - a) <= tolerance`. Default tolerance is `1e-4f`.

## Type Trait Concepts Reference

The following concepts from `Pipe/Core/TypeTraits.h` are used as constraints in PipeMath:

| Concept | Definition | Matches |
|---------|-----------|---------|
| `Number<T>` | `std::is_integral_v<T> \|\| std::is_floating_point_v<T>` | Any integer or floating-point type |
| `Integral<T>` | `std::is_integral_v<T>` | All integral types (bool, char, int variants) |
| `SignedIntegral<T>` | `Integral<T> && std::is_signed_v<T>` | Signed integers only |
| `UnsignedIntegral<T>` | `Integral<T> && std::is_unsigned_v<T>` | Unsigned integers only |
| `FloatingPoint<T>` | `std::is_floating_point_v<T>` | float, double, long double |

## Platform-Specific Notes

- **`CountBits(u64)`** has different implementations per platform (compiler intrinsic vs. software SWAR).
- All other functions are cross-platform with no conditional compilation beyond the standard library.
- `P_API` export macro controls DLL/shared-library visibility; inline/template functions do not require it at call sites.
