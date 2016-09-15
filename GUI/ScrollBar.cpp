/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/ScrollBar.h>
#include <DirectInput.h>
#include <CommonParams.h>
#include <GUI/Manager.h>
#include <Serializing.h>
#include <Utils.h>
#include <MathHelpers.h>
#include <memory>

namespace GUI
{

void ScrollBar::Roller::Init(ScrollBar *Owner, const ControlView &ControlView) throw (Exception)
{    
    std::string data;
    if(ControlView.FindParameter("rollerSize", data)){

        Init("Button", Area::RT_WIDTH_AND_HEIGHT);

        SetSize(Cast<D3DXVECTOR2>(SizeFParser::FromString(data)) / CommonParams::GetScreenHeight());
    }else{
        
        Init("Button", (Owner->GetType() == SB_TYPE_VERTICAL) ? Area::RT_WIDTH_ONLY : Area::RT_HEIGHT_ONLY);
        
        float length = FloatParser::FromString(ControlView.GetParameter("rollerLength"));
        SetLength(length / CommonParams::GetScreenHeight());
    }

    owner = Owner;
}

void ScrollBar::Roller::Invalidate(const D3DXVECTOR2 &CursorPos, FLOAT Tf)
{
    bool inRange = CursorPos.x >= GetPos().x && CursorPos.x <= GetBottomRightPos().x &&
                   CursorPos.y >= GetPos().y && CursorPos.y <= GetBottomRightPos().y;
    
    bool mouseDwn = DirectInput::GetInsance()->IsMouseDown(0);

    if(state == ST_IDLE){

        if(inRange && mouseDwn && Manager::GetInstance()->TryToLockControl(this)){
            state = ST_DOWN;
        
            ClearControls();
            AddControl(&dwnArea);
        }
    }else if(!mouseDwn){
        state = ST_IDLE;

        Manager::GetInstance()->UnlockControl(this);

        ClearControls();
        AddControl(&idlArea);
    }

    if(state == ST_DOWN){
        D3DXVECTOR2 hSizeOfst = {Control::WigthToX(GetSize().x) * 0.5f, GetSize().y * 0.5f};
        D3DXVECTOR2 pos = GetPos() + hSizeOfst;

        float step = 0.0f, delta = 0.0f;
        if(owner->type == SB_TYPE_HORISONTAL){
            step = Control::WigthToX(owner->GetSize().x) * owner->factorStep;
            delta = CursorPos.x - pos.x;
        }else{
            step = owner->GetSize().y * owner->factorStep;
            delta = CursorPos.y - pos.y;
        }

        float steps = floorf(abs(delta) / step);
        if(steps != 0.0f){
            float newFactor = Math::Saturate(owner->factor + (owner->factorStep * steps * Math::Sign(delta)));
            owner->SetFactor(newFactor);
        }
    }
}

void ScrollBar::Roller::SetLength(FLOAT NewLength, bool InPixels)
{
    if(owner->GetType() == SB_TYPE_VERTICAL){
        if(InPixels)
            NewLength /= CommonParams::GetScreenWidth();

        SetSize({NewLength, GetSize().y});
    }else{ 
        if(InPixels)
            NewLength /= CommonParams::GetScreenHeight();

        SetSize({GetSize().x, NewLength});
    }
}

void ScrollBar::SetPos(const D3DXVECTOR2 &NewPos)
{    
    D3DXVECTOR2 delta = NewPos - GetPos();
    roller.SetPos(roller.GetPos() + delta);
    bgControl->SetPos(bgControl->GetPos() + delta);
    Control::SetPos(NewPos);
}

void ScrollBar::SetLength(FLOAT NewLength) throw (Exception)
{
    const D3DXVECTOR2 &pos = GetPos();

    if(type == SB_TYPE_HORISONTAL){
        if(NewLength <= roller.GetSize().x * 2.0f)
            throw ScrollBarException("invalid length for ScrollBar");

        bgControl->SetSize({NewLength, roller.GetSize().y});

        float widthOffset = Control::WigthToX(GetSize().x - roller.GetSize().x);

        float newX = Control::CheckPixelErrorForX(pos.x + (widthOffset * factor));
        roller.SetPos({newX, roller.GetPos().y});

        Control::SetSize(D3DXVECTOR2(NewLength, bgControl->GetSize().y));
    }else{
        if(NewLength <= roller.GetSize().y * 2.0f)
            throw ScrollBarException("invalid length for ScrollBar");

        bgControl->SetSize({roller.GetSize().x, NewLength});

        float heightOffset = GetSize().y - roller.GetSize().y;

        float newY = Control::CheckPixelErrorForY(pos.y + (heightOffset * factor));
        roller.SetPos({roller.GetPos().x, newY});

        Control::SetSize(D3DXVECTOR2(bgControl->GetSize().x, NewLength));
    }
}

void ScrollBar::SetRollerLength(FLOAT Length) throw (Exception)
{
    const D3DXVECTOR2 &rollerMinSize = roller.GetMinSize();

    if(type == SB_TYPE_HORISONTAL && Length < rollerMinSize.y)
        throw ScrollBarException("invalid roller length");

    if(type == SB_TYPE_VERTICAL && Length < rollerMinSize.x)
        throw ScrollBarException("invalid roller length");
    
    roller.SetLength(Length);

    if(type == SB_TYPE_VERTICAL){
        bgControl->SetSize({Length, bgControl->GetSize().y});
        Control::SetSize({Length, bgControl->GetSize().y});
    }else{
        bgControl->SetSize({bgControl->GetSize().x, Length});
        Control::SetSize({bgControl->GetSize().x, Length});
    }
}

void ScrollBar::SetFactor(FLOAT NewFactor) throw (Exception)
{
    if(NewFactor < 0 || NewFactor > 1.0f)
        throw ScrollBarException("Invalid factor");

    if(NewFactor != factor)
        SetNewFactor(NewFactor);
}

void ScrollBar::SetNewLimits(FLOAT MinVal, FLOAT MaxVal) throw (Exception)
{
    if(MinVal >= MaxVal)
        throw ScrollBarException("invalid limits for ScrollBar");

    float oldArea = maxVal - minVal, newArea = MaxVal - MinVal;

    minVal = MinVal;
    maxVal = MaxVal;

    float aspect = oldArea / newArea;
    factorStep *= aspect;

    SetNewFactor((newArea > oldArea) ? factor * aspect : factor);

}

void ScrollBar::SetNewFactor(FLOAT Factor) throw (Exception)
{
    if(Factor < 0.0f || Factor > 1.0f)
        throw ScrollBarException("invalid value for ScrollBar");

    D3DXVECTOR2 newRollerPos = roller.GetPos();

    if(type == SB_TYPE_HORISONTAL){

        float widthOffset = Control::WigthToX(GetSize().x - roller.GetSize().x);
        newRollerPos.x = Control::CheckPixelErrorForX(GetPos().x + (widthOffset * Factor));

    }else{

        float heightOffset = GetSize().y - roller.GetSize().y;
        newRollerPos.y = Control::CheckPixelErrorForY(GetPos().y + (heightOffset * Factor));
    }

    roller.SetPos(newRollerPos);

    for(auto pair : events)
        pair.second(this, Factor);

    factor = Factor;
}

void ScrollBar::SetStep(FLOAT Step) throw (Exception)
{
    if(Step <= 0.0f)
        throw ScrollBarException("invalid step for ScrollBar");

    factorStep = Step / (maxVal - minVal);
}

FLOAT ScrollBar::SetValue(FLOAT Value) throw (Exception)
{
    if(Value < minVal || Value > maxVal)
        throw ScrollBarException("invalid value for ScrollBar");

    SetNewFactor((Value - minVal) / (maxVal - minVal));
}

void ScrollBar::Init(Type ScrollBarType)
{
    std::string controlName = (ScrollBarType == SB_TYPE_VERTICAL) ? "VerticalScrollBar" : "HorisontalScrollBar";
    Init(ScrollBarType, Manager::GetInstance()->GetTheme().GetControlView(controlName));
}

void ScrollBar::Init(Type ScrollBarType, const ControlView &SBControlView)
{
    type = ScrollBarType;

    std::string areaName;
    if(SBControlView.FindParameter("bgArea", areaName)){

        std::unique_ptr<Area> areaPtr(new Area());

        areaPtr->Init(areaName);
        bgControl = areaPtr.release();
    }else{

        std::unique_ptr<Image> imgPtr(new Image());

        const ControlView::ElementData &lineData = SBControlView.GetElement("line");
        imgPtr->Init(lineData.textureFileName);
        imgPtr->SetPositioning(CNTRL_POS_BOTTOM_RIGHT);
        imgPtr->SetSize(lineData.screenSize);
        imgPtr->SetTexCoordBounds(lineData.textureBounds);

        bgControl = imgPtr.release();
    }

    roller.Init(this, SBControlView);
    
    AddControl(bgControl);
    AddControl(&roller);
}

void ScrollBar::Invalidate(const D3DXVECTOR2 &CursorPos, FLOAT Tf)
{
    if(!IsVisible())
        return;

    roller.Invalidate(CursorPos, Tf);

    bool inRange = CursorPos.x >= GetPos().x && CursorPos.x < GetBottomRightPos().x &&
                   CursorPos.y >= GetPos().y && CursorPos.y < GetBottomRightPos().y;

    bool inRollerRange = CursorPos.x >= roller.GetPos().x && CursorPos.x < roller.GetBottomRightPos().x &&
                         CursorPos.y >= roller.GetPos().y && CursorPos.y < roller.GetBottomRightPos().y;

    if(inRange && !inRollerRange && DirectInput::GetInsance()->IsMouseDown(0))
        Manager::GetInstance()->TryToLockControl(this);
    else
        Manager::GetInstance()->UnlockControl(this);

    if(inRange && !inRollerRange && DirectInput::GetInsance()->IsMousePress(0) && Manager::GetInstance()->GetLockedControl() == NULL){
        
        float len = 0.0f, cursorFactor = 0.0f;
        if(type == SB_TYPE_HORISONTAL){

            len = Control::WigthToX(GetSize().x - roller.GetSize().x);
            cursorFactor = Math::Saturate((CursorPos.x - GetPos().x) / len);

        }else{

            len = GetSize().y - roller.GetSize().y;
            cursorFactor = Math::Saturate((CursorPos.y - GetPos().y) / len);
        }

        float stepsCnt = floor(cursorFactor / factorStep);
        float newFactor = factorStep * stepsCnt;

        if(factor != newFactor)
            SetNewFactor(newFactor);
    }

    long mDeltaZ = DirectInput::GetInsance()->GetMouseDelta().z;
    if(inRange && mDeltaZ != 0){
        if(type == SB_TYPE_VERTICAL)
            mDeltaZ = -mDeltaZ;

        float newFactor = Math::Saturate(factor + (factorStep * mDeltaZ));
        if(factor != newFactor)
            SetNewFactor(newFactor);
    }
}

}
