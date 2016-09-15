/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Matrix4x4.h>
#include <Matrix3x3.h>
#include <Vector2.h>

namespace Math
{

Matrix4x4::Matrix4x4():
    a11(1.0f), a12(0.0f), a13(0.0f), a14(0.0f),
    a21(0.0f), a22(1.0f), a23(0.0f), a24(0.0f),
    a31(0.0f), a32(0.0f), a33(1.0f), a34(0.0f),
    a41(0.0f), a42(0.0f), a43(0.0f), a44(1.0f)
{}

Matrix4x4::Matrix4x4(const Point4F &Part1, const Point4F &Part2, const Point4F &Part3, const Point4F &Part4, bool AsRows)
{
    if(AsRows){
        a11 = Part1.x; a12 = Part1.y; a13 = Part1.z; a14 = Part1.w;
        a21 = Part2.x; a22 = Part2.y; a23 = Part2.z; a24 = Part2.w;
        a31 = Part3.x; a32 = Part3.y; a33 = Part3.z; a34 = Part3.w;
        a41 = Part4.x; a42 = Part4.y; a43 = Part4.z; a44 = Part4.w;
    }else{
        a11 = Part1.x; a12 = Part2.y; a13 = Part3.z; a14 = Part4.w;
        a21 = Part1.x; a22 = Part2.y; a23 = Part3.z; a24 = Part4.w;
        a31 = Part1.x; a32 = Part2.y; a33 = Part3.z; a34 = Part4.w;
        a41 = Part1.x; a42 = Part2.y; a43 = Part3.z; a44 = Part4.w;
    }
}

float Matrix4x4::Determinant() const
{
    Matrix3x3 a = {{a22,a23,a24}, {a32,a33,a34}, {a42,a43,a44}};
    Matrix3x3 b = {{a21,a23,a24}, {a31,a33,a34}, {a41,a43,a44}};
    Matrix3x3 c = {{a21,a22,a24}, {a31,a32,a34}, {a41,a42,a44}};
    Matrix3x3 d = {{a21,a22,a23}, {a31,a32,a33}, {a41,a42,a43}};

    return a11 * a.Determinant() - a12 * b.Determinant() + a13 * c.Determinant() - a14 * d.Determinant();
}

Matrix4x4 Matrix4x4::Inverse(const Matrix4x4 &M)
{
    Matrix4x4 mMinors;
    mMinors.a11 = Matrix3x3({M.a22, M.a23, M.a24}, {M.a32, M.a33, M.a34}, {M.a42, M.a43, M.a44}).Determinant();
    mMinors.a12 = Matrix3x3({M.a21, M.a23, M.a24}, {M.a31, M.a33, M.a34}, {M.a41, M.a43, M.a44}).Determinant();
    mMinors.a13 = Matrix3x3({M.a21, M.a22, M.a24}, {M.a31, M.a32, M.a34}, {M.a41, M.a42, M.a44}).Determinant();
    mMinors.a14 = Matrix3x3({M.a21, M.a22, M.a23}, {M.a31, M.a32, M.a33}, {M.a41, M.a42, M.a43}).Determinant();

    mMinors.a21 = Matrix3x3({M.a12, M.a13, M.a14}, {M.a32, M.a33, M.a34}, {M.a42, M.a43, M.a44}).Determinant();
    mMinors.a22 = Matrix3x3({M.a11, M.a13, M.a14}, {M.a31, M.a33, M.a34}, {M.a41, M.a43, M.a44}).Determinant();
    mMinors.a23 = Matrix3x3({M.a11, M.a12, M.a14}, {M.a31, M.a32, M.a34}, {M.a41, M.a42, M.a44}).Determinant();
    mMinors.a24 = Matrix3x3({M.a11, M.a12, M.a13}, {M.a31, M.a32, M.a33}, {M.a41, M.a42, M.a43}).Determinant();

    mMinors.a31 = Matrix3x3({M.a12, M.a13, M.a14}, {M.a22, M.a23, M.a24}, {M.a42, M.a43, M.a44}).Determinant();
    mMinors.a32 = Matrix3x3({M.a11, M.a13, M.a14}, {M.a21, M.a23, M.a24}, {M.a41, M.a43, M.a44}).Determinant();
    mMinors.a33 = Matrix3x3({M.a11, M.a12, M.a14}, {M.a21, M.a22, M.a24}, {M.a41, M.a42, M.a44}).Determinant();
    mMinors.a34 = Matrix3x3({M.a11, M.a12, M.a13}, {M.a21, M.a22, M.a23}, {M.a41, M.a42, M.a43}).Determinant();

    mMinors.a41 = Matrix3x3({M.a12, M.a13, M.a14}, {M.a22, M.a23, M.a24}, {M.a32, M.a33, M.a34}).Determinant();
    mMinors.a42 = Matrix3x3({M.a12, M.a13, M.a14}, {M.a21, M.a23, M.a24}, {M.a31, M.a33, M.a34}).Determinant();
    mMinors.a43 = Matrix3x3({M.a11, M.a12, M.a14}, {M.a21, M.a22, M.a24}, {M.a31, M.a32, M.a34}).Determinant();
    mMinors.a44 = Matrix3x3({M.a11, M.a12, M.a13}, {M.a21, M.a22, M.a23}, {M.a31, M.a32, M.a33}).Determinant();

    Matrix4x4 mCofactors = mMinors;
    mCofactors.a12 *= -1.0f;
    mCofactors.a14 *= -1.0f;
    mCofactors.a21 *= -1.0f;
    mCofactors.a23 *= -1.0f;
    mCofactors.a32 *= -1.0f;
    mCofactors.a34 *= -1.0f;
    mCofactors.a41 *= -1.0f;
    mCofactors.a43 *= -1.0f;

    Matrix4x4 mAdjoint = Transpose(mCofactors);
    
    return Mul(mAdjoint, 1.0f / M.Determinant());
}

Matrix4x4 Matrix4x4::Add(const Matrix4x4 &A, const Matrix4x4 &B)
{
    Matrix4x4 mOut;

    mOut.a11 = A.a11 + B.a11; mOut.a12 = A.a12 + B.a12; mOut.a13 = A.a13 + B.a13; mOut.a14 = A.a14 + B.a14;
    mOut.a21 = A.a21 + B.a21; mOut.a22 = A.a22 + B.a22; mOut.a23 = A.a23 + B.a23; mOut.a24 = A.a24 + B.a24;
    mOut.a31 = A.a31 + B.a31; mOut.a32 = A.a32 + B.a32; mOut.a33 = A.a33 + B.a33; mOut.a34 = A.a34 + B.a34;
    mOut.a41 = A.a41 + B.a41; mOut.a42 = A.a42 + B.a42; mOut.a43 = A.a43 + B.a43; mOut.a44 = A.a44 + B.a44;

    return mOut;
}

Matrix4x4 Matrix4x4::Sub(const Matrix4x4 &A, const Matrix4x4 &B)
{
    Matrix4x4 mOut;

    mOut.a11 = A.a11 - B.a11; mOut.a12 = A.a12 - B.a12; mOut.a13 = A.a13 - B.a13; mOut.a14 = A.a14 - B.a14;
    mOut.a21 = A.a21 - B.a21; mOut.a22 = A.a22 - B.a22; mOut.a23 = A.a23 - B.a23; mOut.a24 = A.a24 - B.a24;
    mOut.a31 = A.a31 - B.a31; mOut.a32 = A.a32 - B.a32; mOut.a33 = A.a33 - B.a33; mOut.a34 = A.a34 - B.a34;
    mOut.a41 = A.a41 - B.a41; mOut.a42 = A.a42 - B.a42; mOut.a43 = A.a43 - B.a43; mOut.a44 = A.a44 - B.a44;

    return mOut;
}

Matrix4x4 Matrix4x4::Transpose(const Matrix4x4 &Matrix)
{
    Matrix4x4 mOut = Matrix;

    std::swap(mOut.a21, mOut.a12);
    std::swap(mOut.a31, mOut.a13);
    std::swap(mOut.a41, mOut.a14);
    std::swap(mOut.a41, mOut.a14);
    std::swap(mOut.a32, mOut.a23);
    std::swap(mOut.a42, mOut.a24);
    std::swap(mOut.a43, mOut.a34);

    return mOut;
}

Matrix4x4 Matrix4x4::Mul(const Matrix4x4 &Matrix, float Val)
{
    Matrix4x4 mOut = Matrix;

    mOut.a11 *= Val; mOut.a12 *= Val; mOut.a13 *= Val; mOut.a14 *= Val;
    mOut.a21 *= Val; mOut.a22 *= Val; mOut.a23 *= Val; mOut.a24 *= Val;
    mOut.a31 *= Val; mOut.a32 *= Val; mOut.a33 *= Val; mOut.a34 *= Val;
    mOut.a41 *= Val; mOut.a42 *= Val; mOut.a43 *= Val; mOut.a44 *= Val;

    return mOut;
}

Matrix4x4 Matrix4x4::Mul(const Matrix4x4 &A, const Matrix4x4 &B)
{
    Matrix4x4 mOut;

    mOut.a11 = A.a11 * B.a11 + A.a12 * B.a21 + A.a13 * B.a31 + A.a14 * B.a41;
    mOut.a12 = A.a11 * B.a12 + A.a12 * B.a22 + A.a13 * B.a32 + A.a14 * B.a42;
    mOut.a13 = A.a11 * B.a13 + A.a12 * B.a23 + A.a13 * B.a33 + A.a14 * B.a43;
    mOut.a14 = A.a11 * B.a14 + A.a12 * B.a24 + A.a13 * B.a34 + A.a14 * B.a44;

    mOut.a21 = A.a21 * B.a11 + A.a22 * B.a21 + A.a23 * B.a31 + A.a24 * B.a41;
    mOut.a22 = A.a21 * B.a12 + A.a22 * B.a22 + A.a23 * B.a32 + A.a24 * B.a42;
    mOut.a23 = A.a21 * B.a13 + A.a22 * B.a23 + A.a23 * B.a33 + A.a24 * B.a43;
    mOut.a24 = A.a21 * B.a14 + A.a22 * B.a24 + A.a23 * B.a34 + A.a24 * B.a44;

    mOut.a31 = A.a31 * B.a11 + A.a32 * B.a21 + A.a33 * B.a31 + A.a34 * B.a41;
    mOut.a32 = A.a31 * B.a12 + A.a32 * B.a22 + A.a33 * B.a32 + A.a34 * B.a42;
    mOut.a33 = A.a31 * B.a13 + A.a32 * B.a23 + A.a33 * B.a33 + A.a34 * B.a43;
    mOut.a34 = A.a31 * B.a14 + A.a32 * B.a24 + A.a33 * B.a34 + A.a34 * B.a44;

    mOut.a41 = A.a41 * B.a11 + A.a42 * B.a21 + A.a43 * B.a31 + A.a44 * B.a41;
    mOut.a42 = A.a41 * B.a12 + A.a42 * B.a22 + A.a43 * B.a32 + A.a44 * B.a42;
    mOut.a43 = A.a41 * B.a13 + A.a42 * B.a23 + A.a43 * B.a33 + A.a44 * B.a43;
    mOut.a44 = A.a41 * B.a14 + A.a42 * B.a24 + A.a43 * B.a34 + A.a44 * B.a44;

    return mOut;
}

Vector4 Matrix4x4::Transform(const Vector4 &V) const
{
    Vector4 vOut;

    vOut.x = a11 * V.x + a21 * V.y + a31 * V.z + a41 * V.w;
    vOut.y = a12 * V.x + a22 * V.y + a32 * V.z + a42 * V.w;
    vOut.z = a13 * V.x + a23 * V.y + a33 * V.z + a43 * V.w;
    vOut.w = a14 * V.x + a24 * V.y + a34 * V.z + a44 * V.w;

    return vOut;
}

Vector4 Matrix4x4::Transform(const Matrix4x4 &Matrix, const Vector4 &V)
{
    return Matrix.Transform(V);
}

Matrix4x4 Matrix4x4::Translation(const Point3F &Pos)
{
    Matrix4x4 mOut;

    mOut.a41 = Pos.x;
    mOut.a42 = Pos.y;
    mOut.a43 = Pos.z;

    return mOut;
}

Matrix4x4 Matrix4x4::Scalling(const Size3F &Size)
{
    Matrix4x4 mOut;

    mOut.a11 = Size.width;
    mOut.a22 = Size.height;
    mOut.a33 = Size.depth;

    return mOut;
}

Matrix4x4 Matrix4x4::RotationX(float Angle)
{
    Matrix4x4 mOut;
    mOut.a32 = -sinf(Angle);
    mOut.a33 = cosf(Angle);

    mOut.a22 = cosf(Angle);
    mOut.a23 = sinf(Angle);

    return mOut;
}

Matrix4x4 Matrix4x4::RotationY(float Angle)
{
    Matrix4x4 mOut;

    mOut.a11 = cosf(Angle);
    mOut.a13 = -sinf(Angle);

    mOut.a31 = sinf(Angle);
    mOut.a33 = cosf(Angle);

    return mOut;
}

Matrix4x4 Matrix4x4::RotationZ(float Angle)
{
    Matrix4x4 mOut;

    mOut.a11 = cosf(Angle);
    mOut.a12 = sinf(Angle);

    mOut.a21 = -sinf(Angle);
    mOut.a22 = cosf(Angle);

    return mOut;
}

Matrix4x4 Matrix4x4::RotationYawPitchRoll(float Yaw, float Pitch, float Roll)
{
    float sRoll = sinf(Roll), cRoll = cosf(Roll);
    float sPitch = sinf(Pitch), cPitch = cosf(Pitch);
    float sYaw = sinf(Yaw), cYaw = cosf(Yaw);

    Matrix4x4 mOut;

    mOut.a11 = sRoll * sPitch * sYaw + cRoll * cYaw;
    mOut.a12 = sRoll * cPitch;
    mOut.a13 = sRoll * sPitch * cYaw - cRoll * sYaw;
    mOut.a14 = 0.0f;

    mOut.a21 = cRoll * sPitch * sYaw - sRoll * cYaw;
    mOut.a22 = cRoll * cPitch;
    mOut.a23 = cRoll * sPitch * cYaw + sRoll * sYaw;
    mOut.a24 = 0.0f;

    mOut.a31 = cPitch * sYaw;
    mOut.a32 = -sPitch;
    mOut.a33 = cPitch * cYaw;
    mOut.a34 = 0.0f;

    mOut.a41 = 0.0f;
    mOut.a42 = 0.0f;
    mOut.a43 = 0.0f;
    mOut.a44 = 1.0f;

    return mOut;
}

Matrix4x4 Matrix4x4::PerspectiveFovLH(FLOAT FOV, FLOAT NearZ, FLOAT FarZ, FLOAT AspectRatio)
{
    float tanFovPer2 = tanf(FOV * 0.5f);

    Matrix4x4 mOut;

    mOut.a11 = 1.0f / (AspectRatio * tanFovPer2);
    mOut.a12 = 0.0f;
    mOut.a13 = 0.0f;
    mOut.a14 = 0.0f;

    mOut.a21 = 0.0f;
    mOut.a12 = 1.0f / tanFovPer2;
    mOut.a13 = 0.0f;
    mOut.a14 = 0.0f;

    mOut.a21 = 0.0f;
    mOut.a12 = 0.0f;
    mOut.a13 = FarZ / (FarZ - NearZ);
    mOut.a14 = 1.0f;

    mOut.a21 = 0.0f;
    mOut.a12 = 0.0f;
    mOut.a13 = -((FarZ * NearZ) / (FarZ - NearZ));
    mOut.a14 = 0.0f;

    return mOut;
}

}