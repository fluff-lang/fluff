#pragma once
#ifndef FLUFF_BASE_H
#define FLUFF_BASE_H

/* NOTE: compilation flags:

   FLUFF_MEMORY_CLEANUP:
     Fills all values stored in heap with 0s before they are free'd.
     Recommended for more strict performance scenarios, but not security-wise.

   

*/

/* -=============
     Includes
   =============- */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>

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
#       define FLUFF_API extern
#   endif
#endif

#ifdef FLUFF_IMPLEMENTATION
#    define FLUFF_PRIVATE_API FLUFF_API
#else
#    define FLUFF_PRIVATE_API FLUFF_API __attribute__((error("call to private api function")))
#endif

#if 1
#    define FLUFF_CONSTEXPR static inline __attribute__((always_inline, unused))
#    define FLUFF_CONSTEXPR_V static __attribute__((unused))
#endif

#define FLUFF_ERROR_RETURN __attribute__((warn_unused_result))

#define FLUFF_LENOF(__array) (sizeof(__array) / sizeof(__array[0]))

#define FLUFF_CLEANUP(__data)           memset(__data, 0, sizeof(* __data));
#define FLUFF_CLEANUP_N(__data, __size) memset(__data, 0, __size);

#define FLUFF_BOOLALPHA(__b) ((__b) ? "true" : "false")

#define FLUFF_UNREACHABLE() __builtin_unreachable();
#define FLUFF_BREAKPOINT()  raise(SIGTRAP);

#define FLUFF_FAILURE 0x0
#define FLUFF_OK      0x1

/* -================
     Basic types
   ================- */

typedef bool    FluffBool;
typedef int64_t FluffInt;
typedef double  FluffFloat;

typedef uint8_t FluffResult;

typedef unsigned int uint;

/* -=========
     Enum
   =========- */

typedef enum FluffEnum {
    FLUFF_ERROR         = 0x0010, 
    FLUFF_RUNTIME_ERROR = 0x0011, 
    FLUFF_COMPILE_ERROR = 0x0012, 
} FluffEnum;

FLUFF_API const char * fluff_enum_to_string(FluffEnum e);

/* -==========
     Utils
   ==========- */

// Calls the corresponding callback in the current configuration.
FLUFF_API void * fluff_alloc(void * ptr, size_t size);
FLUFF_API void   fluff_free(void * ptr);
FLUFF_API void   fluff_write(const char * restrict text);
FLUFF_API void   fluff_error(FluffEnum type, const char * restrict text);

FLUFF_API int fluff_format(char * buf, size_t buf_len, const char * restrict fmt, ...);
FLUFF_API int fluff_vformat(char * buf, size_t buf_len, const char * restrict fmt, va_list args);
FLUFF_API int fluff_vprint(FILE * f, const char * restrict fmt, va_list args);
FLUFF_API int fluff_print(FILE * f, const char * restrict fmt, ...);

FLUFF_API void * fluff_default_alloc(void * ptr, size_t size);
FLUFF_API void   fluff_default_free(void * ptr);
FLUFF_API void   fluff_default_write(const char * restrict text);
FLUFF_API void   fluff_default_error(FluffEnum type, const char * restrict text);

FLUFF_API void fluff_private_test();

FLUFF_API uint64_t fluff_hash(const void * data, size_t size);
FLUFF_API uint64_t fluff_hash_combine(uint64_t a, uint64_t b);

FLUFF_API uint fluff_ffs(uint v);

#endif