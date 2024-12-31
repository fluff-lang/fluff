/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <core/class.h>
#include <core/string.h>
#include <core/module.h>
#include <core/instance.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

/* -==========
     Klass
   ==========- */

/* -=- Initializers -=- */
FLUFF_PRIVATE_API FluffKlass * _new_class(const char * name, size_t len) {
    FluffKlass * self = fluff_alloc(NULL, sizeof(FluffKlass));
    FLUFF_CLEANUP(self);
    _new_string_n(&self->name, name, len);
    return self;
}

FLUFF_PRIVATE_API void _free_class(FluffKlass * self) {
    _free_string(&self->name);
    FLUFF_CLEANUP(self);
    fluff_free(self);
}