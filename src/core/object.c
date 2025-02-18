/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/object.h>
#include <core/class.h>
#include <core/method.h>
#include <core/string.h>
#include <core/module.h>
#include <core/instance.h>
#include <core/vm.h>
#include <core/config.h>

#include <math.h>

/* -==============
     Internals
   ==============- */

FLUFF_CONSTEXPR FluffObject * _bool2int(FluffObject * self) {
    return fluff_new_int_object(self->instance, (FluffInt)self->data._bool);
}

FLUFF_CONSTEXPR FluffObject * _bool2float(FluffObject * self) {
    return fluff_new_float_object(self->instance, (FluffFloat)self->data._bool);
}

FLUFF_CONSTEXPR FluffObject * _bool2string(FluffObject * self) {
    if (self->data._bool)
        return fluff_new_string_object_n(self->instance, "true", 4);
    return fluff_new_string_object_n(self->instance, "false", 5);
}

FLUFF_CONSTEXPR FluffObject * _int2bool(FluffObject * self) {
    return fluff_new_bool_object(self->instance, (FluffBool)(self->data._int));
}

FLUFF_CONSTEXPR FluffObject * _int2float(FluffObject * self) {
    return fluff_new_float_object(self->instance, (FluffFloat)(self->data._int));
}

FLUFF_CONSTEXPR FluffObject * _int2string(FluffObject * self) {
    char buf[16] = { 0 };
    fluff_format(buf, 16, "%ld", self->data._int);
    return fluff_new_string_object_n(self->instance, buf, 16);
}

FLUFF_CONSTEXPR FluffObject * _float2bool(FluffObject * self) {
    return fluff_new_bool_object(self->instance, (FluffBool)(self->data._float));
}

FLUFF_CONSTEXPR FluffObject * _float2int(FluffObject * self) {
    return fluff_new_int_object(self->instance, (FluffInt)(self->data._float));
}

FLUFF_CONSTEXPR FluffObject * _float2string(FluffObject * self) {
    char buf[16] = { 0 };
    fluff_format(buf, 16, "%f", self->data._float);
    return fluff_new_string_object_n(self->instance, buf, 16);
}

FLUFF_CONSTEXPR FluffObject * _string2bool(FluffObject * self) {
    return fluff_new_bool_object(self->instance, (self->data._string.length != 0));
}

#define DEF_OP(__type, __action, __op, __field)\
        FLUFF_CONSTEXPR FluffResult _##__type##_##__action(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {\
            result->data.__field = lhs->data.__field __op rhs->data.__field;\
            return FLUFF_OK;\
        }

#define DEF_UOP(__type, __action, __op, __field)\
        FLUFF_CONSTEXPR FluffResult _##__type##_##__action(FluffObject * self, FluffObject * result) {\
            result->data.__field = __op self->data.__field;\
            return FLUFF_OK;\
        }

#define DEF_CMP_OP(__type, __action, __op, __field)\
        FLUFF_CONSTEXPR FluffResult _##__type##_##__action(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {\
            result->data._bool = (lhs->data.__field __op rhs->data.__field);\
            return FLUFF_OK;\
        }

typedef FluffResult(* OpAction)(FluffObject *, FluffObject *, FluffObject *);
typedef FluffResult(* UOpAction)(FluffObject *, FluffObject *);

typedef struct OperationInfo {
    OpAction  add, sub, mul, div, mod, pow;
    OpAction  and, or, bit_and, bit_or, bit_xor, bit_shl, bit_shr;
    OpAction  eq, ne, gt, ge, lt, le;
    UOpAction bit_not, not, negate;
} OperationInfo;

DEF_CMP_OP(bool, eq,  ==, _bool)
DEF_CMP_OP(bool, ne,  !=, _bool)
DEF_OP(bool,     and, &&, _bool)
DEF_OP(bool,     or,  ||, _bool)
DEF_UOP(bool,    not, !,  _bool)

FLUFF_CONSTEXPR_V OperationInfo bool_op_info = {
    NULL, NULL, NULL, NULL, NULL, NULL, 
    _bool_and, _bool_or, NULL, NULL, NULL, NULL, NULL, 
    _bool_eq, _bool_ne, NULL, NULL, NULL, NULL, 
    NULL, _bool_not, NULL
};

