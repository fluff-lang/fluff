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

#include <util/limits.h>
#include <util/macros.h>

/* -===========
     Macros
   ===========- */

#define FLUFF_OK            0x0
#define FLUFF_FAILURE       0x1
#define FLUFF_MAYBE_FAILURE 0x2

/* -================
     Basic types
   ================- */

typedef bool    FluffBool;
typedef int64_t FluffInt;
typedef double  FluffFloat;

typedef uint8_t FluffResult;

typedef unsigned int uint;

/* -==========
     Utils
   ==========- */

FLUFF_API int fluff_format(char * buf, size_t buf_len, const char * restrict fmt, ...);
FLUFF_API int fluff_vformat(char * buf, size_t buf_len, const char * restrict fmt, va_list args);
FLUFF_API int fluff_vprint(FILE * f, const char * restrict fmt, va_list args);
FLUFF_API int fluff_print(FILE * f, const char * restrict fmt, ...);

FLUFF_API void   * fluff_default_alloc(void * ptr, size_t size);
FLUFF_API void     fluff_default_free(void * ptr);
FLUFF_API void     fluff_default_write(const char * restrict text);
FLUFF_API void     fluff_default_error(const char * restrict text);
FLUFF_API void     fluff_default_panic(const char * restrict what);
FLUFF_API int      fluff_default_read(char * buf, size_t len);

FLUFF_API uint64_t fluff_default_hash(const void * data, size_t size);
FLUFF_API uint64_t fluff_default_hash_combine(uint64_t a, uint64_t b);

FLUFF_API void * fluff_default_new_mutex();
FLUFF_API void   fluff_default_mutex_lock(void * self);
FLUFF_API bool   fluff_default_mutex_try_lock(void * self);
FLUFF_API void   fluff_default_mutex_unlock(void * self);
FLUFF_API void   fluff_default_free_mutex(void * self);

typedef struct FluffInstance FluffInstance;

FLUFF_API void fluff_private_test(FluffInstance * instance);

FLUFF_API void fluff_cli(FluffInstance * instance, int argc, const char ** argv);

#endif