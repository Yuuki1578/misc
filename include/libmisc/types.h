#pragma once

#include <float.h>
#include <limits.h>
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
#define UINT_MIN (0)
#define F32_MAX  FLT_MAX
#define F32_MIN  FLT_MIN
#define F64_MAX  DBL_MAX
#define F64_MIN  DBL_MIN

#define I8(exp)    ((i8)(exp))
#define U8(exp)    ((u8)(exp))
#define I16(exp)   ((i16)(exp))
#define U16(exp)   ((u16)(exp))
#define I32(exp)   ((i32)(exp))
#define U32(exp)   ((u32)(exp))
#define I64(exp)   ((i64)(exp))
#define U64(exp)   ((u64)(exp))
#define IARCH(exp) ((iarch)(exp))
#define UARCH(exp) ((uarch)(exp))
#define F32(exp)   ((f32)(exp))
#define F64(exp)   ((f64)(exp))

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
#  define F128_MAX  LDBL_MAX
#  define F128_MIN  LDBL_MIN
#  define F128(exp) ((f128)(exp))
typedef long double f128;
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
#  define I128_MAX  ((i128)(((u128)1 << 127) - 1))
#  define I128_MIN  (-I128_MAX - 1)
#  define U128_MAX  ((u128)1 << 127)
#  define I128(exp) ((i128)(exp))
#  define U128(exp) ((u128)(exp))
typedef __int128_t  i128;
typedef __uint128_t u128;
#endif

#ifdef MISC_FILE_OFFSET_BITS
#  if MISC_FILE_OFFSET_BITS == 64
#    define FOFFSET_MAX I64_MAX
typedef i64 Offset;
#  elif MISC_FILE_OFFSET_BITS == 32
#    define FOFFSET_MAX I32_MAX
typedef i32 Offset;
#  else
#    define FOFFSET_MAX LONG_MAX
typedef long Offset;
#  endif
#endif

#ifdef __cplusplus
}
}
#endif
