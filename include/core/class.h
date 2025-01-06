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

#define FLUFF_KLASS_PRIMITIVE 0x1
#define FLUFF_KLASS_VIRTUAL   0x2
#define FLUFF_KLASS_PUBLIC    0x4

#define FLUFF_KLASS_VOID   0x0
#define FLUFF_KLASS_BOOL   0x1
#define FLUFF_KLASS_INT    0x2
#define FLUFF_KLASS_FLOAT  0x3
#define FLUFF_KLASS_STRING 0x4
#define FLUFF_KLASS_OBJECT 0x5
#define FLUFF_KLASS_ARRAY  0x6
#define FLUFF_KLASS_FUNC   0x7

/* -==========
     Klass
   ==========- */

typedef struct FluffInstance FluffInstance;
typedef struct FluffModule FluffModule;
typedef struct FluffKlass FluffKlass;
typedef struct FluffObject FluffObject;

typedef struct KlassProperty {
    FluffString   name;
    FluffKlass  * type; // TODO: change this name to "klass"
    FluffObject * def_value;
    size_t        index;
} KlassProperty;

// This struct represents a class.
typedef struct FluffKlass {
    FluffInstance * instance;
    FluffModule   * module;
    FluffKlass    * inherits;
    size_t          inherit_depth;

    FluffString name;

    FluffKlass * next_klass;

    KlassProperty * properties;
    size_t          property_count;

    uint8_t flags;
    size_t  index;
} FluffKlass;

FLUFF_PRIVATE_API FluffKlass * _new_class(const char * name, size_t len, FluffKlass * inherits);
FLUFF_PRIVATE_API void         _free_class(FluffKlass * self);

FLUFF_PRIVATE_API size_t _class_add_property(FluffKlass * self, const char * name, FluffKlass * type, FluffObject * def_value);
FLUFF_PRIVATE_API size_t _class_get_property_index(FluffKlass * self, const char * name);
FLUFF_PRIVATE_API size_t _class_get_alloc_size(FluffKlass * self);

#endif