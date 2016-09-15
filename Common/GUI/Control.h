/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Utils/EventsStorage.h>
#include <Utils/DirectX.h>
#include <D3DHeaders.h>
#include <Shader.h>
#include <vector>
#include <map>
#include <functional>

namespace GUI
{

class Control;
typedef std::vector<Control*> ControlsStorage;

struct TransformedRect
{
    D3DXVECTOR2 center;
    D3DXVECTOR2 halfSize;
    D3DXCOLOR color;
    D3DXVECTOR4 sidesTexCoords;
};

class DrawingObject
{
private:
    ID3D11ShaderResourceView *texture;
    D3DXVECTOR4 tcBounds;
    ID3D11SamplerState *samplerState = NULL;
protected:
    void SetTexture(ID3D11ShaderResourceView *Texture){texture = Texture;}
    virtual void SetTexCoordBounds(const D3DXVECTOR4 &TcBounds){tcBounds = TcBounds;}
public:
    virtual ~DrawingObject(){ReleaseCOM(samplerState);}
    DrawingObject() : texture(NULL){}
    void SetSamplerState(const Utils::DirectX::SamplerStateDescription &SamplerState) throw (Exception);
    ID3D11SamplerState *GetSamplerState() const{ return samplerState;}
    ID3D11ShaderResourceView * GetTexture() const {return texture;}
    const D3DXVECTOR4 &GetTexCoordBounds()const {return tcBounds;}
    virtual void Transform(TransformedRect &Rect) const = 0;
};

typedef std::vector<const DrawingObject*> DrawingObjectsStorage;

class DrawingObjectsContainer
{
public:
    virtual ~DrawingObjectsContainer(){}
    virtual DrawingObjectsStorage GetDrawingObjects() const = 0;
};

class Manager;
class Theme;

class Control : public DrawingObjectsContainer
{
friend class Manager;
public:
    enum Positioning
    {
        CNTRL_POS_CENTER,
        CNTRL_POS_BOTTOM_RIGHT,
        CNTRL_POS_MANUAL
    };
private:    
    bool isVisible, isFocus;
    D3DXVECTOR2 pos, bottomRightPos, size;
    ControlsStorage childControls;
    Positioning positioning;
    typedef Utils::EventsStorage<void(const Control*, const D3DXVECTOR2&)> PosChangeEventsStorage;
    typedef Utils::EventsStorage<void(const Control*, const D3DXVECTOR2&)> SizeChangeEventsStorage;
    PosChangeEventsStorage posChangeEvents;
    SizeChangeEventsStorage sizeChangeEvents;
protected:
    void AddControl(Control *NewControl);
    void RemoveControl(Control *ControlToRemove);
    void ClearControls();        
    const ControlsStorage &GetControls() const{return childControls;}
    Control();
public:
    static D3DXVECTOR2 TransformPoint(const D3DXVECTOR2 &Point);
    static D3DXVECTOR2 TransformSize(const D3DXVECTOR2 &Size);
    static FLOAT WigthToX(float Width, bool CheckPixelError = false);
    static FLOAT XToWigth(float X);
    static FLOAT CheckPixelErrorForX(float X);
    static FLOAT CheckPixelErrorForY(float Y);
    virtual ~Control();    
    void FixPos(Positioning PositioningType = CNTRL_POS_MANUAL);
    virtual void SetSize(const D3DXVECTOR2 &Size);
    virtual const D3DXVECTOR2 &GetSize() const {return size;}
    virtual void SetPos(const D3DXVECTOR2 &Pos);
    virtual const D3DXVECTOR2 &GetPos() const {return pos;}
    virtual bool IsVisible() const {return isVisible;}
    virtual void SetVisibleState(bool IsVisible){isVisible = IsVisible;}
    virtual bool IsFocus() const {return isFocus;}
    virtual void SetFocusState(bool IsFocus){isFocus = IsFocus;}   
    virtual void Invalidate(const D3DXVECTOR2 &CursorPos, float Tf){}
    virtual void OnThemeChange(const Theme &NewTheme) throw (Exception) {}
    virtual void OnManagerResetContext() {};
    void SetBottomRightPos(D3DXVECTOR2 BottomRightPos){bottomRightPos = BottomRightPos;}
    virtual const D3DXVECTOR2 &GetBottomRightPos() const {return bottomRightPos;}
    void SetPositioning(Positioning PositioningType ) {positioning = PositioningType;} 
    Positioning GetPositioning() const { return positioning;}
    Utils::EventId AddOnChangePosEvent(PosChangeEventsStorage::Event NewEvent);
    void RemoveOnChangePosEvent(Utils::EventId DelEvent);
    Utils::EventId AddOnChangeSizeEvent(SizeChangeEventsStorage::Event NewEvent);
    void RemoveOnChangeSizeEvent(Utils::EventId DelEvent);
};

}
