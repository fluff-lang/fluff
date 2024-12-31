#pragma once
#ifndef FLUFF_CORE_CLASS_H
#define FLUFF_CORE_CLASS_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/string.h>

/* -==========
     Klass
   ==========- */

typedef struct FluffModule FluffModule;
typedef struct FluffKlass FluffKlass;

// This struct represents a class.
typedef struct FluffKlass {
    FluffModule * module;

    FluffString name;

    FluffKlass * next_klass;

    size_t index;
} FluffKlass;

FLUFF_PRIVATE_API FluffKlass * _new_class(const char * name, size_t len);
FLUFF_PRIVATE_API void         _free_class(FluffKlass * self);

#endif