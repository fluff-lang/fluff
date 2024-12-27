/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <fluff.h>

#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

FLUFF_API void fluff_private_test() {
    FluffInterpreter * interpret = fluff_new_interpreter();
    fluff_interpreter_read_file(interpret, "../hello.fluff");
    _ast_node_solve(&interpret->ast.root);
    fluff_free_interpreter(interpret);
}

/* -==========
     Utils
   ==========- */

FLUFF_API int fluff_vformat(char * buf, size_t buf_len, const char * restrict fmt, va_list args) {
    return vsnprintf(buf, buf_len, fmt, args);
}

FLUFF_API int fluff_format(char * buf, size_t buf_len, const char * restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = fluff_vformat(buf, buf_len, fmt, args);
    va_end(args);
    return r;
}

FLUFF_API int fluff_vprint(FILE * f, const char * restrict fmt, va_list args) {
    return vfprintf(f, fmt, args);
}

FLUFF_API int fluff_print(FILE * f, const char * restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = fluff_vprint(f, fmt, args);
    va_end(args);
    return r;
}

FLUFF_API void * fluff_default_alloc(void * ptr, size_t size) {
    if (ptr) return realloc(ptr, size);
    return malloc(size);
}

FLUFF_API void fluff_default_free(void * ptr) {
    free(ptr);
}

FLUFF_API void fluff_default_write(const char * restrict text) {
    fluff_print(stdout, "%s", text);
}

FLUFF_API void fluff_default_error(const char * restrict text) {
    fluff_print(stderr, "%s", text);
}

FLUFF_API int fluff_default_read(char * buf, size_t len) {
    // TODO: this
    return -1;
}

FLUFF_API void fluff_default_panic(const char * restrict what) {
    fluff_print(stderr, "%s\n", what);
    exit(-1);
}

FLUFF_API uint64_t fluff_default_hash(const void * data, size_t size) {
    // NOTE: uses a FNV hashing algorithm, since it's the most efficient option.
    const uint64_t fnv_prime = 0x00000100000001b3;
    const uint64_t fnv_basis = 0xcbf29ce484222325;

    const uint8_t * bin     = data;
    const uint8_t * bin_end = bin + size;
    
    uint64_t hash = fnv_basis;
    while (bin != bin_end) hash = (hash ^ (* bin++)) * fnv_prime;
    return hash;
}

FLUFF_API uint64_t fluff_default_hash_combine(uint64_t a, uint64_t b) {
    return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
}

// TODO: mutexes
// TODO: make mutexes time out
FLUFF_API void * fluff_default_new_mutex() {
    mtx_t * self = fluff_alloc(NULL, sizeof(mtx_t));
    if (mtx_init(self, mtx_timed) != thrd_success) {
        fluff_free(self);
        fluff_panic("failed to create mutex");
    }
    return self;
}

FLUFF_API void fluff_default_mutex_lock(void * self) {

}

FLUFF_API void fluff_default_mutex_unlock(void * self) {

}

FLUFF_API void fluff_default_mutex_wait(void * self) {

}

FLUFF_API void fluff_default_free_mutex(void * self) {
    mtx_destroy((mtx_t *)self);
    fluff_free(self);
}

#define ENUM_CASE(str) case str: return #str;

FLUFF_API const char * fluff_enum_to_string(FluffEnum e) {
    switch (e) {
        ENUM_CASE(FLUFF_ERROR)
        ENUM_CASE(FLUFF_RUNTIME_ERROR)
        ENUM_CASE(FLUFF_COMPILE_ERROR)
        default: return "";
    }
}