#pragma once
#ifndef FLUFF_CORE_MODULE_H
#define FLUFF_CORE_MODULE_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/string.h>

/* -===========
     Module
   ===========- */

typedef struct FluffKlass FluffKlass;

typedef struct FluffInstance FluffInstance;
typedef struct FluffModule FluffModule;

// This struct represents a module.
typedef struct FluffModule {
    FluffInstance * instance;

    char name[FLUFF_MAX_MODULE_NAME_LEN + 1];

    FluffKlass * klasses;

    FluffModule * next_module;

    size_t index;
} FluffModule;

FLUFF_API FluffModule * fluff_new_module(const char * name);
FLUFF_API void          fluff_free_module(FluffModule * self);

FLUFF_API const char * fluff_module_get_name(const FluffModule * self);
FLUFF_API FluffKlass * fluff_module_get_class_by_name(FluffModule * self, const char * name);
FLUFF_API FluffKlass * fluff_module_get_class_by_id(FluffModule * self, size_t index);

FLUFF_PRIVATE_API void _new_module(FluffModule * self, const char * name);
FLUFF_PRIVATE_API void _free_module(FluffModule * self);

FLUFF_PRIVATE_API size_t _module_add_class(FluffModule * self, FluffKlass * klass);

#endif