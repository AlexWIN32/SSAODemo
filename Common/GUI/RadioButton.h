/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <GUI/Image.h>
#include <vector>
#include <functional>
#include <Utils.h>

namespace GUI
{

DECLARE_EXCEPTION(RadioButtonException)

class RadioButton;

class RadioButtonsGroup : 
    public Utils::EventsStorage<void(const RadioButtonsGroup*, INT)>
{
private:
    typedef std::vector<RadioButton*> RadioButtonsStorage;
    RadioButtonsStorage radioButtons;
public:
    void AddRadioButton(RadioButton *NewRadioButton);
    INT GetSelectedIndex() throw (Exception);
    void SetSelectedIndex(INT Index) throw (Exception);
    RadioButtonsStorage::const_iterator begin() const {return radioButtons.begin();}
    RadioButtonsStorage::const_iterator end() const {return radioButtons.end();}
    RadioButtonsStorage::iterator begin() {return radioButtons.begin();}
    RadioButtonsStorage::iterator end() {return radioButtons.end();}
};

class RadioButton : 
    public Image,
    public Utils::EventsStorage<void(const RadioButton*, BOOL)>
{
private:
    RadioButtonsGroup *group;
    D3DXVECTOR4 selectedTexBounds, unselectedTexBounds;
    ID3D11ShaderResourceView *selectedTex, *unselectedTex;
    BOOL isSelected;
    void SetSize(const D3DXVECTOR2 &NewSize){Control::SetSize(NewSize);}
public: 
    RadioButton() : group(NULL), selectedTex(NULL), unselectedTex(NULL), isSelected(false){}
    void Init(RadioButtonsGroup *Group) throw (Exception);
    BOOL IsSelected() const {return isSelected;}
    void SetSelected();
    virtual void SetPos(const D3DXVECTOR2 &NewPos);
    virtual void Invalidate(const D3DXVECTOR2 &CursorPos, FLOAT Tf);
};

}