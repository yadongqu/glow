#pragma once
#include <stdint.h>
#include <glad/glad.h>
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

typedef float F32;
typedef double F64;

static const F64 PI = 3.14159265359;

static const U64 U64_MAX = UINT64_MAX;
static const I64 I64_MAX = INT64_MAX;
static const U32 U32_MAX = UINT32_MAX;
static const I32 I32_MAX = INT32_MAX;
static const U16 U16_MAX = UINT16_MAX;
static const I16 I16_MAX = INT16_MAX;
static const U8 U8_MAX = UINT8_MAX;
static const I8 I8_MAX = INT8_MAX;