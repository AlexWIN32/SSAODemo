/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <GUI/Control.h>
#include <GUI/Image.h>
#include <GUI/Button.h>
#include <Utils/EventsStorage.h>
#include <vector>

namespace GUI
{

DECLARE_EXCEPTION(ScrollBarException)

class ScrollBar : 
    public Control,
    public Utils::EventsStorage<void(const ScrollBar*, FLOAT)>
{
public:
    enum Type
    {
        SB_TYPE_VERTICAL,
        SB_TYPE_HORISONTAL
    };
private:
    class Roller : public Button
    {
    private:
        ScrollBar *owner = NULL;
        void Init(const std::string &ControlName, Area::ResizingType Resizing) throw (Exception) {Button::Init(ControlName, Resizing);}
    public:
        void Init(ScrollBar *Owner, const ControlView &ControlView) throw (Exception);
        void SetLength(FLOAT NewLength, bool InPixels = false);        
        virtual void Invalidate(const D3DXVECTOR2 &CursorPos, FLOAT Tf);
    };
    Type type;
    Control *bgControl = NULL;
    Roller roller;
    FLOAT factor, factorStep;
    FLOAT minVal, maxVal;
    void SetSize(const D3DXVECTOR2 &NewSize){Control::SetSize(NewSize);}
    void SetNewLimits(FLOAT MinVal, FLOAT MaxVal) throw (Exception);
    void SetNewFactor(FLOAT Factor) throw (Exception);
public:
    ~ScrollBar() {delete bgControl;}
    ScrollBar() : minVal(0.0f), maxVal(1.0f), factorStep(0.1f), factor(0.0f){}
    void Init(Type ScrollBarType);
    void Init(Type ScrollBarType, const ControlView &SBControlView);
    void SetLength(FLOAT Length) throw (Exception);
    void SetRollerLength(FLOAT Length) throw (Exception);
    void SetMinVal(FLOAT MinVal) throw (Exception){SetNewLimits(MinVal, maxVal);}
    FLOAT GetMinVal() const {return minVal;}
    void SetMaxVal(FLOAT MaxVal) throw (Exception){SetNewLimits(minVal, MaxVal);}
    FLOAT GetMaxVal() const {return maxVal;}
    void SetStep(FLOAT Step) throw (Exception);
    FLOAT GetStep() const {return factorStep * (maxVal - minVal);}
    FLOAT GetFactor() const {return factor;}
    void SetFactor(FLOAT NewFactor) throw (Exception);
    FLOAT SetValue(FLOAT Value) throw (Exception);
    FLOAT GetValue() const {return minVal + (maxVal - minVal) * factor;}
    Type GetType() const {return type;}
    virtual void SetPos(const D3DXVECTOR2 &NewPos);    
    virtual void Invalidate(const D3DXVECTOR2 &CursorPos, FLOAT Tf);
    virtual DrawingObjectsStorage GetDrawingObjects() const {return DrawingObjectsStorage();}
    virtual const D3DXVECTOR2 &GetBottomRightPos() const {return bgControl->GetBottomRightPos();}
};

}