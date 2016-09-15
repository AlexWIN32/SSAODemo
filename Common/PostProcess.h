/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once

#include <D3DHeaders.h>
#include <Texture.h>
#include <Shader.h>
#include <Exception.h>
#include <Meshes.h>

namespace PostProcess
{

DECLARE_EXCEPTION(PostProcessException);

class ScreenSpaceQuad : public Meshes::IMesh
{
protected:
    Meshes::VertexMetadata vertexMetadata;
    ID3D11Buffer *vb = NULL, *ib = NULL;
    void DrawQuad(INT VertexSize) const;
	void Construct(const ScreenSpaceQuad &Val);
	void FreeData();
    Meshes::MaterialData tmpMaterial;
public:
    virtual ~ScreenSpaceQuad(){Release();}
    virtual INT GetSubsetCount() const {return 1;}
    virtual const Meshes::MaterialData &GetSubsetMaterial(INT SubsetNumber) const throw (Exception) {return tmpMaterial;}
    virtual void SetSubsetMaterial(INT SubsetNumber, const Meshes::MaterialData &Material) throw (Exception){}
    virtual const Meshes::VertexMetadata &GetVertexMetadata() const throw (Exception) {return vertexMetadata;}
    virtual void Release();
    virtual void Draw(INT SubsetNumber = -1) const throw (Exception) = 0;
};


class DefaultScreenQuad : public ScreenSpaceQuad
{
public:
    virtual ~DefaultScreenQuad(){}
    DefaultScreenQuad(){}
	DefaultScreenQuad(DefaultScreenQuad &Val);
    DefaultScreenQuad & operator=(DefaultScreenQuad &Val);
    void Init() throw (Exception);
    virtual void Draw(INT SubsetNumber = -1) const throw (Exception);
};

class Effect
{
protected:
    const ScreenSpaceQuad *quad;
    mutable Shaders::VertexShader vs;
    mutable Shaders::PixelShader ps;
public:
    virtual ~Effect(){}
    Effect():quad(NULL){}
    const Shaders::VertexShader &GetVertexShader() const {return vs;}
    Shaders::VertexShader &GetVertexShader() {return vs;}
    const Shaders::PixelShader &GetPixelShader() const {return ps;}
    Shaders::PixelShader &GetPixelShader() {return ps;}    
    virtual void Draw() const = 0;
    virtual void OnResolutionChanged() throw (Exception){}
};

class Blur : public Effect
{
protected:
    Texture::RenderTarget tmp1, orig;    
    size_t blurIterations = 1;
public:    
    typedef std::vector<float> KernelStorage;
    static KernelStorage GetGaussianKernel(INT Radius, FLOAT Deviation = -1);
    Blur() : blurIterations(0){}
    virtual ~Blur(){}    
    virtual void Init(const std::wstring &VertexShaderPath,
                     const std::wstring &PixelShaderPath,
                     const ScreenSpaceQuad *Quad,
                     const Texture::RenderTarget DataRenderTarget,
                     size_t BlurIterations = 1,
                     const KernelStorage &Kernel = KernelStorage()) throw (Exception);
    virtual void Draw() const;
    size_t GetIterationsCount() const {return blurIterations;}
    void SetDataRenderTarget(const Texture::RenderTarget &NewDataRenderTarget){orig = NewDataRenderTarget;}
    void SetBlurIterationsCount(size_t BlurIterations) {blurIterations = BlurIterations;}
    void OnResolutionChanged() throw (Exception);
    void SetKernel(const KernelStorage &Kernel);
};

class RenderPass
{
private:
    ID3D11DepthStencilView *dsv;
    D3D11_VIEWPORT *oldViewport = NULL, *newViewport = NULL;
    void SetViewport(const D3D11_VIEWPORT &NewViewport);
public:
    RenderPass(const RenderPass&) = delete;
    RenderPass &operator= (const RenderPass&) = delete;
    RenderPass(ID3D11RenderTargetView *Rtv);
    RenderPass(ID3D11RenderTargetView *Rtv, ID3D11DepthStencilView *Dsv);
    RenderPass(ID3D11RenderTargetView *Rtv, ID3D11DepthStencilView *Dsv, const D3D11_VIEWPORT &Viewport);
    RenderPass(ID3D11RenderTargetView *Rtv, FLOAT Color[4]);
    RenderPass(ID3D11RenderTargetView *Rtv, ID3D11DepthStencilView *Dsv, const D3D11_VIEWPORT &Viewport, FLOAT Color[4]);
    ~RenderPass();
};

};