DEF_OP(int,     add,     +,  _int)
DEF_OP(int,     sub,     -,  _int)
DEF_OP(int,     mul,     *,  _int)
DEF_CMP_OP(int, eq,      ==, _int)
DEF_CMP_OP(int, ne,      !=, _int)
DEF_CMP_OP(int, gt,      >,  _int)
DEF_CMP_OP(int, ge,      >=, _int)
DEF_CMP_OP(int, lt,      <,  _int)
DEF_CMP_OP(int, le,      <=, _int)
DEF_OP(int,     bit_and, &,  _int)
DEF_OP(int,     bit_or,  |,  _int)
DEF_OP(int,     bit_xor, ^,  _int)
DEF_OP(int,     bit_shl, <<, _int)
DEF_OP(int,     bit_shr, >>, _int)
DEF_UOP(int,    bit_not, ~,  _int)
DEF_UOP(int,    negate,  -,  _int)

FLUFF_CONSTEXPR FluffResult _int_div(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    if (rhs->data._int == 0) {
        fluff_push_error("division by zero");
        return FLUFF_FAILURE;
    }
    result->data._int = lhs->data._int / rhs->data._int;
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _int_mod(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    if (rhs->data._int == 0) {
        fluff_push_error("modulo by zero");
        return FLUFF_FAILURE;
    }
    result->data._int = lhs->data._int % rhs->data._int;
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _int_pow(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    lhs->data._int = pow(lhs->data._int, rhs->data._int);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR_V OperationInfo int_op_info = {
    _int_add, _int_sub, _int_mul, _int_div, _int_mod, _int_pow, 
    NULL, NULL, _int_bit_and, _int_bit_or, _int_bit_xor, _int_bit_shl, _int_bit_shr, 
    _int_eq, _int_ne, _int_gt, _int_ge, _int_lt, _int_le, 
    _int_bit_not, NULL, _int_negate
};

DEF_OP(float,     add,    +,  _float)
DEF_OP(float,     sub,    -,  _float)
DEF_OP(float,     mul,    *,  _float)
DEF_CMP_OP(float, eq,     ==, _float)
DEF_CMP_OP(float, ne,     !=, _float)
DEF_CMP_OP(float, gt,     >,  _float)
DEF_CMP_OP(float, ge,     >=, _float)
DEF_CMP_OP(float, lt,     <,  _float)
DEF_CMP_OP(float, le,     <=, _float)
DEF_UOP(float,    negate, -,  _float)

FLUFF_CONSTEXPR FluffResult _float_div(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    if (rhs->data._float == 0) {
        fluff_push_error("division by zero");
        return FLUFF_FAILURE;
    }
    result->data._float = lhs->data._float / rhs->data._float;
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _float_mod(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    if (rhs->data._float == 0) {
        fluff_push_error("modulo by zero");
        return FLUFF_FAILURE;
    }
    result->data._float = fmod(lhs->data._float, rhs->data._float);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _float_pow(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    result->data._float = powf(lhs->data._float, rhs->data._float);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR_V OperationInfo float_op_info = {
    _float_add, _float_sub, _float_mul, _float_div, _float_mod, _float_pow, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    _float_eq, _float_ne, _float_gt, _float_ge, _float_lt, _float_le, 
    NULL, NULL, _float_negate
};

FLUFF_CONSTEXPR FluffResult _string_add(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    // NOTE: _new_string will already free the string if it's not null
    _new_string_n(&result->data._string, lhs->data._string.data, lhs->data._string.length);
    fluff_string_concat(&result->data._string, &rhs->data._string);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _string_eq(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    result->data._bool = fluff_string_equal(&lhs->data._string, &rhs->data._string);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _string_ne(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    result->data._bool = !fluff_string_equal(&lhs->data._string, &rhs->data._string);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _string_lt(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    result->data._bool = (fluff_string_compare(&lhs->data._string, &rhs->data._string) < 0);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _string_le(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    result->data._bool = (fluff_string_compare(&lhs->data._string, &rhs->data._string) <= 0);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _string_gt(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    result->data._bool = (fluff_string_compare(&lhs->data._string, &rhs->data._string) > 0);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _string_ge(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {
    result->data._bool = (fluff_string_compare(&lhs->data._string, &rhs->data._string) >= 0);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR_V OperationInfo string_op_info = {
    _string_add, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    _string_eq, _string_ne, _string_gt, _string_ge, _string_lt, _string_le, 
    NULL, NULL, NULL
};

/* -===========
     Object
   ===========- */

FLUFF_API FluffObject * fluff_new_object(FluffInstance * instance, FluffKlass * klass) {
    FluffObject * self = fluff_alloc(NULL, sizeof(FluffObject));
    _new_object(self, instance, klass);
    return self;
}

FLUFF_API FluffObject * fluff_new_null_object(FluffInstance * instance, FluffKlass * klass) {
    FluffObject * self = fluff_alloc(NULL, sizeof(FluffObject));
    _new_null_object(self, instance, klass);
    return self;
}

FLUFF_API FluffObject * fluff_new_array_object(FluffInstance * instance, FluffKlass * klass) {
    FluffObject * self = fluff_alloc(NULL, sizeof(FluffObject));
    _new_array_object(self, instance, klass);
    return self;
}

FLUFF_API FluffObject * fluff_new_bool_object(FluffInstance * instance, FluffBool v) {
    FluffObject * self = fluff_alloc(NULL, sizeof(FluffObject));
    _new_bool_object(self, instance, v);
    return self;
}

FLUFF_API FluffObject * fluff_new_int_object(FluffInstance * instance, FluffInt v) {
    FluffObject * self = fluff_alloc(NULL, sizeof(FluffObject));
    _new_int_object(self, instance, v);
    return self;
}

FLUFF_API FluffObject * fluff_new_float_object(FluffInstance * instance, FluffFloat v) {
    FluffObject * self = fluff_alloc(NULL, sizeof(FluffObject));
    _new_float_object(self, instance, v);
    return self;
}

FLUFF_API FluffObject * fluff_new_string_object(FluffInstance * instance, const char * str) {
    FluffObject * self = fluff_alloc(NULL, sizeof(FluffObject));
    _new_string_object(self, instance, str);
    return self;
}

FLUFF_API FluffObject * fluff_new_string_object_n(FluffInstance * instance, const char * str, size_t len) {
    FluffObject * self = fluff_alloc(NULL, sizeof(FluffObject));
    _new_string_object_n(self, instance, str, len);
    return self;
}

FLUFF_API FluffObject * fluff_new_function_object(FluffInstance * instance, FluffMethod * method) {
    FluffObject * self = fluff_alloc(NULL, sizeof(FluffObject));
    _new_function_object(self, instance, method);
    return self;
}

FLUFF_API FluffObject * fluff_ref_object(FluffObject * self) {
    FluffObject * obj = fluff_new_null_object(self->instance, self->klass);
    _ref_object(obj, self);
    return obj;
}

FLUFF_API FluffObject * fluff_clone_object(FluffObject * self) {
    FluffObject * obj = fluff_alloc(NULL, sizeof(FluffObject));
    FLUFF_CLEANUP(obj);
    _clone_object(obj, self);
    return obj;
}

FLUFF_API void fluff_free_object(FluffObject * self) {
    _free_object(self);
    fluff_free(self);
}

FLUFF_API FluffKlass * fluff_object_get_class(FluffObject * self) {
    return self->klass;
}

#define DEF_OP_FN(__name, __op, __connective)\
        FLUFF_API FluffResult fluff_object_##__name(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {\
            if (fluff_object_is_same_class(lhs, rhs->klass)) {\
                if (lhs->klass == fluff_instance_get_core_class(lhs->instance, FLUFF_KLASS_BOOL) && bool_op_info.__name)\
                    return bool_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == fluff_instance_get_core_class(lhs->instance, FLUFF_KLASS_INT) && int_op_info.__name)\
                    return int_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == fluff_instance_get_core_class(lhs->instance, FLUFF_KLASS_FLOAT) && float_op_info.__name)\
                    return float_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == fluff_instance_get_core_class(lhs->instance, FLUFF_KLASS_STRING) && string_op_info.__name)\
                    return string_op_info.__name(lhs, rhs, result);\
            }\
            fluff_push_error(\
                "cannot " __op " an object of type '%.*s' " __connective " type '%.*s'\n",\
                FLUFF_STR_BUFFER_FMT(_class_get_common_data(lhs->klass)->name),\
                FLUFF_STR_BUFFER_FMT(_class_get_common_data(rhs->klass)->name)\
            );\
            return FLUFF_FAILURE;\
        }

#define DEF_UOP_FN(__name, __op)\
        FLUFF_API FluffResult fluff_object_##__name(FluffObject * self, FluffObject * result) {\
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_BOOL) && bool_op_info.__name)\
                return bool_op_info.__name(self, result);\
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_INT) && int_op_info.__name)\
                return int_op_info.__name(self, result);\
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_FLOAT) && float_op_info.__name)\
                return float_op_info.__name(self, result);\
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_STRING) && string_op_info.__name)\
                return string_op_info.__name(self, result);\
            fluff_push_error("cannot " __op " an object of type '%.*s'\n",\
                FLUFF_STR_BUFFER_FMT(_class_get_common_data(self->klass)->name)\
            );\
            return FLUFF_FAILURE;\
        }

#define DEF_OP_CMP_FN(__name)\
        FLUFF_API FluffResult fluff_object_##__name(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {\
            if (fluff_object_is_same_class(lhs, rhs->klass)) {\
                if (lhs->klass == fluff_instance_get_core_class(lhs->instance, FLUFF_KLASS_BOOL) && bool_op_info.__name)\
                    return bool_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == fluff_instance_get_core_class(lhs->instance, FLUFF_KLASS_INT) && int_op_info.__name)\
                    return int_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == fluff_instance_get_core_class(lhs->instance, FLUFF_KLASS_FLOAT) && float_op_info.__name)\
                    return float_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == fluff_instance_get_core_class(lhs->instance, FLUFF_KLASS_STRING) && string_op_info.__name)\
                    return string_op_info.__name(lhs, rhs, result);\
            }\
            fluff_push_error(\
                "cannot compare an object of type '%.*s' with type '%.*s'\n",\
                FLUFF_STR_BUFFER_FMT(_class_get_common_data(lhs->klass)->name),\
                FLUFF_STR_BUFFER_FMT(_class_get_common_data(rhs->klass)->name)\
            );\
            return FLUFF_FAILURE;\
        }

