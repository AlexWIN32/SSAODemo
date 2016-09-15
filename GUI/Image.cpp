/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/Image.h>
#include <GUI/Manager.h>

namespace GUI
{

void Image::Init(const std::string &ImageFileName) throw (Exception)
{
    Init(Manager::GetInstance()->GetTexture(ImageFileName));
}

void Image::Init(ID3D11ShaderResourceView *Texture)
{
    SetTexture(Texture);
    SetTexCoordBounds({0.0f,0.0f, 1.0f,1.0f});
    SetPositioning(CNTRL_POS_CENTER);
}

void Image::SetImage(const std::string &ImageFileName) throw (Exception)
{
    SetTexture(Manager::GetInstance()->GetTexture(ImageFileName));
}

void Image::Transform(TransformedRect &Rect) const
{
    if(GetPositioning() == CNTRL_POS_BOTTOM_RIGHT){
        D3DXVECTOR2 hSzOffst = GetSize() * 0.5f;
        hSzOffst.x = Control::WigthToX(hSzOffst.x);
        Rect.center = TransformPoint(GetPos() + hSzOffst);
    }else
        Rect.center = TransformPoint(GetPos());

    Rect.color = color;
    Rect.halfSize = TransformSize(GetSize() * 0.5f);
    Rect.sidesTexCoords = GetTexCoordBounds();
}

}
