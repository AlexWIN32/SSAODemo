/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include "OptionsMenu.h"
#include "Application.h"
#include <AdapterManager.h>
#include <CommonParams.h>
#include <Utils/ToString.h>
#include <memory>

namespace Demo
{

static GUI::Label *NewLabel(const std::wstring &Caption)
{
    const GUI::Font &fnt = GUI::Manager::GetInstance()->GetTheme().GetFont();

    std::unique_ptr<GUI::Label> resLbl(new GUI::Label());
    resLbl->Init(&fnt);
    resLbl->SetCaption(Caption);
    resLbl->UpdateGlyphs();

    return resLbl.release();
}

static std::wstring GetResolutionString(const SizeUS &Resolution)
{
    std::wstring widthStr = Utils::to_wstring(Resolution.width);
    std::wstring heightStr = Utils::to_wstring(Resolution.height);

    return widthStr + L" x " + heightStr;
}

static GUI::FixedPanel *CreateParamTweaking(FLOAT MinVal,
                                            FLOAT MaxVal,
                                            FLOAT Val,
                                            const GUI::ScrollBar::Event &Event,
                                            GUI::ScrollBar &ScrollBar,
                                            GUI::Label &Label,
                                            Utils::EventId &CreatedEventId)
{
    Label.Init(&GUI::Manager::GetInstance()->GetTheme().GetFont());
    Label.SetCaption(Utils::to_wstring(Val));
    Label.UpdateGlyphs();

    ScrollBar.Init(GUI::ScrollBar::SB_TYPE_HORISONTAL);
    ScrollBar.SetMinVal(MinVal);
    ScrollBar.SetMaxVal(MaxVal);
    ScrollBar.SetLength(0.3);
    ScrollBar.SetStep(0.1);
    ScrollBar.SetValue(Val);

    CreatedEventId = ScrollBar.AddEvent(Event);

    GUI::FixedPanel *panel = new GUI::FixedPanel();
    panel->SetControl(&Label, 0, 0);
    panel->SetControl(&ScrollBar, 0, 1);

    return panel;
}

void OptionsMenu::OnShow()
{
    mainPanel.SetVisibleState(true);

    Application::GetInstance()->SetCursorVisibleState(true);
}

void OptionsMenu::OnHide() 
{
    mainPanel.SetVisibleState(false);

    GUI::Manager::GetInstance()->ResetContext();
    mainPanel.OnManagerResetContext();

    Application::GetInstance()->SetCursorVisibleState(false);
}

void OptionsMenu::DrawObjects()
{
    GUI::Manager::GetInstance()->Draw(mainPanel.GetControls());
}

void OptionsMenu::InvalidateObjects(FLOAT Tf)
{
    GUI::Manager::GetInstance()->Invalidate(Tf, mainPanel.GetControls());
}

void OptionsMenu::ChangeLabelsColor(const GUI::FixedPanel &Panel, const D3DXCOLOR &NewColor)
{
    for(GUI::Control *cntrl : Panel.GetControls()){

        GUI::Label *lbl = dynamic_cast<GUI::Label*>(cntrl);
        GUI::FixedPanel *pnl = dynamic_cast<GUI::FixedPanel*>(cntrl);

        if(lbl != NULL){
            lbl->SetColor(NewColor);
            lbl->UpdateGlyphs();
        }else if(pnl != NULL)
            ChangeLabelsColor(*pnl, NewColor);
    }
}

void OptionsMenu::Init() throw (Exception)
{
    BlurredMenu::Init();

    resolutionCb.Init();
    
    for(const SizeUS &res : DisplaySettings::AdapterManager::GetInstance()->GetDisplayModes())
        resolutionCb.AddItem(GetResolutionString(res));
    
    std::wstring widthStr = Utils::to_wstring(CommonParams::GetScreenWidth());
    std::wstring heightStr = Utils::to_wstring(CommonParams::GetScreenHeight());
    resolutionCb.SetSelectedItem(widthStr + L" x " + heightStr);

    onResChngEventId = resolutionCb.AddEvent([&](const GUI::ComboBox *Owner, INT Index)
    {
        Application::GetInstance()->ChangeResolution();
    });

    screenModeChkB.Init();

    onFullscreenChngEventId = screenModeChkB.AddEvent([&](const GUI::CheckBox *Owner, INT State)
    {
        Application::GetInstance()->ChangeFullscreenMode();
    });

    pointLightChkB.Init();

    onPtLightModeChngEventId = pointLightChkB.AddEvent([&, this](const GUI::CheckBox *Owner, INT State)
    {
        D3DXCOLOR newColor = (State == true) ? D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f) : D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
        ChangeLabelsColor(mainPanel, newColor);
        Application::GetInstance()->SetPointLightMode(State == true);
    });

