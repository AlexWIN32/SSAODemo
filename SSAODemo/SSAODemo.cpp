/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include "SSAODemo.h"
#include "LoadingScreen.h"
#include "OptionsMenu.h"
#include "Application.h"
#include <DeviceKeeper.h>
#include <Meshes.h>
#include <Timer.h>
#include <DirectInput.h>
#include <PostProcess.h>
#include <CommonParams.h>
#include <RenderStatesManager.h>
#include <AdapterManager.h>
#include <GUI.h>

ID3D11Device *DeviceKeeper::device = NULL;
ID3D11DeviceContext *DeviceKeeper::context = NULL;
ID3D11RenderTargetView* DeviceKeeper::renderTargetView = NULL;
ID3D11DepthStencilView* DeviceKeeper::depthStencilView = NULL;
IDXGISwapChain* DeviceKeeper::swapChain = NULL;
UINT DeviceKeeper::multisampleCount = 0;
UINT DeviceKeeper::multisampleQuality = 0;

FLOAT CommonParams::screenWidth = 0.0f;
FLOAT CommonParams::screenHeight = 0.0f;
FLOAT CommonParams::widthOverHeight = 0.0f;
FLOAT CommonParams::heightOverWidth = 0.0f;
D3DXVECTOR2 CommonParams::pixelSize;
HWND CommonParams::window;

static LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if(msg == WM_DESTROY){
        PostQuitMessage(0);
        return 0;
    }

    if(Demo::Application::GetInstance()->ProcessMessage(hwnd, msg, wParam, lParam))
        return 0;

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
    srand(time(NULL));

	const LONG width = 800;
	const LONG height = 600;

	try{
		HWND hWindow = InitWindow(hInstance, width, height, L"SSAO Demo", MsgProc);
		
		D3DParams params;
		params.clientWidth = width;
		params.clientHeight = height;
		params.isWindowed = true;
		params.window = hWindow;

        D3DData d3d;
		d3d = InitD3D(params);

		DeviceKeeper::SetDevice(d3d.device);
		DeviceKeeper::SetDeviceContext(d3d.context);
        DeviceKeeper::SetDepthStencilView(d3d.depthStencilView);
        DeviceKeeper::SetRenderTargetView(d3d.renderTargetView);
        DeviceKeeper::SetSwapChain(d3d.swapChain);
        DeviceKeeper::SetMultisampleCount(1);
        DeviceKeeper::SetMultisampleQuality(d3d.msaaQuality);

        CommonParams::SetScreenSize(width, height);
        CommonParams::SetWindow(hWindow);

		DirectInput::GetInsance()->Init(hInstance, hWindow);

        Demo::Application::GetInstance()->Load();

	}catch(const Exception &ex){
        MessageBoxA(0, ex.What().c_str(), 0, 0);
        return 0;
	}		

	MSG msg = { 0 };
	
	while (msg.message != WM_QUIT)			
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}else{
			DirectInput::GetInsance()->Poll();							

            Demo::Application::GetInstance()->Run();
		}
	
    Demo::Application::ReleaseInstance();

	return (int)msg.wParam;	
}
