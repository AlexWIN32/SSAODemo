/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/ComboBox.h>
#include <GUI/Manager.h>
#include <Serializing.h>
#include <DirectInput.h>
#include <Serializing.h>
#include <CommonParams.h>
#include <memory>

namespace GUI
{

void ComboBox::Button::Invalidate(const D3DXVECTOR2 &CursorPos, float Tf)
{

    bool stateChanged = false;
    if(DirectInput::GetInsance()->IsMouseDown(0) && !lock){
        lock = true;
        stateChanged = true;
    }else if(!DirectInput::GetInsance()->IsMouseDown(0) && lock)
        lock = false;

    if(stateChanged){
        const D3DXVECTOR2 &pos = GetPos(), &size = GetSize();

        D3DXVECTOR2 szOffset(Control::WigthToX(size.x), size.y);

        bool inRange = CursorPos.x >= pos.x && CursorPos.x <= pos.x + szOffset.x &&
                       CursorPos.y >= pos.y && CursorPos.y <= pos.y + szOffset.y;

        bool inItemsListRange = CursorPos.x >= itemsListTopLeft.x && CursorPos.x <= itemsListBottomRight.x &&
                            CursorPos.y >= itemsListTopLeft.y && CursorPos.y <= itemsListBottomRight.y;

        if(inRange && state == ST_IDLE){
            if(Manager::GetInstance()->TryToLockControl(this)){
                state = ST_DOWN;
        
                ClearControls();
                AddControl(&dwnArea);

                for(auto evnt : onShowListEvents)
                    evnt.second();
            }

        }else if(state == ST_DOWN){
            Manager::GetInstance()->UnlockControl(this);

            if(!inItemsListRange)
                Disable();
        }
    }

}

void ComboBox::Button::Disable()
{
    state = ST_IDLE;

    ClearControls();
    AddControl(&idlArea);

    for(auto evnt : onHideListEvents)
        evnt.second();
}

ComboBox::~ComboBox()
{
    for(Label *lbl : itemsLabels)
       delete lbl;
}

void ComboBox::ShowItems()
{
    itemsArea.SetVisibleState(true);

    Manager::GetInstance()->RiseDrawPriority(&itemsArea);

    for(Label *lbl : itemsLabels){
        lbl->SetVisibleState(true);
        Manager::GetInstance()->RiseDrawPriority(lbl);
    }

    if(showSelItmArea)
        selItmArea.SetVisibleState(true);

    Manager::GetInstance()->RiseDrawPriority(&selItmArea);
    Manager::GetInstance()->RiseDrawPriority(&itemsScrlBr);

    if(items.size() > maxViewedItems)
        itemsScrlBr.SetVisibleState(true);
}

void ComboBox::HideItems()
{
    itemsArea.SetVisibleState(false);

    Manager::GetInstance()->LowDrawPriority(&itemsArea);

    for(Label *lbl : itemsLabels){
        lbl->SetVisibleState(false);
        Manager::GetInstance()->LowDrawPriority(lbl);
    }

    Manager::GetInstance()->LowDrawPriority(&selItmArea);
    Manager::GetInstance()->LowDrawPriority(&itemsScrlBr);

    selItmArea.SetVisibleState(false);
    itemsScrlBr.SetVisibleState(false);
}

void ComboBox::MarkSelectedLabel(Label *SelectedLabel, BOOL IsVisible)
{
    SelectedLabel->SetColor(slItmFntColor);
    selItmArea.SetPos(SelectedLabel->GetPos() - itemsArea.GetTopLeftOffset());

    RemoveControl(&selItmArea);
    RemoveControl(SelectedLabel);

    AddControl(&selItmArea);
    AddControl(SelectedLabel);

    selItmArea.SetVisibleState(IsVisible);
    showSelItmArea = true;
}

void ComboBox::Init() throw (Exception)
{
    const ControlView &controlView = Manager::GetInstance()->GetTheme().GetControlView("ComboBox");
    const Font &fnt = Manager::GetInstance()->GetTheme().GetFont();

    std::string btnName = "Button";
    controlView.FindParameter("showItemsButtonName", btnName);

    std::string btnImage = "";
    controlView.FindParameter("showItemsButtonImage", btnImage);

    maxViewedItems = IntParser::FromString(controlView.GetParameter("maxViewedItems"));

    itmFntColor = Cast<D3DXCOLOR>(ColorParser<FloatParser>::FromString(controlView.GetParameter("itemFontColor")));
    slItmFntColor = Cast<D3DXCOLOR>(ColorParser<FloatParser>::FromString(controlView.GetParameter("selectedItemFontColor")));

    valArea.Init(controlView.GetParameter("valueArea"));
    AddControl(&valArea);

    valLabel.Init(&fnt);
    AddControl(&valLabel);

    showItemsBtn.Init();
    AddControl(&showItemsBtn);

    itemsArea.Init(controlView.GetParameter("itemsArea"));
    AddControl(&itemsArea);
    
    ControlView sbControlView = Manager::GetInstance()->GetTheme().GetControlView("VerticalScrollBar");
    sbControlView.SetParameter("bgArea", controlView.GetParameter("scrollBarArea"));
    itemsScrlBr.Init(ScrollBar::SB_TYPE_VERTICAL, sbControlView);
    AddControl(&itemsScrlBr);

    selItmArea.Init(controlView.GetParameter("selectedItemArea"));

    emptyValueWidth = FloatParser::FromString(controlView.GetParameter("emptyValueWidth")) / CommonParams::GetScreenWidth();
    valArea.SetSpaceAreaSize({emptyValueWidth, fnt.GetLineScreenHeight()});

    float showItemsBtnWidth = FloatParser::FromString(controlView.GetParameter("showItemsButtonWidth")) / CommonParams::GetScreenWidth();
    showItemsBtn.SetPos({valArea.GetBottomRightPos().x, 0.0f});
    showItemsBtn.SetSize({showItemsBtnWidth, valArea.GetSize().y});

    showItemsBtn.AddOnShowListEvent([this](){ShowItems();});

    showItemsBtn.AddOnHideListEvent([this](){HideItems();});

    itemsArea.SetPos({0, valArea.GetBottomRightPos().y});
    itemsArea.SetSize({Control::XToWigth(showItemsBtn.GetBottomRightPos().x), fnt.GetLineScreenHeight()});
    itemsArea.SetVisibleState(false);

    valLabel.SetPos(valArea.GetTopLeftOffset());
    valLabel.SetColor(itmFntColor);

    itemsScrlBr.SetPos(valArea.GetBottomRightPos());
    itemsScrlBr.SetLength((float)maxViewedItems * fnt.GetLineScreenHeight());
    itemsScrlBr.SetRollerLength(showItemsBtn.GetSize().x);
    itemsScrlBr.SetVisibleState(false);
    itemsScrlBr.SetMinVal(0.0f);
    itemsScrlBr.SetStep(1.0f);
    itemsScrlBr.AddEvent([this](const ScrollBar *Sb, FLOAT Factor)
    {
        float area = Sb->GetMaxVal() - Sb->GetMinVal() ;
        float offset = ceil((Sb->GetMaxVal() - Sb->GetMinVal()) * Factor);

        selItmArea.SetVisibleState(false);
        showSelItmArea = false;

        int itmInd = offset;
        for(Label *lbl : itemsLabels){
            lbl->SetColor(itmFntColor);
            lbl->SetCaption(items[itmInd]);

            if(itmInd == selectedItemIndex)
                MarkSelectedLabel(lbl, true);

            itmInd++;
        }

    });

    Control::SetSize({valArea.GetSize().x + showItemsBtn.GetSize().x, valArea.GetSize().y});
    Control::SetBottomRightPos(showItemsBtn.GetBottomRightPos());

    maxItmWidth = defaultWidth = emptyValueWidth;
}

void ComboBox::SetPos(const D3DXVECTOR2 &NewPos)
{
    D3DXVECTOR2 newPos = {Control::CheckPixelErrorForX(NewPos.x), Control::CheckPixelErrorForY(NewPos.y)};
    D3DXVECTOR2 delta = newPos - GetPos();
    delta = {Control::CheckPixelErrorForX(delta.x), Control::CheckPixelErrorForY(delta.y)};

    for(Control *cntrl : GetControls())
        cntrl->SetPos(cntrl->GetPos() + delta);
    
    Control::SetPos(newPos);
    Control::SetBottomRightPos(Control::GetBottomRightPos() + delta);

    D3DXVECTOR2 topLeft, bottomRight;
    showItemsBtn.GetItemsListBorders(topLeft, bottomRight);
    
    topLeft += delta;
    bottomRight += delta;

    showItemsBtn.SetItemsListBorders(topLeft, bottomRight);
}

void ComboBox::Invalidate(const D3DXVECTOR2 &CursorPos, float Tf)
{
    valLabel.Invalidate(CursorPos, Tf);
    showItemsBtn.Invalidate(CursorPos, Tf);
    itemsScrlBr.Invalidate(CursorPos, Tf);
    for(Label *lbl : itemsLabels)
        lbl->Invalidate(CursorPos, Tf);

    const Font &fnt = Manager::GetInstance()->GetTheme().GetFont();
    
    bool noLkdCntrl = Manager::GetInstance()->GetLockedControl() == NULL;
    bool btnPress = DirectInput::GetInsance()->IsMousePress(0);

    if(itemsArea.IsVisible() && noLkdCntrl && btnPress){
        bool inArea = CursorPos.x > itemsArea.GetPos().x && CursorPos.x < itemsArea.GetBottomRightPos().x &&
                      CursorPos.y > itemsArea.GetPos().y && CursorPos.y < itemsArea.GetBottomRightPos().y;
        if(inArea){
            for(Label *lbl : itemsLabels)
                lbl->SetColor(itmFntColor);

            float lclY = CursorPos.y - itemsArea.GetPos().y;
            int itmInd = (lclY / fnt.GetLineScreenHeight());
            int itmIndWithOffset = itmInd + ceil(itemsScrlBr.GetValue());

            valLabel.SetCaption(items[itmIndWithOffset]);

            MarkSelectedLabel(itemsLabels[itmInd], true);

            for(auto evnt : events)
                evnt.second(this, itmIndWithOffset);

            selectedItemIndex = itmIndWithOffset;
        }
    }
}

void ComboBox::AddItem(const std::wstring &NewItem) throw (Exception)
{
    if(itemsArea.IsVisible())
        for(Label* lbl : itemsLabels)
            Manager::GetInstance()->LowDrawPriority(lbl);

    if(std::find(items.begin(), items.end(), NewItem) != items.end())
        throw DublicateComboBoxItemException("");
    
    const Font &fnt = Manager::GetInstance()->GetTheme().GetFont();

    std::wstring itmStr;
    for(auto ch : NewItem)
        if(ch != '\n')
            itmStr += ch;

    float itmWidth = fnt.MeasureString(itmStr).x;
    if(itmWidth > maxItmWidth){
        maxItmWidth = itmWidth;

        valArea.SetSpaceAreaSize({maxItmWidth, fnt.GetLineScreenHeight()});
        showItemsBtn.SetPos({valArea.GetBottomRightPos().x, GetPos().y});
        itemsScrlBr.SetPos(valArea.GetBottomRightPos());
    }

    D3DXVECTOR2 tmsAreaSize;

    float newItemsCnt = (float)(items.size() + 1);

    if(items.size() < maxViewedItems){
        Label *lbl = new Label;
        lbl->Init(&fnt);
        lbl->SetCaption(NewItem);
        lbl->SetPos(D3DXVECTOR2(GetPos().x, valArea.GetBottomRightPos().y) + itemsArea.GetTopLeftOffset() + D3DXVECTOR2(0, (float)items.size() * fnt.GetLineScreenHeight()));
        lbl->SetVisibleState(false);

        itemsLabels.push_back(lbl);

        AddControl(lbl);

        tmsAreaSize= {valArea.GetSize().x + showItemsBtn.GetSize().x, newItemsCnt * fnt.GetLineScreenHeight()};

    }else{
        tmsAreaSize= {valArea.GetSize().x, (float)maxViewedItems * fnt.GetLineScreenHeight()};

        itemsScrlBr.SetMaxVal(newItemsCnt - (float)maxViewedItems);
    }
    
    itemsArea.SetSize(tmsAreaSize);
    selItmArea.SetSize({tmsAreaSize.x, fnt.GetLineScreenHeight()});

    showItemsBtn.SetItemsListBorders(itemsArea.GetPos(), {showItemsBtn.GetBottomRightPos().x, itemsArea.GetBottomRightPos().y});

    items.push_back(NewItem);

    Control::SetSize({valArea.GetSize().x + showItemsBtn.GetSize().x, valArea.GetSize().y});
    Control::SetBottomRightPos(showItemsBtn.GetBottomRightPos());

    if(itemsArea.IsVisible())
        for(Label* lbl : itemsLabels)
            Manager::GetInstance()->RiseDrawPriority(lbl);
}

void ComboBox::RemoveItem(const std::wstring &ItemToRemove)
{
    const Font &fnt = Manager::GetInstance()->GetTheme().GetFont();

    ItemsStorage::iterator itmIt = std::find(items.begin(), items.end(), ItemToRemove);
    if(itmIt == items.end())
        return;

    if(itemsArea.IsVisible())
        for(Label* lbl : itemsLabels)
            Manager::GetInstance()->LowDrawPriority(lbl);

    std::wstring selectedItem = L"";
    if(selectedItemIndex != -1)
        selectedItem = items[selectedItemIndex];

    items.erase(itmIt);

    if(ItemToRemove == selectedItem){
        valLabel.SetCaption(L"");
        selectedItemIndex = -1;
    }else
        for(int i = 0; i < items.size(); i++)
            if(items[i] == selectedItem){
                selectedItemIndex = i;
                break;
            }

    if(itemsScrlBr.GetMaxVal() > 1)
        itemsScrlBr.SetMaxVal(itemsScrlBr.GetMaxVal() - 1);

    int itmOffset = 0;
    if(items.size() > maxViewedItems)
        itmOffset = itemsScrlBr.GetValue();
    else if(items.size() == maxViewedItems)
        itemsScrlBr.SetVisibleState(false);
    else{
        itemsScrlBr.SetVisibleState(false);

        ItemsLabelsStorage::iterator it;
        for(it = itemsLabels.begin(); it != itemsLabels.end(); ++it)
            if((*it)->GetCaption() == ItemToRemove){
                RemoveControl(*it);
                delete *it;
                itemsLabels.erase(it);
                break;
            }
    }

    selItmArea.SetVisibleState(false);
    showSelItmArea = false;

    float newMaxItmWidth = emptyValueWidth;

    D3DXVECTOR2 pos = D3DXVECTOR2(GetPos().x, valArea.GetBottomRightPos().y) + itemsArea.GetTopLeftOffset();

    int posOffset = 0;
    for(Label *lbl : itemsLabels){
        lbl->SetColor(itmFntColor);
        lbl->SetCaption(items[itmOffset]);
        lbl->SetPos(pos + D3DXVECTOR2(0, (float)posOffset * fnt.GetLineScreenHeight()));
        
        posOffset++;
        itmOffset++;

        if(lbl->GetCaption() == selectedItem)
            MarkSelectedLabel(lbl, itemsArea.IsVisible());

        float itmWidth = fnt.MeasureString(lbl->GetCaption()).x;
        if(itmWidth > newMaxItmWidth)
            newMaxItmWidth = itmWidth;
    }

    if(newMaxItmWidth != maxItmWidth){

        maxItmWidth = newMaxItmWidth;

        valArea.SetSpaceAreaSize({maxItmWidth, fnt.GetLineScreenHeight()});
        showItemsBtn.SetPos({valArea.GetBottomRightPos().x, GetPos().y});
        itemsScrlBr.SetPos(valArea.GetBottomRightPos());
    }

    D3DXVECTOR2 tmsAreaSize;
    if(items.size() <= maxViewedItems)
        tmsAreaSize = {valArea.GetSize().x + showItemsBtn.GetSize().x, items.size() * fnt.GetLineScreenHeight()};
    else
        tmsAreaSize = {valArea.GetSize().x, (float)maxViewedItems * fnt.GetLineScreenHeight()};

    itemsArea.SetSize(tmsAreaSize);
    selItmArea.SetSize({tmsAreaSize.x, fnt.GetLineScreenHeight()});
    
    if(items.size() != 0)
        showItemsBtn.SetItemsListBorders(itemsArea.GetPos(), {showItemsBtn.GetBottomRightPos().x, itemsArea.GetBottomRightPos().y});
    else
        showItemsBtn.SetItemsListBorders({}, {});

    Control::SetSize({valArea.GetSize().x + showItemsBtn.GetSize().x, valArea.GetSize().y});
    Control::SetBottomRightPos(showItemsBtn.GetBottomRightPos());

    if(itemsArea.IsVisible())
        for(Label* lbl : itemsLabels)
            Manager::GetInstance()->RiseDrawPriority(lbl);
}

bool ComboBox::GetSelectedItem(std::wstring &SelectedItem) const
{
    if(selectedItemIndex == -1)
        return false;

    SelectedItem = items[selectedItemIndex];
    return true;
}

void ComboBox::SetSelectedItem(const std::wstring &SelectedItem) throw (Exception)
{
    size_t indx;
    for(indx = 0; indx < items.size(); indx++)
        if(items[indx] == SelectedItem)
            break;

    if(indx == items.size())
        throw ComboBoxItemNotFoundException("item not found");
    
    valLabel.SetCaption(SelectedItem);

    for(Label *lbl : itemsLabels)
        if(lbl->GetCaption() == SelectedItem)
            MarkSelectedLabel(lbl, itemsArea.IsVisible());

    for(auto evnt : events)
        evnt.second(this, indx);

    selectedItemIndex = indx;
}

const std::wstring &ComboBox::GetItem(INT Ind) const throw (Exception)
{
    if(Ind < 0 || Ind >= items.size())
        throw ComboBoxItemNotFoundException("Item index out of range");

    return items[Ind];
}

Utils::EventId ComboBox::AddOnShowListEvent(ItemsListEventsStorage::Event Event)
{
    return showItemsBtn.AddOnShowListEvent(Event);
}

void ComboBox::RemoveOnShowListEvent(Utils::EventId EventId)
{
    showItemsBtn.RemoveOnShowListEvent(EventId);
}

Utils::EventId ComboBox::AddOnHideListEvent(ItemsListEventsStorage::Event Event)
{
    return showItemsBtn.AddOnHideListEvent(Event);
}

void ComboBox::RemoveOnHideListEvent(Utils::EventId EventId)
{
    showItemsBtn.RemoveOnHideListEvent(EventId);
}

void ComboBox::OnManagerResetContext()
{
    HideItems();
    showItemsBtn.Disable();
};

}
