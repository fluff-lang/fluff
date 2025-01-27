#pragma once
#ifndef FLUFF_CORE_METHOD_H
#define FLUFF_CORE_METHOD_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/string.h>

/* -===========
     Macros
   ===========- */

#define FLUFF_METHOD_STATIC  0x1
#define FLUFF_METHOD_VIRTUAL 0x2
#define FLUFF_METHOD_PUBLIC  0x4
#define FLUFF_METHOD_VARARGS 0x8

/* -===========
     Method
   ===========- */

typedef struct FluffInstance FluffInstance;
typedef struct FluffModule FluffModule;
typedef struct FluffKlass FluffKlass;
typedef struct FluffObject FluffObject;
typedef struct FluffVM FluffVM;

typedef FluffResult(* FluffMethodCallback)(FluffVM *, size_t);

typedef struct MethodProperty {
    char          name[FLUFF_MAX_FIELD_NAME_LEN];
    FluffKlass  * type; // TODO: change this name to "klass"
    size_t        index;
} MethodProperty;

typedef struct FluffMethod {
    size_t ref_count;

    FluffModule * module;
    FluffKlass  * klass;

    char name[FLUFF_MAX_FIELD_NAME_LEN + 1];

    FluffKlass     * ret_type; // TODO: change this name to "klass"
    MethodProperty * properties;
    size_t           property_count;

    FluffMethodCallback callback;

    uint8_t flags;
    size_t  index;
} FluffMethod;

FLUFF_PRIVATE_API FluffMethod * _new_method(const char * name, size_t len);
FLUFF_PRIVATE_API void          _free_method(FluffMethod * self);

FLUFF_PRIVATE_API size_t _method_add_property(FluffMethod * self, const char * name, FluffKlass * type);

/* -===============
     MethodPool
   ===============- */

#endif