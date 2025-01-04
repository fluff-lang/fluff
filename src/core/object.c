/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/object.h>
#include <core/instance.h>
#include <core/module.h>
#include <core/class.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

FLUFF_CONSTEXPR FluffObject * _bool2int(FluffObject * self) {
    return fluff_new_int_object(self->instance, (FluffInt)(self->data._bool));
}

FLUFF_CONSTEXPR FluffObject * _bool2float(FluffObject * self) {
    return fluff_new_float_object(self->instance, (FluffFloat)(self->data._bool));
}

FLUFF_CONSTEXPR FluffObject * _bool2string(FluffObject * self) {
    if (self->data._bool) return fluff_new_string_object_n(self->instance, "true", 4);
    return fluff_new_string_object_n(self->instance, "false", 5);
}

FLUFF_CONSTEXPR FluffObject * _int2bool(FluffObject * self) {
    return fluff_new_bool_object(self->instance, (FluffBool)(self->data._int));
}

FLUFF_CONSTEXPR FluffObject * _int2float(FluffObject * self) {
    return fluff_new_float_object(self->instance, (FluffFloat)(self->data._int));
}

FLUFF_CONSTEXPR FluffObject * _int2string(FluffObject * self) {
    char buf[8] = { 0 };
    fluff_format(buf, 8, "%ld", self->data._int);
    return fluff_new_string_object(self->instance, buf);
}

FLUFF_CONSTEXPR FluffObject * _float2bool(FluffObject * self) {
    return fluff_new_bool_object(self->instance, (FluffBool)(self->data._float));
}

FLUFF_CONSTEXPR FluffObject * _float2int(FluffObject * self) {
    return fluff_new_int_object(self->instance, (FluffInt)(self->data._float));
}

FLUFF_CONSTEXPR FluffObject * _float2string(FluffObject * self) {
    char buf[8] = { 0 };
    fluff_format(buf, 8, "%f", self->data._float);
    return fluff_new_string_object(self->instance, buf);
}

/* -=============
     Instance
   =============- */

FLUFF_API FluffObject * fluff_new_object(FluffInstance * instance, FluffKlass * klass) {
    FluffObject * self = fluff_alloc(NULL, sizeof(FluffObject));
    FLUFF_CLEANUP(self);
    self->instance = instance;
    self->klass    = klass;
    return self;
}

FLUFF_API FluffObject * fluff_new_array_object(FluffInstance * instance, FluffKlass * klass) {
    if (!instance) {
        fluff_push_error("cannot create a primitive object without an instance");
        return NULL;
    }
    FluffObject * self = fluff_new_object(instance, instance->array_klass);
    // TODO: this
    return self;
}

FLUFF_API FluffObject * fluff_new_bool_object(FluffInstance * instance, FluffBool v) {
    if (!instance) {
        fluff_push_error("cannot create a primitive object without an instance");
        return NULL;
    }
    FluffObject * self = fluff_new_object(instance, instance->bool_klass);
    self->data._bool = v;
    return self;
}

FLUFF_API FluffObject * fluff_new_int_object(FluffInstance * instance, FluffInt v) {
    if (!instance) {
        fluff_push_error("cannot create a primitive object without an instance");
        return NULL;
    }
    FluffObject * self = fluff_new_object(instance, instance->int_klass);
    self->data._int = v;
    return self;
}

FLUFF_API FluffObject * fluff_new_float_object(FluffInstance * instance, FluffFloat v) {
    if (!instance) {
        fluff_push_error("cannot create a primitive object without an instance");
        return NULL;
    }
    FluffObject * self = fluff_new_object(instance, instance->float_klass);
    self->data._float = v;
    return self;
}

FLUFF_API FluffObject * fluff_new_string_object(FluffInstance * instance, const char * str) {
    return fluff_new_string_object_n(instance, str, strlen(str));
}

