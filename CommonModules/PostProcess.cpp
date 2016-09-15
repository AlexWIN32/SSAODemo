/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <PostProcess.h>
#include <CommonParams.h>
#include <numeric>

namespace PostProcess
{

void Blur::Init(const std::wstring &VertexShaderPath,
                     const std::wstring &PixelShaderPath,
                     const ScreenSpaceQuad *Quad,
                     const Texture::RenderTarget DataRenderTarget,
                     size_t BlurIterations,
                     const KernelStorage &Kernel) throw (Exception)
{
    orig = DataRenderTarget;
    quad = Quad;
    blurIterations = BlurIterations;

    tmp1.Init(DXGI_FORMAT_R32G32B32A32_FLOAT);    

    vs.Load(VertexShaderPath, "ProcessVertex", Quad->GetVertexMetadata());
    ps.Load(PixelShaderPath, "ProcessPixel");

    ps.CreateSamplerState(0, Utils::DirectX::SamplerStateDescription(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP));
    
    if(!Kernel.size()){

        const INT samplesCount = 11;

        float weights[] = {0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f};
    
        D3DXVECTOR4 weightsData[samplesCount];
        for(int i = 0; i < samplesCount; i++)
            weightsData[i].x = weights[i];

        ps.CreateVariable("weights", 0, 0, weightsData);
    }else{
        std::vector<D3DXVECTOR4> weightsData;

        for(float w : Kernel)
            weightsData.push_back({w, 0.0f, 0.0f, 0.0f});

        ps.CreateVariable("weights", 0, 0, weightsData);
    }

    ps.CreateVariable("texFactors", 0, 1, D3DXVECTOR2(1.0f / (float)orig.GetWidth(), 1.0f / (float)orig.GetHeight()));
    ps.CreateVariable("padding", 0, 2, D3DXVECTOR2());
    ps.CreateVariable<INT>("isVertical", 1, 0, 1);
    ps.CreateVariable("padding2", 1, 1, D3DXVECTOR3());
    
    ps.ApplyVariables();
}

void Blur::SetKernel(const KernelStorage &Kernel)
{
    std::vector<D3DXVECTOR4> weightsData;

    for(float w : Kernel)
        weightsData.push_back({w, 0.0f, 0.0f, 0.0f});

    ps.UpdateVariable("weights", weightsData);
    ps.ApplyVariables();
}

Blur::KernelStorage Blur::GetGaussianKernel(INT Radius, FLOAT Deviation)
{
    //G(x) = (1 / ( sqrt(2 * pi * KernelLength ^ 2)) * exp ^ -(x ^ 2 / 2 * KernelLength ^ 2)
    
    float a = (Deviation == -1) ? Radius * Radius : Deviation * Deviation;
    float f = 1.0f / (sqrtf(2.0f * D3DX_PI * a));
    float g = 2.0f * a;

    Blur::KernelStorage outData(Radius * 2 + 1);
    for(INT x = -Radius; x <= Radius; x++)
        outData[x + Radius] = f * expf(-(x * x) / a);
    
    float summ = std::accumulate(outData.begin(), outData.end(), 0.0f);

    Blur::KernelStorage::iterator it;
    for(it = outData.begin(); it != outData.end(); ++it)
        *it /= summ;

    return outData;
}

void Blur::Draw() const
{    
    vs.Apply();
    
    for(size_t i = 0; i < blurIterations; i++)
    {
        {        
            PostProcess::RenderPass pass(tmp1.GetRenderTargetView());

            ps.UpdateVariable("isVertical", 1);
            ps.ApplyVariables();

            ps.SetResource(0, orig.GetSahderResourceView());
        
            ps.Apply();            

            quad->Draw();

            ps.SetResource(0, NULL);
        }    

        {
            PostProcess::RenderPass pass(orig.GetRenderTargetView());
            
            ps.UpdateVariable("isVertical", 0);
            ps.ApplyVariables();

            ps.SetResource(0, tmp1.GetSahderResourceView());
        
            ps.Apply();    

            quad->Draw();  

            ps.SetResource(0, NULL);
        }   
        
    }
}

void Blur::OnResolutionChanged() throw (Exception)
{
    Texture::RenderTarget newTmp1;

    newTmp1.Init(tmp1.GetFormat(), (USHORT)CommonParams::GetScreenWidth(), (USHORT)CommonParams::GetScreenHeight());

    tmp1 = newTmp1;
}

void ScreenSpaceQuad::Release()
{
   ReleaseCOM(vb);
   ReleaseCOM(ib);
}

struct ScreenQuadVertex
{
    D3DXVECTOR3 pos = {0.0f, 0.0f, 0.0f};
    D3DXVECTOR2 tc = {0.0f, 0.0f};
    ScreenQuadVertex(){}
    ScreenQuadVertex(const D3DXVECTOR3 &Pos, const D3DXVECTOR2 &Tc) : pos(Pos), tc(Tc){}
};

void DefaultScreenQuad::Init() throw (Exception)
{
    std::vector<ScreenQuadVertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
    };

