/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/Area.h>
#include <GUI/Manager.h>
#include <CommonParams.h>
#include <Serializing.h>
#include <memory>

namespace GUI
{

static void SetNewCornerPos(Image *Corner, const D3DXVECTOR2 &NewPos)
{
    D3DXVECTOR2 hlfSzOffst = D3DXVECTOR2(Control::WigthToX(Corner->GetSize().x, true), Corner->GetSize().y) * 0.5f;
    Corner->SetPos(NewPos + hlfSzOffst);
    Corner->SetBottomRightPos(NewPos + hlfSzOffst * 2.0f);
}

static void PlaceBorder(Image *Border, const D3DXVECTOR2 &NewPos, const D3DXVECTOR2 &NewSize)
{
    D3DXVECTOR2 newSize = {Control::CheckPixelErrorForY(NewSize.x), Control::CheckPixelErrorForY(NewSize.y)};
    D3DXVECTOR2 hlfSzOffst = D3DXVECTOR2(Control::WigthToX(newSize.x), newSize.y) * 0.5f;

    Border->SetPos(NewPos + hlfSzOffst);
    Border->SetSize(newSize);
    Border->SetBottomRightPos(NewPos + hlfSzOffst * 2.0f);
}

Image *Area::CreateImage(const ControlView::ElementData &ElementData)
{
    std::unique_ptr<GUI::Image> imgPtr(new GUI::Image());
    imgPtr->Init(ElementData.textureFileName);
    imgPtr->SetPositioning(Control::CNTRL_POS_MANUAL);
    imgPtr->SetSize(ElementData.screenSize);
    imgPtr->SetTexCoordBounds(ElementData.textureBounds);

    AddControl(imgPtr.get());

    return imgPtr.release();
}

Area::~Area()
{
    for(Control* cntrl  : GetControls())
        delete cntrl;
}

void Area::Init(const ControlView &View, ResizingType Resizing) throw (Exception)
{
    upLftCorner = CreateImage(View.GetElement("upperLeftCorner"));
    upRghtCorner = CreateImage(View.GetElement("upperRightCorner")); 
    dwnLftCorner = CreateImage(View.GetElement("loverLeftCorner"));
    dwnRghtCorner = CreateImage(View.GetElement("loverRightCorner"));

    bool canChangeWidth = Resizing == RT_WIDTH_AND_HEIGHT || Resizing == RT_WIDTH_ONLY;
    bool canChangeHeight = Resizing == RT_WIDTH_AND_HEIGHT || Resizing == RT_HEIGHT_ONLY;

    ControlView::ElementData ElementData;
    if(View.FindElement("leftBorder", ElementData) && canChangeHeight){
        lftBorder = CreateImage(ElementData);
        rghtBorder = CreateImage(View.GetElement("rightBorder"));
    }

    if(View.FindElement("topBorder", ElementData) && canChangeWidth){
        topBorder = CreateImage(ElementData);
        btmBorder = CreateImage(View.GetElement("bottomBorder"));        
    }

    if(lftBorder != NULL & topBorder != NULL)
        spaceArea = CreateImage(View.GetElement("spaceArea"));
    
    std::string paramData;
    if(View.FindParameter("TopLeftOffset", paramData)){
        Point2F tpLftOffst = Point2FParser::FromString(paramData);
        topLeftOffset = {Control::WigthToX(tpLftOffst.x, true), tpLftOffst.y};
    }else
        topLeftOffset = {Control::WigthToX(upLftCorner->GetSize().x, true), upLftCorner->GetSize().y};

    minSize.x = upLftCorner->GetSize().x + upRghtCorner->GetSize().x;
    minSize.y = upLftCorner->GetSize().y + dwnLftCorner->GetSize().y;

    if(topBorder != NULL)
        minSize.x += topBorder->GetSize().x;

    if(lftBorder != NULL)
        minSize.y += lftBorder->GetSize().y;

    SetSize(minSize);
}

void Area::Init(const std::string &ControlName, ResizingType Resizing) throw (Exception)
{
    const ControlView &controlView = GUI::Manager::GetInstance()->GetTheme().GetControlView(ControlName);
    Init(controlView, Resizing);
}

void Area::SetPos(const D3DXVECTOR2 &NewPos)
{
    D3DXVECTOR2 newPos = {Control::CheckPixelErrorForX(NewPos.x), Control::CheckPixelErrorForY(NewPos.y)};
    D3DXVECTOR2 delta = newPos - GetPos();
    delta = {Control::CheckPixelErrorForX(delta.x), Control::CheckPixelErrorForY(delta.y)};

    for(Control* cntrl  : GetControls())
        cntrl->SetPos(cntrl->GetPos() + delta);

    SetBottomRightPos(GetBottomRightPos() + delta);
    Control::SetPos(newPos);
}

void Area::SetSize(const D3DXVECTOR2 &NewSize)
{
    if(NewSize.x < minSize.x || NewSize.y < minSize.y)
        return;

    const D3DXVECTOR2 &uLSize = upLftCorner->GetSize();
    const D3DXVECTOR2 &uRSize = upRghtCorner->GetSize();
    const D3DXVECTOR2 &dRSize = dwnRghtCorner->GetSize();
    const D3DXVECTOR2 &dLSize = dwnLftCorner->GetSize();

    const D3DXVECTOR2 &tpLft = GetPos();

    D3DXVECTOR2 newSize = {Control::CheckPixelErrorForY(NewSize.x), Control::CheckPixelErrorForY(NewSize.y)};

    if(lftBorder != NULL && topBorder != NULL){

        SetNewCornerPos(upLftCorner, tpLft);

        D3DXVECTOR2 brdrOffst = {upLftCorner->GetBottomRightPos().x, tpLft.y};
        D3DXVECTOR2 brdrSize = {newSize.x - uLSize.x - uRSize.x, topBorder->GetSize().y};
        PlaceBorder(topBorder, brdrOffst, brdrSize);

        SetNewCornerPos(upRghtCorner, {topBorder->GetBottomRightPos().x, tpLft.y});

        brdrOffst = {topBorder->GetBottomRightPos().x, upRghtCorner->GetBottomRightPos().y};
        brdrSize = {rghtBorder->GetSize().x, newSize.y - uRSize.y - dRSize.y};
        PlaceBorder(rghtBorder, brdrOffst, brdrSize);

        SetNewCornerPos(dwnRghtCorner, {topBorder->GetBottomRightPos().x, rghtBorder->GetBottomRightPos().y});

        brdrOffst = {upLftCorner->GetBottomRightPos().x, rghtBorder->GetBottomRightPos().y};
        brdrSize = {topBorder->GetSize().x, btmBorder->GetSize().y};
        PlaceBorder(btmBorder, brdrOffst, brdrSize);

        SetNewCornerPos(dwnLftCorner, {tpLft.x, rghtBorder->GetBottomRightPos().y});

        brdrOffst = {tpLft.x, upLftCorner->GetBottomRightPos().y};
        brdrSize = {lftBorder->GetSize().x, rghtBorder->GetSize().y};
        PlaceBorder(lftBorder, brdrOffst, brdrSize);

        brdrOffst = {upLftCorner->GetBottomRightPos().x, upLftCorner->GetBottomRightPos().y};
        brdrSize = {topBorder->GetSize().x, lftBorder->GetSize().y};
        PlaceBorder(spaceArea, brdrOffst, brdrSize);

    }else if(lftBorder != NULL){

        if(newSize.x > minSize.x)
            newSize.x = minSize.x;

        SetNewCornerPos(upLftCorner, tpLft);
        SetNewCornerPos(upRghtCorner, {upLftCorner->GetBottomRightPos().x, tpLft.y});

        D3DXVECTOR2 brdrOffst = {tpLft.x, upLftCorner->GetBottomRightPos().y};
        D3DXVECTOR2 brdrSize = {lftBorder->GetSize().x, newSize.y - uLSize.y - dLSize.y}; 
        PlaceBorder(lftBorder, brdrOffst, brdrSize);
    
        brdrOffst = {upLftCorner->GetBottomRightPos().x, upLftCorner->GetBottomRightPos().y};
        brdrSize = {rghtBorder->GetSize().x, newSize.y - uRSize.y - dRSize.y};
        PlaceBorder(rghtBorder, brdrOffst, brdrSize);

        SetNewCornerPos(dwnLftCorner, {tpLft.x, lftBorder->GetBottomRightPos().y});
        SetNewCornerPos(dwnRghtCorner, {lftBorder->GetBottomRightPos().x, lftBorder->GetBottomRightPos().y});

    }else if(topBorder != NULL){

        if(newSize.y > minSize.y)
            newSize.y = minSize.y;

        SetNewCornerPos(upLftCorner, tpLft);
        SetNewCornerPos(dwnLftCorner, {tpLft.x, upLftCorner->GetBottomRightPos().y});

        D3DXVECTOR2 brdrOffst = {upLftCorner->GetBottomRightPos().x, tpLft.y};
        D3DXVECTOR2 brdrSize = {newSize.x - dLSize.x - dRSize.x, topBorder->GetSize().y}; 
        PlaceBorder(topBorder, brdrOffst, brdrSize);

        brdrOffst = {upLftCorner->GetBottomRightPos().x, upLftCorner->GetBottomRightPos().y};
        brdrSize = {newSize.x - dLSize.x - dRSize.x, btmBorder->GetSize().y}; 
        PlaceBorder(btmBorder, brdrOffst, brdrSize);

        SetNewCornerPos(upRghtCorner, {topBorder->GetBottomRightPos().x, tpLft.y});
        SetNewCornerPos(dwnRghtCorner, {topBorder->GetBottomRightPos().x, topBorder->GetBottomRightPos().y});
    }

    float newWidth = Control::XToWigth(upRghtCorner->GetBottomRightPos().x - tpLft.x);
    float newHeight = dwnLftCorner->GetBottomRightPos().y - tpLft.y;
    Control::SetSize({newWidth, newHeight});

    SetBottomRightPos(dwnRghtCorner->GetBottomRightPos());
}

void Area::SetSpaceAreaSize(const D3DXVECTOR2 &NewSize)
{
    D3DXVECTOR2 newSize = {Control::CheckPixelErrorForX(NewSize.x), Control::CheckPixelErrorForY(NewSize.y)};

    D3DXVECTOR2 fullSize;
    fullSize.x = upLftCorner->GetSize().x + newSize.x + upRghtCorner->GetSize().x;
    fullSize.y = upLftCorner->GetSize().y + newSize.y + dwnLftCorner->GetSize().y;
    SetSize(fullSize);
}

}
