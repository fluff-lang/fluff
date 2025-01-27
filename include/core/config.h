#pragma once
#ifndef FLUFF_CORE_CONFIG_H
#define FLUFF_CORE_CONFIG_H

/* -=============
     Includes
   =============- */

#include <base.h>

/* -===========
     Macros
   ===========- */

/* Checkes if a condition is true, if it's not, panics with a custom message */
#define fluff_assert(__cond, ...) {\
            if (!(bool)(__cond))\
                fluff_panic_fmt("[" __FILE__ "]:\n\tassertion (" #__cond ") failed! " __VA_ARGS__);\
        }

/*! Creates a fluff version given major, minor, and patch numbers */
#define fluff_make_version(__major, __minor, __patch)\
        ((FluffVersion){ .major = (__major), .minor = (__minor), .patch = (__patch) })

/*! Gives the current fluff version relative to the header */
#define FLUFF_CURRENT_VERSION fluff_make_version(0, 1, 0)

/* -=============
     Version
   =============- */

/*! Represents a fluff version */
typedef struct FluffVersion {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} FluffVersion;

/*! Checkes if 2 fluff versions are compatible */
FLUFF_API FluffResult fluff_version_is_compatible(FluffVersion a, FluffVersion b);

/* -==================
     Configuration
   ==================- */

/*! Callback to allocate or reallocate a chunk of memory in the heap */
typedef void * (* FluffAllocFn)(void *, size_t);

/*! Callback to free a chunk of memory in the heap */
typedef void (* FluffFreeFn)(void *);

/*! Callback to write into the console */
typedef void (* FluffWriteFn)(const char * restrict);

/*! Callback to read from the console */
typedef int (* FluffReadFn)(char *, int);

/*! Callback to hash data */
typedef uint64_t (* FluffHashFn)(const void *, size_t);

/*! Callback to combine two hashed values */
typedef uint64_t (* FluffHashCombineFn)(uint64_t, uint64_t);

/*! Callback to create a mutex */
typedef void * (* FluffMutexNewFn)();

/*! Callback to lock a mutex */
typedef void (* FluffMutexLockFn)(void *);

/*! Callback to try to lock a mutex */
typedef bool (* FluffMutexTryLockFn)(void *);

/*! Callback to unlock a mutex */
typedef void (* FluffMutexUnlockFn)(void *);

/*! Callback to free a mutex */
typedef void (* FluffMutexFreeFn)(void *);

/*! This struct determines multiple settings for the language. */
typedef struct FluffConfig {
    FluffAllocFn  alloc_fn;
    FluffFreeFn   free_fn;
    FluffWriteFn  write_fn;
    FluffWriteFn  error_fn;
    FluffWriteFn  panic_fn;
    FluffReadFn   read_fn;
    
    FluffHashFn        hash_fn;
    FluffHashCombineFn hash_combine_fn;

    FluffMutexNewFn     new_mutex_fn;
    FluffMutexLockFn    mutex_lock_fn;
    FluffMutexUnlockFn  mutex_unlock_fn;
    FluffMutexTryLockFn mutex_try_lock_fn;
    FluffMutexFreeFn    free_mutex_fn;

    bool strict_mode;
    bool manual_mem;
} FluffConfig;

/*! Initializes fluff. If 'cfg' is NULL then the default configuration will be used instead. */
FLUFF_API FluffResult fluff_init(FluffConfig * cfg, FluffVersion version);

/*! Closes fluff. */
FLUFF_API void fluff_close();

/*! Obtains the fluff current configuration. */
FLUFF_API FluffConfig fluff_get_config();

/*! Obtains the fluff default configuration */
FLUFF_API FluffConfig fluff_get_default_config();

/*! Creates a configuration preset given program arguments. */
FLUFF_API FluffConfig fluff_make_config_by_args(int argc, const char ** argv);

/*!
 * Allocates memory in the heap
 * This works as a mix of malloc() and realloc().
 * Memory can be also expanded by using the 'ptr' and 'old_size' arguments.
 * @param 'ptr' is the old pointer for memory expansion
 * @param 'size' is the size of the memory chunk in bytes
 * @param 'old_size' is the size of the old memory chunk in bytes
 * @return The memory chunk
*/
FLUFF_API void * fluff_alloc(void * ptr, size_t size);

/*!
 * Frees memory from the heap
 * This works identically to free().
 * @param 'ptr' is pointer to be free'd
 * @param 'size' is the size of the pointer to be free'd
*/
FLUFF_API void fluff_free(void * ptr);

/*!
 * Writes text to the hosts CLI.
 * @param 'text' is text to be written
*/
FLUFF_API void fluff_write(const char * restrict text);

/*!
 * Same as fluff_write() but may write to the error output instead.
 * @param 'text' is text to be written
*/
FLUFF_API void fluff_error(const char * restrict text);

/*!
 * Acts as fluff_error() but immediately aborts execution.
 * @param 'text' is text to be written
*/
FLUFF_API void fluff_panic(const char * restrict what);

/*!
 * Reads user input from the hosts CLI.
 * If a valid parameter is not specified, the function will return the length of the input buffer only.
 * @param 'buf' is the buffer to write the input to
 * @param 'len' is the length of the buffer pointed to by 'buf'
 * @return Length of the input buffer
*/
FLUFF_API int fluff_read(char * buf, int len);

/*! Hashes a value */
FLUFF_API uint64_t fluff_hash(const void * data, size_t size);

/*! Combines 2 prehashed values */
FLUFF_API uint64_t fluff_hash_combine(uint64_t a, uint64_t b);

/*! Creates a mutex (POSIX style call)
 * @see https://www.man7.org/linux/man-pages/man3/pthread_mutex_init.3.html
*/
FLUFF_API void * fluff_new_mutex();

/*! Locks a mutex (POSIX style call)
 * @see https://www.man7.org/linux/man-pages/man3/pthread_mutex_lock.3.html
*/
FLUFF_API void fluff_mutex_lock(void * self);

/*! Tries to lock a mutex (POSIX style call)
 * @see https://www.man7.org/linux/man-pages/man3/pthread_mutex_trylock.3.html
*/
FLUFF_API bool fluff_mutex_try_lock(void * self);

/*! Unlocks a mutex (POSIX style call)
 * @see https://www.man7.org/linux/man-pages/man3/pthread_mutex_unlock.3.html
*/
FLUFF_API void fluff_mutex_unlock(void * self);

/*! Free's a mutex (POSIX style call)
 * @see https://www.man7.org/linux/man-pages/man3/pthread_mutex_destroy.3.html
*/
FLUFF_API void fluff_free_mutex(void * self);

/*! Simple formatter abstraction */
FLUFF_API void fluff_error_fmt(const char * restrict fmt, ...);
FLUFF_API void fluff_write_fmt(const char * restrict fmt, ...);
FLUFF_API void fluff_panic_fmt(const char * restrict fmt, ...);

/*! Simple formatter abstraction */
FLUFF_PRIVATE_API void fluff_write_callback_fmt_v(FluffWriteFn fn, const char * restrict fmt, va_list args);
FLUFF_PRIVATE_API void fluff_write_callback_fmt(FluffWriteFn fn, const char * restrict fmt, ...);

#endif