DEF_OP_FN(add, "add", "with")
DEF_OP_FN(sub, "subtract", "with")
DEF_OP_FN(mul, "multiply", "by")
DEF_OP_FN(div, "divide", "by")
DEF_OP_FN(mod, "compute division remainder of", "by")
DEF_OP_FN(pow, "power", "to")
DEF_OP_CMP_FN(eq)
DEF_OP_CMP_FN(ne)
DEF_OP_CMP_FN(gt)
DEF_OP_CMP_FN(ge)
DEF_OP_CMP_FN(lt)
DEF_OP_CMP_FN(le)
DEF_OP_FN(and, "compare", "with")
DEF_OP_FN(or, "compare", "with")
DEF_OP_FN(bit_and, "bitwise AND", "with")
DEF_OP_FN(bit_or, "bitwise OR", "with")
DEF_OP_FN(bit_xor, "bitwise XOR", "with")
DEF_OP_FN(bit_shl, "bitwise left shift", "by")
DEF_OP_FN(bit_shr, "bitwise right shift", "by")
DEF_UOP_FN(bit_not, "negate")
DEF_UOP_FN(not, "negate")
DEF_UOP_FN(negate, "negate")

FLUFF_API FluffResult fluff_object_promote(FluffObject * self, FluffObject * result) {
    // NOTE: this does virtually nothing, so why even bother?
    return FLUFF_OK;
}

