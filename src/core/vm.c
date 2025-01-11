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

/* -============
     VMFrame
   ============- */

FLUFF_PRIVATE_API void _vm_frame_push(VMFrame * self, FluffObject * obj) {
    VMEntry * entry = fluff_alloc(NULL, sizeof(VMEntry));
    // printf("allocated entry %p\n", entry);
    FLUFF_CLEANUP(entry);
    _ref_object(&entry->obj, obj);
    entry->prev      = self->last_entry;
    self->last_entry = entry;
    ++self->entry_count;
}

FLUFF_PRIVATE_API void _vm_frame_pop(VMFrame * self) {
    if (self->entry_count == 0 || !self->last_entry) return;
    VMEntry * entry = self->last_entry;
    // printf("deallocated entry %p\n", entry);
    self->last_entry = entry->prev;
    _free_object(&entry->obj);
    FLUFF_CLEANUP(entry);
    fluff_free(entry);
    --self->entry_count;
}

FLUFF_PRIVATE_API void _vm_frame_popn(VMFrame * self, size_t count) {
    while (count-- > 0) {
        _vm_frame_pop(self);
    }
}

FLUFF_PRIVATE_API void _vm_frame_clear(VMFrame * self) {
    _vm_frame_popn(self, self->entry_count);
}

FLUFF_PRIVATE_API VMEntry * _vm_frame_at(VMFrame * self, int idx) {
    size_t index = (size_t)(idx < 0 ? -idx : self->entry_count - idx) - 1;
    if (index > self->entry_count) return NULL;

    VMEntry * entry = self->last_entry;
    while (index > 0 && entry) {
        entry = entry->prev;
        --index;
    }
    return entry;
}

/* -=======
     VM
   =======- */

/* -=- Initializers -=- */
FLUFF_API FluffVM * fluff_new_vm(FluffInstance * instance, FluffModule * module) {
    FluffVM * self = fluff_alloc(NULL, sizeof(FluffVM));
    _new_vm(self, instance, module);
    return self;
}

FLUFF_API void fluff_free_vm(FluffVM * self) {
    _free_vm(self);
    fluff_free(self);
}

FLUFF_API FluffObject * fluff_vm_at(FluffVM * self, int idx) {
    VMEntry * entry = _vm_frame_at(&self->current_frame, idx);
    if (!entry) return NULL;
    return &entry->obj;
}

