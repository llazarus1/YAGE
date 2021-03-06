#ifndef _MATRIX_H
#define _MATRIX_H

#include "Vector.h"

class Quaternion;

/*! This class represents a 4x4 matrix used to define affine transformations and is built
 *  specifically for easy interaction with OpenGL. It is important to note that this class
 *  does NOT use the typical, C-style memory layout for matrices. Here is a diagram:
 *
 *   | m00 m10 m20 tx |    |  m0  m4  m8 m12 |    |  m0  m1  m2  m3 |
 *   | m01 m11 m21 ty |    |  m1  m5  m9 m13 |    |  m4  m5  m6  m7 |
 *   | m02 m12 m22 tz |    |  m2  m6 m10 m14 |    |  m8  m9 m10 m11 |
 *   |  0   0   0  tw |    |  m3  m7 m11 m15 |    | m12 m13 m14 m15 |
 *
 *      OpenGL style          OpenGL matrix            standard C
 *                             arrangement             convention
 *
 * This matrix class is based on the OpenGL layout for interoperability.
 * \brief A 4x4 matrix used to describe affine transformations.
 * \author Brent Wilson */
class Matrix {
    static const int Matrix4x4 = 16;
    Real m_mat[Matrix4x4];

public:
    /*! Creates a new matrix based on the given euler angles.
     * \seealso fromEuler */
    static Matrix FromEuler(const Radian &pitch, const Radian &yaw, const Radian &roll);

    /*! Creates a new matrix based on the given axis/angle.
     * \seealso fromAxisAngle */
    static Matrix FromAxisAngle(const Radian &angle, const Vector3 &axis);

    /*! Creates a new matrix that will transform 'from' to 'to'.
     * \seealso findMatrix */
    static Matrix FindMatrix(const Vector3 &from, const Vector3 &to);

    /*! Creates a new matrix based on the given axes.
     * \seealso fromAxes */
    static Matrix FromAxes(const Vector3 &xAxis, const Vector3 &yAxis, const Vector3 &zAxis);

    /*! Creates an identity matrix. */
    static Matrix Identity();

    /*! Creates a transformation matrix */
    static Matrix Translation(const Vector3 &translation);

    /*! Creates an affine transformation matrix. */
    static Matrix Affine(const Quaternion &quat, const Vector3 &translation);

    /*! Creates an inverse affine transformation matrix. This is slightly quicker than
     *  calling Matrix::Affine(q, t).getInverse() */
    static Matrix InverseAffine(const Quaternion &quat, const Vector3 &translation);

    /*! Creates a perspective projection matrix, which is what causes things to shrink in
     *  size as they move further away from the origin. */
    static Matrix Perspective(int width, int height, Radian fov, Real near, Real far);

    /*! Creates a perspective projection matrix, which is what causes things to shrink in
     *  size as they move further away from the origin. */
    static Matrix Perspective(float ratio, Radian fov, Real near, Real far);

    /*! Creates an orthographic projection matrix, which is a form of parallel projection,
     *  meaning as things move a way from the origin, their size remains constant.
     * \note Remember that for this case, -1 is close and 1 is far away. This isn't like
     *  OpenGL where you are looking down -Z and a large negative number is farther in
     *  front. */
    static Matrix Ortho(Real left, Real right, Real bottom, Real top, Real near = -1.0, Real far = 1.0);

    /*! Sets up an orthographic projection centered at a particular location. */
    static Matrix CenterOrtho(Real width, Real height, const Vector2 &center, Real near, Real far);

public:
#pragma mark Initialization and destruction
    /*! Creates an identity matrix.
     * \todo Defaulting to the identity matrix may have an impact on speed. We should look
     *  into this and avoid the loadIdentity call if needed. */
    Matrix();

    /*! Builds a rotation matrix equivilent to the given quaternion.
     * \seealso fromQuaternion */
    Matrix(const Quaternion &quat);

    /*! Builds a martrix from an array of floating point values, laid out in the OpenGL
     *  fashion (as described in the class description). */
    Matrix(const Real *oldMatrix);

    /*! D'tor */
    ~Matrix();