FLUFF_API void * fluff_object_unbox(FluffObject * self) {
    if (!self || !self->instance || !self->klass) {
        fluff_push_error("cannot unbox an incomplete object");
        return NULL;
    }
    if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_BOOL))
        return &self->data._bool;
    if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_INT))
        return &self->data._int;
    if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_FLOAT))
        return &self->data._float;
    if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_STRING))
        return &self->data._string;
    // if (self->klass == self->instance->array_klass)
    //     return &self->data._array;
    if (self->data._data)
        return _object_table_get_subobjects(_object_get_table(self->data._data));
    fluff_push_error("failed to unbox object");
    return NULL;
}

FLUFF_API bool fluff_object_is_same(FluffObject * lhs, FluffObject * rhs) {
    // TODO: account for _data being possibly shifted
    return (lhs && rhs && (lhs == rhs || lhs->data._data == rhs->data._data));
}

FLUFF_API bool fluff_object_is_same_class(FluffObject * self, FluffKlass * klass) {
    return (self->klass == klass || (
        self->klass->flags == FLUFF_KLASS_PRIMITIVE && self->klass->index == klass->index
    ));
}

FLUFF_API FluffObject * fluff_object_as(FluffObject * self, FluffKlass * klass) {
    if (fluff_object_is_same_class(self, klass)) return FLUFF_OK;
    if (!self->klass || !klass) {
        fluff_push_error("cannot convert an object to/from an incomplete type");
        return NULL;
    }

    if (FLUFF_HAS_FLAG(klass->flags, FLUFF_KLASS_PRIMITIVE)) {
        if (klass == fluff_instance_get_core_class(klass->instance, FLUFF_KLASS_BOOL)) {
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_INT))
                return _bool2int(self);
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_FLOAT))
                return _bool2float(self);
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_STRING))
                return _bool2string(self);
        }
        if (klass == fluff_instance_get_core_class(klass->instance, FLUFF_KLASS_INT)) {
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_BOOL))
                return _int2bool(self);
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_FLOAT))
                return _int2float(self);
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_STRING))
                return _int2string(self);
        }
        if (klass == fluff_instance_get_core_class(klass->instance, FLUFF_KLASS_FLOAT)) {
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_BOOL))
                return _float2bool(self);
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_INT))
                return _float2int(self);
            if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_STRING))
                return _float2string(self);
        }
    } else {
        FluffObject * obj = _object_cast(self, klass);
        if (obj) return obj;
    }

    fluff_push_error(
        "cannot convert an object of type '%.*s' to type '%.*s'", 
        FLUFF_STR_BUFFER_FMT(_class_get_common_data(self->klass)->name), 
        FLUFF_STR_BUFFER_FMT(_class_get_common_data(klass)->name)
    );
    return NULL;
}

