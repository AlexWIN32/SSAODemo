/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <D3DHeaders.h>

class DeviceKeeper
{
private:
	static ID3D11Device* device;
	static ID3D11DeviceContext* context;
    static ID3D11RenderTargetView* renderTargetView;
	static ID3D11DepthStencilView* depthStencilView;
    static IDXGISwapChain* swapChain;
    static UINT multisampleCount, multisampleQuality;
public:
	static void SetDevice(ID3D11Device *Device){ device = Device; }
    static ID3D11Device *GetDevice(){ return device; }
	static void SetDeviceContext(ID3D11DeviceContext *Context){ context = Context; }	
	static ID3D11DeviceContext*GetDeviceContext(){ return context; }
    static void SetRenderTargetView(ID3D11RenderTargetView* RenderTargetView) {renderTargetView = RenderTargetView;} 
    static ID3D11RenderTargetView* GetRenderTargetView() {return renderTargetView;} 
    static void SetDepthStencilView(ID3D11DepthStencilView* DepthStencilView) {depthStencilView = DepthStencilView;} 
    static ID3D11DepthStencilView* GetDepthStencilView() {return depthStencilView;}
    static void SetSwapChain(IDXGISwapChain* SwapChain) {swapChain = SwapChain;}
    static IDXGISwapChain* GetSwapChain(){return swapChain;}
    static void SetMultisampleCount(UINT MiltisampleCount){multisampleCount = MiltisampleCount;}
    static UINT GetMultisampleCount(){return multisampleCount;}
    static void SetMultisampleQuality(UINT MiltisampleQuality){multisampleQuality = MiltisampleQuality;}
    static UINT GetMultisampleQuality(){return multisampleQuality;}
};
