/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/Button.h>
#include <GUI/Manager.h>
#include <GUI/Label.h>
#include <DirectInput.h>
#include <CommonParams.h>

namespace GUI
{

void Button::Init(const std::string &ControlName, Area::ResizingType Resizing) throw (Exception)
{
    const ControlView &controlView = Manager::GetInstance()->GetTheme().GetControlView(ControlName);

    idlArea.Init(controlView.GetParameter("idleArea"), Resizing);

    dwnArea.Init(controlView.GetParameter("downedArea"), Resizing);

    AddControl(&idlArea);
    SetSize(idlArea.GetSize());
}

void Button::SetText(const std::wstring &Text)
{
    Label *lbl = dynamic_cast<Label*>(captionControl);
    if(!lbl){
        RemoveControl(captionControl);
        delete captionControl;

        lbl = new Label();
        lbl->Init(&Manager::GetInstance()->GetTheme().GetFont());

        AddControl(lbl);

        captionControl = lbl;
    }

    lbl->SetCaption(Text);
    lbl->SetPos(GetPos() + idlArea.GetTopLeftOffset());
    auto pos = lbl->GetPos();

    const D3DXVECTOR2 &lblSize = lbl->GetSize();

    idlArea.SetSpaceAreaSize(lblSize);
    dwnArea.SetSpaceAreaSize(lblSize);

    minSize = idlArea.GetSize();
}

void Button::SetImage(const ImageData& Image)
{
    GUI::Image *img = dynamic_cast<GUI::Image*>(captionControl);
    if(!img){
        RemoveControl(captionControl);
        delete captionControl;

        img = new GUI::Image();

        AddControl(img);

        captionControl = img;
    }

    img->SetPos(GetPos() + idlArea.GetTopLeftOffset());
    img->SetImage(Image.textureFileName);
    img->SetSize(Image.screenSize);
    img->SetTexCoordBounds(Image.textureBounds);

    idlArea.SetSpaceAreaSize(img->GetSize());
    dwnArea.SetSpaceAreaSize(img->GetSize());

    minSize = idlArea.GetSize();
}

void Button::SetPos(const D3DXVECTOR2 &NewPos)
{
    D3DXVECTOR2 newPos = {Control::CheckPixelErrorForX(NewPos.x), Control::CheckPixelErrorForY(NewPos.y)};
    D3DXVECTOR2 delta = newPos - GetPos();

    idlArea.SetPos(idlArea.GetPos() + delta);
    dwnArea.SetPos(dwnArea.GetPos() + delta);
    if(captionControl)
        captionControl->SetPos(captionControl->GetPos() + delta);
}

void Button::SetSize(const D3DXVECTOR2 &NewSize)
{
    if(NewSize.x < minSize.x || NewSize.y < minSize.y)
        return;

    D3DXVECTOR2 newSize = {Control::CheckPixelErrorForX(NewSize.x), Control::CheckPixelErrorForY(NewSize.y)};

    idlArea.SetSize(newSize);
    dwnArea.SetSize(newSize);

    if(captionControl){
        const D3DXVECTOR2 &spcAreaSz = dwnArea.GetSpaceAreaSize();
        const D3DXVECTOR2 &capSize = captionControl->GetSize();
    
        D3DXVECTOR2 hSzOffst = (spcAreaSz - capSize) * 0.5f;
        hSzOffst.x = Control::WigthToX(hSzOffst.x, true);
        hSzOffst.y = Control::CheckPixelErrorForY(hSzOffst.y);

        captionControl->SetPos(GetPos() + idlArea.GetTopLeftOffset() + hSzOffst);
    }
}

void Button::Invalidate(const D3DXVECTOR2 &CursorPos, float Tf)
{
    if(captionControl)
        captionControl->Invalidate(CursorPos, Tf);

    const D3DXVECTOR2 &pos = GetPos(), &size = GetSize();

    D3DXVECTOR2 szOffset(Control::WigthToX(size.x), size.y);
    D3DXVECTOR2 capOffset(1 / CommonParams::GetScreenWidth(), 1 / CommonParams::GetScreenHeight());

    bool inRange = CursorPos.x >= pos.x && CursorPos.x <= pos.x + szOffset.x &&
                   CursorPos.y >= pos.y && CursorPos.y <= pos.y + szOffset.y;

    if(inRange && DirectInput::GetInsance()->IsMouseDown(0) && state == ST_IDLE){
        if(Manager::GetInstance()->TryToLockControl(this)){
            state = ST_DOWN;
        
            ClearControls();
            AddControl(&dwnArea);

            if(captionControl){
                AddControl(captionControl);
                captionControl->SetPos(captionControl->GetPos() + capOffset);
            }

            OnDown(CursorPos, Tf);
        }

    }else if((!inRange || !DirectInput::GetInsance()->IsMouseDown(0)) && state == ST_DOWN){
        if(!inRange)
            state = ST_IDLE;
        else 
            state = ST_PRESS;
        
        Manager::GetInstance()->UnlockControl(this);

        ClearControls();
        AddControl(&idlArea);

        if(captionControl){
            AddControl(captionControl);
            captionControl->SetPos(captionControl->GetPos() - capOffset);
        }

        OnUp(CursorPos, Tf);
    }

    if(state == ST_PRESS){
        state = ST_IDLE;
        for(auto evnt: events)
            evnt.second(this);
    }
}

}
