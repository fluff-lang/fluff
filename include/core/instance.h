#pragma once
#ifndef FLUFF_CORE_INSTANCE_H
#define FLUFF_CORE_INSTANCE_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/method.h>
#include <core/module.h>

/* -=============
     Instance
   =============- */

typedef struct FluffKlass FluffKlass;
typedef struct FluffMethod FluffMethod;

// This struct represents an instance.
typedef struct FluffInstance {
    FluffModule * modules;
    FluffString   modules_path;

    FluffModule core_module;
} FluffInstance;

FLUFF_API FluffInstance * fluff_new_instance();
FLUFF_API void            fluff_free_instance(FluffInstance * self);

FLUFF_API FluffModule * fluff_instance_add_module(FluffInstance * self, FluffModule * module);
FLUFF_API void          fluff_instance_remove_module(FluffInstance * self, const char * name);
FLUFF_API FluffModule * fluff_instance_get_module_by_name(FluffInstance * self, const char * name);
FLUFF_API FluffModule * fluff_instance_get_core_module(FluffInstance * self);
FLUFF_API FluffKlass  * fluff_instance_get_core_class(FluffInstance * self, uint8_t type);

FLUFF_API FluffModule * fluff_instance_add_modules_path(FluffInstance * self, const char * path);
FLUFF_API const char  * fluff_instance_get_modules_path(FluffInstance * self);

FLUFF_PRIVATE_API void _new_instance(FluffInstance * self);
FLUFF_PRIVATE_API void _free_instance(FluffInstance * self);

FLUFF_PRIVATE_API void _instance_add_void_class(FluffInstance * self);
FLUFF_PRIVATE_API void _instance_add_bool_class(FluffInstance * self);
FLUFF_PRIVATE_API void _instance_add_int_class(FluffInstance * self);
FLUFF_PRIVATE_API void _instance_add_float_class(FluffInstance * self);
FLUFF_PRIVATE_API void _instance_add_string_class(FluffInstance * self);
FLUFF_PRIVATE_API void _instance_add_object_class(FluffInstance * self);
FLUFF_PRIVATE_API void _instance_add_array_class(FluffInstance * self);
FLUFF_PRIVATE_API void _instance_add_func_class(FluffInstance * self);

#endif