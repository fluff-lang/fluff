/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/method.h>
#include <core/class.h>
#include <core/string.h>
#include <core/module.h>
#include <core/instance.h>
#include <core/object.h>
#include <core/vm.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

/* -===========
     Method
   ===========- */

/* -=- Initializers -=- */
FLUFF_PRIVATE_API FluffMethod * _new_method(const char * name, size_t len) {
    FluffMethod * self = fluff_alloc(NULL, sizeof(FluffMethod));
    FLUFF_CLEANUP(self);
    strncpy(self->name, name, FLUFF_MIN(len, FLUFF_MAX_FIELD_NAME_LEN));
    self->ref_count = 1;
    return self;
}

FLUFF_PRIVATE_API void _free_method(FluffMethod * self) {
    if (--self->ref_count > 0) return;

    FLUFF_CLEANUP_N(self->properties, self->property_count);
    fluff_free(self->properties);
    FLUFF_CLEANUP(self);
    fluff_free(self);
}

FLUFF_PRIVATE_API size_t _method_add_property(FluffMethod * self, const char * name, FluffKlass * type) {
    MethodProperty property = { 0 };
    property.index = self->property_count;
    property.type  = type;

    self->properties = fluff_alloc(self->properties, sizeof(MethodProperty) * (++self->property_count));
    strncpy(property.name, name, FLUFF_MAX_FIELD_NAME_LEN);
    self->properties[property.index] = property;
    return property.index;
}