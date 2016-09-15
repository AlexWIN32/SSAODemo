/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Vector2Fwd.h>
#include <stdint.h>

namespace Math
{

class Matrix3x3
{
private:
    union{
        struct{
            float a11, a12, a13;
            float a21, a22, a23;
            float a31, a32, a33;
        };
        float a[3][3];
    };
public:
    Matrix3x3();
    Matrix3x3(const Point3F &Part1, const Point3F &Part2, const Point3F &Part3, bool AsRows = true);
    float Determinant() const;
    void Inverse();
    void Transpose();
    float &operator() (int32_t R, int32_t C) {return a[R][C];}
    float operator() (int32_t R, int32_t C) const {return a[R][C];}
    Point3F Transform(const Point3F &V) const;
    Matrix3x3 operator *(float Val) const {return Mul(*this, Val);}
    Matrix3x3 operator *(const Matrix3x3 &Matrix) const{return Mul(*this, Matrix);}
    Matrix3x3 operator +(const Matrix3x3 &Matrix) const{return Add(*this, Matrix);}
    Matrix3x3 operator -(const Matrix3x3 &Matrix) const{return Sub(*this, Matrix);}
    static Matrix3x3 Transpose(const Matrix3x3 &Matrix);
    static Matrix3x3 Mul(const Matrix3x3 &Matrix, float Val);
    static Matrix3x3 Mul(const Matrix3x3 &A, const Matrix3x3 &B);
    static Matrix3x3 Add(const Matrix3x3 &A, const Matrix3x3 &B);
    static Matrix3x3 Sub(const Matrix3x3 &A, const Matrix3x3 &B);
    static Point3F Transform(const Matrix3x3 &Matrix, const Point3F &V);
    static Matrix3x3 Inverse(const Matrix3x3 &Matrix);
    static Matrix3x3 Rotation(float Angle);
    static Matrix3x3 Translation(const Point2F &Pos);
    static Matrix3x3 Scalling(const SizeF &Scalling);
};

}