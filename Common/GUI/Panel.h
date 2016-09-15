/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <GUI/Control.h>
#include <Utils/EventsStorage.h>
#include <vector>
#include <map>

namespace GUI
{

DECLARE_EXCEPTION(PanelException);
DECLARE_CHILD_EXCEPTION(InvalidRowException, PanelException);
DECLARE_CHILD_EXCEPTION(InvalidColumnException, PanelException);

class FixedPanel : public Control
{
private:
    typedef std::vector<Control*> Row;
    typedef std::vector<Row> Table;
    typedef std::map<Control*, Utils::EventId> ControlsToEventsMapping;
    ControlsToEventsMapping controlsToEvents;
    std::vector<float> columnsOffsets, rowsOffsets;
    Table table;
    ControlsStorage ownedControls;
    FLOAT colSpacing, rowSpacing;
    virtual void SetSize(const D3DXVECTOR2 &Size) { Control::SetSize(Size); }
    void Refresh();
    void CalculateOffsets();
    void ExpandTable(INT Column, INT Row);
public:
    ~FixedPanel();
    virtual void SetPos(const D3DXVECTOR2 &Pos);
    void SetControl(Control *NewControl, INT Column, INT Row, BOOL PassOwnership = false);
    void RemoveControl(Control *ControlToRemove);
    size_t GetRowCount() const { return table.size();}
    size_t GetColumnCount(INT Row) const throw (Exception);
    Control *GetControl(INT Row, INT Column) const throw (Exception);
    virtual DrawingObjectsStorage GetDrawingObjects() const {return DrawingObjectsStorage();}
    FLOAT GetColSpacing() const {return colSpacing;}
    void SetColSpacing(FLOAT ColSpacing){colSpacing = ColSpacing;}
    FLOAT GetRowSpacing() const {return rowSpacing;}
    void SetRowSpacing(FLOAT RowSpacing){rowSpacing = RowSpacing;}
    virtual void SetVisibleState(bool IsVisible);
    virtual void Invalidate(const D3DXVECTOR2 &CursorPos, float Tf);
    virtual void OnManagerResetContext();
    const ControlsStorage &GetControls() const{return Control::GetControls();}
    void Clear();
};

}