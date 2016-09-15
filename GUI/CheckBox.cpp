/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/CheckBox.h>
#include <GUI/Manager.h>
#include <DirectInput.h>

namespace GUI
{

void CheckBox::Init() throw (Exception)
{
    const ControlView &controlView = Manager::GetInstance()->GetTheme().GetControlView("CheckBox");
    
    const ControlView::ElementData &chkData = controlView.GetElement("checked");
    chkTex = Manager::GetInstance()->GetTexture(chkData.textureFileName);
    checkedTexBounds = chkData.textureBounds;

    const ControlView::ElementData &unchkData = controlView.GetElement("unchecked");
    unchkTex = Manager::GetInstance()->GetTexture(unchkData.textureFileName);
    uncheckedTexBounds = unchkData.textureBounds;
    
    SetTexCoordBounds(uncheckedTexBounds);
    SetSize(unchkData.screenSize);
    SetTexture(unchkTex);
    SetPositioning(CNTRL_POS_BOTTOM_RIGHT);
}

void CheckBox::SetChecked(bool IsChecked)
{
    if(IsChecked == isChecked)
        return;

    SetTexture(IsChecked ? chkTex : unchkTex);
    SetTexCoordBounds(IsChecked ? checkedTexBounds : uncheckedTexBounds);

    for(auto evnt: events)
        evnt.second(this, IsChecked);

    isChecked = IsChecked;
}

void CheckBox::SetPos(const D3DXVECTOR2 &NewPos)
{
    Control::SetPos(NewPos);
    Control::FixPos();
}

void CheckBox::Invalidate(const D3DXVECTOR2 &CursorPos, FLOAT Tf)
{
    bool inRange = CursorPos.x >= GetPos().x && CursorPos.x < GetBottomRightPos().x &&
                   CursorPos.y >= GetPos().y && CursorPos.y < GetBottomRightPos().y;

    if(inRange && DirectInput::GetInsance()->IsMouseDown(0))
        Manager::GetInstance()->TryToLockControl(this);
    else if(!DirectInput::GetInsance()->IsMouseDown(0))
        Manager::GetInstance()->UnlockControl(this);

    if(inRange && DirectInput::GetInsance()->IsMousePress(0) && Manager::GetInstance()->GetLockedControl() == NULL){
        bool newState = !isChecked;

        SetTexture(newState ? chkTex : unchkTex);
        SetTexCoordBounds(newState ? checkedTexBounds : uncheckedTexBounds);

        for(auto evnt: events)
            evnt.second(this, newState);

        isChecked = newState;
    }
}

}