    /*! Finds the matrix needed to rotate the 'from' vector to the 'to' vector. Consider
     *  the equation M * X = Y, where M is an affine transformation matrix and X and Y are
     *  vectors. This constructor effectively takes X and Y as arguments and solves for M.
     * \param from The vector that the new matrix should transform to 'to'.
     * \param to The vector the new matrix should transform 'from' to. */
    void findMatrix(const Vector3 &from, const Vector3 &to);

#pragma mark Axes Conversion
    /*! Builds a matrix representing the transformation into the space defined by the
     *  given three axes. For example, the default space (represented by the identity
     *  matrix) is:
     *
     *    X: (1, 0, 0)
     *    Y: (0, 1, 0)
     *    Z: (0, 0, 1)
     *
     * \param xAxis The X axis of the defined space.
     * \param yAxis The Y axis of the defined space.
     * \param zAxis The Z axis of the defined space. */
    void fromAxes(const Vector3 &xAxis, const Vector3 &yAxis, const Vector3 &zAxis);

    /*! */
    void toAxes(Vector3 &xAxis, Vector3 &yAxis, Vector3 &zAxis);

#pragma mark Axis/Angle Conversions
    /*! Creates a rotation matrix based off of an axis and an angle of rotation.
     * \param angle The degree of rotation around the given axis.
     * \param axis The axis about with the rotation should happen. */
    void fromAxisAngle(const Radian &rad, const Vector3 &axis);
    void fromAxisAngle(const Radian &rad, const Real &x, const Real &y, const Real &z);

    void toAxisAngle(Radian &rad, Vector3 &axis) const;
    void toAxisAngle(Radian &rad, Real &x, Real &y, Real &z) const;

#pragma mark Euler Conversions
    /*! Creates a rotation matrix based off of the given euler angles. The assumed order
     *  of application is: pitch, yaw, and roll.
     * \param pitch The degree of rotation around the X axis, in radians.
     * \param yaw The degree of rotation around the Y axis, in radians.
     * \param roll The degree of rotation around the Z axis, in radians. */
    void fromEuler(const Radian &pitch, const Radian &yaw, const Radian &roll);

    void toEuler(Radian &pitch, Radian &yaw, Radian &roll) const;

#pragma mark Vector Application
    void apply(Vector3 &vec) const;

    void apply(Vector4 &vec) const;
    
#pragma mark Matrix Operations
//These create a matrix (A) and multiply it by the current matrix (B)
//and sets the current matrix to the result: B = A * B
//This allows you to easily chain events as the current matrix will apply
//folowed by the created matrix.
    void scale(const Vector3 &vec);
    void scale(Real x, Real y, Real z);
    void translate(const Vector3 &vec);
    void translate(Real x, Real y, Real z);
    void rotate(Radian radians, const Vector3 &axis);
    void rotate(Radian radians, Real x, Real y, Real z);

#pragma mark Accessors
    void loadIdentity();
    void set(const Matrix &matrix);
    void set(const Real *matrix);
    const Real *getMatrix() const { return m_mat; }

    void setScale(const Vector3 &scale);
    void setScale(Real x, Real y, Real z);
    void setTranslation(const Vector3 &offset);
    void setTranslation(Real x, Real y, Real z);
    void setRotation(Radian radians, const Vector3 &scale);
    void setRotation(Radian radians, Real x, Real y, Real z);

    Vector3 getTranslation() const;

    Matrix getInverse() const;

#pragma mark Operators
    void postMultiply(const Matrix &rhs);
    void preMultiply(const Matrix &rhs);

    Matrix operator*(const Matrix &lhs) const;
    Vector3 operator*(const Vector3 &lhs) const;
    Vector4 operator*(const Vector4 &lhs) const;

    Matrix& operator=(const Matrix &other);
    bool operator==(const Matrix &other) const;
    bool operator!=(const Matrix &other) const;

    const Real& operator[](int index) const;
    Real& operator[](int index);

    const Real& operator()(int row, int col) const;
    Real& operator()(int row, int col);

private:
    void multiply(const Real* m1, const Real* m2, Real newMatrix[16]);

};

std::ostream& operator<<(std::ostream &lhs, const Matrix &rhs);
#endif
