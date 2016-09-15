/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once 
#include <D3DHeaders.h>
#include <Basis.h>

namespace Camera
{

class ICamera
{
public:
	virtual ~ICamera(){}
    virtual const D3DXMATRIX &GetViewMatrix() const = 0;
    virtual const D3DXMATRIX &GetProjMatrix() const = 0;
	virtual const D3DXVECTOR3 &GetPos() const = 0;
    virtual void Invalidate(float Tf) {}
};

class EyeCamera : 
    private Basis::UVNBasis,
	public ICamera
{
private:
	D3DXVECTOR2 angles;
    BOOL isFlying;
    mutable D3DXMATRIX invertMatrix;
    D3DXMATRIX projMatrix;
    float speed;
public:
    EyeCamera():isFlying(false), angles(0,0), speed(1.0f){}
    float GetSpeed() const {return speed;} 
    void SetSpeed(float Speed) {speed = Speed;}
    BOOL IsFlying() const {return isFlying;}
    void SetFlyingMode(BOOL IsFlying) {isFlying = IsFlying;}
    const D3DXVECTOR3 &GetDir() const {return UVNBasis::GetDir();}
    virtual const D3DXVECTOR3 &GetPos() const {return UVNBasis::GetPos();}
    virtual const D3DXVECTOR3 &GetUp() const {return UVNBasis::GetUp();}
    void SetDir(const D3DXVECTOR3 &Dir);
    void SetPos(const D3DXVECTOR3 &Pos){UVNBasis::SetPos(Pos);}  
    void SetProjMatrix(const D3DXMATRIX &ProjMatrix) { projMatrix = ProjMatrix; }
    virtual const D3DXMATRIX &GetProjMatrix() const { return projMatrix; }
	virtual const D3DXMATRIX &GetViewMatrix() const;
	virtual void Invalidate(float Tf);
};

class TargetCamera : public ICamera
{
private:
    D3DXVECTOR3 pos = {0.0f, 0.0f, 0.0f};
    D3DXVECTOR3 target = {0.0f, 0.0f, 0.0f};
    D3DXVECTOR3 up = {0.0f, 1.0f, 0.0f};
    D3DXMATRIX projMatrix, viewMatrix;
public:
    void SetTarget(const D3DXVECTOR3 &NewTarget);
    const D3DXVECTOR3 &GetTarget() const {return target;}
    void SetPos(const D3DXVECTOR3 &NewPos);
    virtual const D3DXVECTOR3 &GetPos() const {return pos;}
    void SetUp(const D3DXVECTOR3 &NewUp);
    const D3DXVECTOR3 &GetUp() const {return up;}
    void SetProjMatrix(const D3DXMATRIX &ProjMatrix) { projMatrix = ProjMatrix; }
    virtual const D3DXMATRIX &GetProjMatrix() const { return projMatrix; }
    virtual const D3DXMATRIX &GetViewMatrix() const { return viewMatrix; }
};

}