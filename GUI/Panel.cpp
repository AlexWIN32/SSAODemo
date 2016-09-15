/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/Panel.h>
#include <numeric>

namespace GUI
{

FixedPanel::~FixedPanel()
{
    for(auto it : controlsToEvents)
        it.first->RemoveOnChangeSizeEvent(it.second);

    for(Control *ownCntrl : ownedControls)
        delete ownCntrl;
}

void FixedPanel::Clear()
{
    for(auto it : controlsToEvents)
        it.first->RemoveOnChangeSizeEvent(it.second);
    
    controlsToEvents.clear();

    for(Control *ownCntrl : ownedControls)
        delete ownCntrl;
    
    ownedControls.clear();

    table.clear();
    columnsOffsets.clear();
    rowsOffsets.clear();
}

void FixedPanel::CalculateOffsets()
{
    columnsOffsets.resize(table[0].size());
    rowsOffsets.resize(table.size());

    std::vector<float> columnsMaxWidths, rowsMaxHeights;

    for(int cl = 0; cl < table[0].size(); ++cl){
        float maxWidth = 0;

        for(const Row &row : table){
            Control *cntrl = row[cl];
            if(cntrl && cntrl->GetSize().x > maxWidth)
                maxWidth = cntrl->GetSize().x;
        }

        columnsMaxWidths.push_back(maxWidth + colSpacing);
    }

    for(int rw = 0; rw < table.size(); ++rw){
        float maxHeight = 0;

        for(Control *cntrl : table[rw])
            if(cntrl && cntrl->GetSize().y > maxHeight)
                maxHeight = cntrl->GetSize().y;

        rowsMaxHeights.push_back(maxHeight + rowSpacing);
    }

    for(int rw = 0; rw < table.size(); ++rw){
        float offst = 0.0f;

        for(int rw2 = 0; rw2 < rw; ++rw2)
            offst += rowsMaxHeights[rw2];

        rowsOffsets[rw] = Control::CheckPixelErrorForY(offst);
    }

    for(int cl = 0; cl < table[0].size(); ++cl){
        float offst = 0.0f;

        for(int cl2 = 0; cl2 < cl; ++cl2)
            offst += Control::WigthToX(columnsMaxWidths[cl2]);

        columnsOffsets[cl] = Control::CheckPixelErrorForX(offst);
    }
}

void FixedPanel::ExpandTable(INT Column, INT Row)
{
    if(table.size() == 0)
    {
        table.push_back(FixedPanel::Row(1, NULL));
    }
    else if(table.size() <= Row || table[0].size() <= Column){
                
        int colIndDelta = Column - (table[0].size() - 1);
        if(colIndDelta > 0)
            for(FixedPanel::Row &row : table)
                row.insert(row.end(), colIndDelta, NULL);
        else
            colIndDelta = 0;

        int rowIndDelta = Row - (table.size() - 1);
        if(rowIndDelta > 0){
            FixedPanel::Row newRow(table[0].size() + colIndDelta, NULL);
            table.insert(table.end(), rowIndDelta, newRow);
        }
    }
}

void FixedPanel::Refresh()
{
    CalculateOffsets();

    D3DXVECTOR2 maxPos(0, 0);

    for(int rw = 0; rw < table.size(); ++rw){
        const Row &row = table[rw];
        for(int cl = 0; cl < row.size(); ++cl){
            D3DXVECTOR2 offset(columnsOffsets[cl], rowsOffsets[rw]);

            Control * cntrl = row[cl];
            if(cntrl){
                cntrl->SetPos(GetPos() + offset);

                if(maxPos.x < cntrl->GetBottomRightPos().x)
                    maxPos.x = cntrl->GetBottomRightPos().x;

                if(maxPos.y < cntrl->GetBottomRightPos().y)
                    maxPos.y = cntrl->GetBottomRightPos().y;
            }
        }
    }

    D3DXVECTOR2 posOffset = maxPos - GetPos();

    D3DXVECTOR2 newSize(Control::XToWigth(posOffset.x), posOffset.y);
    newSize.x = Control::CheckPixelErrorForY(newSize.x);
    newSize.y = Control::CheckPixelErrorForY(newSize.y);
    Control::SetSize(newSize);

    D3DXVECTOR2 newBtmRghtPos = GetPos() + posOffset;
    newBtmRghtPos.x = Control::CheckPixelErrorForX(newBtmRghtPos.x);
    newBtmRghtPos.y = Control::CheckPixelErrorForY(newBtmRghtPos.y);
    Control::SetBottomRightPos(GetPos() + posOffset);
}

void FixedPanel::SetControl(Control *NewControl, INT Column, INT Row, BOOL PassOwnership)
{
    ExpandTable(Column, Row);

    Control *oldControl = table[Row][Column];

    if(oldControl != NULL)
        Control::RemoveControl(oldControl);

    table[Row][Column] = NewControl;

    if(NewControl != NULL){
        Utils::EventId evntId = NewControl->AddOnChangeSizeEvent([this](const Control*, const D3DXVECTOR2&){Refresh();});
        controlsToEvents.insert({NewControl, evntId});
    
         Control::AddControl(NewControl);
    }

    if(PassOwnership)
        ownedControls.push_back(NewControl);

    Refresh();
}

void FixedPanel::RemoveControl(Control *ControlToRemove)
{
    if(!ControlToRemove)
        return;

    for(Row &row : table)
        for(Control * &cntrl : row){
            if(cntrl != ControlToRemove)
                return;

            Control::RemoveControl(ControlToRemove);

            auto it = controlsToEvents.find(ControlToRemove);

            if(it != controlsToEvents.end()){
                ControlToRemove->RemoveOnChangeSizeEvent(it->second);
                controlsToEvents.erase(it);
            }

            auto ocIt = std::find(ownedControls.begin(), ownedControls.end(), ControlToRemove);
            if(ocIt != ownedControls.end()){
                delete *ocIt;
                ownedControls.erase(ocIt);
            }
        }
}

void FixedPanel::SetPos(const D3DXVECTOR2 &NewPos)
{
    D3DXVECTOR2 newPos = {Control::CheckPixelErrorForX(NewPos.x), Control::CheckPixelErrorForY(NewPos.y)};

    D3DXVECTOR2 delta = newPos - GetPos();

    for(Control *cntrl : GetControls())
        cntrl->SetPos(cntrl->GetPos() + delta);

    Control::SetPos(newPos);
    Control::SetBottomRightPos(GetPos() + D3DXVECTOR2(Control::WigthToX(GetSize().x), GetSize().y));
}

Control *FixedPanel::GetControl(INT Row, INT Column) const throw (Exception)
{
    if(Row < 0 || Row >= table.size())
        throw InvalidRowException("");

    if(Column < 0 || Column >= table[0].size())
        throw InvalidColumnException("");

    return table[Row][Column];
}

void FixedPanel::SetVisibleState(bool IsVisible)
{
    for(Control *cntrl : GetControls())
        cntrl->SetVisibleState(IsVisible);
}

void FixedPanel::Invalidate(const D3DXVECTOR2 &CursorPos, float Tf)
{
    for(Control *cntrl : GetControls())
        cntrl->Invalidate(CursorPos, Tf);
}

void FixedPanel::OnManagerResetContext()
{
    for(Control *cntrl : GetControls())
        cntrl->OnManagerResetContext();
}

}