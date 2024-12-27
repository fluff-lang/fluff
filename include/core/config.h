#pragma once
#ifndef FLUFF_CORE_CONFIG_H
#define FLUFF_CORE_CONFIG_H

/*

FIXME: MISSING FUNCTIONALITY
      - loading FluffConfig by program args
      - error handling without immediate panic

*/

/* -=============
     Includes
   =============- */

#include <base.h>

/* -===========
     Macros
   ===========- */

#ifdef FLUFF_DEBUG
#   define fluff_assert(__cond, __fmt, ...)\
           { if (!(bool)(__cond)) fluff_error_fmt(FLUFF_ERROR, "%s:%d: assertion " #__cond " failed: " __fmt, __FILE__, __LINE__, ##__VA_ARGS__); }
#else
#   define fluff_assert(__cond, ...)\
           { if (!(bool)(__cond)) fluff_error_fmt(FLUFF_ERROR, "assertion " #__cond " failed! " __VA_ARGS__); }
#endif

#ifdef FLUFF_DEBUG
#   define fluff_log(__fmt, ...) fluff_write("%s:%d: " __fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#   define fluff_log(...)
#endif

#define FLUFF_MAKE_VERSION(__major, __minor, __patch)\
        ((FluffVersion){ .major = (__major), .minor = (__minor), .patch = (__patch) })
#define FLUFF_CURRENT_VERSION FLUFF_MAKE_VERSION(0, 1, 0)

/* -=============
     Version
   =============- */

// This struct represents a fluff version.
typedef struct FluffVersion {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} FluffVersion;

// Checkes if 2 fluff versions are compatible.
FLUFF_API FluffResult fluff_version_is_compatible(FluffVersion a, FluffVersion b);

/* -==================
     Configuration
   ==================- */

// See 'FluffConfig'
typedef void   * (* FluffAllocFn)(void *, size_t);
typedef void     (* FluffFreeFn)(void *);
typedef void     (* FluffWriteFn)(const char * restrict);
typedef void     (* FluffErrorFn)(const char * restrict);
typedef int      (* FluffReadFn)(char *, int);

typedef uint64_t (* FluffHashFn)(const void *, size_t);
typedef uint64_t (* FluffHashCombineFn)(uint64_t, uint64_t);

typedef void (* FluffPanicFn)(const char * restrict);

typedef void * (* FluffMutexNewFn)();
typedef void   (* FluffMutexLockFn)(void *);
typedef void   (* FluffMutexUnlockFn)(void *);
typedef void   (* FluffMutexWaitFn)(void *);
typedef void   (* FluffMutexFreeFn)(void *);

// This struct determines multiple settings for the language.
typedef struct FluffConfig {
    FluffAllocFn  alloc_fn; // Custom memory allocation function (default: malloc and realloc).
    FluffFreeFn   free_fn;  // Custom memory deallocation function (default: free).
    FluffWriteFn  write_fn; // Custom function to write into stdio (default: fprintf).
    FluffErrorFn  error_fn; // Custom function to write into stderr (default: fprintf).
    FluffReadFn   read_fn;  // Custom function to read from stdin (default: fread).
    FluffPanicFn  panic_fn; // Custom function for panic.
    
    FluffHashFn        hash_fn;         // Custom hashing function.
    FluffHashCombineFn hash_combine_fn; // Custom hash combination function.

    // NOTE: if you change one of these functions, it is recommended to change the others to avoid UB.
    FluffMutexNewFn    new_mutex_fn;    // Custom mutex creation function.
    FluffMutexLockFn   mutex_lock_fn;   // Custom mutex lock function.
    FluffMutexUnlockFn mutex_unlock_fn; // Custom mutex unlock function.
    FluffMutexWaitFn   mutex_wait_fn;   // Custom mutex wait function (wait until unlocked).
    FluffMutexFreeFn   free_mutex_fn;   // Custom mutex free function.

    bool strict_mode; // Enables all strict compilation modes.
    bool manual_mem;  // Enables manual memory management. Only use it if you know what you're doing!
} FluffConfig;

// Initializes fluff. If 'cfg' is NULL then the default configuration will be used instead.
FLUFF_API FluffResult fluff_init(FluffConfig * cfg, FluffVersion version) FLUFF_ERROR_RETURN;

// Closes fluff.
FLUFF_API void fluff_close();

// Obtains the fluff current configuration.
FLUFF_API FluffConfig fluff_get_config();

// Obtains the fluff default configuration
FLUFF_API FluffConfig fluff_get_default_config();

// Creates a configuration preset given program arguments.
FLUFF_API FluffConfig fluff_make_config_by_args(int argc, const char ** argv);

// Calls the corresponding callback in the current configuration.
FLUFF_API void * fluff_alloc(void * ptr, size_t size);
FLUFF_API void   fluff_free(void * ptr);
FLUFF_API void   fluff_write(const char * restrict text);
FLUFF_API void   fluff_error(const char * restrict text);
FLUFF_API void   fluff_panic(const char * restrict what);
FLUFF_API int    fluff_read(char * buf, int len);

FLUFF_API uint64_t fluff_hash(const void * data, size_t size);
FLUFF_API uint64_t fluff_hash_combine(uint64_t a, uint64_t b);

FLUFF_API void * fluff_new_mutex();
FLUFF_API void   fluff_mutex_lock(void * self);
FLUFF_API void   fluff_mutex_unlock(void * self);
FLUFF_API void   fluff_mutex_wait(void * self);
FLUFF_API void   fluff_free_mutex(void * self);

FLUFF_API void fluff_write_fmt(const char * restrict fmt, ...);
FLUFF_API void fluff_error_fmt(FluffEnum type, const char * restrict fmt, ...);

#endif