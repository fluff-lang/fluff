#pragma once
#ifndef FLUFF_CORE_OBJECT_H
#define FLUFF_CORE_OBJECT_H

/* -=============
     Includes
   =============- */

#include <base.h>

/* -===========
     Object
   ===========- */

typedef struct FluffKlass FluffKlass;
typedef struct FluffObject FluffObject;

typedef struct LinkedObject LinkedObject;

typedef struct LinkedObject {
    LinkedObject * prev;
} LinkedObject;

FLUFF_API FluffInstance * fluff_new_instance();
FLUFF_API void            fluff_free_instance(FluffInstance * self);

FLUFF_API void            fluff_set_instance(FluffInstance * self);
FLUFF_API FluffInstance * fluff_get_instance();

FLUFF_API FluffModule * fluff_instance_add_module(FluffInstance * self, FluffModule * module);
FLUFF_API void          fluff_instance_remove_module(FluffInstance * self, const char * name);

FLUFF_API FluffModule * fluff_instance_get_module_by_name(FluffInstance * self, const char * name);

FLUFF_PRIVATE_API void _new_instance(FluffInstance * self);
FLUFF_PRIVATE_API void _free_instance(FluffInstance * self);

#endif