    D3D11_BUFFER_DESC vertexBufferDesc;
    memset(&vertexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(ScreenQuadVertex) * vertices.size();
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;    
    	
    D3D11_SUBRESOURCE_DATA vertexData;
    memset(&vertexData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
    vertexData.pSysMem = &vertices[0];	

    HR(DeviceKeeper::GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &vb));

    UINT indices[] = {0,1,2,2,3,0};
    
    D3D11_BUFFER_DESC indexBufferDesc;
    memset(&indexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(indices);
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;   

    D3D11_SUBRESOURCE_DATA indicesData;
    memset(&indicesData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
    indicesData.pSysMem = indices;	

    try{
        HR(DeviceKeeper::GetDevice()->CreateBuffer(&indexBufferDesc, &indicesData, &ib));
    }catch(const Exception &ex){
        ReleaseCOM(vb);
        throw ex;
    }

    D3D11_INPUT_ELEMENT_DESC desc[2] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    vertexMetadata = Meshes::VertexMetadata(desc, desc + 2);
}

void ScreenSpaceQuad::Construct(const ScreenSpaceQuad &Val)
{
	FreeData();
	
	Val.vb->AddRef();
	Val.ib->AddRef();

	vb = Val.vb;
	ib = Val.ib;
}

void ScreenSpaceQuad::FreeData()
{
	ReleaseCOM(vb);
	ReleaseCOM(ib);
}

void ScreenSpaceQuad::DrawQuad(INT VertexSize) const
{
    UINT stride = VertexSize;
	UINT offset = 0;

	DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	DeviceKeeper::GetDeviceContext()->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DeviceKeeper::GetDeviceContext()->DrawIndexed(6, 0, 0);
}

DefaultScreenQuad::DefaultScreenQuad(DefaultScreenQuad &Val)
{
	Construct(Val);
}

DefaultScreenQuad & DefaultScreenQuad::operator=(DefaultScreenQuad &Val)
{
	FreeData();
	Construct(Val);
	return *this;
}

void DefaultScreenQuad::Draw(INT SubsetNumber) const throw (Exception)
{
    DrawQuad(sizeof(ScreenQuadVertex));
}

static void SetRenderTarget(float Color[4], ID3D11DepthStencilView *Dsv, ID3D11RenderTargetView *Rtv)
{
    DeviceKeeper::GetDeviceContext()->OMSetRenderTargets(1, &Rtv, Dsv);
    
    DeviceKeeper::GetDeviceContext()->ClearRenderTargetView(Rtv, Color);			
    DeviceKeeper::GetDeviceContext()->ClearDepthStencilView(Dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RenderPass::SetViewport(const D3D11_VIEWPORT &NewViewport)
{
    oldViewport = new D3D11_VIEWPORT();

    UINT viewportsCnt = 1;
    DeviceKeeper::GetDeviceContext()->RSGetViewports(&viewportsCnt, oldViewport);

    newViewport = new D3D11_VIEWPORT(NewViewport);
    DeviceKeeper::GetDeviceContext()->RSSetViewports(1, newViewport);
}

RenderPass::RenderPass(ID3D11RenderTargetView *Rtv)
{
    dsv = DeviceKeeper::GetDepthStencilView();

    float color[4];
	color[0] = 1;//Red
	color[1] = 1;//Green
	color[2] = 1;//Blue
	color[3] = 0;//Alpha

    SetRenderTarget(color, dsv, Rtv);
}

RenderPass::RenderPass(ID3D11RenderTargetView *Rtv, FLOAT Color[4])
{
    dsv = DeviceKeeper::GetDepthStencilView();

    SetRenderTarget(Color, dsv, Rtv);
}

RenderPass::RenderPass(ID3D11RenderTargetView *Rtv, ID3D11DepthStencilView *Dsv, const D3D11_VIEWPORT &Viewport)
{
    dsv = Dsv;

    float color[4];
    color[0] = 1;//Red
    color[1] = 1;//Green
    color[2] = 1;//Blue
    color[3] = 0;//Alpha

    SetRenderTarget(color, dsv, Rtv);

    SetViewport(Viewport);
}

RenderPass::RenderPass(ID3D11RenderTargetView *Rtv, ID3D11DepthStencilView *Dsv, const D3D11_VIEWPORT &Viewport, FLOAT Color[4])
{
    dsv = Dsv;
    SetRenderTarget(Color, dsv, Rtv);
    SetViewport(Viewport);
}

RenderPass::~RenderPass()
{
    ID3D11RenderTargetView *rtv = DeviceKeeper::GetRenderTargetView();
    DeviceKeeper::GetDeviceContext()->OMSetRenderTargets(1, &rtv, DeviceKeeper::GetDepthStencilView());
    DeviceKeeper::GetDeviceContext()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if(newViewport != NULL){
        DeviceKeeper::GetDeviceContext()->RSSetViewports(1, oldViewport);
        delete oldViewport;
        delete newViewport;
    }
}

}