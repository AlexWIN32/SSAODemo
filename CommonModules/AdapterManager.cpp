/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <AdapterManager.h>
#include <Utils/AutoCOM.h>
#include <Utils/ToString.h>
#include <DeviceKeeper.h>
#include <InitFunctions.h>

namespace DisplaySettings
{

AdapterManager *AdapterManager::instance = NULL;

void AdapterManager::Init() throw (Exception)
{
    IDXGIFactory1 *factoryPtr = NULL;
    HR(CreateDXGIFactory1(__uuidof(IDXGIFactory),
                        reinterpret_cast<void**>(&factoryPtr)),
                        [](){ return AdapterManagerException("Cant create GI Factory");});
    
    Utils::AutoCOM<IDXGIFactory1> pFactory = factoryPtr;

    IDXGIAdapter *adapterPtr;
    HR(pFactory->EnumAdapters(0, &adapterPtr), 
        [](){return AdapterManagerException("Cant get adapter info");});

    Utils::AutoCOM<IDXGIAdapter> pAdapter = adapterPtr;

    DXGI_ADAPTER_DESC desc;
    HR(pAdapter->GetDesc(&desc), [](){return AdapterManagerException("Cant get adapter description");});

    name = desc.Description;

    IDXGIOutput* outputPtr = NULL; 
    HR(pAdapter->EnumOutputs(0, &outputPtr), [](){return AdapterManagerException("Cant get display params");});

    Utils::AutoCOM<IDXGIOutput> pOutput = outputPtr;

    UINT numModes = 0;
    HR(pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, NULL), 
       [](){return AdapterManagerException("Cant get display modes");});

    if(numModes != 0){
        std::vector<DXGI_MODE_DESC> displayModesDesc(numModes);

        HR(pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, &displayModesDesc[0]), 
           [](){return AdapterManagerException("Cant get display modes");});

        for(const auto &desc : displayModesDesc){
            SizeUS dispMode(desc.Width, desc.Height);

            if(std::find(displayModes.begin(), displayModes.end(), dispMode) == displayModes.end())
                displayModes.push_back(dispMode);
        }
    }
}

void AdapterManager::ChangeResolution(const SizeUS &NewResolution) throw (Exception)
{
    if(std::find(displayModes.begin(), displayModes.end(), NewResolution) == displayModes.end())
        throw AdapterManagerException("Resolution " + Utils::to_string(NewResolution.width) + "x" + 
                                                      Utils::to_string(NewResolution.height) + " not supported"); 

    newResolution = NewResolution;

    DXGI_SWAP_CHAIN_DESC scDesc;
    HR(DeviceKeeper::GetSwapChain()->GetDesc(&scDesc), []{ return D3DException("cant get swap chain description");});
    
    DXGI_MODE_DESC targetDesc = {};
    targetDesc.Format = scDesc.BufferDesc.Format;
    targetDesc.Width = NewResolution.width;
    targetDesc.Height = NewResolution.height;

    DeviceKeeper::GetSwapChain()->ResizeTarget(&targetDesc);
}

bool AdapterManager::OnResolutionChangeResizing(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
    if(Msg != WM_SIZE || DeviceKeeper::GetDeviceContext() == NULL)
        return false;

    if(newResolution.width == 0 || newResolution.height == 0)
        return false;

    D3DData data;
    data.context = DeviceKeeper::GetDeviceContext();
    data.depthStencilView = DeviceKeeper::GetDepthStencilView();
    data.device = DeviceKeeper::GetDevice();
    data.renderTargetView = DeviceKeeper::GetRenderTargetView();
    data.swapChain = DeviceKeeper::GetSwapChain();

    ::ChangeResolution(data, newResolution.width, newResolution.height);

    DeviceKeeper::SetDepthStencilView(data.depthStencilView);
    DeviceKeeper::SetRenderTargetView(data.renderTargetView);

    return true;
}

}