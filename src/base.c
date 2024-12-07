/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <fluff.h>

#include <stdio.h>
#include <stdlib.h>

FLUFF_API void fluff_private_test() {
    FluffInterpreter * interpret = fluff_new_interpreter();
    fluff_interpreter_read_file(interpret, "../hello.fluf");
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

FLUFF_API void fluff_default_write(const char * restrict fmt) {
    fluff_print(stdout, "%s", fmt);
}

FLUFF_API void fluff_default_error(FluffEnum type, const char * restrict fmt) {
    const char * error_name = "error";
    switch (type) {
        case FLUFF_RUNTIME_ERROR:
            { error_name = "runtime error"; break; }
        case FLUFF_COMPILE_ERROR:
            { error_name = "compile error"; break; }
        default: break;
    }
    fluff_print(stderr, "%s: %s\n", error_name, fmt);
    FLUFF_BREAKPOINT();
    exit(-1);
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

FLUFF_API uint64_t fluff_hash(const void * data, size_t size) {
    // NOTE: uses a FNV hashing algorithm, since it's the most efficient option.
    const uint64_t fnv_prime = 0x00000100000001b3;
    const uint64_t fnv_basis = 0xcbf29ce484222325;

    const uint8_t * bin     = data;
    const uint8_t * bin_end = bin + size;
    
    uint64_t hash = fnv_basis;
    while (bin != bin_end) hash = (hash ^ (* bin++)) * fnv_prime;
    return hash;
}

FLUFF_API uint64_t fluff_hash_combine(uint64_t a, uint64_t b) {
    return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
}

FLUFF_API uint fluff_ffs(uint v) {
#if !defined(__builtin_ffs)
    uint r = 0;
    while (v) { v >>= 1; ++r; }
    return r;
#else
    return __builtin_ffs(v);
#endif
}