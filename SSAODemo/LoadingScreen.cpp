/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include "LoadingScreen.h"
#include <CommonParams.h>
#include <RenderStatesManager.h>

namespace Demo
{

LoadingScreen *LoadingScreen::instance = NULL;

void LoadingScreen::Init() throw (Exception)
{
    const GUI::Font &fnt = GUI::Manager::GetInstance()->GetTheme().GetFont();

    float sbWidth = 200.0f / (CommonParams::GetScreenWidth() * CommonParams::GetWidthOverHeight());

    float pbHSzOffst = GUI::Control::WigthToX(sbWidth * 0.5f, true);

    pb.Init();
    pb.SetSize({sbWidth, 0.01f});
    pb.SetPos({0.5f - pbHSzOffst, 0.8f});

    std::wstring text = L"Loading";

    D3DXVECTOR2 textSize = fnt.MeasureString(text);

    lbl.Init(&fnt);
    lbl.SetPos({0.5f - GUI::Control::WigthToX(textSize.x * 0.5f, true), 0.8f - fnt.GetLineScreenHeight()});
    lbl.SetCaption(text);
    lbl.UpdateGlyphs();
}

void LoadingScreen::ProcessNewProgress(FLOAT NewProgress, const MessagesStorage &Messages)
{
    pb.SetProgress(NewProgress);
}

void LoadingScreen::Render()
{
    GUI::Manager::GetInstance()->Draw({&lbl, &pb});
}

}