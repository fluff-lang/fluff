/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <core/instance.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

FLUFF_THREAD_LOCAL static FluffInstance * global_instance = NULL;

/* -=============
     Instance
   =============- */

/* -=- Initializers -=- */
FLUFF_API FluffInstance * fluff_new_instance() {
    FluffInstance * self = fluff_alloc(NULL, sizeof(FluffInstance));
    _new_instance(self);
    return self;
}

FLUFF_API void fluff_free_instance(FluffInstance * self) {
    fluff_free(self);
}

FLUFF_API void fluff_set_instance(FluffInstance * self) {
    global_instance = self;
}

FLUFF_API FluffInstance * fluff_get_instance() {
    fluff_assert(global_instance, "invalid global instance");
    return global_instance;
}

/* -=- Private -=- */
FLUFF_PRIVATE_API void _new_instance(FluffInstance * self) {
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API void _free_instance(FluffInstance * self) {
    if (global_instance == self) global_instance = NULL;
    FLUFF_CLEANUP(self);
}