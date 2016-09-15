/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <DeviceKeeper.h>
#include <vector>
#include <D3DHeaders.h>
#include <Utils/VertexArray.h>

namespace Utils
{

namespace DirectX
{

struct BlendParameters
{
    D3D11_BLEND source = D3D11_BLEND_ZERO;
    D3D11_BLEND destination = D3D11_BLEND_ZERO;
    D3D11_BLEND_OP operation = D3D11_BLEND_OP_ADD;

    BlendParameters(){}
    BlendParameters(D3D11_BLEND Source, D3D11_BLEND Destination, D3D11_BLEND_OP Operation)
        :source(Source), destination(Destination), operation(Operation){}
};

struct RenderTargetBlend
{
    BlendParameters color, alpha;
    BOOL enabled = false;
    UINT8 writeMask = 0;

    RenderTargetBlend(){}
    RenderTargetBlend(const BlendParameters &Color, const BlendParameters &Alpha, BOOL Enabled, UINT8 WriteMask)
        :color(Color), alpha(Alpha), enabled(Enabled), writeMask(WriteMask){}
};

inline D3D11_BLEND_DESC CreateBlendDescription(const std::vector<RenderTargetBlend> &RenderTargets, 
                                               BOOL AlphaToCoverageEnable = false,
                                               BOOL IndependentBlendEnable = false) throw (Exception)
{
    if(RenderTargets.size() > 8)
        throw D3DException("Invalid render states count");

    D3D11_BLEND_DESC blendDesc = {};

    for(UINT i = 0; i < RenderTargets.size(); i++){

        const RenderTargetBlend &rt = RenderTargets[i];

        blendDesc.RenderTarget[i].BlendEnable = rt.enabled;
        blendDesc.RenderTarget[i].SrcBlend = rt.color.source;
        blendDesc.RenderTarget[i].DestBlend = rt.color.destination;
        blendDesc.RenderTarget[i].BlendOp = rt.color.operation;
        blendDesc.RenderTarget[i].SrcBlendAlpha = rt.alpha.source;
        blendDesc.RenderTarget[i].DestBlendAlpha = rt.alpha.destination;
        blendDesc.RenderTarget[i].BlendOpAlpha = rt.alpha.operation;
        blendDesc.RenderTarget[i].RenderTargetWriteMask = rt.writeMask;
    }

    blendDesc.AlphaToCoverageEnable = AlphaToCoverageEnable;
    blendDesc.IndependentBlendEnable = IndependentBlendEnable;

    return blendDesc;
}

struct SamplerStateDescription
{
    D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    D3D11_TEXTURE_ADDRESS_MODE addressU = D3D11_TEXTURE_ADDRESS_WRAP;
    D3D11_TEXTURE_ADDRESS_MODE addressV = D3D11_TEXTURE_ADDRESS_WRAP;
    D3D11_TEXTURE_ADDRESS_MODE addressW = D3D11_TEXTURE_ADDRESS_WRAP;

    UINT maxAnisotropy = 1;

    D3D11_COMPARISON_FUNC comparisonFunc = D3D11_COMPARISON_ALWAYS;

    FLOAT borderColor[4];

    FLOAT minLOD = 0, maxLOD = D3D11_FLOAT32_MAX;

    SamplerStateDescription()
    {
        memset(borderColor, 0, sizeof(FLOAT) * 4);
    }

    SamplerStateDescription(D3D11_FILTER Filter, D3D11_TEXTURE_ADDRESS_MODE AddressMode)
    {
        filter = Filter;
        addressU = addressV = addressW = AddressMode;
        memset(borderColor, 0, sizeof(FLOAT) * 4);
    }

    SamplerStateDescription(D3D11_FILTER Filter, FLOAT BorderColor[4])
    {
        filter = Filter;
        addressU = addressV = addressW = D3D11_TEXTURE_ADDRESS_BORDER;
        memcpy(borderColor, BorderColor, sizeof(FLOAT) * 4);
    }
};

inline ID3D11SamplerState* CreateSamplerState(const SamplerStateDescription &Description)
{
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = Description.filter;
    samplerDesc.AddressU = Description.addressU;
    samplerDesc.AddressV = Description.addressV;
    samplerDesc.AddressW = Description.addressW;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = Description.maxAnisotropy;
    samplerDesc.ComparisonFunc = Description.comparisonFunc;
    memcpy(samplerDesc.BorderColor, Description.borderColor, sizeof(UINT) * 4);
    samplerDesc.MinLOD = Description.minLOD;
    samplerDesc.MaxLOD = Description.maxLOD;

    ID3D11SamplerState *state;
    HR(DeviceKeeper::GetDevice()->CreateSamplerState(&samplerDesc, &state));

    return state;
}

template<class TData> 
ID3D11Buffer* CreateBuffer(const std::vector<TData> &Data, 
            D3D11_BIND_FLAG BindFlags,
            D3D11_USAGE Usage = D3D11_USAGE_DEFAULT) throw (Exception)
{
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = Usage;
    bd.ByteWidth = sizeof(TData) * Data.size();
    bd.BindFlags = BindFlags;

    ID3D11Buffer* buffer;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &Data[0];
    HR(DeviceKeeper::GetDevice()->CreateBuffer(&bd, &initData, &buffer));

    return buffer;
}

inline ID3D11Buffer* CreateBuffer(size_t BufferSize, 
                  D3D11_BIND_FLAG BindFlags,
                  D3D11_USAGE Usage = D3D11_USAGE_DEFAULT,
                  INT CPUAccessFlags = 0) throw (Exception)
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = Usage;
    bufferDesc.ByteWidth = BufferSize;
    bufferDesc.BindFlags = BindFlags;
    bufferDesc.CPUAccessFlags = CPUAccessFlags;

    ID3D11Buffer *buffer;
    HR(DeviceKeeper::GetDevice()->CreateBuffer(&bufferDesc, NULL, &buffer));

    return buffer;
}

inline ID3D11Buffer* CreateBuffer(const VertexArray &Array,
                  D3D11_USAGE Usage = D3D11_USAGE_DEFAULT,
                  INT CPUAccessFlags = 0) throw (Exception)
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = Usage;
    bufferDesc.ByteWidth = Array.GetVerticesCount() * Array.GetVertixSize();
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = CPUAccessFlags;

    ID3D11Buffer *buffer;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = Array.GetRawData();
    HR(DeviceKeeper::GetDevice()->CreateBuffer(&bufferDesc, &initData, &buffer));

    return buffer;
}

}

}