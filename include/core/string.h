#pragma once
#ifndef FLUFF_CORE_STRING_H
#define FLUFF_CORE_STRING_H

/* -=============
     Includes
   =============- */

#include <base.h>

/* -===========
     String
   ===========- */

// This represents a dynamically allocated string.
typedef struct FluffString {
    // NOTE: in fluff, indices can be negative, meaning we have 2 extra
    //       bits of information on each string. We are going to use those
    //       extra bits later on.
    int    type     : 2;
    size_t length   : sizeof(size_t) * 8 - 1;
    size_t capacity : sizeof(size_t) * 8 - 1;
    char * data;
} FluffString;

// Creates a string from a C string.
FLUFF_API FluffString * fluff_new_string(const char * str);

// Creates a string from a C string, given size.
FLUFF_API FluffString * fluff_new_string_n(const char * str, size_t size);

// Creates a string by repeating [c] characters [size] times.
FLUFF_API FluffString * fluff_new_string_c(char c, size_t size);

// Creates a string as a copy to another.
FLUFF_API FluffString * fluff_copy_string(const FluffString * other);

// Destroys a string.
FLUFF_API void fluff_free_string(FluffString * self);

// Reserves space in memory for [new_capacity] characters.
// NOTE: if the new capacity <= old capacity, this function does nothing.
FLUFF_API void fluff_string_reserve(FluffString * self, size_t new_capacity);

// Reserves space in memory for [new_size] characters and fill all the new ones with zeros.
FLUFF_API void fluff_string_resize(FluffString * self, size_t new_size);

// Clears space in memory up.
FLUFF_API void fluff_string_clear(FluffString * self);

// Concatenates two strings.
FLUFF_API FluffString * fluff_string_concat(FluffString * lhs, const FluffString * rhs);
FLUFF_API FluffString * fluff_string_concat_s(FluffString * lhs, const char * rh);
FLUFF_API FluffString * fluff_string_concat_sn(FluffString * lhs, const char * rhs, size_t rhs_len);

// Inserts a string inside another given [pos].
FLUFF_API FluffString * fluff_string_insert(FluffString * lhs, size_t pos, const FluffString * rhs);
FLUFF_API FluffString * fluff_string_insert_s(FluffString * lhs, size_t pos, const char * rhs);
FLUFF_API FluffString * fluff_string_insert_sn(FluffString * lhs, size_t pos, const char * rhs, size_t rhs_len);

// Repeats the text contained within string [count] times.
FLUFF_API FluffString * fluff_string_repeat(FluffString * self, size_t count);

// Compares two strings using the strcmp() method.
FLUFF_API int fluff_string_compare(const FluffString * lhs, const FluffString * rhs);
FLUFF_API int fluff_string_compare_s(const FluffString * lhs, const char * rhs);
FLUFF_API int fluff_string_compare_sn(const FluffString * lhs, const char * rhs, size_t rhs_len);

// Checkes if two strings are the exact same
// NOTE: this function is recommended over string_compare if the strings don't have the same size.
FLUFF_API bool fluff_string_equal(const FluffString * lhs, const FluffString * rhs);
FLUFF_API bool fluff_string_equal_s(const FluffString * lhs, const char * rhs);
FLUFF_API bool fluff_string_equal_sn(const FluffString * lhs, const char * rhs, size_t rhs_len);

// Gives the amount of UTF-8 characters in the string given the current locale.
FLUFF_API size_t fluff_string_count(const FluffString * self);

// Checkes if the string is empty.
FLUFF_API bool fluff_string_is_empty(const FluffString * self);

FLUFF_PRIVATE_API void _new_string(FluffString * self, const char * str);
FLUFF_PRIVATE_API void _new_string_n(FluffString * self, const char * str, size_t size);
FLUFF_PRIVATE_API void _new_string_c(FluffString * self, char c, size_t size);
FLUFF_PRIVATE_API void _copy_string(FluffString * self, const FluffString * other);
FLUFF_PRIVATE_API void _free_string(FluffString * self);

#endif