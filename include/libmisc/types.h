#pragma once

typedef enum : int {
    SOME = 1,
    NONE = 0,
} Option_Kind;

struct __Misc_Option_Variant {
    Option_Kind is_some;
    void* data;
};

typedef struct __Misc_Option_Variant Option_Variant;

typedef union {
    Option_Kind is_some;
    Option_Variant inner;
} Option;

Option option_new(Option_Kind kind, ...);
void* option_unwrap(Option* option);
void* option_unwrap_or(Option* option, void* default_value);
void* option_unwrap_or_else(Option* option, void* (*fn)(void));

typedef enum : int {
    OK = 1,
    ERR = 0,
} Result_Kind;

struct __Misc_Result_Variant {
    Result_Kind is_ok;
    void* data;
};

typedef struct __Misc_Result_Variant Result_Variant;

typedef union {
    Result_Kind is_ok;
    Result_Variant ok;
    Result_Variant err;
} Result;

Result result_new(Result_Kind kind, ...);
void* result_unwrap(Result* result);
void* result_unwrap_or(Result* result, void* default_value);
void* result_unwrap_or_else(Result* result, void* (*fn)(Result_Variant* err));
