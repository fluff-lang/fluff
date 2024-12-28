/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <core/config.h>

#include <stdlib.h>

/* -==============
     Internals
   ==============- */

#define DEFAULT_CONFIG (FluffConfig){\
            .alloc_fn         = fluff_default_alloc,\
            .free_fn          = fluff_default_free,\
            .write_fn         = fluff_default_write,\
            .error_fn         = fluff_default_error,\
            .panic_fn         = fluff_default_panic,\
            .hash_fn          = fluff_default_hash,\
            .hash_combine_fn  = fluff_default_hash_combine,\
            .new_mutex_fn     = fluff_default_new_mutex,\
            .mutex_lock_fn    = fluff_default_mutex_lock,\
            .mutex_unlock_fn  = fluff_default_mutex_unlock,\
            .free_mutex_fn    = fluff_default_free_mutex,\
            .strict_mode      = false,\
            .manual_mem       = false,\
        };

const FluffConfig global_default_config = DEFAULT_CONFIG;

static FluffConfig global_config = DEFAULT_CONFIG;

/* -============
     Version
   ============- */

/* -=- Initializers -=- */
FLUFF_API FluffResult fluff_version_is_compatible(FluffVersion a, FluffVersion b) {
    return a.major == b.major && a.minor == b.minor;
}

/* -==================
     Configuration
   ==================- */

/* -=- Initializers -=- */
FLUFF_API FluffResult fluff_init(FluffConfig * cfg, FluffVersion version) {
    if (!fluff_version_is_compatible(version, FLUFF_CURRENT_VERSION))
        return FLUFF_FAILURE;

    // Sets each callback if they were not binded yet
    if (!cfg) return FLUFF_OK;
    
    if (cfg->alloc_fn)        global_config.alloc_fn        = cfg->alloc_fn;
    if (cfg->free_fn)         global_config.free_fn         = cfg->free_fn;
    if (cfg->write_fn)        global_config.write_fn        = cfg->write_fn;
    if (cfg->error_fn)        global_config.error_fn        = cfg->error_fn;
    if (cfg->panic_fn)        global_config.panic_fn        = cfg->panic_fn;
    if (cfg->hash_fn)         global_config.hash_fn         = cfg->hash_fn;
    if (cfg->hash_combine_fn) global_config.hash_combine_fn = cfg->hash_combine_fn;
    if (cfg->new_mutex_fn)    global_config.new_mutex_fn    = cfg->new_mutex_fn;
    if (cfg->mutex_lock_fn)   global_config.mutex_lock_fn   = cfg->mutex_lock_fn;
    if (cfg->mutex_unlock_fn) global_config.mutex_unlock_fn = cfg->mutex_unlock_fn;
    if (cfg->free_mutex_fn)   global_config.free_mutex_fn   = cfg->free_mutex_fn;

    return FLUFF_OK;
}

FLUFF_API void fluff_close() {
    
}

FLUFF_API FluffConfig fluff_make_config_by_args(int argc, const char ** argv) {
    FluffConfig cfg = global_default_config;
    if (argc == 0 || argv == NULL) return cfg;
    return cfg;
}

FLUFF_API FluffConfig fluff_get_config() {
    return global_config;
}

FLUFF_API FluffConfig fluff_get_default_config() {
    return global_default_config;
}

/* -=- Configuration callbacks -=- */
FLUFF_API void * fluff_alloc(void * ptr, size_t size) {
    void * r = global_config.alloc_fn(ptr, size);
    fluff_assert(r, "allocation failure");
    return r;
}

FLUFF_API void fluff_free(void * ptr) {
    global_config.free_fn(ptr);
}

FLUFF_API void fluff_write(const char * restrict text) {
    global_config.write_fn(text);
}

FLUFF_API void fluff_error(const char * restrict text) {
    global_config.error_fn(text);
}

FLUFF_API void fluff_panic(const char * restrict what) {
    global_config.panic_fn(what);
}

FLUFF_API int fluff_read(char * buf, int len) {
    return global_config.read_fn(buf, len);
}

FLUFF_API uint64_t fluff_hash(const void * data, size_t size) {
    return global_config.hash_fn(data, size);
}

FLUFF_API uint64_t fluff_hash_combine(uint64_t a, uint64_t b) {
    return global_config.hash_combine_fn(a, b);
}

FLUFF_API void * fluff_new_mutex() {
    return global_config.new_mutex_fn();
}

FLUFF_API void fluff_mutex_lock(void * self) {
    global_config.mutex_lock_fn(self);
}

FLUFF_API void fluff_mutex_unlock(void * self) {
    global_config.mutex_unlock_fn(self);
}

FLUFF_API void fluff_free_mutex(void * self) {
    global_config.free_mutex_fn(self);
}

FLUFF_API void _write_callback_fmt_v(FluffWriteFn fn, const char * restrict fmt, va_list args) {
    va_list args1, args2;
    va_copy(args1, args);
    va_copy(args2, args);

    int len = fluff_vformat(NULL, 0, fmt, args1) + 1;
    if (len < 0) fluff_panic("format failure");
    va_end(args1);

    char buf[len];

    if (fluff_vformat(buf, len, fmt, args2) < 0)
        fluff_panic("format failure");
    va_end(args2);

    buf[len - 1] = '\0';

    fn(buf);
}

FLUFF_API void _write_callback_fmt(FluffWriteFn fn, const char * restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _write_callback_fmt_v(fn, fmt, args);
    va_end(args);
}

FLUFF_API void fluff_write_fmt(const char * restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _write_callback_fmt_v(global_config.write_fn, fmt, args);
    va_end(args);
}

FLUFF_API void fluff_error_fmt(const char * restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _write_callback_fmt_v(global_config.error_fn, fmt, args);
    va_end(args);
}

FLUFF_API void fluff_panic_fmt(const char * restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _write_callback_fmt_v(global_config.panic_fn, fmt, args);
    va_end(args);
}