/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/class.h>
#include <core/method.h>
#include <core/string.h>
#include <core/module.h>
#include <core/instance.h>
#include <core/object.h>
#include <core/vm.h>
#include <core/config.h>

/* -================
     CommonKlass
   ================- */

/* -=- C/Dtors -=- */
FLUFF_PRIVATE_API void _free_common_class(CommonKlass * self) {
    while (self->property_count != 0) {
        KlassProperty * property = &self->properties[--self->property_count];
        if (property->def_value) fluff_free_object(property->def_value);
    }
    FLUFF_CLEANUP_N(self->properties, self->property_count);
    fluff_free(self->properties);
}

/* -=- Property management -=- */
FLUFF_PRIVATE_API size_t _common_class_add_property(CommonKlass * self, const char * name, FluffKlass * klass, FluffObject * def_value) {
    if (self->property_count == FLUFF_MAX_CLASS_PROPERTIES) return SIZE_MAX;

    for (size_t i = 0; i < self->property_count; ++i) {
        if (!strncmp(self->properties[i].name, name, FLUFF_MAX_FIELD_NAME_LEN)) {
            fluff_push_error("the class %.*s already has a property named '%s'\n", 
                FLUFF_STR_BUFFER_FMT(self->name), name
            );
            return SIZE_MAX;
        }
    }

    KlassProperty property = { 0 };
    property.index     = self->property_count;
    property.klass     = klass;
    property.def_value = def_value;
    strncpy(property.name, name, FLUFF_MAX_FIELD_NAME_LEN);

    self->properties = fluff_alloc(self->properties, sizeof(KlassProperty) * (++self->property_count));
    self->properties[property.index] = property;
    return property.index;
}

FLUFF_PRIVATE_API size_t _common_class_get_property_index(CommonKlass * self, const char * name) {
    for (size_t i = 0; i < self->property_count; ++i) {
        if (!strncmp(self->properties[i].name, name, FLUFF_MAX_FIELD_NAME_LEN))
            return i;
    }
    return SIZE_MAX;
}

/* -=- Method management -=- */
FLUFF_PRIVATE_API size_t _common_class_add_method(CommonKlass * self, FluffMethod * method) {
    return 0;
}

FLUFF_PRIVATE_API size_t _common_class_get_method_index(CommonKlass * self, const char * name) {
    return 0;
}

/* -=- Misc -=- */
FLUFF_PRIVATE_API size_t _common_class_get_alloc_size(CommonKlass * self) {
    size_t size = sizeof(ObjectTable);
    size += sizeof(FluffObject) * (self->property_count + (self->inherits ? 1 : 0));
    return size;
}

/* -=================
     GenericKlass
   =================- */

/* -=- C/Dtors -=- */
FLUFF_PRIVATE_API void _free_generic_class(GenericKlass * self) {
    FLUFF_CLEANUP_N(self->generics, self->generic_count);
    fluff_free(self->generics);
}

/* -==========
     Klass
   ==========- */

/* -=- C/Dtors -=- */
FLUFF_PRIVATE_API FluffKlass * _new_common_class(const char * name, size_t len, FluffKlass * inherits) {
    if (!name || len == 0) return NULL;

    FluffKlass * self = fluff_alloc(NULL, sizeof(FluffKlass));
    FLUFF_CLEANUP(self);
    strncpy(self->common.name, name, FLUFF_MIN(len, FLUFF_MAX_FIELD_NAME_LEN));
    self->common.inherits = inherits;
    if (inherits) {
        self->flags                = inherits->flags;
        self->common.inherit_depth = inherits->common.inherit_depth + 1;
        // TODO: enforce max inherited classes limit (128)
    }
    return self;
}

FLUFF_PRIVATE_API FluffKlass * _new_generic_class(FluffKlass * base, FluffKlass ** generics, size_t count) {
    FluffKlass * self = fluff_alloc(NULL, sizeof(FluffKlass));
    FLUFF_CLEANUP(self);
    self->flags                 = FLUFF_SET_FLAG(self->flags, FLUFF_KLASS_GENERIC_DERIVED);
    self->generic.base          = base;
    self->generic.generic_count = count;
    self->generic.generics      = fluff_alloc(NULL, sizeof(FluffKlass *) * count);
    memcpy(self->generic.generics, generics, sizeof(FluffKlass *) * count);
    return self;
}

FLUFF_PRIVATE_API void _free_class(FluffKlass * self) {
    if (FLUFF_HAS_FLAG(self->flags, FLUFF_KLASS_GENERIC_DERIVED))
        _free_generic_class(&self->generic);
    else
        _free_common_class(&self->common);

    FLUFF_CLEANUP(self);
    fluff_free(self);
}

FLUFF_PRIVATE_API CommonKlass * _class_get_common_data(FluffKlass * self) {
    if (FLUFF_HAS_FLAG(self->flags, FLUFF_KLASS_GENERIC_DERIVED))
        return &self->generic.base->common;
    return &self->common;
}

FLUFF_PRIVATE_API void _class_dump(FluffKlass * self) {
    if (FLUFF_HAS_FLAG(self->flags, FLUFF_KLASS_GENERIC_DERIVED)) {
        printf("generic derived '%.*s' [with ", FLUFF_STR_BUFFER_FMT(self->generic.base->common.name));

        for (size_t i = 0; i < self->generic.generic_count; ++i) {
            if (i != 0) printf(", ");
            printf("'%.*s'", FLUFF_STR_BUFFER_FMT(self->generic.generics[i]->common.name));
        }

        printf("]\n");
    } else if (FLUFF_HAS_FLAG(self->flags, FLUFF_KLASS_GENERIC_BASE)) {
        printf("generic base '%.*s'\n", FLUFF_STR_BUFFER_FMT(self->common.name));
    } else {
        printf("class '%.*s'\n", FLUFF_STR_BUFFER_FMT(self->common.name));
    }
}