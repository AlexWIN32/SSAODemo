/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <D3DHeaders.h>
#include <Exception.h>
#include <string>
#include <unordered_map>
#include <functional>

DECLARE_EXCEPTION(RenderStatesManagerException)

class RenderStatesManager
{
public:
    enum RenderStateType
    {
        RS_TYPE_RASTERIZER,
        RS_TYPE_DEPTH_STENCIL,
        RS_TYPE_BLEND,
        RS_TYPE_UNKNOWN
    };
    struct BlendStateDescription
    {
        D3D11_BLEND_DESC blendDescription;
        FLOAT blendFactor[4];
        UINT sampleMask;
        BlendStateDescription() : sampleMask(0)
        {
            memset(&blendDescription, 0, sizeof(D3D11_BLEND_DESC));
            memset(blendFactor, 0, sizeof(blendFactor));
        }
        BlendStateDescription(const D3D11_BLEND_DESC &BlendDescription, FLOAT BlendFactor[4], UINT SampleMask)
            : blendDescription(BlendDescription), sampleMask(SampleMask)
        {
            memcpy(blendFactor, BlendFactor, sizeof(blendFactor));
        }
    };
    struct DepthStencilDescription
    {
        D3D11_DEPTH_STENCIL_DESC stencilDescription;
        UINT stencilRef;
        DepthStencilDescription() : stencilRef(0)
        {
            memset(&stencilDescription, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));
        }
        DepthStencilDescription(const D3D11_DEPTH_STENCIL_DESC &StencilDescription, UINT StencilRef) 
            : stencilDescription(StencilDescription), stencilRef(StencilRef)
        {}
    };
    typedef std::vector<std::string> NamesStorage;
    typedef std::function<void()> Procedure;
private:
    struct BlendData
    {
        ID3D11BlendState* state;
        FLOAT blendFactor[4];
        UINT sampleMask;
    };
    struct DepthStencilData
    {
        ID3D11DepthStencilState* state;
        UINT ref;
    };
    typedef std::unordered_map<std::string, BlendData> BlendStatesStorage;
    typedef std::unordered_map<std::string, DepthStencilData> DepthStencilStatesStorage;
	typedef std::unordered_map<std::string, ID3D11RasterizerState*> RasteriserStatesStorage;
    BlendStatesStorage blendStates;
    DepthStencilStatesStorage depthStencilStates;
	RasteriserStatesStorage rasteriserStates;
    RenderStatesManager(){}
    ~RenderStatesManager();
    static RenderStatesManager *instance;
public:
    static RenderStatesManager *GetInstance()
    {
        if(!instance)
            instance = new RenderStatesManager();
        return instance;
    }
    static void ReleaseInstance()
    {
        delete instance;
        instance = NULL;
    }   
    RenderStatesManager(RenderStatesManager &) = delete;
    RenderStatesManager & operator=(RenderStatesManager &) = delete;
    void CreateRenderState(const std::string &StateName, const DepthStencilDescription &DepthStencilDesc) throw (Exception);
    void CreateRenderState(const std::string &StateName, const BlendStateDescription &BlendDesc) throw (Exception);
	void CreateRenderState(const std::string &StateName, const D3D11_RASTERIZER_DESC &RasteriserDesc) throw (Exception);
    RenderStateType ApplyState(const std::string &StateName) throw (Exception);
    void RemoveRenderState(const std::string &StateName);
    void ResetBlendState();
    void ResetDepthStencilState();
	void ResetRasteriserState();
    void ProcessWithStates(const NamesStorage &StatesNames, const Procedure &Proc) throw (Exception);
};