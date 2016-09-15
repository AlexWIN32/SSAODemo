/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <D3DHeaders.h>
#include <Vector2.h>
#include <vector>
#include <string>

namespace DisplaySettings
{

DECLARE_EXCEPTION(AdapterManagerException)

class AdapterManager
{
public:
    typedef std::vector<SizeUS> DisplayModesStorage;
private:
    DisplayModesStorage displayModes;
    std::wstring name;
    static AdapterManager *instance;
    SizeUS newResolution;
    AdapterManager(){}
public:
    static AdapterManager *GetInstance()
    {
        if(instance == NULL)
            instance = new AdapterManager();
        return instance;
    }
    static void ReleaseInstance()
    {
        delete instance;
        instance = NULL;
    }
    AdapterManager(AdapterManager &) = delete;
    AdapterManager &operator=(AdapterManager &) = delete;
    const DisplayModesStorage &GetDisplayModes() const {return displayModes;}
    void Init() throw (Exception);
    const std::wstring &GetName() const {return name;}
    void ChangeResolution(const SizeUS &NewResolution) throw (Exception);
    bool OnResolutionChangeResizing(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam);
};

}