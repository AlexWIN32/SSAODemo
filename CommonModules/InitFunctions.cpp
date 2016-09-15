/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <InitFunctions.h>
#include <Utils/AutoCOM.h>

HWND InitWindow(HINSTANCE Instance, LONG ClientWidth, LONG ClientHeight, const std::wstring &Caption, MainWndProc WndProc) throw (Exception)
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = Instance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if (!RegisterClass(&wc))
		throw Exception("RegisterClass Failed");

    DWORD style = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, ClientWidth, ClientHeight };
	AdjustWindowRect(&R, style, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	HWND mhMainWnd = CreateWindow(L"D3DWndClassName", Caption.c_str(),
		style, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, Instance, 0);
	if (!mhMainWnd)
		throw Exception("CreateWindow Failed");

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return mhMainWnd;
}

D3DData InitD3D(const D3DParams &Params) throw (Exception)
{
	D3DData data;

	D3D_FEATURE_LEVEL featureLevel;

	//Create device
	HRESULT hr = D3D11CreateDevice(
		NULL,                 // default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		0,                 // no software device
		D3D11_CREATE_DEVICE_DEBUG,// create flags
		0, 0,              // default feature level array
		D3D11_SDK_VERSION,
		&data.device,
		&featureLevel,
		&data.context);

	if (FAILED(hr))
		throw Exception("D3D11CreateDevice Failed");

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
		throw Exception("Direct3D Feature Level 11 unsupported");

    if(Params.multisampleCount > D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT)
        throw D3DException("invalid multisample count");

	// Check MSAA quality support for our back buffer format.
	HR(data.device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, Params.multisampleCount, &data.msaaQuality));

	if(data.msaaQuality <= 0)
        throw D3DException("MSAA not supported");

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = Params.clientWidth;
	sd.BufferDesc.Height = Params.clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = Params.multisampleCount;
	sd.SampleDesc.Quality = data.msaaQuality - 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = Params.window;
	sd.Windowed = Params.isWindowed;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."
	IDXGIDevice* dxgiDevice = 0;
	HR(data.device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	HR(dxgiFactory->CreateSwapChain(data.device, &sd, &data.swapChain));

    dxgiFactory->MakeWindowAssociation(Params.window, DXGI_MWA_NO_ALT_ENTER);

	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	// Get the pointer to the back buffer.	
	ID3D11Texture2D* backBuffer;
	HR(data.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));

	// Create the render target view with the back buffer pointer.
	HR(data.device->CreateRenderTargetView(backBuffer, 0, &data.renderTargetView));
	ReleaseCOM(backBuffer);

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = Params.clientWidth;
	depthStencilDesc.Height = Params.clientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count =  Params.multisampleCount;
	depthStencilDesc.SampleDesc.Quality = data.msaaQuality - 1;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

    ID3D11Texture2D* depthStencilBuffer;
	HR(data.device->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBuffer));

	HR(data.device->CreateDepthStencilView(depthStencilBuffer, NULL, &data.depthStencilView));

    ReleaseCOM(depthStencilBuffer);

	// Bind the render target view and depth/stencil view to the pipeline.
	data.context->OMSetRenderTargets(1, &data.renderTargetView, data.depthStencilView);

	// Set the viewport transform.
	data.screenViewport.TopLeftX = 0;
	data.screenViewport.TopLeftY = 0;
	data.screenViewport.Width = static_cast<float>(Params.clientWidth);
	data.screenViewport.Height = static_cast<float>(Params.clientHeight);
	data.screenViewport.MinDepth = 0.0f;
	data.screenViewport.MaxDepth = 1.0f;

	data.context->RSSetViewports(1, &data.screenViewport);

	return data;
}

void ChangeResolution(D3DData &Data, LONG NewWidth, LONG NewHeight) throw (Exception)
{
    ID3D11RenderTargetView* nullRTV = NULL;
    Data.context->OMSetRenderTargets(1, &nullRTV, NULL);

    Data.renderTargetView->Release();
    Data.depthStencilView->Release();

    Data.context->ClearState();
    
	DXGI_SWAP_CHAIN_DESC scDesc;
    HR(Data.swapChain->GetDesc(&scDesc), []{ return D3DException("cant get swap chain description");});

    HR(Data.swapChain->ResizeBuffers(scDesc.BufferCount, 
        NewWidth, 
        NewHeight, 
        scDesc.BufferDesc.Format,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH),
       []{ return D3DException("cant resize swap chain buffers");});

	// Get the pointer to the back buffer.	
	ID3D11Texture2D* backBufferPtr;
	HR(Data.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferPtr)));

    Utils::AutoCOM<ID3D11Texture2D> backBuffer = backBufferPtr;
	// Create the render target view with the back buffer pointer.
	HR(Data.device->CreateRenderTargetView(backBuffer, 0, &Data.renderTargetView));

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = NewWidth;
	depthStencilDesc.Height = NewHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

    ID3D11Texture2D* depthStencilBufferPtr;
	HR(Data.device->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBufferPtr));

    Utils::AutoCOM<ID3D11Texture2D> depthStencilBuffer = depthStencilBufferPtr;
 
    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
	DepthStencilViewDesc.Format = depthStencilDesc.Format;
	DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	HR(Data.device->CreateDepthStencilView(depthStencilBuffer, &DepthStencilViewDesc, &Data.depthStencilView));

	// Bind the render target view and depth/stencil view to the pipeline.
	Data.context->OMSetRenderTargets(1, &Data.renderTargetView, Data.depthStencilView);

	// Set the viewport transform.
	Data.screenViewport.TopLeftX = 0;
	Data.screenViewport.TopLeftY = 0;
	Data.screenViewport.Width = static_cast<float>(NewWidth);
	Data.screenViewport.Height = static_cast<float>(NewHeight);
	Data.screenViewport.MinDepth = 0.0f;
	Data.screenViewport.MaxDepth = 1.0f;

	Data.context->RSSetViewports(1, &Data.screenViewport);

}
