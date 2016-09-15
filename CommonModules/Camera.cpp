/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Camera.h>
#include <DirectInput.h>
#include <MathHelpers.h>

namespace Camera
{

void EyeCamera::SetDir(const D3DXVECTOR3 &Dir)
{
    angles.x = Math::DirectionToAngle({Dir.x, Dir.z});
    angles.y = Dir.y; 
    UVNBasis::SetDir(Dir);
}

void EyeCamera::Invalidate(float Tf)
{
	DirectInput::MouseState mState = DirectInput::GetInsance()->GetMouseDelta();	

	if (angles.y >= D3DX_PI * 0.5)
		angles.y = D3DX_PI * 0.5;

	if (angles.y <= -D3DX_PI * 0.5)
		angles.y = -D3DX_PI * 0.5;

	if (mState.x || mState.y){
        angles -= D3DXVECTOR2(mState.x, mState.y) / 80.0f;

        D3DXVECTOR3 newDir(cosf(angles.x), angles.y, sinf(angles.x));
		D3DXVec3Normalize(&newDir, &newDir);
		UVNBasis::SetDir(newDir);
	}

    float moveFactor = 0.0f;
    if(DirectInput::GetInsance()->IsKeyboardDown(DIK_W))
        moveFactor = 1.0f;

    if(DirectInput::GetInsance()->IsKeyboardDown(DIK_S))
        moveFactor = -1.0f;

    if(moveFactor != 0.0f){
        if(isFlying)
            SetPos(GetPos() + GetDir() * Tf * moveFactor * speed);
        else{                                    
            D3DXVECTOR3 xzPlaneDir(GetDir().x, 0.0f, GetDir().z);            
            SetPos(GetPos() + xzPlaneDir * Tf * moveFactor * speed);
        }
    }

    moveFactor = 0.0f;
    if(DirectInput::GetInsance()->IsKeyboardDown(DIK_D))
        moveFactor = 1.0f;

    if(DirectInput::GetInsance()->IsKeyboardDown(DIK_A))
        moveFactor = -1.0f;

    if(moveFactor != 0.0f){
        GetViewMatrix();
        SetPos(GetPos() + GetRight()* Tf * moveFactor * speed);
    }
}

const D3DXMATRIX &EyeCamera::GetViewMatrix() const
{
    const D3DXMATRIX &mat = UVNBasis::GetMatrix();
    D3DXMatrixInverse(&invertMatrix, NULL, &mat);
    return invertMatrix;
}

void TargetCamera::SetTarget(const D3DXVECTOR3 &NewTarget)
{
    if(target != NewTarget){
        target = NewTarget;
        D3DXMatrixLookAtLH(&viewMatrix, &pos, &target, &up);
    }
}

void TargetCamera::SetPos(const D3DXVECTOR3 &NewPos)
{
    if(pos != NewPos){
        pos = NewPos;
        D3DXMatrixLookAtLH(&viewMatrix, &pos, &target, &up);
    }
}

void TargetCamera::SetUp(const D3DXVECTOR3 &NewUp)
{
    if(up != NewUp){
        up = NewUp;
        D3DXMatrixLookAtLH(&viewMatrix, &pos, &target, &up);
    }
}

}