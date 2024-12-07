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

typedef struct FluffVersion {
    uint8_t major : 4;
    uint8_t minor : 5;
    uint8_t patch : 7;
} FluffVersion;

// Checkes if 2 fluf versions are compatible.
FLUFF_API FluffResult fluff_version_is_compatible(FluffVersion a, FluffVersion b) FLUFF_ERROR_RETURN;

/* -==================
     Configuration
   ==================- */

// See 'FluffConfig'
typedef void * (* FluffAllocFn)(void *, size_t);
typedef void   (* FluffFreeFn)(void *);
typedef void   (* FluffWriteFn)(const char * restrict text);
typedef void   (* FluffErrorFn)(FluffEnum, const char * restrict text);

// This struct determines multiple settings for the language.
typedef struct FluffConfig {
    FluffAllocFn alloc_fn; // Custom function for memory allocation.
    FluffFreeFn  free_fn;  // Custom function for memory deallocation.
    FluffWriteFn write_fn; // Custom function for writing to stdio.
    FluffErrorFn error_fn; // Custom function for error handling.

    bool strict_mode : 1; // Enables all strict compilation modes.
    bool manual_mem  : 1; // Enables manual memory management. Only use it if you know what you're doing!
} FluffConfig;

// Initializes fluf. If 'cfg' is NULL then the default configuration will be used instead.
FLUFF_API FluffResult fluff_init(FluffConfig * cfg, FluffVersion version) FLUFF_ERROR_RETURN;

// Closes fluf.
FLUFF_API void fluff_close();

// Obtains the fluf current configuration.
FLUFF_API FluffConfig fluff_get_config();

// Obtains the fluf default configuration
FLUFF_API const FluffConfig * fluff_get_default_config();

// Creates a configuration preset given program arguments.
FLUFF_API FluffConfig fluff_make_config_by_args(int argc, const char ** argv);

FLUFF_API void fluff_write_fmt(const char * restrict fmt, ...);
FLUFF_API void fluff_error_fmt(FluffEnum type, const char * restrict fmt, ...);

#endif