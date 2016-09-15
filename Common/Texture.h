/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Exception.h>
#include <windows.h>
#include <functional>
#include <Vector2Fwd.h>

struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;

enum DXGI_FORMAT;

namespace Texture
{

DECLARE_EXCEPTION(TextureException);

typedef std::function<void(UCHAR*, const POINT &)> PixelOperator;

ID3D11ShaderResourceView * LoadTexture2DFromFile(const std::wstring &FileName) throw (Exception);
ID3D11ShaderResourceView * LoadTexture2DFromFile(const std::string &FileName) throw (Exception);
ID3D11ShaderResourceView * LoadTexture2DFromFile(const std::wstring &Path, const Point2 &Pos, const SizeUS &RegionSize) throw (Exception);
ID3D11ShaderResourceView * CreateTexture2D(const SizeUS &Size, DXGI_FORMAT Format, const char* Data) throw (Exception);
ID3D11ShaderResourceView * CreateTexture2D(const SizeUS &Size, DXGI_FORMAT Format, const PixelOperator &Operator) throw (Exception);

DECLARE_EXCEPTION(RenderTargetException);

class RenderTarget
{
private:
    ID3D11RenderTargetView *rtv;
    ID3D11ShaderResourceView *srv;
    USHORT width, height;
    DXGI_FORMAT format;
public:
    RenderTarget(const RenderTarget&);
    RenderTarget &operator= (const RenderTarget&);
    RenderTarget();
    virtual ~RenderTarget();
    USHORT GetWidth() const {return width;} 
    USHORT GetHeight() const {return height;} 
    DXGI_FORMAT GetFormat() const {return format;}
    ID3D11RenderTargetView *GetRenderTargetView() const {return rtv;}
    ID3D11ShaderResourceView *GetSahderResourceView() const {return srv;}
    void Init(DXGI_FORMAT Format, BOOL UseViewport = false, UINT ViewportInd = -1) throw (Exception);
    void Init(DXGI_FORMAT Format, USHORT Width, USHORT Height) throw (Exception);
};

class RenderTargetCube
{
private:
    ID3D11RenderTargetView *rtv[6];
    ID3D11ShaderResourceView *srv;
    USHORT width, height;
    DXGI_FORMAT format;
    void Construct(const RenderTargetCube &Val);
public:
    RenderTargetCube(const RenderTargetCube&);
    RenderTargetCube &operator= (const RenderTargetCube&);
    RenderTargetCube();
    virtual ~RenderTargetCube();
    USHORT GetWidth() const {return width;} 
    USHORT GetHeight() const {return height;} 
    DXGI_FORMAT GetFormat() const {return format;}
    void Init(DXGI_FORMAT Format, USHORT Size, UINT MipLevelsCnt = 1) throw (Exception);
    ID3D11RenderTargetView *GetRenderTargetView(UINT Edge) const throw (Exception);
    ID3D11ShaderResourceView *GetShaderResourceView() const {return srv;}
};


}