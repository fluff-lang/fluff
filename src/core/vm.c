/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/vm.h>
#include <core/module.h>
#include <core/instance.h>
#include <core/class.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

/* -=======
     VM
   =======- */

/* -=- Initializers -=- */
FLUFF_API FluffVM * fluff_new_vm(FluffModule * module) {
    FluffVM * self = fluff_alloc(NULL, sizeof(FluffVM));
    _new_vm(self, module);
    return self;
}

FLUFF_API void fluff_free_vm(FluffVM * self) {
    _free_vm(self);
    fluff_free(self);
}

FLUFF_PRIVATE_API void _new_vm(FluffVM * self, FluffModule * module) {
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API void _free_vm(FluffVM * self) {
    FLUFF_CLEANUP(self);
}