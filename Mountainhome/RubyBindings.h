/*
 *  RubyBindings.h
 *  System
 *
 *  Created by loch on 2/15/10.
 *  Copyright 2010 Brent Wilson. All rights reserved.
 *
 */

#ifndef _RUBYBINDINGS_H_
#define _RUBYBINDINGS_H_
#include <Base/Singleton.h>
#include <Engine/Engine.h>

///\todo XXXBMW: Wtf hack.... Compile fails if I don't do this. Clearly not how it's
// supposed to work, though. I'll have to look into this later.
#define HAVE_STRUCT_TIMESPEC 1
#include <ruby.h>

template <typename T>
class ManyObjectBinding {
public:
    static void RegisterObject(VALUE robj, T* cobj);
    static T* GetObject(VALUE robj);
    static VALUE GetValue(T *cobj);

protected:
    static std::map<VALUE, T*> RubyToC;
    static std::map<T*, VALUE> CToRuby;
    static VALUE Class;

};

template <typename T> std::map<VALUE, T*> ManyObjectBinding<T>::RubyToC;
template <typename T> std::map<T*, VALUE> ManyObjectBinding<T>::CToRuby;
template <typename T> VALUE ManyObjectBinding<T>::Class;

template <typename T>
void ManyObjectBinding<T>::RegisterObject(VALUE robj, T* cobj) {
    if (CToRuby.find(cobj) != CToRuby.end()) {
        THROW(DuplicateItemError, "Object already exists for " << cobj << "!");
    }

    if (RubyToC.find(robj) != RubyToC.end()) {
        THROW(DuplicateItemError, "Object already exists for " << robj << "!");
    }

    RubyToC[robj] = cobj;
    CToRuby[cobj] = robj;
}

template <typename T>
VALUE ManyObjectBinding<T>::GetValue(T *cobj) {
    if (CToRuby.find(cobj) == CToRuby.end()) {
        THROW(InternalError, "Object does not exist for " << cobj << "!");
    }

    return CToRuby[cobj];
}

template <typename T>
T* ManyObjectBinding<T>::GetObject(VALUE robj) {
    if (RubyToC.find(robj) == RubyToC.end()) {
        THROW(InternalError, "Object does not exist for " << robj << "!");
    }

    return RubyToC[robj];
}

#pragma mark Helper functions
VALUE require_setup_wrapper(VALUE arg);
void translate_ruby_exception(int error);

#endif