FLUFF_API FluffObject * fluff_object_get_member(FluffObject * self, const char * name) {
    if (!self || !self->klass) {
        fluff_push_error("cannot get a member on an incomplete object");
        return NULL;
    }
    if (!self->data._data) {
        fluff_push_error("cannot get a member on a null object");
        return NULL;
    }

    FluffObject * obj = self;
    while (obj && obj->klass) {
        const size_t inherits = (_class_get_common_data(obj->klass)->inherits ? 1 : 0);
        //printf("walking thru type %s at %p (inherits = %zu)\n", obj->klass->common.name.data, obj, inherits);

        size_t idx = _common_class_get_property_index(_class_get_common_data(obj->klass), name);
        if (idx != SIZE_MAX) return _object_table_get_subobjects(_object_get_table(obj)) + idx + inherits;
        if (inherits == 0)   return NULL;

        obj = _object_table_get_subobjects(_object_get_table(obj));
    }
    return NULL;
}

FLUFF_API FluffObject * fluff_object_get_item(FluffObject * self, const char * name) {
    // TODO: arrays
    // TODO: custom subscripting
    return NULL;
}

/* -=- Private -=- */
FLUFF_PRIVATE_API void _new_object(FluffObject * self, FluffInstance * instance, FluffKlass * klass) {
    FLUFF_CLEANUP(self);
    self->instance = instance;
    self->klass    = klass;
    if (klass && !FLUFF_HAS_FLAG(klass->flags, FLUFF_KLASS_PRIMITIVE)) {
        // TODO: native classes
        _object_alloc(self, NULL);
    }
}

FLUFF_PRIVATE_API void _new_null_object(FluffObject * self, FluffInstance * instance, FluffKlass * klass) {
    FLUFF_CLEANUP(self);
    self->instance = instance;
    self->klass    = klass;
}

