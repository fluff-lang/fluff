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

const FluffConfig global_default_config = {
    .alloc_fn = fluff_default_alloc, 
    .free_fn  = fluff_default_free, 
    .write_fn = fluff_default_write, 
    .error_fn = fluff_default_error, 

    .strict_mode = false, 
    .manual_mem  = false, 
};

static FluffConfig global_config = global_default_config;

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

    if (!cfg) return FLUFF_OK;
    if (cfg->alloc_fn) global_config.alloc_fn = cfg->alloc_fn;
    if (cfg->free_fn)  global_config.free_fn  = cfg->free_fn;
    if (cfg->write_fn) global_config.write_fn = cfg->write_fn;
    if (cfg->error_fn) global_config.error_fn = cfg->error_fn;

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

FLUFF_API const FluffConfig * fluff_get_default_config() {
    return &global_default_config;
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

FLUFF_API void fluff_error(FluffEnum type, const char * restrict text) {
    global_config.error_fn(type, text);
}

FLUFF_API void fluff_write_fmt(const char * restrict fmt, ...) {
    va_list args1;
    va_start(args1, fmt);
    int len = fluff_vformat(NULL, 0, fmt, args1) + 1;
    va_end(args1);

    va_list args2;
    va_start(args2, fmt);
    char buf[len];
    fluff_vformat(buf, len, fmt, args2);
    buf[len - 1] = '\0';
    va_end(args2);

    fluff_write(buf);
}

FLUFF_API void fluff_error_fmt(FluffEnum type, const char * restrict fmt, ...) {
    va_list args1;
    va_start(args1, fmt);
    int len = fluff_vformat(NULL, 0, fmt, args1) + 1;
    va_end(args1);

    va_list args2;
    va_start(args2, fmt);
    char buf[len];
    fluff_vformat(buf, len, fmt, args2);
    buf[len - 1] = '\0';
    va_end(args2);

    fluff_error(FLUFF_ERROR, buf);
}