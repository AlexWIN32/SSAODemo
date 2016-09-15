/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once 
#include <D3DHeaders.h>
#include <GUI/Control.h>
#include <GUI/Theme.h>
#include <string>
#include <map>

namespace GUI
{

class Manager
{
private:
    typedef INT StringHash;
    typedef std::map<StringHash, ID3D11ShaderResourceView *> TexturesStorage;
    typedef std::map<StringHash, Theme> ThemesStorage;
    enum DrawingPriority
    {
        DP_LOW,
        DP_MEDIUM,
        DP_HIGH
    };
    typedef std::map<DrawingPriority, DrawingObjectsStorage> SortedDrawingObjectsStorage;
    ThemesStorage themes;
    ControlsStorage controls;
    TexturesStorage textures;
    Shaders::VertexShader vs;
    Shaders::PixelShader ps;
    Shaders::GeometryShader gs;
    ID3D11Buffer *dataVb;
    D3DXVECTOR2 prevCursorPos, cursorDelta;
    INT cotrolsCount;
    StringHash currentTheme;
    Control *lockedControl;
    ControlsStorage drawPriorityControls;
    static const INT MaxControlsCount = 10000;
    void CollectDrawingObjects(const Control *Cntrl, SortedDrawingObjectsStorage &AllObjects, DrawingPriority ParentPriority);
    static Manager *instance;
    Manager() : dataVb(NULL), cotrolsCount(0), currentTheme(0), lockedControl(NULL){}
    ~Manager();
public:
    static Manager *GetInstance()
    {
        if(!instance)
            instance = new Manager();
        return instance;
    }
    static void ReleaseInstance()
    {
        delete instance;
        instance = NULL;
    }
    BOOL TryToLockControl(Control *ControlToLock);
    void UnlockControl(Control *ControlToUnlock);
    void RiseDrawPriority(Control *ControlToRise);
    void LowDrawPriority(Control *ControlToLow);
    Control * GetLockedControl() {return lockedControl;}
    Manager(Manager &) = delete;
    Manager & operator=(Manager &) = delete;
    ID3D11ShaderResourceView *GetTexture(const std::string &FileName) throw (Exception);
    void Init() throw (Exception);
    void AddExternalTheme(const Theme &NewTheme) throw (Exception);
    const Theme &GetTheme(const std::string &ThemeName = "") throw (Exception);
    void SetTheme(const std::string &NewThemeName) throw (Exception);
    void AddControl(Control *NewControl);
    void RemoveControl(Control *ControlToRemove);
    void Invalidate(float Tf);
    void Draw() throw (Exception);
    void Invalidate(float Tf, const ControlsStorage &Controls);
    void Draw(const ControlsStorage &Controls) throw (Exception);
    const D3DXVECTOR2 &GetCursorDelta(){return cursorDelta;}
    void ResetContext();
};

}