FLUFF_PRIVATE_API void _new_array_object(FluffObject * self, FluffInstance * instance, FluffKlass * klass) {
    FLUFF_CLEANUP(self);
    self->instance = instance;
    self->klass    = fluff_instance_get_core_class(instance, FLUFF_KLASS_ARRAY);
    // TODO: this
}

FLUFF_PRIVATE_API void _new_bool_object(FluffObject * self, FluffInstance * instance, FluffBool v) {
    FLUFF_CLEANUP(self);
    self->instance   = instance;
    self->klass      = fluff_instance_get_core_class(instance, FLUFF_KLASS_BOOL);
    self->data._bool = v;
}

FLUFF_PRIVATE_API void _new_int_object(FluffObject * self, FluffInstance * instance, FluffInt v) {
    FLUFF_CLEANUP(self);
    self->instance  = instance;
    self->klass     = fluff_instance_get_core_class(instance, FLUFF_KLASS_INT);
    self->data._int = v;
}

FLUFF_PRIVATE_API void _new_float_object(FluffObject * self, FluffInstance * instance, FluffFloat v) {
    FLUFF_CLEANUP(self);
    self->instance    = instance;
    self->klass       = fluff_instance_get_core_class(instance, FLUFF_KLASS_FLOAT);
    self->data._float = v;
}

FLUFF_PRIVATE_API void _new_string_object(FluffObject * self, FluffInstance * instance, const char * str) {
    FLUFF_CLEANUP(self);
    self->instance  = instance;
    self->klass     = fluff_instance_get_core_class(instance, FLUFF_KLASS_STRING);
    _new_string(&self->data._string, str);
}

FLUFF_PRIVATE_API void _new_string_object_n(FluffObject * self, FluffInstance * instance, const char * str, size_t len) {
    FLUFF_CLEANUP(self);
    self->instance  = instance;
    self->klass     = fluff_instance_get_core_class(instance, FLUFF_KLASS_STRING);
    _new_string_n(&self->data._string, str, len);
}

FLUFF_PRIVATE_API void _new_function_object(FluffObject * self, FluffInstance * instance, FluffMethod * method) {
    FLUFF_CLEANUP(self);
    self->instance     = instance;
    self->klass        = fluff_instance_get_core_class(instance, FLUFF_KLASS_FUNC);
    self->data._method = method;
}

FLUFF_PRIVATE_API void _clone_object(FluffObject * self, FluffObject * obj) {
    self->klass    = obj->klass;
    self->instance = obj->instance;
    if (self->klass) {
        if (FLUFF_HAS_FLAG(self->klass->flags, FLUFF_KLASS_PRIMITIVE)) {
            self->data = obj->data;
        } else {
            _object_alloc(self, obj);
        }
    }
}

FLUFF_PRIVATE_API void _ref_object(FluffObject * self, FluffObject * obj) {
    self->instance = obj->instance;
    self->klass    = obj->klass;
    if (obj->klass) {
        if (FLUFF_HAS_FLAG(obj->klass->flags, FLUFF_KLASS_PRIMITIVE)) {
            self->data = obj->data;
        } else {
            ++_object_get_table(obj)->ref_count;
            self->data._data = obj->data._data;
        }
    }
}

