/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/RadioButton.h>
#include <GUI/Manager.h>
#include <DirectInput.h>

namespace GUI
{

void RadioButtonsGroup::AddRadioButton(RadioButton *NewRadioButton)
{
    radioButtons.push_back(NewRadioButton);

    NewRadioButton->AddEvent([this](const RadioButton *Source, BOOL NewState)
    {
        if(!NewState)
            return;

        for(INT i = 0; i < radioButtons.size(); i++)
            if(radioButtons[i] == Source)
                for(auto e: events) e.second(this, i);
    });
}

INT RadioButtonsGroup::GetSelectedIndex() throw (Exception)
{
    INT ind = 0;
    for(RadioButton *rb : radioButtons)
        if(rb->IsSelected())
            return ind;
        else 
            ind++;

    throw RadioButtonException("Invalid radioButtons group state");
}

void RadioButtonsGroup::SetSelectedIndex(INT Index) throw (Exception)
{
    if(Index < 0 || Index > radioButtons.size())
        throw RadioButtonException("Invalid selected index");

    radioButtons[Index]->SetSelected();
}

void RadioButton::SetSelected()
{
    if(isSelected)
        return;

    SetTexture(selectedTex);
    SetTexCoordBounds(selectedTexBounds);

    for(auto evnt: events)
        evnt.second(this, true);

    isSelected = true;

    for(RadioButton *btn : *group)
        if(btn != this){
            btn->SetTexture(unselectedTex);
            btn->SetTexCoordBounds(unselectedTexBounds);

            for(auto evnt: btn->events)
                evnt.second(btn, false);

            btn->isSelected = false;
        }
}

void RadioButton::Init(RadioButtonsGroup *Group) throw (Exception)
{
    group = Group;
    group->AddRadioButton(this);

    const ControlView &controlView = Manager::GetInstance()->GetTheme().GetControlView("RadioButton");
    
    const ControlView::ElementData &selData = controlView.GetElement("selected");
    selectedTex = Manager::GetInstance()->GetTexture(selData.textureFileName);
    selectedTexBounds = selData.textureBounds;

    const ControlView::ElementData &unselData = controlView.GetElement("unselected");
    unselectedTex = Manager::GetInstance()->GetTexture(unselData.textureFileName);
    unselectedTexBounds = unselData.textureBounds;
    
    SetTexCoordBounds(unselectedTexBounds);
    SetSize(unselData.screenSize);
    SetTexture(unselectedTex);
    SetPositioning(CNTRL_POS_BOTTOM_RIGHT);
}

void RadioButton::SetPos(const D3DXVECTOR2 &NewPos)
{
    Control::SetPos(NewPos);
    Control::FixPos();
}

void RadioButton::Invalidate(const D3DXVECTOR2 &CursorPos, FLOAT Tf)
{
    const D3DXVECTOR2 &pos = GetPos();
    const D3DXVECTOR2 &btmRightPos = GetBottomRightPos();
   
    bool inRange = CursorPos.x >= pos.x && CursorPos.x <= btmRightPos.x &&
                   CursorPos.y >= pos.y && CursorPos.y <= btmRightPos.y;

    if(inRange && DirectInput::GetInsance()->IsMouseDown(0))
        Manager::GetInstance()->TryToLockControl(this);
    else if(!DirectInput::GetInsance()->IsMouseDown(0))
        Manager::GetInstance()->UnlockControl(this);

    if(inRange && DirectInput::GetInsance()->IsMousePress(0) && Manager::GetInstance()->GetLockedControl() == NULL)
        SetSelected();
}

}
