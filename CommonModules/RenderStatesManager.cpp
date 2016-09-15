/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <RenderStatesManager.h>
#include <DeviceKeeper.h>

RenderStatesManager *RenderStatesManager::instance = NULL;

void RenderStatesManager::CreateRenderState(const std::string &StateName, const DepthStencilDescription &DepthStencilDesc) throw (Exception)
{
	if(blendStates.find(StateName) != blendStates.end() || 
	   depthStencilStates.find(StateName) != depthStencilStates.end() || 
	   rasteriserStates.find(StateName) != rasteriserStates.end())
        throw RenderStatesManagerException("State " + StateName + " is used");

    DepthStencilData depthStencilData;
    depthStencilData.ref = DepthStencilDesc.stencilRef;

    HR(DeviceKeeper::GetDevice()->CreateDepthStencilState(&DepthStencilDesc.stencilDescription, &depthStencilData.state));

    depthStencilStates.insert(std::make_pair(StateName, depthStencilData));
}

void RenderStatesManager::CreateRenderState(const std::string &StateName, const BlendStateDescription &BlendDesc) throw (Exception)
{
    if(blendStates.find(StateName) != blendStates.end() || 
	   depthStencilStates.find(StateName) != depthStencilStates.end() || 
	   rasteriserStates.find(StateName) != rasteriserStates.end())
        throw RenderStatesManagerException("State " + StateName + " is used");

    BlendData blendData;
    memcpy(blendData.blendFactor, BlendDesc.blendFactor, sizeof(blendData.blendFactor));
    blendData.sampleMask = BlendDesc.sampleMask;
    
    HR(DeviceKeeper::GetDevice()->CreateBlendState(&BlendDesc.blendDescription, &blendData.state));

    blendStates.insert(std::make_pair(StateName, blendData));
}

void RenderStatesManager::CreateRenderState(const std::string &StateName, const D3D11_RASTERIZER_DESC &RasteriserDesc) throw (Exception)
{
	if(blendStates.find(StateName) != blendStates.end() || 
	   depthStencilStates.find(StateName) != depthStencilStates.end() || 
	   rasteriserStates.find(StateName) != rasteriserStates.end())
        throw RenderStatesManagerException("State " + StateName + " is used");

	ID3D11RasterizerState *rasterState = NULL;
	HR(DeviceKeeper::GetDevice()->CreateRasterizerState(&RasteriserDesc, &rasterState), []{ return D3DException("cant create rasteriser state");});

	rasteriserStates.insert({StateName, rasterState});
}

void RenderStatesManager::RemoveRenderState(const std::string &StateName)
{
    auto bsIt = blendStates.find(StateName);
    if(bsIt != blendStates.end()){
		ReleaseCOM(bsIt->second.state);
        blendStates.erase(bsIt);
        ResetBlendState();
        return;
    }

    auto dsIt = depthStencilStates.find(StateName);
    if(dsIt != depthStencilStates.end()){
		ReleaseCOM(dsIt->second.state);
        depthStencilStates.erase(dsIt);
        ResetDepthStencilState();
		return;
    }

	auto rsIt = rasteriserStates.find(StateName);
	if(rsIt != rasteriserStates.end()){
		ReleaseCOM(rsIt->second);
		rasteriserStates.erase(rsIt);
		ResetRasteriserState();		
	}
}

RenderStatesManager::RenderStateType RenderStatesManager::ApplyState(const std::string &StateName) throw (Exception)
{
    BlendStatesStorage::const_iterator bsCi = blendStates.find(StateName);
    if(bsCi != blendStates.end()){
        const BlendData &blendData = bsCi->second;
        DeviceKeeper::GetDeviceContext()->OMSetBlendState(blendData.state, blendData.blendFactor, blendData.sampleMask);
        return RS_TYPE_BLEND;
    }

    DepthStencilStatesStorage::const_iterator dsCi = depthStencilStates.find(StateName);
    if(dsCi != depthStencilStates.end()){
        DeviceKeeper::GetDeviceContext()->OMSetDepthStencilState(dsCi->second.state, dsCi->second.ref);
        return RS_TYPE_DEPTH_STENCIL;
    }

	RasteriserStatesStorage::const_iterator rsCi = rasteriserStates.find(StateName);
    if(rsCi != rasteriserStates.end()){
		DeviceKeeper::GetDeviceContext()->RSSetState(rsCi->second);
        return RS_TYPE_RASTERIZER;
    }
    
    throw RenderStatesManagerException("State " + StateName + " not found");
}

void RenderStatesManager::ResetBlendState()
{
    DeviceKeeper::GetDeviceContext()->OMSetBlendState(NULL, NULL, 0xffffffff);
}

void RenderStatesManager::ResetDepthStencilState()
{
    DeviceKeeper::GetDeviceContext()->OMSetDepthStencilState(NULL, 0);
}

void RenderStatesManager::ResetRasteriserState()
{
	DeviceKeeper::GetDeviceContext()->RSSetState(NULL);
}

void RenderStatesManager::ProcessWithStates(const NamesStorage &StatesNames, const Procedure &Proc) throw (Exception)
{
    bool depthStateChanged = false, rasterizerStateChanged = false, blendStateChanged = false;

    for(const std::string &name : StatesNames){

        RenderStateType res = ApplyState(name);

        if(res == RS_TYPE_BLEND)
            blendStateChanged = true;
        else if(res == RS_TYPE_DEPTH_STENCIL)
            depthStateChanged = true;
        else if(res == RS_TYPE_RASTERIZER)
            rasterizerStateChanged = true;
    }

    Proc();

    if(depthStateChanged)
        ResetDepthStencilState();

    if(rasterizerStateChanged)
        ResetRasteriserState();

    if(blendStateChanged)
        ResetBlendState();
}

RenderStatesManager::~RenderStatesManager()
{
    for(auto &bs : blendStates)
        ReleaseCOM(bs.second.state);

    for(auto &ds : depthStencilStates)
        ReleaseCOM(ds.second.state);

	for(auto &rs : rasteriserStates)
		ReleaseCOM(rs.second);
}