FLUFF_API FluffObject * fluff_new_string_object_n(FluffInstance * instance, const char * str, size_t len) {
    if (!instance) {
        fluff_push_error("cannot create a primitive object without an instance");
        return NULL;
    }
    FluffObject * self = fluff_new_object(instance, instance->string_klass);
    _new_string_n(&self->data._string, str, len);
    return self;
}

FLUFF_API void fluff_free_object(FluffObject * self) {
    FLUFF_CLEANUP(self);
    fluff_free(self);
}

FLUFF_API FluffKlass * fluff_object_get_class(FluffObject * self) {
    return self->klass;
}

FLUFF_API FluffResult fluff_object_add(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_sub(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_mul(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_div(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_mod(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_pow(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_eq(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_ne(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_gt(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_ge(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_lt(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_le(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_and(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_or(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_bit_and(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_bit_or(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_bit_xor(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_bit_shl(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_bit_shr(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_bit_not(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_not(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_negate(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API FluffResult fluff_object_promote(FluffObject * lhs, FluffObject * rhs) {

    return FLUFF_FAILURE;
}

FLUFF_API void * fluff_object_unbox(FluffObject * self) {
    if (!self || !self->instance|| !self->klass) {
        fluff_push_error("cannot unbox an incomplete object");
        return NULL;
    }
    if (self->klass == self->instance->bool_klass)
        return &self->data._bool;
    if (self->klass == self->instance->int_klass)
        return &self->data._int;
    if (self->klass == self->instance->float_klass)
        return &self->data._float;
    if (self->klass == self->instance->string_klass)
        return &self->data._string;
    if (self->klass == self->instance->int_klass)
        return &self->data._bool;
    fluff_push_error("failed to unbox object");
    return NULL;
}

FLUFF_API bool fluff_object_is_same(FluffObject * lhs, FluffObject * rhs) {
    return (lhs && rhs && (lhs == rhs || lhs->data._klass_data == rhs->data._klass_data));
}

FLUFF_API bool fluff_object_is_same_class(FluffObject * self, FluffKlass * klass) {
    return self->klass == klass;
}

FLUFF_API FluffObject * fluff_object_as(FluffObject * self, FluffKlass * klass) {
    if (self->klass == klass) return self;
    if (!self->klass || klass) {
        fluff_push_error("cannot convert an object to/from an incomplete type");
        return NULL;
    }
    if (klass == klass->instance->bool_klass) {
        if (self->klass == self->instance->bool_klass)
            return self;
        if (self->klass == self->instance->int_klass)
            return _bool2int(self);
        if (self->klass == self->instance->float_klass)
            return _bool2float(self);
        if (self->klass == self->instance->string_klass)
            return _bool2string(self);
    }
    if (klass == klass->instance->int_klass) {
        if (self->klass == self->instance->int_klass)
            return self;
        if (self->klass == self->instance->bool_klass)
            return _int2bool(self);
        if (self->klass == self->instance->float_klass)
            return _int2float(self);
        if (self->klass == self->instance->string_klass)
            return _int2string(self);
    }
    if (klass == klass->instance->float_klass) {
        if (self->klass == self->instance->float_klass)
            return self;
        if (self->klass == self->instance->bool_klass)
            return _float2bool(self);
        if (self->klass == self->instance->int_klass)
            return _float2int(self);
        if (self->klass == self->instance->string_klass)
            return _float2string(self);
    }
    fluff_push_error(
        "cannot convert an object of type %s to type %s", 
        self->klass->name.data, klass->name.data
    );
    return NULL;
}

FLUFF_API FluffObject * fluff_object_get_member(FluffObject * self, const char * name);
FLUFF_API FluffObject * fluff_object_get_item(FluffObject * self, const char * name);

FLUFF_API FluffObject * fluff_object_get_member(FluffObject * self, const char * name);

FLUFF_PRIVATE_API void _new_object(FluffObject * self);
FLUFF_PRIVATE_API void _free_object(FluffObject * self);