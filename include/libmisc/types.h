#pragma once

#include <float.h>
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#  include <BaseTsd.h>
#else
#  include <sys/types.h>
#endif

#define I8_MAX   INT8_MAX
#define I8_MIN   INT8_MIN
#define U8_MAX   UINT8_MAX
#define I16_MAX  INT16_MAX
#define I16_MIN  INT16_MIN
#define U16_MAX  UINT16_MAX
#define I32_MAX  INT32_MAX
#define I32_MIN  INT32_MIN
#define U32_MAX  UINT32_MAX
#define I64_MAX  INT64_MAX
#define I64_MIN  INT64_MIN
#define U64_MAX  UINT64_MAX
#define I128_MAX ((i128)(((u128)1 << 127) - 1))
#define I128_MIN (-I128_MAX - 1)
#define U128_MAX ((u128)1 << 127)
#define UINT_MIN (0)
#define F32_MAX  FLT_MAX
#define F32_MIN  FLT_MIN
#define F64_MAX  DBL_MAX
#define F64_MIN  DBL_MIN

#ifdef __cplusplus
namespace misc {
extern "C" {
#endif

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;
typedef size_t   uarch;
typedef float    f32;
typedef double   f64;

#if defined(__unix__) && defined(__LP64__)
typedef long double f128;
#  define F128_MAX LDBL_MAX
#  define F128_MIN LDBL_MIN
#endif

#if defined(_WIN32) || defined(_WIN64)
#  ifdef _WIN32
#    define IARCH_MAX I32_MAX
#    define IARCH_MIN I32_MIN
#    define UARCH_MAX U32_MAX
#  elif defined(_WIN64)
#    define IARCH_MAX I64_MAX
#    define IARCH_MIN 164_MIN
#    define UARCH_MAX U64_MAX
#  endif
typedef SSIZE_T iarch;
#else
#  ifndef __LP64__
#    define IARCH_MAX I32_MAX
#    define IARCH_MIN I32_MIN
#    define UARCH_MAX U32_MAX
#  else
#    define IARCH_MAX I64_MAX
#    define IARCH_MIN I64_MIN
#    define UARCH_MAX U64_MAX
#  endif
typedef ssize_t iarch;
#endif

#ifdef __SIZEOF_INT128__
typedef __int128_t  i128;
typedef __uint128_t u128;
#endif

#ifdef __cplusplus
}
}
#endif