FLUFF_API FluffResult fluff_vm_push(FluffVM * self, FluffObject * obj) {
    _vm_frame_push(&self->current_frame, obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_object(FluffVM * self, FluffKlass * klass) {
    FluffObject obj;
    _new_object(&obj, self->instance, klass);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_null_object(FluffVM * self, FluffKlass * klass) {
    FluffObject obj;
    _new_null_object(&obj, self->instance, klass);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_bool(FluffVM * self, FluffBool v) {
    FluffObject obj;
    _new_bool_object(&obj, self->instance, v);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_int(FluffVM * self, FluffInt v) {
    FluffObject obj;
    _new_int_object(&obj, self->instance, v);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_float(FluffVM * self, FluffFloat v) {
    FluffObject obj;
    _new_float_object(&obj, self->instance, v);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_string(FluffVM * self, const char * str) {
    FluffObject obj;
    _new_string_object(&obj, self->instance, str);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_string_n(FluffVM * self, const char * str, size_t len) {
    FluffObject obj;
    _new_string_object_n(&obj, self->instance, str, len);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_pop(FluffVM * self) {
    _vm_frame_pop(&self->current_frame);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_popn(FluffVM * self, size_t n) {
    _vm_frame_popn(&self->current_frame, n);
    return FLUFF_OK;
}

FLUFF_API size_t fluff_vm_top(FluffVM * self) {
    return (self->current_frame.entry_count > 0 ? self->current_frame.entry_count - 1 : 0);
}

FLUFF_API size_t fluff_vm_size(FluffVM * self) {
    return self->current_frame.entry_count;
}

FLUFF_API size_t fluff_vm_frame_top(FluffVM * self) {
    return (self->frame_count > 0 ? self->frame_count - 1 : 0);
}

FLUFF_API size_t fluff_vm_frame_size(FluffVM * self) {
    return self->frame_count;
}

FLUFF_PRIVATE_API FluffResult fluff_vm_invoke(FluffVM * self, FluffObject * object, size_t argc) {
    FluffMethod * method = object->data._method;
    if (!method) {
        fluff_push_error("attempt to call a null method");
        return FLUFF_FAILURE;
    }
    if (method->callback) {
        // TODO: typechecking
        _vm_push_frame(self, argc);
        FluffResult res = method->callback(self, argc);
        // TODO: make void functions not return anything
        if (method->ret_type == method->ret_type->instance->void_klass)
            fluff_vm_push_object(self, method->ret_type->instance->void_klass);
        _vm_pop_frame(self, 1);
        return res;
    }
    fluff_push_error("attempt to call an incomplete method ('%s')", method->name.data);
    return FLUFF_FAILURE;
}

FLUFF_PRIVATE_API void _new_vm(FluffVM * self, FluffInstance * instance, FluffModule * module) {
    FLUFF_CLEANUP(self);
    self->instance = instance;
    self->module   = module;
}

FLUFF_PRIVATE_API void _free_vm(FluffVM * self) {
    _vm_clear_frames(self);
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API FluffResult _vm_push_frame(FluffVM * self, size_t preserve) {
    if (preserve > self->current_frame.entry_count) {
        fluff_push_error("attempted to preserve %zu entries on a %zu entry frame", 
            preserve, self->current_frame.entry_count
        );
        return FLUFF_FAILURE;
    }

    if (self->frame_count >= self->frame_capacity)
        self->frames = fluff_alloc(self->frames, sizeof(VMFrame) * (++self->frame_capacity));

    VMFrame * last_frame = &self->frames[self->frame_count++];

    VMEntry * top_entry    = NULL;
    VMEntry * bottom_entry = NULL;
    if (preserve != 0) {
        top_entry    = _vm_frame_at(&self->current_frame, -((int)preserve));
        bottom_entry = self->current_frame.last_entry;
        
        self->current_frame.entry_count -= preserve;
        self->current_frame.last_entry   = top_entry->prev;
        top_entry->prev                  = NULL;
    }

    * last_frame = self->current_frame;

    if (preserve != 0) {
        self->current_frame.last_entry  = bottom_entry;
        self->current_frame.entry_count = preserve;
    }

    return FLUFF_OK;
}

FLUFF_PRIVATE_API FluffResult _vm_pop_frame(FluffVM * self, size_t preserve) {
    if (preserve > self->current_frame.entry_count) {
        fluff_push_error("attempted to preserve %zu entries on a %zu entry frame", 
            preserve, self->current_frame.entry_count
        );
        return FLUFF_FAILURE;
    }

    if (self->frame_count == 0) return FLUFF_OK;

    VMFrame * last_frame = &self->frames[--self->frame_count];

    VMEntry * top_entry    = NULL;
    VMEntry * bottom_entry = NULL;

    if (preserve > 0) {
        top_entry    = _vm_frame_at(&self->current_frame, -((int)preserve));
        bottom_entry = self->current_frame.last_entry;

        self->current_frame.last_entry   = top_entry->prev;
        self->current_frame.entry_count -= preserve;

        top_entry->prev          = last_frame->last_entry;
        last_frame->last_entry   = bottom_entry;
        last_frame->entry_count += preserve;
    }

    _vm_frame_clear(&self->current_frame);

    self->current_frame = self->frames[self->frame_count];
    return FLUFF_OK;
}

FLUFF_PRIVATE_API void _vm_clear_frames(FluffVM * self) {
    while (self->frame_count > 0) {
        _vm_pop_frame(self, 0);
    }
    _vm_frame_clear(&self->current_frame);
    fluff_free(self->frames);
    self->frames         = NULL;
    self->frame_capacity = 0;
}