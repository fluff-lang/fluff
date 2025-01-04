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

// FLUFF_CONSTEXPR_V OperationInfo string_op_info = {
//     NULL, NULL, NULL, NULL, NULL, NULL, _bool_and, _bool_or, NULL, NULL, NULL, 
//     _bool_eq, _bool_ne, _bool_gt, _bool_ge, _bool_lt, _bool_le, 
//     NULL, _bool_not, NULL
// };

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

#define DEF_OP_FN(__name, __op, __connective)\
        FLUFF_API FluffResult fluff_object_##__name(FluffObject * lhs, FluffObject * rhs, FluffObject * result) {\
            if (!fluff_object_is_same_class(lhs, rhs->klass)) {\
                fluff_push_error(\
                    "cannot " __op " an object of type %s " __connective " type %s\n",\
                    lhs->klass->name.data, rhs->klass->name.data\
                );\
                return FLUFF_FAILURE;\
            }\
            if (lhs->klass == lhs->instance->bool_klass && bool_op_info.__name)\
                return bool_op_info.__name(lhs, rhs, result);\
            if (lhs->klass == lhs->instance->int_klass && int_op_info.__name)\
                return int_op_info.__name(lhs, rhs, result);\
            if (lhs->klass == lhs->instance->float_klass && float_op_info.__name)\
                return float_op_info.__name(lhs, rhs, result);\
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
            fluff_push_error(\
                "cannot " __op " an object of type %s\n", self->klass->name.data\
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
            }\
            fluff_push_error(\
                "cannot compare an object of type %s with type %s\n",\
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