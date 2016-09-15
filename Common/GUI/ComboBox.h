/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <GUI/Area.h>
#include <GUI/Label.h>
#include <GUI/ScrollBar.h>
#include <GUI/Button.h>
#include <Utils.h>

namespace GUI
{

DECLARE_EXCEPTION(ComboBoxException);
DECLARE_CHILD_EXCEPTION(DublicateComboBoxItemException, ComboBoxException);
DECLARE_CHILD_EXCEPTION(ComboBoxItemNotFoundException, ComboBoxException);

class ComboBox : 
    public Control,
    public Utils::EventsStorage<void(const ComboBox*, INT)>
{
public:
    typedef Utils::EventsStorage<void()> ItemsListEventsStorage;
protected:
    typedef std::vector<std::wstring> ItemsStorage;
    class Button : public GUI::Button
    {
    public:
        
    private:
        D3DXVECTOR2 itemsListTopLeft, itemsListBottomRight;
        ItemsListEventsStorage onShowListEvents, onHideListEvents;
        BOOL lock;
    public:
        Button() : lock(false) {}
        void SetText(const std::wstring &Text) throw (Exception) = delete;
        void SetImage(const ImageData& Image) throw (Exception) = delete;
        virtual void Invalidate(const D3DXVECTOR2 &CursorPos, float Tf);
        void SetItemsListBorders(D3DXVECTOR2 TopLeft, D3DXVECTOR2 BottomRight)
        {
            itemsListTopLeft = TopLeft;
            itemsListBottomRight = BottomRight;
        }
        void GetItemsListBorders(D3DXVECTOR2 &TopLeft, D3DXVECTOR2 &BottomRight)
        {
            TopLeft = itemsListTopLeft;
            BottomRight = itemsListBottomRight;
        }
        Utils::EventId AddOnShowListEvent(ItemsListEventsStorage::Event Event)
        {
            return onShowListEvents.AddEvent(Event);
        }
        void RemoveOnShowListEvent(Utils::EventId EventId)
        {
            onShowListEvents.RemoveEvent(EventId);
        }
        Utils::EventId AddOnHideListEvent(ItemsListEventsStorage::Event Event)
        {
            return onHideListEvents.AddEvent(Event);
        }
        void RemoveOnHideListEvent(Utils::EventId EventId)
        {
            onHideListEvents.RemoveEvent(EventId);
        }
        void Disable();
    };
    INT maxViewedItems;
    INT selectedItemIndex;
    Area valArea, itemsArea, selItmArea;
    Label valLabel;
    ItemsStorage items;
    Button showItemsBtn;
    ScrollBar itemsScrlBr;
    FLOAT defaultWidth, maxItmWidth;
    typedef std::vector<Label*> ItemsLabelsStorage;
    ItemsLabelsStorage itemsLabels;
    D3DXCOLOR itmFntColor, slItmFntColor;
    BOOL showSelItmArea;
    FLOAT emptyValueWidth;
    void SetSize(const D3DXVECTOR2 &NewSize) {Control::SetSize(NewSize);}
    void MarkSelectedLabel(Label *SelectedLabel, BOOL IsVisible);
    void ShowItems();
    void HideItems();
public:
    ~ComboBox();
    ComboBox() : maxViewedItems(0), selectedItemIndex(-1), defaultWidth(0.0f), maxItmWidth(0.0f), showSelItmArea(false), emptyValueWidth(0.0f) {}
    virtual void Init() throw (Exception);
    virtual DrawingObjectsStorage GetDrawingObjects() const {return DrawingObjectsStorage();}
    virtual void SetPos(const D3DXVECTOR2 &NewPos);
    virtual void Invalidate(const D3DXVECTOR2 &CursorPos, float Tf);
    virtual void OnManagerResetContext();
    void AddItem(const std::wstring &NewItem) throw (Exception);
    void RemoveItem(const std::wstring &ItemToRemove);
    ItemsStorage::const_iterator begin() const { return items.begin();}
    ItemsStorage::const_iterator end() const { return items.end();}
    size_t GetItemsCount() const {return items.size();}
    const std::wstring &GetItem(INT Ind) const throw (Exception);
    bool GetSelectedItem(std::wstring &SelectedItem) const;
    void SetSelectedItem(const std::wstring &SelectedItem) throw (Exception);
    Utils::EventId AddOnShowListEvent(ItemsListEventsStorage::Event Event);
    void RemoveOnShowListEvent(Utils::EventId EventId);
    Utils::EventId AddOnHideListEvent(ItemsListEventsStorage::Event Event);
    void RemoveOnHideListEvent(Utils::EventId EventId);
};

}