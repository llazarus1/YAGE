/*
 *  Vector4.h
 *  SceneGraph
 *
 *  Created by Brent Wilson on 11/8/06.
 *  Copyright 2006 Brent Wilson. All rights reserved.
 *
 */

#ifndef _VECTOR4_H_
#define _VECTOR4_H_
#ifndef _VECTOR_H_
#error Please include Vector.h rather than Vector4.h
#endif
#include "VectorBase.h"

template<>
class Vector<4> : public VectorBase<Vector<4>, 4> {
public:
    union {
        Real array[4];
        struct { Real x, y, z, w; };
        struct { Real s, t, u, v; };
        struct { Real i, j, k, l; };
        struct { Real r, g, b, a; };
    };
    
    inline Vector() {}
    inline Vector(Real v): VectorBase<Vector<4>, 4>(v) {}
    inline Vector(const float *v): VectorBase<Vector<4>, 4>(v) {}
    inline Vector(const double *v): VectorBase<Vector<4>, 4>(v) {}
    inline Vector(const Vector<4> &v): VectorBase<Vector<4>, 4>(v) {}
    inline Vector(const Real &a, const Real &b, const Real &c, const Real &d): x(a), y(b), z(c), w(d) {}
    
    Real& operator[](int index) { return array[index]; }
    const Real& operator[](int index) const { return array[index]; }
};

typedef Vector<4> Vector4;
typedef Vector<4> Color4;
typedef Vector<4> RGBAf;

template<>
class IVector<4> {
public:
    union {
        int array[4];
        struct { int x, y, z, w; };
        struct { int s, t, u, v; };
        struct { int i, j, k, l; };
        struct { int r, g, b, a; };
    };

    inline IVector() {}
    inline IVector(const IVector<4> &v): x(v.x), y(v.y), z(v.z), w(v.w) {}
    inline IVector(int a, int b, int c, int d): x(a), y(b), z(c), w(d) {}
    
    int& operator[](int index) { return array[index]; }
    const int& operator[](int index) const { return array[index]; }
};

typedef IVector<4> IVector4;

#endif
