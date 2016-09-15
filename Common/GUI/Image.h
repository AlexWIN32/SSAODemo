/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <GUI/Control.h>

namespace GUI
{

class Image : public Control, public DrawingObject 
{
private:
    D3DXCOLOR color;
public:
    Image() : color(1.0f, 1.0f, 1.0f, 1.0f){}
    virtual ~Image(){}
    const D3DXCOLOR &GetColor() const {return color;}
    void SetColor(const D3DXCOLOR &NewColor){color = NewColor;}
    void Init(const std::string &ImageFileName) throw (Exception);
    void Init(ID3D11ShaderResourceView *Texture);
    void SetImage(const std::string &ImageFileName) throw (Exception);
    void SetTexCoordBounds(const D3DXVECTOR4 &TcBounds){DrawingObject::SetTexCoordBounds(TcBounds);}
    virtual void Transform(TransformedRect &Rect) const;
    virtual DrawingObjectsStorage GetDrawingObjects() const { return {this}; }
};

};