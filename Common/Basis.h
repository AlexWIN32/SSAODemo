/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once 
#include <D3DHeaders.h>

namespace Basis
{

class IBasis
{
private:
    mutable D3DXVECTOR3 pos = {0.0f, 0.0f, 0.0f};
    mutable D3DXVECTOR3 dir = {1.0f, 0.0f, 0.0f};
    mutable D3DXVECTOR3 up = {0.0f, 1.0f, 0.0f};
    mutable D3DXVECTOR3 right = {0.0f, 0.0f, 1.0f};
    mutable D3DXMATRIX matrix;
    mutable bool needMatrixUpdate = false;
protected:
    D3DXVECTOR3 &Pos() const {return pos;}
    D3DXVECTOR3 &Dir() const {return dir;}
    D3DXVECTOR3 &Up() const {return up;}
    D3DXVECTOR3 &Right() const {return right;}
    D3DXMATRIX &Matrix() const {return matrix;}
    void SetNeedUpdate() {needMatrixUpdate = true;}
    void DropNeedUpdate() const {needMatrixUpdate = false;}
    bool NeedMatrixUpdate() const {return needMatrixUpdate;}
public:
    virtual ~IBasis(){}
    virtual void UpdateMatrix() const = 0;
    const D3DXVECTOR3 &GetDir() const;
    const D3DXVECTOR3 &GetPos() const;
    const D3DXVECTOR3 &GetUp() const;
    const D3DXVECTOR3 &GetRight() const;
    const D3DXMATRIX &GetMatrix() const;
};

class UVNBasis : public IBasis
{
private:
    bool invertUp = false;
public:
    virtual ~UVNBasis(){}
    virtual void UpdateMatrix() const;
    void SetInvertUp(bool InvertUp){invertUp = InvertUp;}
    bool GetInvertUp() const {return invertUp;}
	void SetDir(const D3DXVECTOR3 &Dir);
    void SetPos(const D3DXVECTOR3 &Pos);    
};

class AircraftPrincipalBasis : public IBasis
{
public:
    virtual ~AircraftPrincipalBasis(){}
    void SetPos(const D3DXVECTOR3 &NewPos);
    virtual void UpdateMatrix() const;
    virtual void Invalidate(FLOAT YawOffset, FLOAT PitchOffset, FLOAT RollOffset);
};

}