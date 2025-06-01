#pragma once

#define MISC_VERSIONING_H (0.10LF)

#ifdef __clang__
#define nonnull _Nonnull
#define nullable _Nullable
#endif

#define MISC_CXX_EXTERN extern "C" {
#define MISC_CXX_ENDEXTERN }
#define MISC_DEPRECATED(REASON, ALT) __attribute__((deprecated(REASON, ALT)))

#ifndef CONV
#define CONV(TYPE, EXPR) ((TYPE)EXPR)
#endif

#if __STDC_VERSION__ < 202311L

#define auto __auto_type

typedef void *__MiscNullptr;
typedef __MiscNullptr nullptr_t;

#define nullptr ((nullptr_t)0)

#if !defined(__STDBOOL_H) || !defined(__bool_true_false_are_defined)

typedef _Bool __MiscBool;
typedef __MiscBool bool;

#define true ((bool)1)
#define false ((bool)0)
#endif

#endif
