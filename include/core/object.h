#pragma once
#ifndef FLUFF_CORE_OBJECT_H
#define FLUFF_CORE_OBJECT_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/string.h>

/* -================
     ObjectArray
   ================- */


/* -===========
     Object
   ===========- */

typedef struct FluffInstance FluffInstance;
typedef struct FluffKlass FluffKlass;
typedef struct FluffObject FluffObject;

typedef struct ObjectTable {
    size_t        ref_count;
    FluffObject * vptr;
} ObjectTable;

typedef struct FluffObject {
    FluffInstance * instance;
    FluffKlass    * klass;
    union {
        FluffBool   _bool;
        FluffInt    _int;
        FluffFloat  _float;
        FluffString _string;

        // ObjectArray _array;

        void * _data;
    } data;
} FluffObject;

FLUFF_API FluffObject * fluff_new_object(FluffInstance * instance, FluffKlass * klass);
FLUFF_API FluffObject * fluff_new_null_object(FluffInstance * instance, FluffKlass * klass);
FLUFF_API FluffObject * fluff_new_array_object(FluffInstance * instance, FluffKlass * klass);
FLUFF_API FluffObject * fluff_new_bool_object(FluffInstance * instance, FluffBool v);
FLUFF_API FluffObject * fluff_new_int_object(FluffInstance * instance, FluffInt v);
FLUFF_API FluffObject * fluff_new_float_object(FluffInstance * instance, FluffFloat v);
FLUFF_API FluffObject * fluff_new_string_object(FluffInstance * instance, const char * str);
FLUFF_API FluffObject * fluff_new_string_object_n(FluffInstance * instance, const char * str, size_t len);
FLUFF_API FluffObject * fluff_ref_object(FluffObject * self);
FLUFF_API FluffObject * fluff_clone_object(FluffObject * self);
FLUFF_API void          fluff_free_object(FluffObject * self);

FLUFF_API FluffKlass * fluff_object_get_class(FluffObject * self);

FLUFF_API FluffResult fluff_object_add(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_sub(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_mul(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_div(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_mod(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_pow(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_eq(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_ne(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_gt(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_ge(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_lt(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_le(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_and(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_or(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_bit_and(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_bit_or(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_bit_xor(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_bit_shl(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_bit_shr(FluffObject * lhs, FluffObject * rhs, FluffObject * result);
FLUFF_API FluffResult fluff_object_bit_not(FluffObject * self, FluffObject * result);
FLUFF_API FluffResult fluff_object_not(FluffObject * self, FluffObject * result);
FLUFF_API FluffResult fluff_object_negate(FluffObject * self, FluffObject * result);
FLUFF_API FluffResult fluff_object_promote(FluffObject * self, FluffObject * result);

FLUFF_API bool        fluff_object_is_same(FluffObject * lhs, FluffObject * rhs);
FLUFF_API bool        fluff_object_is_same_class(FluffObject * self, FluffKlass * klass);
FLUFF_API FluffResult fluff_object_as(FluffObject * self, FluffKlass * klass, FluffObject * result);

FLUFF_API FluffObject * fluff_object_get_member(FluffObject * self, const char * name);
FLUFF_API FluffObject * fluff_object_get_item(FluffObject * self, const char * name);

FLUFF_API void * fluff_object_unbox(FluffObject * self);

FLUFF_PRIVATE_API void _new_object(FluffObject * self, FluffInstance * instance, FluffKlass * klass, FluffObject * top_obj);
FLUFF_PRIVATE_API void _new_null_object(FluffObject * self, FluffInstance * instance, FluffKlass * klass);
FLUFF_PRIVATE_API void _new_array_object(FluffObject * self, FluffInstance * instance, FluffKlass * klass);
FLUFF_PRIVATE_API void _new_bool_object(FluffObject * self, FluffInstance * instance, FluffBool v);
FLUFF_PRIVATE_API void _new_int_object(FluffObject * self, FluffInstance * instance, FluffInt v);
FLUFF_PRIVATE_API void _new_float_object(FluffObject * self, FluffInstance * instance, FluffFloat v);
FLUFF_PRIVATE_API void _new_string_object(FluffObject * self, FluffInstance * instance, const char * str);
FLUFF_PRIVATE_API void _new_string_object_n(FluffObject * self, FluffInstance * instance, const char * str, size_t len);
FLUFF_PRIVATE_API void _clone_object(FluffObject * self, FluffObject * obj);
FLUFF_PRIVATE_API void _free_object(FluffObject * self);

FLUFF_PRIVATE_API void _object_alloc(FluffObject * self, FluffInstance * instance, FluffObject * top_obj);
FLUFF_PRIVATE_API void _object_alloc_common_class(FluffObject * self, FluffInstance * instance, FluffObject * top_obj);
FLUFF_PRIVATE_API void _object_alloc_native_class(FluffObject * self, FluffInstance * instance, FluffObject * top_obj);
FLUFF_PRIVATE_API void _object_get_data(FluffObject * self, ObjectTable * table, FluffObject ** subobjects);
FLUFF_PRIVATE_API void _object_set_data(FluffObject * self, ObjectTable * table, FluffObject * subobjects, size_t obj_count);

FLUFF_PRIVATE_API void          _object_ref(FluffObject * self);
FLUFF_PRIVATE_API void          _object_deref(FluffObject * self);
FLUFF_PRIVATE_API FluffObject * _object_realloc_as_linked(FluffObject * self);

#endif