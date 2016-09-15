/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <D2DBaseTypes.h>
typedef D3DCOLORVALUE DXGI_RGBA;

#include <Texture.h>
#include "dxgiformat.h"
#include <D3DHeaders.h>
#include <DirectXTex.h>
#include <DeviceKeeper.h>
#include <CommonParams.h>
#include <Vector2.h>
#include <locale>
#include <codecvt>
#include <vector>
#include <Utils.h>
using namespace DirectX;

namespace Texture
{

#ifdef LoadImage
#undef LoadImage
#endif

static void LoadImage(const std::wstring &FilePath, ScratchImage &Image, TexMetadata &ImageInfo) throw (Exception)
{
    size_t dotPos = FilePath.find_last_of('.');
    if (dotPos == std::string::npos)
        throw TextureException(L"Unknown extension for " + FilePath);

    std::wstring ext = FilePath.substr(dotPos + 1);

    if (ext == L"DDS" || ext == L"dds")
        HR(LoadFromDDSFile(FilePath.c_str(), DDS_FLAGS_NONE, &ImageInfo, Image));
    else if (ext == L"tga" || ext == L"TGA")
        HR(LoadFromTGAFile(FilePath.c_str(), &ImageInfo, Image));
    else
        HR(LoadFromWICFile(FilePath.c_str(), 0, &ImageInfo, Image));

    if (ImageInfo.depth > 1)
        throw TextureException(FilePath + L": cant load cubemap or 3D texture");
}

ID3D11ShaderResourceView * LoadTexture2DFromFile(const std::wstring &FileName) throw (Exception)
{
	ScratchImage img;
	TexMetadata info;

    LoadImage(FileName, img, info);

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Height = info.height;
	textureDesc.Width = info.width;
	textureDesc.MipLevels = info.mipLevels;
	textureDesc.ArraySize = 1;
	textureDesc.Format = info.format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;	
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D* texture;
	HR(DeviceKeeper::GetDevice()->CreateTexture2D(&textureDesc, NULL, &texture));
    Utils::AutoCOM<ID3D11Texture2D> texturePtr = texture;

	size_t rowPitch, slicePitch;
	ComputePitch(info.format, info.width, info.height, rowPitch, slicePitch);
	
	const Image *img2 = img.GetImage(0, 0, 0);
	DeviceKeeper::GetDeviceContext()->UpdateSubresource(texturePtr.Get(), 0, NULL, img2->pixels, rowPitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	ID3D11ShaderResourceView* textureSRV;	
	HR(DeviceKeeper::GetDevice()->CreateShaderResourceView(texturePtr.Get(), &srvDesc, &textureSRV));
	
	DeviceKeeper::GetDeviceContext()->GenerateMips(textureSRV);

	return textureSRV;
}

ID3D11ShaderResourceView * LoadTexture2DFromFile(const std::string &FileName) throw (Exception)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return LoadTexture2DFromFile(converter.from_bytes(FileName));
}

ID3D11ShaderResourceView* LoadTexture2DFromFile(const std::wstring &Path, const Point2 &Pos, const SizeUS &RegionSize)
{
    DirectX::ScratchImage img;
    DirectX::TexMetadata info;

    LoadImage(Path, img, info);

    if(Pos.x > info.width)
        throw IOException("invalid pos x");

    if(Pos.y > info.height)
        throw IOException("invalid pos y");

    if(RegionSize.width == 0 || RegionSize.width > info.width)
        throw IOException("invalid size width");

    if(RegionSize.height == 0 || RegionSize.height > info.height)
        throw IOException("invalid size height");

    const DirectX::Image* img2 = img.GetImage(0,0,0);
    UCHAR* pixelsData = img2->pixels;

    size_t rowPitch, slicePitch;
    DirectX::ComputePitch(info.format, info.width, info.height, rowPitch, slicePitch);
    
    std::vector<UINT> srwData(RegionSize.width * RegionSize.height);

    for(int y = Pos.y; y < Pos.y + RegionSize.height; y++)
        for(int x = Pos.x; x < Pos.x + RegionSize.width; x++){
            UCHAR* pixel = &pixelsData[rowPitch * y + sizeof(UINT) * x];

            UINT index = (y - Pos.y) * RegionSize.width + (x - Pos.x);
            srwData[index] = *reinterpret_cast<UINT*>(pixel);
        }

    return Texture::CreateTexture2D(RegionSize,
                                   DXGI_FORMAT_R8G8B8A8_UNORM,
                                   reinterpret_cast<CHAR*>(&srwData[0]));
}

ID3D11ShaderResourceView * CreateTexture2D(const SizeUS &Size, DXGI_FORMAT Format, const char* Data) throw (Exception)
{
    D3D11_TEXTURE2D_DESC texDesc = {0};
	texDesc.Width = Size.width;
	texDesc.Height = Size.height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	

    size_t rowPitch, slicePitch;
    DirectX::ComputePitch(Format, Size.width, Size.height, rowPitch, slicePitch);

    D3D11_SUBRESOURCE_DATA initData = {0};
	initData.SysMemPitch = rowPitch;
    initData.pSysMem = Data;

    ID3D11Texture2D* tex = NULL;
	HR(DeviceKeeper::GetDevice()->CreateTexture2D(&texDesc, &initData, &tex));
    Utils::AutoCOM<ID3D11Texture2D> texturePtr = tex;
	
    ID3D11ShaderResourceView *outSRV;
    HR(DeviceKeeper::GetDevice()->CreateShaderResourceView(texturePtr.Get(), 0, &outSRV));

    return outSRV;
}

ID3D11ShaderResourceView * CreateTexture2D(const SizeUS &Size, DXGI_FORMAT Format, const PixelOperator &Operator) throw (Exception)
{
    size_t rowPitch, slicePitch;
    ComputePitch(Format, Size.width, Size.height, rowPitch, slicePitch);

    UINT pixelSize = rowPitch / Size.height;

    std::vector<UCHAR> data(slicePitch);

    for(UINT y = 0; y < Size.height; y++)
        for(UINT x = 0; x < Size.width; x++){

            UCHAR *px = &data[(y * Size.width + x) * pixelSize];

            POINT pt;
            pt.x = x;
            pt.y = y;

            Operator(px, pt);
        }
    
    return Texture::CreateTexture2D(Size, Format, reinterpret_cast<CHAR*>(&data[0]));
}

RenderTargetCube::RenderTargetCube() : srv(NULL), width(0), height(0), format(DXGI_FORMAT_UNKNOWN)
{
    memset(rtv, NULL, sizeof(ID3D11RenderTargetView*) * 6); 
}

RenderTargetCube::~RenderTargetCube()
{
    for(int i = 0; i < 6; i++)
        ReleaseCOM(rtv[i]);

    ReleaseCOM(srv);
}

void RenderTargetCube::Construct(const RenderTargetCube &Val)
{
    width = Val.width;
    height = Val.height;
    format = Val.format;

    for(int i = 0; i < 6; i++)
        ReleaseCOM(rtv[i]);

    ReleaseCOM(srv);

    if(Val.srv)
        Val.srv->AddRef();

    srv = Val.srv;

    for(int i = 0; i < 6; i++){
        if(Val.rtv[i])
            Val.rtv[i]->AddRef();

        rtv[i] = Val.rtv[i];
    }
}

RenderTargetCube::RenderTargetCube(const RenderTargetCube &Val)
{
    Construct(Val);
}

RenderTargetCube &RenderTargetCube::operator= (const RenderTargetCube &Val)
{
    Construct(Val);
    return *this;
}

void RenderTargetCube::Init(DXGI_FORMAT Format, USHORT Size, UINT MipLevelsCnt) throw (Exception)
{
    width = height = Size;;
    format = Format;

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = Size;
    texDesc.Height = Size;
    texDesc.ArraySize = 6;
    texDesc.MipLevels = MipLevelsCnt;
    texDesc.SampleDesc.Count = 1;
    texDesc.Format = Format;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    
    if(MipLevelsCnt == 0)
        texDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

    ID3D11Texture2D* cubeTexPtr = 0;
    HR(DeviceKeeper::GetDevice()->CreateTexture2D(&texDesc, 0, &cubeTexPtr), [] {return TextureException("cant create cube texture");});

    Utils::AutoCOM<ID3D11Texture2D> cubeTex(cubeTexPtr);

    for(int i = 0; i < 6; ++i){

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.ArraySize = 1;
        rtvDesc.Texture2DArray.FirstArraySlice = i;

        HR(DeviceKeeper::GetDevice()->CreateRenderTargetView(cubeTex, &rtvDesc, &rtv[i]), [] {return TextureException("cant create cube RTV");});
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = (MipLevelsCnt == 0) ? -1 : MipLevelsCnt;

    HR(DeviceKeeper::GetDevice()->CreateShaderResourceView(cubeTex, &srvDesc, &srv));
}

ID3D11RenderTargetView *RenderTargetCube::GetRenderTargetView(UINT Edge) const throw (Exception)
{
    if(Edge >= 6)
        throw RenderTargetException("Invalid render target cub edge");

    return rtv[Edge];
}

RenderTarget::RenderTarget() : rtv(NULL), srv(NULL), width(0), height(0), format(DXGI_FORMAT_UNKNOWN){}

void RenderTarget::Init(DXGI_FORMAT Format, BOOL UseViewport, UINT ViewportInd) throw (Exception)
{
    SizeUS viewportSize(CommonParams::GetScreenWidth(), CommonParams::GetScreenHeight());

    if(UseViewport){
        UINT viewportsCnt = 0;
        DeviceKeeper::GetDeviceContext()->RSGetViewports(&viewportsCnt, NULL);

        if(!viewportsCnt)
            throw TextureException("Viewports not set");

        std::vector<D3D11_VIEWPORT> viewpors;
        viewpors.resize(viewportsCnt);

        DeviceKeeper::GetDeviceContext()->RSGetViewports(&viewportsCnt, static_cast<D3D11_VIEWPORT*>(&viewpors[0]));

        D3D11_VIEWPORT viewport;
        if(ViewportInd == -1)
            viewport = viewpors[0];
        else{
            if(ViewportInd >= viewportsCnt)
                throw TextureException("Invalid viewport index");
            viewport = viewpors[ViewportInd];
        }

        viewportSize.width = viewport.Width;
        viewportSize.height = viewport.Height;
    }

    Init(Format, viewportSize.width, viewportSize.height);
}

RenderTarget::RenderTarget(const RenderTarget &Val)
{
    if(Val.rtv)
        Val.rtv->AddRef();

    if(Val.srv)
        Val.srv->AddRef();

    rtv = Val.rtv;
    srv = Val.srv;

    width = Val.width;
    height = Val.height;
    format = Val.format;
}

RenderTarget &RenderTarget::operator= (const RenderTarget &Val)
{
    if(rtv)
        rtv->Release();

    if(srv)
        srv->Release();

    if(Val.rtv)
        Val.rtv->AddRef();

    if(Val.srv)
        Val.srv->AddRef();    

    rtv = Val.rtv;
    srv = Val.srv;    

    width = Val.width;
    height = Val.height;
    format = Val.format;

    return *this;
}

void RenderTarget::Init(DXGI_FORMAT Format, USHORT Width, USHORT Height) throw (Exception)
{
    width = Width;
    height = Height;
    format = Format;

    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = Width;
	textureDesc.Height = Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = Format;
	textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    ID3D11Texture2D* texture;
    HR(DeviceKeeper::GetDevice()->CreateTexture2D(&textureDesc, NULL, &texture));
    Utils::AutoCOM<ID3D11Texture2D> texturePtr = texture;

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    memset(&rtvDesc, 0, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    rtvDesc.Format = Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    HR(DeviceKeeper::GetDevice()->CreateRenderTargetView(texturePtr.Get(), &rtvDesc, &rtv));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset(&srvDesc, 0, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;	
	srvDesc.Texture2D.MipLevels = 1;

    HR(DeviceKeeper::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv));
}

RenderTarget::~RenderTarget()
{
    ReleaseCOM(rtv);
    ReleaseCOM(srv);
}

}