    ssaoChkB.Init();

    onSsaoModeChngEventId = ssaoChkB.AddEvent([&, this](const GUI::CheckBox *Owner, INT State)
    {
        Application::GetInstance()->SetSsaoMode(State == true);
    });

    auto occlRadEvent = [&,this](const GUI::ScrollBar *Sb, FLOAT NewFactor)
    {
        float value = Sb->GetMinVal() + (Sb->GetMaxVal() - Sb->GetMinVal()) * NewFactor;
        occlusionRadiusLbl.SetCaption(Utils::to_wstring(value));
        Application::GetInstance()->ChangeOcclusionRadius(value);
    };

    GUI::FixedPanel *ssaoOcclRadPanel = CreateParamTweaking(0.1f, 2.0f, 0.8,
                                                            occlRadEvent,
                                                            occlusionRadiusSb, occlusionRadiusLbl,
                                                            onOcclRdChngEventId);

    auto harshnessEvent = [&,this](const GUI::ScrollBar *Sb, FLOAT NewFactor)
    {
        float value = Sb->GetMinVal() + (Sb->GetMaxVal() - Sb->GetMinVal()) * NewFactor;
        harshnessLbl.SetCaption(Utils::to_wstring(value));
        Application::GetInstance()->ChangeHarshness(value);
    };

    GUI::FixedPanel *ssaoHarshnessPanel = CreateParamTweaking(0.1f, 5.0f, 1.5,
                                                        harshnessEvent,
                                                        harshnessSb, harshnessLbl,
                                                        onHarshnessChngEventId);

    ssaoOptionsPanel.SetColSpacing(0.005f);
    ssaoOptionsPanel.SetRowSpacing(0.01f);
    ssaoOptionsPanel.SetControl(NewLabel(L"Occlusion radius"), 0, 0, true);
    ssaoOptionsPanel.SetControl(ssaoOcclRadPanel, 1, 0, true);
    ssaoOptionsPanel.SetControl(NewLabel(L"Harshness"), 0, 1, true);
    ssaoOptionsPanel.SetControl(ssaoHarshnessPanel, 1, 1, true);
    ssaoOptionsPanel.SetControl(NewLabel(L"SSAO"), 0, 2, true);
    ssaoOptionsPanel.SetControl(&ssaoChkB, 1, 2);
    ssaoOptionsPanel.SetControl(NewLabel(L"Point light"), 0, 3, true);
    ssaoOptionsPanel.SetControl(&pointLightChkB, 1, 3);

    adapterInfoPanel.SetColSpacing(0.01f);
    adapterInfoPanel.SetRowSpacing(0.01f);
    adapterInfoPanel.SetControl(NewLabel(L"Screen resolution"), 0, 0, true);
    adapterInfoPanel.SetControl(&resolutionCb, 1, 0);
    adapterInfoPanel.SetControl(NewLabel(L"Full screen"), 0, 1, true);
    adapterInfoPanel.SetControl(&screenModeChkB, 1, 1);
    adapterInfoPanel.SetControl(NewLabel(L"Adapter name"), 0, 2, true);
    adapterInfoPanel.SetControl(NewLabel(DisplaySettings::AdapterManager::GetInstance()->GetName()), 1, 2, true);

    std::wstring plContorlsLblCaption = L"'Up', 'Down', 'Left', 'Right' - move light\n"\
                                        L"'Q', 'E' - rise or down light";

    plContorlsLbl.Init(&GUI::Manager::GetInstance()->GetTheme().GetFont());
    plContorlsLbl.SetCaption(Utils::to_wstring(plContorlsLblCaption));
    plContorlsLbl.UpdateGlyphs();

