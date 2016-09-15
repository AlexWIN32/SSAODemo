/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/Control.h>
#include <Utils.h>
#include <CommonParams.h>
#include <algorithm>

namespace GUI
{

void DrawingObject::SetSamplerState(const Utils::DirectX::SamplerStateDescription &SamplerState) throw (Exception)
{
    ReleaseCOM(samplerState);

    samplerState = Utils::DirectX::CreateSamplerState(SamplerState);
}

Control::Control() :
    isVisible(true), 
    isFocus(false), 
    pos(0.0f, 0.0f), 
    size(0.0f, 0.0f),
    positioning(CNTRL_POS_MANUAL)
{}

Control::~Control()
{
    //ControlsStorage::const_iterator ci;
    //for(ci = childControls.begin(); ci != childControls.end(); ++ci)
    //    delete *ci;
}

void Control::AddControl(Control *NewControl)
{
    childControls.push_back(NewControl);
}

void Control::RemoveControl(Control *ControlToRemove)
{
    Utils::RemoveFromVector(childControls, ControlToRemove, [](Control *Cntrl) { /*delete Cntrl;*/});
}

void Control::ClearControls()
{
    std::for_each(childControls.begin(), childControls.end(), [](Control *Cntrl) { /*delete Cntrl;*/});
    childControls.clear();
}

void Control::SetSize(const D3DXVECTOR2 &NewSize)
{
    if(NewSize.x <= 0.0f || NewSize.y <= 0.0f)
        return;

    if(size == NewSize)
        return;

    D3DXVECTOR2 sizeDelta = NewSize - size;
    sizeDelta.x = Control::WigthToX(sizeDelta.x);
    SetBottomRightPos(GetBottomRightPos() + sizeDelta);

    for(auto pair : sizeChangeEvents)
        pair.second(this, NewSize);

    size = NewSize;
}

void Control::SetPos(const D3DXVECTOR2 &NewPos)
{
    if(pos == NewPos)
        return;

    if(positioning != CNTRL_POS_MANUAL){
        D3DXVECTOR2 szOffst = {Control::WigthToX(GetSize().x), GetSize().y};
        if(positioning == CNTRL_POS_CENTER)
            SetBottomRightPos(NewPos + szOffst * 0.5f);
        else
            SetBottomRightPos(NewPos + szOffst);
    }

    for(auto pair : posChangeEvents)
        pair.second(this, NewPos);

    pos = NewPos;
}

D3DXVECTOR2 Control::TransformPoint(const D3DXVECTOR2 &Point)
{
    D3DXVECTOR2 out;
    out.x = (2.0f * Point.x) - 1.0f;
    out.y = (-2.0f * Point.y) + 1.0f;
    return out;
}

D3DXVECTOR2 Control::TransformSize(const D3DXVECTOR2 &Size)
{
    D3DXVECTOR2 out;
    out.x = 2.0f * Size.x * CommonParams::GetHeightOverWidth();
    out.y = 2.0f * Size.y;
    return out;
}

FLOAT Control::CheckPixelErrorForX(float X)
{
    float pixels = X * CommonParams::GetScreenWidth();
    float error = pixels - floor(pixels);

    if(error >= 0.5)
        X += (1.0 - error) / CommonParams::GetScreenWidth();
    else
        X -= error / CommonParams::GetScreenWidth();

    return X;
}

FLOAT Control::CheckPixelErrorForY(float Y)
{
    float pixels = Y * CommonParams::GetScreenHeight();
    float error = pixels - floor(pixels);

    if(error >= 0.5)
        Y += (1.0 - error) / CommonParams::GetScreenHeight();
    else
        Y -= error / CommonParams::GetScreenHeight();
        
    return Y;
}

FLOAT Control::WigthToX(float Width, bool CheckPixelError)
{
    float x = Width * CommonParams::GetHeightOverWidth();

    if(CheckPixelError)
       x = CheckPixelErrorForX(x);

    return x;
}

FLOAT Control::XToWigth(float X)
{
    return X * CommonParams::GetWidthOverHeight();
}

void Control::FixPos(Positioning PositioningType)
{
    Positioning posType = positioning;

    if(posType == CNTRL_POS_MANUAL)
        posType = PositioningType;

    if(posType == CNTRL_POS_MANUAL)
        return;

    D3DXVECTOR2 szOffst = (posType == CNTRL_POS_CENTER) ? size * 0.5f : size;

    szOffst.x = Control::WigthToX(szOffst.x);

    D3DXVECTOR2 corner = pos + szOffst;
    
    float absX = corner.x * CommonParams::GetScreenWidth();
    float absY = corner.y * CommonParams::GetScreenHeight();

    D3DXVECTOR2 error(absX - floor(absX), absY - floor(absY));

    if(error.x >= 0.5){
        pos.x += (1.0 - error.x) / CommonParams::GetScreenWidth();
        bottomRightPos.x += (1.0 - error.x) / CommonParams::GetScreenWidth();
    }else{
        pos.x -= error.x / CommonParams::GetScreenWidth();
        bottomRightPos.x -= error.x / CommonParams::GetScreenWidth();
    }

    if(error.y >= 0.5){
        pos.y += (1.0 - error.y) / CommonParams::GetScreenHeight();
        bottomRightPos.y += (1.0 - error.y) / CommonParams::GetScreenHeight();
    }else{
        pos.y -= error.y / CommonParams::GetScreenHeight();
        bottomRightPos.y -= error.y / CommonParams::GetScreenHeight();
    }

}

Utils::EventId Control::AddOnChangePosEvent(PosChangeEventsStorage::Event NewEvent)
{
    return posChangeEvents.AddEvent(NewEvent);
}

void Control::RemoveOnChangePosEvent(Utils::EventId DelEvent)
{
    posChangeEvents.RemoveEvent(DelEvent);
}

Utils::EventId Control::AddOnChangeSizeEvent(SizeChangeEventsStorage::Event NewEvent)
{
    return sizeChangeEvents.AddEvent(NewEvent);
}

void Control::RemoveOnChangeSizeEvent(Utils::EventId DelEvent)
{
    sizeChangeEvents.RemoveEvent(DelEvent);
}

}
