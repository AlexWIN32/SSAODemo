/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <GUI/Image.h>
#include <Utils/EventsStorage.h>

namespace GUI
{

DECLARE_EXCEPTION(CheckBoxException);

class CheckBox : 
    public Image,
    public Utils::EventsStorage<void(const CheckBox*, INT)>
{
private:
    D3DXVECTOR4 checkedTexBounds, uncheckedTexBounds;
    ID3D11ShaderResourceView *chkTex, *unchkTex;
    bool isChecked;
    void SetSize(const D3DXVECTOR2 &NewSize){Control::SetSize(NewSize);}
public:
    CheckBox() : chkTex(NULL), unchkTex(NULL), isChecked(false){}
    void Init() throw (Exception);
    bool IsChecked() const {return isChecked;}
    void SetChecked(bool IsChecked);
    virtual void SetPos(const D3DXVECTOR2 &NewPos);
    virtual void Invalidate(const D3DXVECTOR2 &CursorPos, FLOAT Tf);
};

}