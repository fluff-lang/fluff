/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <fluff.h>

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

/* -==============
     Internals
   ==============- */

FLUFF_API void fluff_private_test() {
    FluffInstance * instance = fluff_get_instance();

    FluffModule * module = fluff_new_module("main");
    fluff_instance_add_module(instance, module);

    // FluffInterpreter * interpret = fluff_new_interpreter(module);
    // fluff_interpreter_read_file(interpret, "../hello.fluff");
    // fluff_free_interpreter(interpret);

    FluffKlass * foo_klass = _new_class("Foo", 3, NULL);
    _class_add_property(foo_klass, "a", instance->int_klass, fluff_new_int_object(instance, 1));
    _module_add_class(module, foo_klass);

    FluffKlass * bar_klass = _new_class("Bar", 3, foo_klass);
    //_class_add_property(bar_klass, "b", instance->int_klass, fluff_new_int_object(instance, 2));
    _module_add_class(module, bar_klass);

    FluffKlass * baz_klass = _new_class("Baz", 3, bar_klass);
    _class_add_property(baz_klass, "c", instance->int_klass, fluff_new_int_object(instance, 3));
    _module_add_class(module, baz_klass);

    FluffObject * obj_a = fluff_new_object(instance, baz_klass);
    FluffObject * obj_b = fluff_object_as(obj_a, foo_klass);
    FluffObject * obj_c = fluff_object_as(obj_b, bar_klass);
    printf("%p %p\n", obj_a, obj_c);
    fluff_free_object(obj_b);
    fluff_free_object(obj_a);
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

FLUFF_API void * fluff_default_new_mutex() {
    pthread_mutex_t * self = fluff_alloc(NULL, sizeof(pthread_mutex_t));
    // NOTE: this always returns 0 so it's not going to be handled
    pthread_mutex_init(self, NULL);
    return self;
}

FLUFF_API void fluff_default_mutex_lock(void * self) {
    if (pthread_mutex_lock((pthread_mutex_t *)self) == EDEADLK)
        fluff_panic_fmt("deadlock on thread %p", self);
}

FLUFF_API bool fluff_default_mutex_try_lock(void * self) {
    return (pthread_mutex_trylock((pthread_mutex_t *)self) == EBUSY);
}

FLUFF_API void fluff_default_mutex_unlock(void * self) {
    if (pthread_mutex_unlock((pthread_mutex_t *)self) == EPERM)
        fluff_panic_fmt("thread doesn't own the mutex %p", self);
}

FLUFF_API void fluff_default_free_mutex(void * self) {
    pthread_mutex_destroy((pthread_mutex_t *)self);
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