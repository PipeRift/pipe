// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms

#pragma warning(disable:4003)

#define PRIMITIVE_CAT(a, ...) a##__VA_ARGS__
#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)

#define IIF(c) PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...) __VA_ARGS__
#define IIF_1(t, ...) t
#define IIF_(t, ...) __VA_ARGS__

#define COMPL(b) PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0
#define COMPL_ 0

#define CHECK_N(x, n, ...) n
#define CHECK_1(...) CHECK_N(__VA_ARGS__, 0, )
#define PROBE(x) x, 1,

#define NOT(x) CHECK_1(PRIMITIVE_CAT(NOT_, x))
#define NOT_0 PROBE(~)

#define MBOOL(x) COMPL(NOT(x))

#define IF(c) IIF(MBOOL(COMPL(c)))
#define IFNOT(c) IIF(NOT(MBOOL(COMPL(c))))

// Converts a macro parameter like __LINE__ to an string literal
#define ENSURE_LITERAL(a) PRIMITIVE_ENSURE_LITERAL(a)
#define PRIMITIVE_ENSURE_LITERAL(a) #a
