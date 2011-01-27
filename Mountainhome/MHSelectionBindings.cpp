/*
 *  MHSelectionBindings.cpp
 *  Mountainhome
 *
 *  Created by loch on 11/5/10.
 *  Copyright 2010 Mountainhome Project. All rights reserved.
 *
 */

#include "MHSelectionBindings.h"
#include "MHActorBindings.h"

MHSelectionBindings::MHSelectionBindings()
: RubyBindings<MHSelection, false>(
    rb_define_class("MHSelection", rb_cObject),
    "MHSelectionBindings")
{
    rb_define_method(_class, "each", RUBY_METHOD_FUNC(MHSelectionBindings::Each), 0);
    rb_define_method(_class, "each_actor", RUBY_METHOD_FUNC(MHSelectionBindings::EachActor), 0);
    rb_define_method(_class, "each_tile", RUBY_METHOD_FUNC(MHSelectionBindings::EachTile), 0);
}

MHSelectionBindings::~MHSelectionBindings() {}

VALUE MHSelectionBindings::Each(VALUE rSelf) {
    EachActor(rSelf);
    EachTile(rSelf);

    return rSelf;
}

VALUE MHSelectionBindings::EachActor(VALUE rSelf) {
    MHSelection *cSelf = Get()->getPointer(rSelf);
    std::list <MHActor*> &selected = const_cast<std::list<MHActor*> &>(cSelf->getSelectedActors());

    std::list <MHActor*>::iterator itr = selected.begin();
    for(; itr != selected.end(); itr++) {
        rb_yield(MHActorBindings::Get()->getValue(*itr));
    }

    return rSelf;
}

VALUE MHSelectionBindings::EachTile(VALUE rSelf) {
    MHSelection *cSelf = Get()->getPointer(rSelf);
    std::list <Vector3> &selected = const_cast<std::list<Vector3> &>(cSelf->getSelectedTiles());

    std::list <Vector3>::iterator itr = selected.begin();
    for(; itr != selected.end(); itr++) {
        rb_yield(rb_ary_new3(3, INT2NUM((*itr)[0]), INT2NUM((*itr)[1]), INT2NUM((*itr)[2])));
    }

    return rSelf;
}