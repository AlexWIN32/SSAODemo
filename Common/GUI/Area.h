/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <GUI/Control.h>
#include <GUI/Image.h>
#include <GUI/Theme.h>
#include <string>
#include <map>

namespace GUI
{

DECLARE_EXCEPTION(AreaException)

class Area : public Control
{
private:
    Image *upLftCorner = NULL, *upRghtCorner = NULL, *dwnLftCorner = NULL, *dwnRghtCorner = NULL;
    Image *lftBorder = NULL, *rghtBorder = NULL, *topBorder = NULL, *btmBorder = NULL;
    Image *spaceArea = NULL;
    DrawingObjectsStorage drawingObjects;
    D3DXVECTOR2 minSize, topLeftOffset;
    Image *CreateImage(const ControlView::ElementData &ElementData);
public:
    enum ResizingType 
    {
        RT_WIDTH_ONLY,
        RT_HEIGHT_ONLY,
        RT_WIDTH_AND_HEIGHT
    };
    virtual ~Area();
    void Init(const ControlView &View, ResizingType Resizing = RT_WIDTH_AND_HEIGHT) throw (Exception);
    void Init(const std::string &ControlName, ResizingType Resizing = RT_WIDTH_AND_HEIGHT) throw (Exception);
    void SetPos(const D3DXVECTOR2 &NewPos);
    void SetSize(const D3DXVECTOR2 &NewSize);
    const D3DXVECTOR2 &GetTopLeftOffset() const {return topLeftOffset;}
    const D3DXVECTOR2 &GetSpaceAreaSize() const {return spaceArea->GetSize();}
    const D3DXVECTOR2 &GetMinSize() const {return minSize;}
    void SetSpaceAreaSize(const D3DXVECTOR2 &NewSize);
    virtual DrawingObjectsStorage GetDrawingObjects() const {return DrawingObjectsStorage();}
};

};