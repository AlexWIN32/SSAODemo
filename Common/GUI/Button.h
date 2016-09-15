/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <GUI/Area.h>
#include <GUI/Control.h>
#include <GUI/Theme.h>
#include <Utils/EventsStorage.h>

namespace GUI
{

class Button : 
    public Control,
    public Utils::EventsStorage<void(const Button*)>
{
protected:
    enum State{
        ST_IDLE,
        ST_DOWN,
        ST_PRESS
    };
    State state;
    Area idlArea;
    Area dwnArea;
    Control *captionControl;
    D3DXVECTOR2 minSize;
    virtual void OnDown(const D3DXVECTOR2 &CursorPos, float Tf){};
    virtual void OnUp(const D3DXVECTOR2 &CursorPos, float Tf){};
public:
    Button() : captionControl(NULL), state(ST_IDLE){}
    ~Button(){delete captionControl;}
    void Init(const std::string &ControlName = "Button", Area::ResizingType Resizing = Area::RT_WIDTH_AND_HEIGHT) throw (Exception);
    void SetText(const std::wstring &Text) throw (Exception);
    void SetImage(const ImageData& Image) throw (Exception);
    virtual void SetPos(const D3DXVECTOR2 &NewPos);
    virtual void SetSize(const D3DXVECTOR2 &NewSize);
    virtual void Invalidate(const D3DXVECTOR2 &CursorPos, float Tf);
    virtual DrawingObjectsStorage GetDrawingObjects() const {return DrawingObjectsStorage();}
    virtual const D3DXVECTOR2 &GetPos() const {return idlArea.GetPos();}
    virtual const D3DXVECTOR2 &GetBottomRightPos() const {return idlArea.GetBottomRightPos();}
    virtual const D3DXVECTOR2 &GetSize() const {return idlArea.GetSize();}
    virtual const D3DXVECTOR2 &GetMinSize() const {return idlArea.GetMinSize();}
};

}