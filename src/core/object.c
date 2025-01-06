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

#include <math.h>

/* -==============
     Internals
   ==============- */

FLUFF_CONSTEXPR FluffResult _bool2int(FluffObject * self, FluffObject * result) {
    result->data._int = (FluffInt)(self->data._bool);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _bool2float(FluffObject * self, FluffObject * result) {
    result->data._float = (FluffFloat)(self->data._bool);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _bool2string(FluffObject * self, FluffObject * result) {
    if (self->data._bool) _new_string_n(&result->data._string, "true", 4);
    _new_string_n(&result->data._string, "false", 5);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _int2bool(FluffObject * self, FluffObject * result) {
    result->data._bool = (FluffBool)(self->data._int);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _int2float(FluffObject * self, FluffObject * result) {
    result->data._float = (FluffFloat)(self->data._int);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _int2string(FluffObject * self, FluffObject * result) {
    char buf[16] = { 0 };
    fluff_format(buf, 16, "%ld", self->data._int);
    _new_string_n(&result->data._string, buf, 16);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _float2bool(FluffObject * self, FluffObject * result) {
    result->data._bool = (FluffBool)(self->data._float);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _float2int(FluffObject * self, FluffObject * result) {
    result->data._int = (FluffInt)(self->data._float);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _float2string(FluffObject * self, FluffObject * result) {
    char buf[16] = { 0 };
    fluff_format(buf, 16, "%f", self->data._float);
    _new_string_n(&result->data._string, buf, 16);
    return FLUFF_OK;
}

FLUFF_CONSTEXPR FluffResult _string2bool(FluffObject * self, FluffObject * result) {
    self->data._bool = (self->data._string.length != 0);
    return FLUFF_OK;
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

/* -=============
     Instance
   =============- */

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
    FluffObject * self = fluff_new_object(instance, instance->array_klass);
    _new_array_object(self, instance, klass);
    return self;
}

FLUFF_API FluffObject * fluff_new_bool_object(FluffInstance * instance, FluffBool v) {
    FluffObject * self = fluff_new_object(instance, instance->bool_klass);
    _new_bool_object(self, instance, v);
    return self;
}

FLUFF_API FluffObject * fluff_new_int_object(FluffInstance * instance, FluffInt v) {
    FluffObject * self = fluff_new_object(instance, instance->int_klass);
    _new_int_object(self, instance, v);
    return self;
}

FLUFF_API FluffObject * fluff_new_float_object(FluffInstance * instance, FluffFloat v) {
    FluffObject * self = fluff_new_object(instance, instance->float_klass);
    _new_float_object(self, instance, v);
    return self;
}

FLUFF_API FluffObject * fluff_new_string_object(FluffInstance * instance, const char * str) {
    return fluff_new_string_object_n(instance, str, strlen(str));
}

FLUFF_API FluffObject * fluff_new_string_object_n(FluffInstance * instance, const char * str, size_t len) {
    FluffObject * self = fluff_new_object(instance, instance->string_klass);
    _new_string_object_n(self, instance, str, len);
    return self;
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
                if (lhs->klass == lhs->instance->bool_klass && bool_op_info.__name)\
                    return bool_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == lhs->instance->int_klass && int_op_info.__name)\
                    return int_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == lhs->instance->float_klass && float_op_info.__name)\
                    return float_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == lhs->instance->string_klass && string_op_info.__name)\
                    return string_op_info.__name(lhs, rhs, result);\
            }\
            fluff_push_error(\
                "cannot " __op " an object of type '%s' " __connective " type '%s'\n",\
                lhs->klass->name.data, rhs->klass->name.data\
            );\
            return FLUFF_FAILURE;\
        }

#define DEF_UOP_FN(__name, __op)\
        FLUFF_API FluffResult fluff_object_##__name(FluffObject * self, FluffObject * result) {\
            if (self->klass == self->instance->bool_klass && bool_op_info.__name)\
                return bool_op_info.__name(self, result);\
            if (self->klass == self->instance->int_klass && int_op_info.__name)\
                return int_op_info.__name(self, result);\
            if (self->klass == self->instance->float_klass && float_op_info.__name)\
                return float_op_info.__name(self, result);\
            if (self->klass == self->instance->string_klass && string_op_info.__name)\
                return string_op_info.__name(self, result);\
            fluff_push_error(\
                "cannot " __op " an object of type '%s'\n", self->klass->name.data\
            );\
            return FLUFF_FAILURE;\
        }

#define DEF_OP_CMP_FN(__name)\
        FLUFF_API FluffResult fluff_object_##__name(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {\
            if (fluff_object_is_same_class(lhs, rhs->klass)) {\
                if (lhs->klass == lhs->instance->bool_klass && bool_op_info.__name)\
                    return bool_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == lhs->instance->int_klass && int_op_info.__name)\
                    return int_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == lhs->instance->float_klass && float_op_info.__name)\
                    return float_op_info.__name(lhs, rhs, result);\
                if (lhs->klass == lhs->instance->string_klass && string_op_info.__name)\
                    return string_op_info.__name(lhs, rhs, result);\
            }\
            fluff_push_error(\
                "cannot compare an object of type '%s' with type '%s'\n",\
                lhs->klass->name.data, rhs->klass->name.data\
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
    // TODO: account for _data being possibly shifted
    return (lhs && rhs && (lhs == rhs || lhs->data._data == rhs->data._data));
}

FLUFF_API bool fluff_object_is_same_class(FluffObject * self, FluffKlass * klass) {
    return (self->klass == klass || (
        self->klass->flags == FLUFF_KLASS_PRIMITIVE && self->klass->index == klass->index
    ));
}

FLUFF_API FluffResult fluff_object_as(FluffObject * self, FluffKlass * klass, FluffObject * result) {
    if (fluff_object_is_same_class(self, klass)) return FLUFF_OK;
    if (!self->klass || klass) {
        fluff_push_error("cannot convert an object to/from an incomplete type");
        return FLUFF_FAILURE;
    }
    if (klass == klass->instance->bool_klass) {
        if (self->klass == self->instance->int_klass)
            return _bool2int(self, result);
        if (self->klass == self->instance->float_klass)
            return _bool2float(self, result);
        if (self->klass == self->instance->string_klass)
            return _bool2string(self, result);
    }
    if (klass == klass->instance->int_klass) {
        if (self->klass == self->instance->bool_klass)
            return _int2bool(self, result);
        if (self->klass == self->instance->float_klass)
            return _int2float(self, result);
        if (self->klass == self->instance->string_klass)
            return _int2string(self, result);
    }
    if (klass == klass->instance->float_klass) {
        if (self->klass == self->instance->bool_klass)
            return _float2bool(self, result);
        if (self->klass == self->instance->int_klass)
            return _float2int(self, result);
        if (self->klass == self->instance->string_klass)
            return _float2string(self, result);
    }
    fluff_push_error(
        "cannot convert an object of type '%s' to type '%s'", 
        self->klass->name.data, klass->name.data
    );
    return FLUFF_FAILURE;
}

FLUFF_API FluffObject * fluff_object_get_member(FluffObject * self, const char * name) {
    if (!self->data._data) {
        fluff_push_error("cannot get a member on a null object");
        return NULL;
    }

    FluffObject * data = (FluffObject *)self->data._data;

    FluffKlass * klass = self->klass;
    while (klass) {
        size_t idx = _class_get_property_index(klass, name);
        if (idx != SIZE_MAX) {
            return &data[idx + (klass->inherits ? 1 : 0)];
        }
        klass = klass->inherits;
        if (klass) data = data->data._data;
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
        self->data._data = _object_alloc_common_class(self, instance);
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
    self->klass    = instance->array_klass;
    // TODO: this
}

FLUFF_PRIVATE_API void _new_bool_object(FluffObject * self, FluffInstance * instance, FluffBool v) {
    FLUFF_CLEANUP(self);
    self->instance   = instance;
    self->klass      = instance->bool_klass;
    self->data._bool = v;
}

FLUFF_PRIVATE_API void _new_int_object(FluffObject * self, FluffInstance * instance, FluffInt v) {
    FLUFF_CLEANUP(self);
    self->instance  = instance;
    self->klass     = instance->int_klass;
    self->data._int = v;
}

FLUFF_PRIVATE_API void _new_float_object(FluffObject * self, FluffInstance * instance, FluffFloat v) {
    FLUFF_CLEANUP(self);
    self->instance    = instance;
    self->klass       = instance->float_klass;
    self->data._float = v;
}

FLUFF_PRIVATE_API void _new_string_object(FluffObject * self, FluffInstance * instance, const char * str) {
    _new_string_object_n(self, instance, str, strlen(str));
}

FLUFF_PRIVATE_API void _new_string_object_n(FluffObject * self, FluffInstance * instance, const char * str, size_t len) {
    FLUFF_CLEANUP(self);
    self->instance  = instance;
    self->klass     = instance->bool_klass;
    _new_string_n(&self->data._string, str, len);
}

FLUFF_PRIVATE_API void _clone_object(FluffObject * self, FluffObject * obj) {
    // TODO: cloning native and common objects
    self->klass    = obj->klass;
    self->instance = obj->instance;
    if (self->klass && FLUFF_HAS_FLAG(self->klass->flags, FLUFF_KLASS_PRIMITIVE)) {
        self->data = obj->data;
    }
}

FLUFF_PRIVATE_API void _free_object(FluffObject * self) {
    if (self->klass) {
        if (self->klass == self->instance->string_klass) {
            _free_string(&self->data._string);
        } else if (!FLUFF_HAS_FLAG(self->klass->flags, FLUFF_KLASS_PRIMITIVE) && self->data._data) {
            _object_deref(self);
        }
    }
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API void * _object_alloc_common_class(FluffObject * self, FluffInstance * instance) {
    // TODO: vtables for virtual classes
    // TODO: optimize objects to be linear instead of recursive (make the table contiguous)

    /* NOTE: current object layout

    Given D -> C -> B -> A

    | D |
    | C | -> | B |
             | A | -> | (none) |
    
    */

    const size_t inherit_count = (self->klass->inherits ? 1 : 0);
    size_t       obj_count     = self->klass->property_count + inherit_count;
    FluffObject  objects[obj_count];

    // Allocate the inherited objects
    if (self->klass->inherits)
        _new_object(objects, instance, self->klass->inherits);

    // Allocate the properties
    for (size_t i = 0; i < self->klass->property_count; ++i) {
        FluffObject * obj = &objects[i + inherit_count];
        if (self->klass->properties[i].def_value)
            _clone_object(obj, self->klass->properties[i].def_value);
        else
            _new_object(obj, instance, self->klass->properties[i].type);
    }

    void * data = fluff_alloc(NULL, sizeof(objects));
    FLUFF_CLEANUP_N(data, obj_count);
    memcpy(data, objects, sizeof(objects));
    return data;
}

FLUFF_PRIVATE_API void * _object_alloc_native_class(FluffObject * self, FluffInstance * instance) {
    // TODO: native classes (once callbacks for classes are properly planned)
    return NULL;
}

FLUFF_PRIVATE_API void _object_ref(FluffObject * self) {
    // TODO: reference counting
}

FLUFF_PRIVATE_API void _object_deref(FluffObject * self) {
    // TODO: reference counting
    size_t obj_count = self->klass->property_count + (self->klass->inherits ? 1 : 0);
    for (size_t i = 0; i < obj_count; ++i) {
        FluffObject * obj = &((FluffObject *)self->data._data)[i];
        if (obj->klass && !FLUFF_HAS_FLAG(obj->klass->flags, FLUFF_KLASS_PRIMITIVE))
            _object_deref(obj);
    }
    fluff_free(self->data._data);
}