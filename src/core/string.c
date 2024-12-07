/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <core/string.h>
#include <core/config.h>

#include <string.h>
#include <wchar.h>

/* -===========
     String
   ===========- */

/* -=- Initializers -=- */
FLUFF_API FluffString * fluff_new_string(const char * str) {
    FluffString * self = fluff_alloc(NULL, sizeof(FluffString));
    _new_string(self, str);
    return self;
}

FLUFF_API FluffString * fluff_new_string_n(const char * str, size_t size) {
    FluffString * self = fluff_alloc(NULL, sizeof(FluffString));
    _new_string_n(self, str, size);
    return self;
}

FLUFF_API FluffString * fluff_new_string_c(char c, size_t size) {
    FluffString * self = fluff_alloc(NULL, sizeof(FluffString));
    _new_string_c(self, c, size);
    return self;
}

FLUFF_API FluffString * fluff_copy_string(const FluffString * other) {
    return fluff_new_string_n(other->data, other->length);
}

FLUFF_API void fluff_free_string(FluffString * self) {
    _free_string(self);
    fluff_free(self);
}

/* -=- Data -=- */
FLUFF_API void fluff_string_reserve(FluffString * self, size_t new_capacity) {
    if (self->capacity > new_capacity) return;
    self->capacity = new_capacity + 1;
    self->data     = fluff_alloc(self->data, self->capacity);
    memset(&self->data[self->length + 1], 0, self->capacity - self->length);
}

FLUFF_API void fluff_string_resize(FluffString * self, size_t new_size) {
    fluff_string_reserve(self, new_size);
    self->length = new_size;
    self->data[new_size] = '\0';
}

FLUFF_API void fluff_string_clear(FluffString * self) {
    _free_string(self);
    _new_string(self, NULL);
}

FLUFF_API FluffString * fluff_string_concat(FluffString * lhs, const FluffString * rhs) {
    // NOTE: these constants exist in order to allow concatenating
    //       strings with themselves, do not remove them.
    const size_t lhs_len = lhs->length;
    const size_t rhs_len = rhs->length;
    fluff_string_resize(lhs, lhs_len + rhs_len);
    memcpy(&lhs->data[lhs_len], rhs->data, rhs_len);
    return lhs;
}

FLUFF_API FluffString * fluff_string_insert(FluffString * lhs, size_t pos, const FluffString * rhs) {
    const size_t lhs_len = lhs->length;
    const size_t rhs_len = rhs->length;
    const size_t len     = lhs_len + rhs_len;
    pos = (lhs_len < pos ? lhs_len : pos);

    fluff_string_resize(lhs, len);
    memmove(&lhs->data[lhs_len + pos], &lhs->data[pos], lhs_len - pos);
    memcpy(&lhs->data[pos], rhs->data, rhs_len);
    return lhs;
}

FLUFF_API FluffString * fluff_string_repeat(FluffString * self, size_t count) {
    const size_t len = self->length;
    fluff_string_resize(self, len * count);
    while (count-- > 1)
        memcpy(&self->data[self->length - count * len], self->data, len);
    return self;
}

/* -=- Metadata -=- */
FLUFF_API int fluff_string_compare(const FluffString * lhs, const FluffString * rhs) {
    return strncmp(lhs->data, rhs->data, (lhs->length < rhs->length ? lhs->length : rhs->length));
}

FLUFF_API int fluff_string_compare_c(const FluffString * lhs, const char * rhs) {
    const size_t rhs_len = strlen(rhs);
    return strncmp(lhs->data, rhs, (lhs->length < rhs_len ? lhs->length : rhs_len));
}

FLUFF_API bool fluff_string_equal(const FluffString * lhs, const FluffString * rhs) {
    return lhs->length == rhs->length && strncmp(lhs->data, rhs->data, lhs->length) == 0;
}

FLUFF_API bool fluff_string_equal_c(const FluffString * lhs, const char * rhs) {
    const size_t rhs_len = strlen(rhs);
    return lhs->length == rhs_len && strncmp(lhs->data, rhs, rhs_len) == 0;
}

FLUFF_API size_t fluff_string_count(const FluffString * self) {
    mbstate_t state = { 0 };
    size_t count = 0;
    const char * ptr = self->data;
    const char * end = ptr + self->length;
    while (ptr < end) {
        if (* ptr == '\0') break;
        int next = mbrlen(ptr, end - ptr, &state);
        if (next < 0) break;
        ptr += next;
        ++count;
    }
    return count;
}

FLUFF_API bool fluff_string_is_empty(const FluffString * self) {
    return self->length == 0;
}

/* -=- Private -=- */
FLUFF_PRIVATE_API void _new_string(FluffString * self, const char * str) {
    self->type     = 0;
    self->length   = (str ? strlen(str) : 0);
    self->capacity = self->length + 1;
    self->data     = fluff_alloc(NULL, self->capacity);
    if (str) memcpy(self->data, str, self->length);
    self->data[self->length] = '\0';
}

FLUFF_PRIVATE_API void _new_string_n(FluffString * self, const char * str, size_t size) {
    self->length   = size;
    self->capacity = size + 1;
    self->data     = fluff_alloc(NULL, self->capacity);
    if (str) {
        memcpy(self->data, str, self->length);
        self->data[self->length] = '\0';
    } else {
        memset(self->data, 0, self->capacity);
    }
}

FLUFF_PRIVATE_API void _new_string_c(FluffString * self, char c, size_t size) {
    self->length   = size;
    self->capacity = size + 1;
    self->data     = fluff_alloc(NULL, self->capacity);
    memset(self->data, c, size);
    self->data[size] = '\0';
}

FLUFF_PRIVATE_API void _copy_string(FluffString * self, const FluffString * other) {
    _new_string_n(self, other->data, other->length);
}

FLUFF_PRIVATE_API void _free_string(FluffString * self) {
    fluff_free(self->data);
    FLUFF_CLEANUP(self);
}