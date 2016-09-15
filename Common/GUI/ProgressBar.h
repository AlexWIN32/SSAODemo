/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <GUI/Control.h>
#include <GUI/Area.h>
#include <GUI/Theme.h>

namespace GUI
{

class ProgressBar : public Control
{
protected:
    Control *bgControl = NULL;
    Area progressArea;
    FLOAT progress = 0.0f;
public:
    ~ProgressBar() {delete bgControl;}
    void Init() throw (Exception);
    virtual void Init(const ControlView &PBControlView) throw (Exception);
    virtual void SetSize(const D3DXVECTOR2 &Size);
    virtual void SetPos(const D3DXVECTOR2 &Pos);
    FLOAT GetProgress() const {return progress;}
    virtual void SetProgress(FLOAT NewProgress) throw (Exception);
    virtual void Invalidate(const D3DXVECTOR2 &CursorPos, FLOAT Tf){}
    virtual DrawingObjectsStorage GetDrawingObjects() const {return DrawingObjectsStorage();}
    virtual const D3DXVECTOR2 &GetBottomRightPos() const {return bgControl->GetBottomRightPos();}
};

};