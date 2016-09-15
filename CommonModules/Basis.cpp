/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Basis.h>
#include <MathHelpers.h>

namespace Basis
{

const D3DXVECTOR3 &IBasis::GetDir() const
{
    if(needMatrixUpdate){
        UpdateMatrix();
        needMatrixUpdate = false;
    }

    return dir;
}

const D3DXVECTOR3 &IBasis::GetPos() const
{
    if(needMatrixUpdate){
        UpdateMatrix();
        needMatrixUpdate = false;
    }

    return pos;
}

const D3DXVECTOR3 &IBasis::GetUp() const
{
    if(needMatrixUpdate){
        UpdateMatrix();
        needMatrixUpdate = false;
    }

    return up;
}

const D3DXVECTOR3 &IBasis::GetRight() const
{
    if(needMatrixUpdate){
        UpdateMatrix();
        needMatrixUpdate = false;
    }

    return right;
}

const D3DXMATRIX &IBasis::GetMatrix() const
{
    if(needMatrixUpdate){
        UpdateMatrix();
        needMatrixUpdate = false;
    }

    return matrix;
}

static void GetBasisMatrix(const D3DXVECTOR3 &Dir, 
                           const D3DXVECTOR3 &Up, 
                           const D3DXVECTOR3 &Right, 
                           const D3DXVECTOR3 &Pos,
                           D3DXMATRIX &Matrix)
{
    Matrix._11 = Right.x;
    Matrix._12 = Right.y;
    Matrix._13 = Right.z;
    Matrix._14 = 0;

    Matrix._21 = Up.x;
    Matrix._22 = Up.y;
    Matrix._23 = Up.z;
    Matrix._24 = 0;

    Matrix._31 = Dir.x;
    Matrix._32 = Dir.y;
    Matrix._33 = Dir.z;
    Matrix._34 = 0;

    Matrix._41 = Pos.x;
    Matrix._42 = Pos.y;
    Matrix._43 = Pos.z;
    Matrix._44 = 1.0;
}

static void Rotate(const D3DXVECTOR3 &Axis, FLOAT Angle, D3DXVECTOR3 &Dir, D3DXVECTOR3 &Right, bool InverseCross)
{
    Dir = Math::Normalize(Math::RotationAxis(Dir, Axis, Angle));

    if(InverseCross)
        Right = Math::Normalize(Math::Cross(Axis, Dir));
    else
        Right = Math::Normalize(Math::Cross(Dir, Axis));
}

void UVNBasis::SetDir(const D3DXVECTOR3 &NewDir)
{
    if(Dir() != NewDir)
        SetNeedUpdate();

    Dir() = NewDir;
}

void UVNBasis::SetPos(const D3DXVECTOR3 &NewPos)
{
    if(Pos() != NewPos)
        SetNeedUpdate();

    Pos() = NewPos;
}

void UVNBasis::UpdateMatrix() const
{
    if (!NeedMatrixUpdate())
        return;

    DropNeedUpdate();

    D3DXVECTOR3 &pos = Pos(), &up = Up(), &dir = Dir(), &right = Right();

	D3DXVECTOR3 up2(0.0, 1.0, 0.0);

    if(invertUp && Math::Dot({dir.x, dir.y, 0.0f}, {1.0f, 0.0f, 0.0f}) > 0.0f)
        up2 = {0.0, -1.0, 0.0};

    right = Math::Normalize(Math::Cross(up2, dir));

    if(right.x == 0.0f && right.y == 0.0f && right.z == 0.0f)
        right = Math::Normalize(Math::Cross({0.0f, 0.0f, 1.0f}, dir));

    up = Math::Normalize(Math::Cross(dir, right));

    GetBasisMatrix(dir, up, right, pos, Matrix());

}

void AircraftPrincipalBasis::UpdateMatrix() const
{
    D3DXVECTOR3 &pos = Pos(), &up = Up(), &dir = Dir(), right = Right();

    GetBasisMatrix(dir, up, right, pos, Matrix());
}

void AircraftPrincipalBasis::Invalidate(FLOAT YawOffset, FLOAT PitchOffset, FLOAT RollOffset)
{
    D3DXVECTOR3 &pos = Pos(), &up = Up(), &dir = Dir(), right = Right();

    Rotate(up, YawOffset, dir, right, false);
    Rotate(right, PitchOffset, dir, up, true);
    Rotate(dir, RollOffset, up, right, false);
}

void AircraftPrincipalBasis::SetPos(const D3DXVECTOR3 &NewPos)
{
    if(Pos() != NewPos)
        SetNeedUpdate();

    Pos() = NewPos;
}


}