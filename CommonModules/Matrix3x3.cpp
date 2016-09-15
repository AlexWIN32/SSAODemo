/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Matrix3x3.h>
#include <Vector2.h>

namespace Math
{

Matrix3x3::Matrix3x3():
    a11(1), a12(0),a13(0),
    a21(0), a22(1),a23(0),
    a31(0), a32(0),a33(1)
{}

Matrix3x3::Matrix3x3(const Point3F &Part1, const Point3F &Part2, const Point3F &Part3, bool AsRows)
{
    if(AsRows){
        a11 = Part1.x; a12 = Part1.y; a13 = Part1.z;
        a21 = Part2.x; a22 = Part2.y; a23 = Part2.z;
        a31 = Part3.x; a32 = Part3.y; a33 = Part3.z;
    }else{
        a11 = Part1.x; a12 = Part2.x; a13 = Part3.x;
        a21 = Part1.y; a22 = Part2.y; a23 = Part3.y;
        a31 = Part1.z; a32 = Part2.z; a33 = Part3.z;
    }
}

float Matrix3x3::Determinant() const 
{
    float p1 = a11 * (a22 * a33 - a23 * a32);
    float p2 = a12 * (a21 * a33 - a23 * a31);
    float p3 = a13 * (a21 * a32 - a22 * a31);

    return (p1 - p2) + p3;
}

void Matrix3x3::Transpose()
{
    std::swap(a21,a12);
    std::swap(a31,a13);
    std::swap(a32,a23);
}

Matrix3x3 Matrix3x3::Transpose(const Matrix3x3 &Matrix)
{
    Matrix3x3 out = Matrix;
    out.Transpose();
    return out;
}

Matrix3x3 Matrix3x3::Mul(const Matrix3x3 &Matrix, float Val)
{
    Matrix3x3 out;
    out.a11 = Matrix.a11 * Val;
    out.a12 = Matrix.a12 * Val;
    out.a13 = Matrix.a13 * Val;

    out.a21 = Matrix.a21 * Val;
    out.a22 = Matrix.a22 * Val;
    out.a23 = Matrix.a23 * Val;

    out.a31 = Matrix.a31 * Val;
    out.a32 = Matrix.a32 * Val;
    out.a33 = Matrix.a33 * Val;

    return out;
}

Point3F Matrix3x3::Transform(const Point3F &V) const
{
    float x = V.x * a11 + V.y * a21 + V.z * a31;
    float y = V.x * a12 + V.y * a22 + V.z * a32;
    float z = V.x * a13 + V.y * a23 + V.z * a33;

    return {x, y, z};
}

Point3F Matrix3x3::Transform(const Matrix3x3 &Matrix, const Point3F &V)
{
    return Matrix.Transform(V);
}

void  Matrix3x3::Inverse()
{
    *this = Inverse(*this);
}

Matrix3x3 Matrix3x3::Inverse(const Matrix3x3 &Matrix)
{
    Matrix3x3 mMinors;
    mMinors.a11 = Matrix.a22 * Matrix.a33 - Matrix.a23 * Matrix.a32;
    mMinors.a12 = Matrix.a21 * Matrix.a33 - Matrix.a23 * Matrix.a31;
    mMinors.a13 = Matrix.a21 * Matrix.a32 - Matrix.a22 * Matrix.a31;

    mMinors.a21 = Matrix.a12 * Matrix.a33 - Matrix.a32 * Matrix.a13;
    mMinors.a22 = Matrix.a11 * Matrix.a33 - Matrix.a31 * Matrix.a13;
    mMinors.a23 = Matrix.a11 * Matrix.a32 - Matrix.a12 * Matrix.a31;

    mMinors.a31 = Matrix.a12 * Matrix.a23 - Matrix.a22 * Matrix.a13;
    mMinors.a32 = Matrix.a11 * Matrix.a23 - Matrix.a21 * Matrix.a13;
    mMinors.a33 = Matrix.a11 * Matrix.a22 - Matrix.a21 * Matrix.a12;

    Matrix3x3 mCofactors = mMinors;
    mCofactors.a21 *= -1.0f;
    mCofactors.a12 *= -1.0f;
    mCofactors.a32 *= -1.0f;
    mCofactors.a23 *= -1.0f;

    Matrix3x3 mAdjoint = Matrix3x3::Transpose(mCofactors);

    return Matrix3x3::Mul(mAdjoint, 1.0f / Matrix.Determinant());
}

Matrix3x3 Matrix3x3::Mul(const Matrix3x3 &A, const Matrix3x3 &B)
{
    Matrix3x3 out;
    out.a11 = A.a11 * B.a11 + A.a12 * B.a21 + A.a13 * B.a31;
    out.a12 = A.a11 * B.a12 + A.a12 * B.a22 + A.a13 * B.a32;
    out.a13 = A.a11 * B.a13 + A.a12 * B.a23 + A.a13 * B.a33;

    out.a21 = A.a21 * B.a11 + A.a22 * B.a21 + A.a23 * B.a31;
    out.a22 = A.a21 * B.a12 + A.a22 * B.a22 + A.a23 * B.a32;
    out.a23 = A.a21 * B.a13 + A.a22 * B.a23 + A.a23 * B.a33;

    out.a31 = A.a31 * B.a11 + A.a32 * B.a21 + A.a33 * B.a31;
    out.a32 = A.a31 * B.a12 + A.a32 * B.a22 + A.a33 * B.a32;
    out.a33 = A.a31 * B.a13 + A.a32 * B.a23 + A.a33 * B.a33;

    return out;
}

Matrix3x3 Matrix3x3::Add(const Matrix3x3 &A, const Matrix3x3 &B)
{
    Matrix3x3 out;
    out.a11 = A.a11 + B.a11;
    out.a12 = A.a12 + B.a12;
    out.a13 = A.a13 + B.a13;

    out.a21 = A.a21 + B.a21;
    out.a22 = A.a22 + B.a22;
    out.a23 = A.a23 + B.a23;

    out.a31 = A.a31 + B.a31;
    out.a32 = A.a32 + B.a32;
    out.a33 = A.a33 + B.a33;

    return out;
}

Matrix3x3 Matrix3x3::Sub(const Matrix3x3 &A, const Matrix3x3 &B)
{
    Matrix3x3 out;
    out.a11 = A.a11 - B.a11;
    out.a12 = A.a12 - B.a12;
    out.a13 = A.a13 - B.a13;

    out.a21 = A.a21 - B.a21;
    out.a22 = A.a22 - B.a22;
    out.a23 = A.a23 - B.a23;

    out.a31 = A.a31 - B.a31;
    out.a32 = A.a32 - B.a32;
    out.a33 = A.a33 - B.a33;

    return out;
}

Matrix3x3 Matrix3x3::Rotation(float Angle)
{
    Matrix3x3 out;
    out.a11 = cosf(Angle);
    out.a21 = -sinf(Angle);
    out.a12 = sinf(Angle);
    out.a22 = cos(Angle);
    return out;
}

Matrix3x3 Matrix3x3::Translation(const Point2F &Pos)
{
    Matrix3x3 out;
    out.a31 = Pos.x;
    out.a32 = Pos.y;
    return out;
}

Matrix3x3 Matrix3x3::Scalling(const SizeF &Scalling)
{
    Matrix3x3 out;
    out.a11 = Scalling.width;
    out.a22 = Scalling.width;
    return out;
}

}