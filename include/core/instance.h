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

    FluffKlass * void_klass;
    FluffKlass * bool_klass;
    FluffKlass * int_klass;
    FluffKlass * float_klass;
    FluffKlass * string_klass;
    FluffKlass * object_klass;
    FluffKlass * array_klass;
    FluffKlass * func_klass;
} FluffInstance;

FLUFF_API FluffInstance * fluff_new_instance();
FLUFF_API void            fluff_free_instance(FluffInstance * self);

FLUFF_API void            fluff_set_instance(FluffInstance * self);
FLUFF_API FluffInstance * fluff_get_instance();

FLUFF_API FluffModule * fluff_instance_add_module(FluffInstance * self, FluffModule * module);
FLUFF_API void          fluff_instance_remove_module(FluffInstance * self, const char * name);
FLUFF_API FluffModule * fluff_instance_get_module_by_name(FluffInstance * self, const char * name);

FLUFF_API FluffModule * fluff_instance_add_modules_path(FluffInstance * self, const char * path);
FLUFF_API const char  * fluff_instance_get_modules_path(FluffInstance * self);

FLUFF_PRIVATE_API void _new_instance(FluffInstance * self);
FLUFF_PRIVATE_API void _free_instance(FluffInstance * self);

#endif