#pragma once
#ifndef FLUFF_CORE_VM_H
#define FLUFF_CORE_VM_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/object.h>
#include <core/string.h>

/* -============
     VMEntry
   ============- */

typedef struct VMEntry VMEntry;

typedef struct VMEntry {
    VMEntry   * prev;
    FluffObject obj;
} VMEntry;

/* -============
     VMFrame
   ============- */

typedef struct VMFrame {
    VMEntry * last_entry;
    size_t    entry_count;
} VMFrame;

FLUFF_PRIVATE_API void      _vm_frame_push(VMFrame * self, FluffObject * obj);
FLUFF_PRIVATE_API void      _vm_frame_pop(VMFrame * self);
FLUFF_PRIVATE_API void      _vm_frame_popn(VMFrame * self, size_t count);
FLUFF_PRIVATE_API void      _vm_frame_clear(VMFrame * self);
FLUFF_PRIVATE_API VMEntry * _vm_frame_at(VMFrame * self, int idx);

/* -=======
     VM
   =======- */

typedef struct FluffModule FluffModule;

// This struct represents a VM.
typedef struct FluffVM {
    FluffInstance * instance;
    FluffModule   * module;

    VMFrame   current_frame;
    VMFrame * frames;
    size_t    frame_count, frame_capacity;
} FluffVM;

FLUFF_API FluffVM * fluff_new_vm(FluffInstance * instance, FluffModule * module);
FLUFF_API void      fluff_free_vm(FluffVM * self);

FLUFF_API FluffObject * fluff_vm_at(FluffVM * self, int idx);
FLUFF_API FluffResult   fluff_vm_push(FluffVM * self, FluffObject * obj);
FLUFF_API FluffResult   fluff_vm_push_object(FluffVM * self, FluffKlass * klass);
FLUFF_API FluffResult   fluff_vm_push_null_object(FluffVM * self, FluffKlass * klass);
FLUFF_API FluffResult   fluff_vm_push_bool(FluffVM * self, FluffBool v);
FLUFF_API FluffResult   fluff_vm_push_int(FluffVM * self, FluffInt v);
FLUFF_API FluffResult   fluff_vm_push_float(FluffVM * self, FluffFloat v);
FLUFF_API FluffResult   fluff_vm_push_string(FluffVM * self, const char * str);
FLUFF_API FluffResult   fluff_vm_push_string_n(FluffVM * self, const char * str, size_t len);
FLUFF_API FluffResult   fluff_vm_pop(FluffVM * self);
FLUFF_API FluffResult   fluff_vm_popn(FluffVM * self, size_t n);
FLUFF_API size_t        fluff_vm_top(FluffVM * self);
FLUFF_API size_t        fluff_vm_size(FluffVM * self);
FLUFF_API size_t        fluff_vm_frame_top(FluffVM * self);
FLUFF_API size_t        fluff_vm_frame_size(FluffVM * self);

FLUFF_API FluffResult fluff_vm_invoke(FluffVM * self, FluffObject * object, size_t argc);

FLUFF_PRIVATE_API void _new_vm(FluffVM * self, FluffInstance * instance, FluffModule * module);
FLUFF_PRIVATE_API void _free_vm(FluffVM * self);

FLUFF_PRIVATE_API FluffResult _vm_push_frame(FluffVM * self, size_t preserve);
FLUFF_PRIVATE_API FluffResult _vm_pop_frame(FluffVM * self, size_t preserve);
FLUFF_PRIVATE_API void        _vm_clear_frames(FluffVM * self);

#endif