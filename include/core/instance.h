#pragma once
#ifndef FLUFF_CORE_INSTANCE_H
#define FLUFF_CORE_INSTANCE_H

/* -=============
     Includes
   =============- */

#include <base.h>

/* -=============
     Instance
   =============- */

typedef struct FluffInstance {
    
} FluffInstance;

FLUFF_API FluffInstance * fluff_new_instance();
FLUFF_API void            fluff_free_instance(FluffInstance * self);

FLUFF_API void            fluff_set_instance(FluffInstance * self);
FLUFF_API FluffInstance * fluff_get_instance();

FLUFF_PRIVATE_API void _new_instance(FluffInstance * self);
FLUFF_PRIVATE_API void _free_instance(FluffInstance * self);

#endif