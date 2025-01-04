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

#define FLUFF_KLASS_PUBLIC 0x1

/* -==========
     Klass
   ==========- */

typedef struct FluffInstance FluffInstance;
typedef struct FluffModule FluffModule;
typedef struct FluffKlass FluffKlass;

// This struct represents a class.
typedef struct FluffKlass {
    FluffInstance * instance;
    FluffModule   * module;
    FluffKlass    * inherits;

    FluffString name;

    FluffKlass * next_klass;

    uint8_t flags;

    size_t index;
} FluffKlass;

FLUFF_PRIVATE_API FluffKlass * _new_class(const char * name, size_t len, FluffKlass * inherits);
FLUFF_PRIVATE_API void         _free_class(FluffKlass * self);

#endif