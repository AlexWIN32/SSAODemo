/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Dialogs/Menu.h>
#include <GUI.h>
#include <Vector2.h>

namespace Demo
{

class OptionsMenu : public Dialogs::BlurredMenu
{
private:
    GUI::ComboBox resolutionCb;
    GUI::CheckBox screenModeChkB;
    GUI::CheckBox pointLightChkB;
    GUI::CheckBox ssaoChkB;
    GUI::ScrollBar occlusionRadiusSb;
    GUI::Label occlusionRadiusLbl;
    GUI::ScrollBar harshnessSb;
    GUI::Label harshnessLbl;
    GUI::FixedPanel mainPanel;
    GUI::FixedPanel ssaoOptionsPanel;
    GUI::FixedPanel adapterInfoPanel;
    GUI::Label plContorlsLbl;
protected:
    virtual void OnShow();
    virtual void OnHide();
    virtual void DrawObjects();
    virtual void InvalidateObjects(FLOAT Tf);
    void ChangeLabelsColor(const GUI::FixedPanel &Panel, const D3DXCOLOR &NewColor);
    Utils::EventId onResChngEventId = 0;
    Utils::EventId onFullscreenChngEventId = 0;
    Utils::EventId onOcclRdChngEventId = 0;
    Utils::EventId onHarshnessChngEventId = 0;
    Utils::EventId onPtLightModeChngEventId = 0;
    Utils::EventId onSsaoModeChngEventId = 0;
public:
    virtual ~OptionsMenu();
    virtual void Init() throw (Exception);
    SizeUS GetResolution() const;
    void SetResolution(const SizeUS &Resolution) throw (Exception);
    void SetFullscreenMode(bool IsFullscreen) throw (Exception);
    BOOL GetFullscreenMode() const {return screenModeChkB.IsChecked();}
    FLOAT GetOcclusionRadius() const {return occlusionRadiusSb.GetValue();}
    void SetOcclusionRadius(FLOAT NewRadius);
    FLOAT GetHarshness() const {return harshnessSb.GetValue();} 
    void SetHarshness(FLOAT NewHarshness);
    void SetPointLightMode(BOOL Enable);
    BOOL GetPointLightMode() const {return pointLightChkB.IsChecked();}
    void SetSsaoMode(BOOL Enable);
    BOOL GetSsaoMode() const {return ssaoChkB.IsChecked();}
};

}
