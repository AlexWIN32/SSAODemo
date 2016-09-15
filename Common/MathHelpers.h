/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <D3DHeaders.h>
#include <Matrix3x3.h>
#include <Matrix4x4.h>
#include <Vector2.h>

namespace Math
{

template<class T>
inline T Lerp(const T &A, const T& B, float Factor)
{
    return A + (B - A) * Factor;
}

template <class T>
inline T Sign(T Val)
{
    return Val < 0.0f ? -1 : 1;
}

template <class T>
inline const T &Min(const T &A, const T &B)
{
    return A < B ? A : B;
}

template <class T>
inline BasePoint2<T> Abs(const BasePoint2<T> &Point)
{
    return {(T)abs(Point.x), (T)abs(Point.y)};
}

template <class T>
inline Size<T> Abs(const Size<T> &Size)
{
    return {(T)abs(Size.width), (T)abs(Size.height)};
}

inline float Rand(float min, float max)
{
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

inline INT Rand(INT Min, INT Max)
{
    return Min + rand() % (Max - Min);
}

inline float RandSNorm()
{
    return 2.0f * Rand(0.0f, 1.0f) - 1.0f;
}

inline float Round(float Val)
{
    float btm = floor(Val);

    return abs(Val - btm) >= 0.5f ? ceil(Val) : btm;
}

inline D3DXMATRIX Inverse(const D3DXMATRIX &Matrix)
{
    D3DXMATRIX inv;    
    D3DXMatrixInverse(&inv, NULL, &Matrix);
    return inv;
}

inline D3DXMATRIX Transpose(const D3DXMATRIX &Matrix)
{
    D3DXMATRIX trans;
    D3DXMatrixTranspose(&trans, &Matrix);
    return trans;
}

inline D3DXVECTOR3 Normalize(const D3DXVECTOR3 &Vector)
{
    D3DXVECTOR3 vOut;
    D3DXVec3Normalize(&vOut, &Vector);
    return vOut;
}

inline D3DXVECTOR3 Cross(const D3DXVECTOR3 &A, const D3DXVECTOR3 &B)
{
    D3DXVECTOR3 vOut;
    D3DXVec3Cross(&vOut, &A, &B);
    return vOut;
}

inline FLOAT Dot(const D3DXVECTOR3 &A, const D3DXVECTOR3 &B)
{
    return D3DXVec3Dot(&A, &B);
}

inline FLOAT Dot(const D3DXVECTOR4 &A, const D3DXVECTOR4 &B)
{
    return D3DXVec4Dot(&A, &B);
}

inline D3DXVECTOR3 RotationAxis(const D3DXVECTOR3 &Point,
                                const D3DXVECTOR3 &Axis,
                                FLOAT Angle,
                                FLOAT UpVectorFactor = 1.0f,
                                FLOAT RightVectorFactor = 1.0f)
{
    D3DXVECTOR3 axisProj = Axis * Dot(Axis, Point);

    D3DXVECTOR3 right = Point - axisProj;

    D3DXVECTOR3 up = Cross(Axis, right);

    return right * cosf(Angle) * RightVectorFactor + up * sinf(Angle) * UpVectorFactor;
}

inline D3DXVECTOR3 TransformCoord(const D3DXVECTOR3 &A, const D3DXMATRIX &Matrix)
{
    D3DXVECTOR3 vOut;
    D3DXVec3TransformCoord(&vOut, &A, &Matrix);
    return vOut;
}

inline D3DXVECTOR3 TransformNormal(const D3DXVECTOR3 &A, const D3DXMATRIX &Matrix)
{
    D3DXVECTOR3 vOut;
    D3DXVec3TransformNormal(&vOut, &A, &Matrix);
    return vOut;
}

inline FLOAT Length(const D3DXVECTOR3 &V)
{
    return D3DXVec3Length(&V);
}

inline FLOAT Length(const D3DXVECTOR2 &V)
{
    return D3DXVec2Length(&V);
}

inline D3DXVECTOR4 Transform(const D3DXVECTOR4 &A, const D3DXMATRIX &Matrix)
{
    D3DXVECTOR4 vOut;
    D3DXVec4Transform(&vOut, &A, &Matrix);
    return vOut;
}

inline D3DXMATRIX RotationYawPitchRoll(const D3DXVECTOR3 &Rotation)
{
    D3DXMATRIX mRot;
    D3DXMatrixRotationYawPitchRoll(&mRot, Rotation.y, Rotation.x, Rotation.z);
    return mRot;
}

inline D3DXMATRIX PerspectiveFovLH(FLOAT FOV, FLOAT NearZ, FLOAT FarZ, FLOAT AspectRation)
{
    D3DXMATRIX projMatrix;
    D3DXMatrixPerspectiveFovLH(&projMatrix, FOV, AspectRation, NearZ, FarZ);
    return projMatrix;
}

inline D3DXMATRIX Identity()
{
    D3DXMATRIX mI;
    D3DXMatrixIdentity(&mI);
    return mI;
}

inline D3DXMATRIX RotationX(float Angle)
{
    D3DXMATRIX mRot;
    D3DXMatrixRotationX(&mRot, Angle);
    return mRot;
}

inline D3DXMATRIX RotationY(float Angle)
{
    D3DXMATRIX mRot;
    D3DXMatrixRotationY(&mRot, Angle);
    return mRot;
}

inline D3DXMATRIX RotationZ(float Angle)
{
    D3DXMATRIX mRot;
    D3DXMatrixRotationZ(&mRot, Angle);
    return mRot;
}

inline D3DXMATRIX Translation(const D3DXVECTOR3 &Translation)
{
    D3DXMATRIX mTrans;
    D3DXMatrixTranslation(&mTrans, Translation.x, Translation.y, Translation.z);
    return mTrans;
}

inline float Saturate(float Val)
{
    if(Val > 1.0f)
        return 1.0f;
    else if(Val < 0.0f)
        return 0.0f;
    else 
        return Val;
}

inline FLOAT DirectionToAngle(const D3DXVECTOR2 &Direction)
{
    FLOAT angle = atan(Direction.y / Direction.x);

    if(Direction.x < 0)
        angle = D3DX_PI + angle;

    return angle;
}

inline Vector3 SphericalToDec(float AngleX, float AngleY, float Radius = 1.0f)
{
    Vector3 out;
    out.x = cosf(AngleX) * sinf(AngleY) * Radius;
    out.y = cosf(AngleY) * Radius;
    out.z = sinf(AngleX) * sinf(AngleY) * Radius;

    return out;
}

}