    mainPanel.SetPos({0.05f, 0.05f});
    mainPanel.SetRowSpacing(0.02f);
    mainPanel.SetControl(&ssaoOptionsPanel, 0, 0);
    mainPanel.SetControl(&adapterInfoPanel, 0, 1);
    mainPanel.SetControl(&plContorlsLbl, 0, 2);
    mainPanel.SetVisibleState(false);

    //plContorlsLbl.SetPos(mainPanel.GetBottomRightPos());
    //GUI::Manager::GetInstance()->AddControl(&plContorlsLbl);
}

void OptionsMenu::SetResolution(const SizeUS &Resolution) throw (Exception)
{
    resolutionCb.RemoveEvent(onResChngEventId);

    resolutionCb.SetSelectedItem(GetResolutionString(Resolution));

    onResChngEventId = resolutionCb.AddEvent([&](const GUI::ComboBox *Owner, INT Index)
    {
        Application::GetInstance()->ChangeResolution();
    });
}

void OptionsMenu::SetPointLightMode(BOOL Enable)
{
    pointLightChkB.RemoveEvent(onPtLightModeChngEventId);

    pointLightChkB.SetChecked(Enable);

    onPtLightModeChngEventId = pointLightChkB.AddEvent([&, this](const GUI::CheckBox *Owner, INT State)
    {
        D3DXCOLOR newColor = (State == true) ? D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f) : D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
        ChangeLabelsColor(mainPanel, newColor);
        Application::GetInstance()->SetPointLightMode(State == true);
    });
}

void OptionsMenu::SetSsaoMode(BOOL Enable)
{
    ssaoChkB.RemoveEvent(onSsaoModeChngEventId);

    ssaoChkB.SetChecked(Enable);

    onSsaoModeChngEventId = ssaoChkB.AddEvent([&, this](const GUI::CheckBox *Owner, INT State)
    {
        Application::GetInstance()->SetSsaoMode(State == true);
    });
}

void OptionsMenu::SetOcclusionRadius(FLOAT NewRadius)
{
    occlusionRadiusSb.RemoveEvent(onOcclRdChngEventId);

    occlusionRadiusSb.SetValue(NewRadius);
    occlusionRadiusLbl.SetCaption(Utils::to_wstring(NewRadius));

    onOcclRdChngEventId = occlusionRadiusSb.AddEvent([this](const GUI::ScrollBar *Sb, FLOAT NewFactor)
    {
        float value = Sb->GetMinVal() + (Sb->GetMaxVal() - Sb->GetMinVal()) * NewFactor;
        occlusionRadiusLbl.SetCaption(Utils::to_wstring(value));
        Application::GetInstance()->ChangeOcclusionRadius(value);
    });
}

void OptionsMenu::SetHarshness(FLOAT NewHarshness)
{
    harshnessSb.RemoveEvent(onHarshnessChngEventId);

    harshnessSb.SetValue(NewHarshness);
    harshnessLbl.SetCaption(Utils::to_wstring(NewHarshness));

    onHarshnessChngEventId = harshnessSb.AddEvent([this](const GUI::ScrollBar *Sb, FLOAT NewFactor)
    {
        float value = Sb->GetMinVal() + (Sb->GetMaxVal() - Sb->GetMinVal()) * NewFactor;
        harshnessLbl.SetCaption(Utils::to_wstring(value));
        Application::GetInstance()->ChangeHarshness(value);
    });
}

void OptionsMenu::SetFullscreenMode(bool IsFullscreen) throw (Exception)
{
    screenModeChkB.RemoveEvent(onFullscreenChngEventId);

    screenModeChkB.SetChecked(IsFullscreen);

    onFullscreenChngEventId = screenModeChkB.AddEvent([](const GUI::CheckBox *Cb, INT State)
    {
        Application::GetInstance()->ChangeFullscreenMode();
    });
}

SizeUS OptionsMenu::GetResolution() const
{
    std::wstring selItm;
    if(!resolutionCb.GetSelectedItem(selItm))
        return SizeUS();

    size_t spcInd = selItm.find(' ');
    std::wstring widthStr = selItm.substr(0, spcInd);
    std::wstring heightStr = selItm.substr(selItm.find(' ', spcInd + 1) + 1);

    return SizeUS(std::stoi(widthStr), std::stoi(heightStr));
}

OptionsMenu::~OptionsMenu()
{
    mainPanel.Clear();
    ssaoOptionsPanel.Clear();
    adapterInfoPanel.Clear();
}

}