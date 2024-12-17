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
typedef void     (* FluffWriteFn)(const char * restrict text);
typedef void     (* FluffErrorFn)(FluffEnum, const char * restrict text);
typedef uint64_t (* FluffHashFn)(void *, size_t);
typedef uint64_t (* FluffHashCombineFn)(uint64_t, uint64_t);

// This struct determines multiple settings for the language.
typedef struct FluffConfig {
    FluffAllocFn       alloc_fn;        // Custom memory allocation function (default: malloc and realloc).
    FluffFreeFn        free_fn;         // Custom memory deallocation function (default: free).
    FluffWriteFn       write_fn;        // Custom CLI write function (default: printf).
    FluffErrorFn       error_fn;        // Custom error handling function.
    FluffHashFn        hash_fn;         // Custom function for hashing.
    FluffHashCombineFn hash_combine_fn; // Custom function for hash combination.

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

FLUFF_API void fluff_write_fmt(const char * restrict fmt, ...);
FLUFF_API void fluff_error_fmt(FluffEnum type, const char * restrict fmt, ...);

#endif