/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <windows.h>
#include <D3DHeaders.h>
#include <Exception.h>

typedef LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND InitWindow(HINSTANCE Instance, LONG ClientWidth, LONG ClientHeight, const std::wstring &Caption, MainWndProc WndProc) throw (Exception);

struct D3DParams
{
    LONG clientWidth = 0;
    LONG clientHeight = 0;
    bool isWindowed = false;
    HWND window = 0;
    UINT multisampleCount = 1;
    D3DParams(){}
    D3DParams(LONG ClientWidth, LONG ClientHeight, bool IsWindowed, HWND Window, UINT MultisampleCount = 1)
        :clientWidth(ClientWidth), clientHeight(ClientHeight), isWindowed(IsWindowed), window(Window),
         multisampleCount(MultisampleCount)
    {}
};

struct D3DData
{
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;	
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	D3D11_VIEWPORT screenViewport;
    UINT msaaQuality = 0;
};

D3DData InitD3D(const D3DParams &Params) throw (Exception);
void ChangeResolution(D3DData &Data, LONG NewWidth, LONG NewHeight) throw (Exception);
