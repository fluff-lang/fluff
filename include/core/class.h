#pragma once
#ifndef FLUFF_CORE_CLASS_H
#define FLUFF_CORE_CLASS_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/string.h>

/* -===========
     Macros
   ===========- */

/* -=- Class attributes -=- */
#define FLUFF_KLASS_PRIMITIVE       0x01
#define FLUFF_KLASS_PUBLIC          0x02
#define FLUFF_KLASS_NATIVE          0x04
#define FLUFF_KLASS_GENERIC_BASE    0x08
#define FLUFF_KLASS_GENERIC_DERIVED 0x10

/* -=- Primitives -=- */
#define FLUFF_KLASS_VOID   0x0
#define FLUFF_KLASS_BOOL   0x1
#define FLUFF_KLASS_INT    0x2
#define FLUFF_KLASS_FLOAT  0x3
#define FLUFF_KLASS_STRING 0x4
#define FLUFF_KLASS_OBJECT 0x5
#define FLUFF_KLASS_ARRAY  0x6
#define FLUFF_KLASS_FUNC   0x7

/* -================
     CommonKlass
   ================- */

typedef struct FluffInstance FluffInstance;
typedef struct FluffModule FluffModule;
typedef struct FluffKlass FluffKlass;
typedef struct FluffObject FluffObject;
typedef struct FluffMethod FluffMethod;

/*! Represents a fluff property */
typedef struct KlassProperty {
    char          name[FLUFF_MAX_FIELD_NAME_LEN + 1];
    FluffKlass  * klass;
    FluffObject * def_value;
    size_t        index;
} KlassProperty;

/*! Represents a common class */
typedef struct CommonKlass {
    FluffKlass * inherits;
    size_t       inherit_depth;

    char name[FLUFF_MAX_FIELD_NAME_LEN + 1];

    FluffKlass * next_klass;

    KlassProperty * properties;
    size_t          property_count;
} CommonKlass;

FLUFF_PRIVATE_API void _free_common_class(CommonKlass * self);

FLUFF_PRIVATE_API size_t _common_class_add_property(CommonKlass * self, const char * name, FluffKlass * klass, FluffObject * def_value);
FLUFF_PRIVATE_API size_t _common_class_get_property_index(CommonKlass * self, const char * name);

FLUFF_PRIVATE_API size_t _common_class_add_method(CommonKlass * self, FluffMethod * method);
FLUFF_PRIVATE_API size_t _common_class_get_method_index(CommonKlass * self, const char * name);

FLUFF_PRIVATE_API size_t _common_class_get_alloc_size(CommonKlass * self);

/* -=================
     GenericKlass
   =================- */

/*! Represents a generic class */
typedef struct GenericKlass {
    FluffKlass * base;

    FluffKlass ** generics;
    size_t        generic_count;
} GenericKlass;

FLUFF_PRIVATE_API void _free_generic_class(GenericKlass * self);

/* -==========
     Klass
   ==========- */

/*! Represents a fluff class */
typedef struct FluffKlass {
    FluffInstance * instance;
    FluffModule   * module;

    FluffKlass * next_klass;

    union {
        GenericKlass generic;
        CommonKlass  common;
    };

    uint8_t flags;
    size_t  index;
} FluffKlass;

FLUFF_PRIVATE_API FluffKlass * _new_common_class(const char * name, size_t len, FluffKlass * inherits);
FLUFF_PRIVATE_API FluffKlass * _new_generic_class(FluffKlass * base, FluffKlass ** generics, size_t count);
FLUFF_PRIVATE_API void         _free_class(FluffKlass * self);

FLUFF_PRIVATE_API CommonKlass * _class_get_common_data(FluffKlass * self);

FLUFF_PRIVATE_API void _class_dump(FluffKlass * self);

#endif