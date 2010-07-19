/*
    MHPath.h
    Mountainhome

    Created by Andrew Jean on 7/16/2010
    Last Modified on 7/16/2010

    Copyright 2010 Mountainhome Project. All rights reserved.
*/

#ifndef _MHPATH_H_
#define _MHPATH_H_

#include <Base/Vector.h>

#include "PathFinder.h"
#include <stack>

#include "RubyBindings.h"

class MHWorld;

class MHPath : public RubyBindings<MHPath, true> {
    std::stack <Vector3> path;

public:
    // Ruby bindings and binding setup
    static void SetupBindings();

    static VALUE NextStep(VALUE self);
    static VALUE EndOfPath(VALUE self);

    // C-side functions
    MHPath(Vector3 source, Vector3 dest, MHWorld *world);
    ~MHPath();

    bool endOfPath();
    Vector3 getNextStep();
};

#endif