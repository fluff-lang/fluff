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
    _new_string_n(&self->name, name, len);
    return self;
}

FLUFF_PRIVATE_API void _free_method(FluffMethod * self) {
    while (self->property_count != 0) {
        MethodProperty * property = &self->properties[--self->property_count];
        _free_string(&property->name);
    }
    FLUFF_CLEANUP_N(self->properties, self->property_count);
    fluff_free(self->properties);
    _free_string(&self->name);
    FLUFF_CLEANUP(self);
    fluff_free(self);
}

FLUFF_PRIVATE_API size_t _method_add_property(FluffMethod * self, const char * name, FluffKlass * type) {
    MethodProperty property = { 0 };
    property.index = self->property_count;
    property.type  = type;

    self->properties = fluff_alloc(self->properties, sizeof(MethodProperty) * (++self->property_count));
    _new_string(&property.name, name);
    self->properties[property.index] = property;
    return property.index;
}

FLUFF_PRIVATE_API FluffResult _method_invoke(FluffMethod * self, FluffVM * vm) {
    if (self->callback) return self->callback(vm);
    fluff_push_error("attempt to call an incomplete method ('%s')", 
        self->name.data
    );
    return FLUFF_FAILURE;
}