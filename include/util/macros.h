#pragma once
#ifndef FLUFF_UTIL_MACROS_H
#define FLUFF_UTIL_MACROS_H

/* -===========
     Macros
   ===========- */

#if defined(_WIN32) || defined(_WIN64)
#   ifdef FLUFF_IMPLEMENTATION
#       define FLUFF_API __declspec(dllexport)
#   else
#       define FLUFF_API __declspec(dllimport)
#   endif
#else
#   ifdef FLUFF_IMPLEMENTATION
#       define FLUFF_API
#   else
#       define FLUFF_API
#   endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#    ifndef FLUFF_IMPLEMENTATION
#        define FLUFF_PRIVATE_API FLUFF_API __attribute__((error("call to private api function")))
#    else
#        define FLUFF_PRIVATE_API FLUFF_API
#    endif
#    define FLUFF_CONSTEXPR    static inline __attribute__((always_inline, unused))
#    define FLUFF_CONSTEXPR_V  static __attribute__((unused))
#else
#    define FLUFF_PRIVATE_API FLUFF_API
#    define FLUFF_CONSTEXPR   static inline
#    define FLUFF_CONSTEXPR_V static
#endif

#define FLUFF_THREAD_LOCAL   _Thread_local
#define FLUFF_ATOMIC(__type) _Atomic(__type)

#define FLUFF_ERROR_RETURN __attribute__((warn_unused_result))

#define FLUFF_CLEANUP(__data)           memset(__data, 0, sizeof(* __data))
#define FLUFF_CLEANUP_N(__data, __size) memset(__data, 0, __size)
#define FLUFF_LENOF(__array)            (sizeof(__array) / sizeof(__array[0]))

#define FLUFF_BOOLALPHA(__b) ((__b) ? "true" : "false")

#define FLUFF_SET_FLAG(__v, __f)    ((__v) | (__f))
#define FLUFF_UNSET_FLAG(__v, __f)  ((__v) & ~(__f))
#define FLUFF_TOGGLE_FLAG(__v, __f) ((__v) ^ (__f))
#define FLUFF_HAS_FLAG(__v, __f)    ((__v) & (__f))

#define FLUFF_UNREACHABLE() __builtin_unreachable();
#define FLUFF_BREAKPOINT()  raise(SIGTRAP);

#define FLUFF_MAX(__a, __b)            ((__a) > (__b) ? (__a) : (__b))
#define FLUFF_MIN(__a, __b)            ((__a) < (__b) ? (__a) : (__b))
#define FLUFF_CLAMP(__v, __min, __max) ((__v) < (__min) ? (__min) : ((__v) > (__max) ? (__max) : (__v)))

#define FLUFF_STR_BUFFER_FMT(__v) (int)FLUFF_LENOF(__v), __v

#endif