FLUFF_PRIVATE_API void _free_object(FluffObject * self) {
    if (self->klass) {
        if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_STRING)) {
            _free_string(&self->data._string);
        } else if (self->klass == fluff_instance_get_core_class(self->instance, FLUFF_KLASS_FUNC)) {
            _free_method(self->data._method);
        } else if (!FLUFF_HAS_FLAG(self->klass->flags, FLUFF_KLASS_PRIMITIVE) && self->data._data) {
            _object_deref(self);
        }
    }
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API void _object_alloc(FluffObject * self, FluffObject * clone_obj) {
    const size_t inherits = (_class_get_common_data(self->klass)->inherits ? 1 : 0);

    ObjectTable * table = _object_table_alloc(self->klass);
    table->ref_count = 1;

    FluffObject * subobjs = _object_table_get_subobjects(table);

    FluffObject * clone_subobjs = NULL;
    if (clone_obj) clone_subobjs = _object_table_get_subobjects(_object_get_table(clone_obj));

    if (inherits) {
        if (clone_obj) {
            _clone_object(subobjs, clone_subobjs);
            ++clone_subobjs;
        } else {
            _new_object(subobjs, self->instance, _class_get_common_data(self->klass)->inherits);
        }
        _object_get_table(subobjs)->vptr = self;
        // printf("(inherits %s, vptr = %p)\n", (self->klass ? self->klass->common.name.data : NULL), table);
        ++subobjs;
    }

    for (size_t i = 0; i < _class_get_common_data(self->klass)->property_count; ++i) {
        KlassProperty * property = &_class_get_common_data(self->klass)->properties[i];
        if (clone_obj) {
            _clone_object(subobjs, clone_subobjs);
            ++clone_subobjs;
        } else if (property->def_value) {
            _clone_object(subobjs, property->def_value);
        } else {
            _new_object(subobjs, self->instance, property->klass);
        }
        // printf("allocated member '%s' (%s) at %p\n", 
        //     property->name.data, property->type->name.data, subobjs
        // );
        ++subobjs;
    }

    self->data._data = table;

    // printf("allocated object of type '%s' on ptr %p\n", self->klass->common.name.data, table);
}

FLUFF_PRIVATE_API ObjectTable * _object_get_table(FluffObject * self) {
    return (ObjectTable *)self->data._data;
}

FLUFF_PRIVATE_API ObjectTable * _object_table_alloc(FluffKlass * klass) {
    const size_t size = _common_class_get_alloc_size(_class_get_common_data(klass));
    uint8_t * data = fluff_alloc(NULL, size);
    FLUFF_CLEANUP_N(data, size);
    return (ObjectTable *)data;
}

FLUFF_PRIVATE_API FluffObject * _object_table_get_subobjects(ObjectTable * self) {
    if (!self) return NULL;
    return (FluffObject *)(self + 1);
}

FLUFF_PRIVATE_API FluffObject * _object_cast(FluffObject * self, FluffKlass * klass) {
    /* NOTE: casting direction, given C->B->A

        | A | -> 0
        | B | -> 1
        | C | -> 2

    */
    if (_class_get_common_data(self->klass)->inherit_depth < _class_get_common_data(klass)->inherit_depth)
        return _object_upcast(self, klass);
    if (_class_get_common_data(self->klass)->inherit_depth > _class_get_common_data(klass)->inherit_depth)
        return _object_downcast(self, klass);
    return self;
}

FLUFF_PRIVATE_API FluffObject * _object_downcast(FluffObject * self, FluffKlass * klass) {
    FluffObject * obj = self;
    while (obj && obj->klass) {
        if (obj->klass == klass) {
            return fluff_ref_object(obj);
        }
        if (!_class_get_common_data(obj->klass)->inherits) return NULL;

        obj = _object_table_get_subobjects(_object_get_table(obj));
    }
    return NULL;
}

FLUFF_PRIVATE_API FluffObject * _object_upcast(FluffObject * self, FluffKlass * klass) {
    // TODO: make the class being virtual a requirement for upcasting and downcasting
    FluffObject * obj = self;
    while (obj) {
        if (fluff_object_is_same_class(obj, klass))
            return obj;
        obj = _object_get_table(obj)->vptr;
    }
    return NULL;
}

FLUFF_PRIVATE_API void _object_deref(FluffObject * self) {
    ObjectTable * table = _object_get_table(self);
    if (--table->ref_count > 0) return;

    // printf("object at %p (%s) has to be freed\n", self, self->klass->common.name.data);

    FluffObject * objs = _object_table_get_subobjects(table);

    const size_t inherits  = (self->klass->common.inherits ? 1 : 0);
    const size_t obj_count = self->klass->common.property_count + inherits;
    
    if (inherits) {
        // printf("vtable to %p found\n", _object_get_table(objs)->vptr);
        _object_get_table(objs)->vptr = NULL;
    }

    for (size_t i = 0; i < obj_count; ++i) {
        _free_object(&objs[i]);
    }
    fluff_free(self->data._data);
    self->data._data = NULL;
}