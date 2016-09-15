/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Shader.h>
#include <Utils.h>
#include <MathHelpers.h>
#include <algorithm>

namespace Shaders
{

static ID3D10Blob* CompileShader(const std::wstring &FileName, const std::string &EntryPoint, const std::string &Profile) throw (Exception)
{
	ID3D10Blob* errorsMsg = NULL;
	ID3D10Blob* shaderBuffer = NULL;

	HRESULT hr = D3DX11CompileFromFile(
		FileName.c_str(),
		NULL,
		NULL,
		EntryPoint.c_str(),
		Profile.c_str(),
		D3D10_SHADER_ENABLE_STRICTNESS,
		0,
		NULL,
		&shaderBuffer,
		&errorsMsg,
		NULL);

    if(errorsMsg){
        std::string message(reinterpret_cast<char*>(errorsMsg->GetBufferPointer()));
        ReleaseCOM(errorsMsg);
		throw ShaderCompilationException(message);
    }

    HR(hr);        

    return shaderBuffer; 
}

void PixelShader::Load(const std::wstring &FileName, const std::string &EntryPoint) throw (Exception)
{
    Utils::AutoCOM<ID3D10Blob> shaderBuffer = CompileShader(FileName, EntryPoint, "ps_5_0");

    HR(DeviceKeeper::GetDevice()->CreatePixelShader(
        shaderBuffer->GetBufferPointer(),
        shaderBuffer->GetBufferSize(),
        NULL,
        &ps));
}

void PixelShader::SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const
{
    DeviceKeeper::GetDeviceContext()->PSSetShaderResources(Register, 1, &SRW);
}

void PixelShader::SetShader(BOOL CleanUp) const
{
     DeviceKeeper::GetDeviceContext()->PSSetShader((CleanUp) ? NULL : ps, NULL, 0);
}

void PixelShader::SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const
{
    DeviceKeeper::GetDeviceContext()->PSSetConstantBuffers(Register, 1, &ConstantBuffer);
}

void PixelShader::SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const
{
    DeviceKeeper::GetDeviceContext()->PSSetSamplers(Register, 1, &SamplerState);
}

PixelShader::~PixelShader()
{
    ReleaseCOM(ps);
}

void PixelShader::Construct(const PixelShader &Val)
{
    Shader::Construct(Val);
    
    if(ps)    
        ps->Release();

    Val.ps->AddRef();

    ps = Val.ps;   
}

PixelShader::PixelShader(const PixelShader &Val)
{    
    Construct(Val);
}

PixelShader &PixelShader::operator= (const PixelShader &Val)
{    
    Construct(Val);

    return *this;
}

void PixelShader::ConstructAsRef(const PixelShader &Val)
{
    Shader::ConstructAsRef(Val);

    if(ps)    
        ps->Release();

    Val.ps->AddRef();

    ps = Val.ps;
}

void VertexShader::Load(const std::wstring &FileName, const std::string &EntryPoint, const VertexMetadata &InputLayout) throw (Exception)
{
    Utils::AutoCOM<ID3D10Blob> shaderBuffer = CompileShader(FileName, EntryPoint, "vs_5_0");

    HR(DeviceKeeper::GetDevice()->CreateVertexShader(
        shaderBuffer->GetBufferPointer(),
        shaderBuffer->GetBufferSize(),
        NULL, 
        &vs));

    HR(DeviceKeeper::GetDevice()->CreateInputLayout(
        &InputLayout[0],
        InputLayout.size(),
        shaderBuffer->GetBufferPointer(),
        shaderBuffer->GetBufferSize(),
        &layout));
}

void VertexShader::SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const
{
    DeviceKeeper::GetDeviceContext()->VSSetShaderResources(Register, 1, &SRW);
}

void VertexShader::SetShader(BOOL CleanUp) const
{
     DeviceKeeper::GetDeviceContext()->VSSetShader((CleanUp) ? NULL : vs, NULL, 0);
}

void VertexShader::SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const
{
    DeviceKeeper::GetDeviceContext()->VSSetConstantBuffers(Register, 1, &ConstantBuffer);
}

void VertexShader::SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const
{
    DeviceKeeper::GetDeviceContext()->VSSetSamplers(Register, 1, &SamplerState);
}

void VertexShader::Apply() const
{
    Shader::Apply();
    DeviceKeeper::GetDeviceContext()->IASetInputLayout(layout);
}

VertexShader::~VertexShader()
{
    ReleaseCOM(vs);
    ReleaseCOM(layout);
}

void VertexShader::ResetRefs(const VertexShader &Val)
{
    if(vs)
        vs->Release();

    if(layout)
        layout->Release();    

    Val.vs->AddRef();
    Val.layout->AddRef();

    vs = Val.vs;
    layout = Val.layout;
}

void VertexShader::Construct(const VertexShader &Val)
{
    Shader::Construct(Val);

    ResetRefs(Val);
}

void VertexShader::ConstructAsRef(const VertexShader &Val)
{
    Shader::ConstructAsRef(Val);

    ResetRefs(Val);
}

VertexShader::VertexShader(const VertexShader &Val)
{    
    Construct(Val);
}

VertexShader &VertexShader::operator= (const VertexShader &Val)
{    
    Construct(Val);
    return *this;
}

bool Shader::VariableDescriptionKey::operator< (const VariableDescriptionKey &V)  const
{
    if(regId < V.regId)
        return true;
    else if(regId > V.regId)
        return false;
        
    return pos < V.pos;
}

void Shader::CreateConstantBuffer(RegisterType Register, UINT BufferSize) throw (Exception)
{
    ConstantBuffersStorage::iterator it = constantBuffers.find(Register);
    if(it != constantBuffers.end()){
        it->second->Release();
        constantBuffers.erase(it);
    }

    ID3D11Buffer *buffer = Utils::DirectX::CreateBuffer(BufferSize, 
                                    D3D11_BIND_CONSTANT_BUFFER, 
                                    D3D11_USAGE_DYNAMIC, 
                                    D3D11_CPU_ACCESS_WRITE);

    constantBuffers.insert(std::make_pair(Register, buffer));
}

void Shader::CreateSamplerState(const std::string &VarName,
                        RegisterType Register,
                        const Utils::DirectX::SamplerStateDescription &Description,
                        const std::string &StateName) throw (Exception)
{
    CreateSamplerState(Register, Description, StateName);
    smpStatesToRegsMapping[VarName] = Register;
}

void Shader::CreateSamplerState(RegisterType Register,
                                const Utils::DirectX::SamplerStateDescription &Description,
                                const std::string &StateName) throw (Exception)
{	
    SamplerStatesStorage::iterator it = samplerStates.find(Register);
    if(it != samplerStates.end()){

        SamplerStatesContainer &container = it->second;

        auto it2 = container.find(StateName);
        if(it2 != container.end()){
            it2->second->Release();
            container.erase(it2);
        }

        if(container.size() == 0)
            samplerStates.erase(it);
    }

    samplerStates[Register][StateName] = Utils::DirectX::CreateSamplerState(Description);
}

Shader::~Shader()
{
	FreeData();
}

void Shader::FreeData()
{
    ConstantBuffersStorage::const_iterator ci;
	for (ci = constantBuffers.begin(); ci != constantBuffers.end(); ++ci)
		ci->second->Release();

    for(auto &pair : samplerStates)
        for(auto &pair2 : pair.second)
            pair2.second->Release();
}

void Shader::CreateRawVariable(const std::string &VarName, RegisterType RegId, INT Pos, INT SizeInBytes) throw (Exception)
{
    std::hash<std::string> hash;
    VariableId varId = hash(VarName);

    if(std::find_if(vars.begin(), vars.end(), [&](const VariablesData::value_type &v) -> bool
    {
        return v.first.varId == varId;
    }) != vars.end())
        throw ShaderVariableException("variable " + VarName + " already used");

    VariableDescriptionKey desc;
    desc.pos = Pos;
    desc.regId = RegId;
    desc.varId = varId;

    if(vars.find(desc) != vars.end())
        throw ShaderVariableException("parameters for variable " + VarName + " already used");

    vars.insert({desc, std::vector<char>(SizeInBytes)});

}

void Shader::ApplyVariables() throw (Exception)
{
    if(vars.size() == 0)
        return;

    for(const auto &pair : vars)
        if(pair.second.size() == 0)
            throw ShaderVariableException("Not all variables initialized");

    std::vector<char> bufferData;

    INT prevRegId = vars.begin()->first.regId;

    for(const auto &pair : vars){
        
        INT regId = pair.first.regId;

        if(regId != prevRegId){

            SetBufferData(prevRegId, reinterpret_cast<char*>(&bufferData[0]), bufferData.size());

            bufferData.clear();

            prevRegId = regId;
        }

        bufferData.insert(bufferData.end(), pair.second.begin(), pair.second.end());
    }

    RegisterType lastRegId = vars.rbegin()->first.regId;
    SetBufferData(lastRegId, reinterpret_cast<char*>(&bufferData[0]), bufferData.size());
}

void Shader::Apply() const
{
    SetShader();

    for(const auto &pair : GetConstantBuffers())
        SetConstantBuffer(pair.first, pair.second);

    for(const auto &pair : GetSamplerStates()){
        const SamplerStatesContainer &container = pair.second;
        if(container.size() != 0)
            SetSamplerState(pair.first, container.begin()->second);
    }
}

void Shader::CleanUp()
{
    SetShader(true);
    ResetResources();
}

void Shader::Construct(const Shader &Val)
{        
    FreeData();

    vars = Val.vars;

    constantBuffers.clear();
    samplerStates.clear();

    ConstantBuffersStorage::const_iterator cbci;
    for(cbci = Val.constantBuffers.begin(); cbci != Val.constantBuffers.end(); ++cbci){
        ID3D11Buffer *buffer = cbci->second;
        D3D11_BUFFER_DESC desc;
        cbci->second->GetDesc(&desc);

        ID3D11Buffer *newBuffer;
	    HR(DeviceKeeper::GetDevice()->CreateBuffer(&desc, NULL, &newBuffer));
        DeviceKeeper::GetDeviceContext()->CopyResource(newBuffer, buffer);

        constantBuffers.insert(std::make_pair(cbci->first, newBuffer));
    }

    for(const auto &pair : Val.samplerStates){

        SamplerStatesContainer newContainer;

        for(const auto &pair2 : pair.second){

            D3D11_SAMPLER_DESC desc;
            pair2.second->GetDesc(&desc);

            ID3D11SamplerState *newState;
            HR(DeviceKeeper::GetDevice()->CreateSamplerState(&desc, &newState));

            newContainer.insert({pair2.first, newState});
        }

        samplerStates.insert({pair.first, newContainer});
    }
}

void Shader::ConstructAsRef(const Shader &Val)
{
    FreeData();

    vars = Val.vars;

    constantBuffers = Val.constantBuffers;
    samplerStates = Val.samplerStates;
    
    ConstantBuffersStorage::const_iterator cbci;
    for(cbci = constantBuffers.begin(); cbci != constantBuffers.end(); ++cbci)
        cbci->second->AddRef();    

	SamplerStatesStorage::const_iterator smpci;
    for(smpci = samplerStates.begin(); smpci != samplerStates.end(); ++smpci)
        for(auto &pair : smpci->second)
            pair.second->AddRef();
}  

void Shader::SetBufferData(RegisterType RegId, const char *Data, INT DataSize)
{
    if(constantBuffers.find(RegId) == constantBuffers.end())
        CreateConstantBuffer(RegId, DataSize);

    ID3D11Buffer *buffer = constantBuffers[RegId];

    D3D11_MAPPED_SUBRESOURCE rawData;
    HR(DeviceKeeper::GetDeviceContext()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &rawData));

    char *ptr = reinterpret_cast<char*>(rawData.pData);
    std::copy(Data, Data + DataSize, ptr);

    DeviceKeeper::GetDeviceContext()->Unmap(buffer, 0);
}

void Shader::SetVariableData(const std::string &VarName, const char *Data) throw (Exception)
{
    std::hash<std::string> hash;
    VariableId varId = hash(VarName);

    auto it = std::find_if(vars.begin(), vars.end(), [&](const VariablesData::value_type &v) -> bool
    {
        return v.first.varId == varId;
    });

    if(it == vars.end())
        throw ShaderVariableException("variable " + VarName + " not found");

    std::copy(Data, Data + it->second.size(), it->second.begin());
}

void Shader::UpdateVariable(const std::string &VarName, const D3DXMATRIX &VariableData) throw (Exception)
{        
    D3DXMATRIX mTrans = Math::Transpose(VariableData);
    const char *ptr = reinterpret_cast<const char*>(&mTrans);

    SetVariableData(VarName, ptr);
}

void Shader::ChangeSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState)
{
    SetSamplerState(Register, SamplerState);
}

void Shader::ChangeSamplerState(const std::string &VarName, ID3D11SamplerState *SamplerState) throw (Exception)
{
    auto it = smpStatesToRegsMapping.find(VarName);
    if(it == smpStatesToRegsMapping.end())
        throw ShaderVariableException("variable " + VarName + " not found");

    ChangeSamplerState(it->second, SamplerState);
}

void Shader::ChangeSamplerState(RegisterType Register, const std::string &StateName) throw (Exception)
{
    auto it = samplerStates.find(Register);
    if(it == samplerStates.end())
        throw ShaderSamplerStateException("sampler state " + StateName + " not found");

    const SamplerStatesContainer &container  = it->second;

    auto it2 = container.find(StateName);
    if(it2 == container.end())
        throw ShaderSamplerStateException("sampler state " + StateName + " not found");

    ChangeSamplerState(Register, it2->second);
}

void Shader::ChangeSamplerState(const std::string &VarName, const std::string &StateName) throw (Exception)
{
    auto it = smpStatesToRegsMapping.find(VarName);
    if(it == smpStatesToRegsMapping.end())
        throw ShaderVariableException("variable " + VarName + " not found");

    ChangeSamplerState(it->second, StateName);
}

void Shader::SetResource(RegisterType Register, ID3D11ShaderResourceView *SRW)
{
    SetShaderResourceView(Register, SRW);

    if(std::find(resourcesToClear.begin(), resourcesToClear.end(), Register) == resourcesToClear.end())
        resourcesToClear.push_back(Register);
}

void Shader::SetResource(const std::string &VarName, ID3D11ShaderResourceView *SRW) throw (Exception)
{
    auto it = smpStatesToRegsMapping.find(VarName);
    if(it == smpStatesToRegsMapping.end())
        throw ShaderVariableException("variable " + VarName + " not found");

    SetResource(it->second, SRW);
}

void Shader::ResetResources()
{
    for(RegisterType reg : resourcesToClear)
        SetShaderResourceView(reg, NULL);
}

void DomainShader::Construct(const DomainShader &Val)
{
    Shader::Construct(Val);
    
    if(ds)
        ds->Release();

    Val.ds->AddRef();

    ds = Val.ds;
}

void DomainShader::ConstructAsRef(const DomainShader &Val)
{
    Shader::ConstructAsRef(Val);

    if(ds)
        ds->Release();

    Val.ds->AddRef();

    ds = Val.ds;
};

DomainShader::DomainShader(const DomainShader &Val)
{
    Construct(Val);
}

DomainShader &DomainShader::operator= (const DomainShader &Val)
{
    Construct(Val);
    return *this;
}

DomainShader::~DomainShader()
{
    ReleaseCOM(ds);
}

void DomainShader::Load(const std::wstring &FileName, const std::string &EntryPoint) throw (Exception)
{
    Utils::AutoCOM<ID3D10Blob> buffer = CompileShader(FileName, EntryPoint, "ds_5_0");

    HR(DeviceKeeper::GetDevice()->CreateDomainShader(
        buffer->GetBufferPointer(),
        buffer->GetBufferSize(),
        NULL, 
        &ds));
}

void DomainShader::SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const
{
    DeviceKeeper::GetDeviceContext()->DSSetShaderResources(Register, 1, &SRW);
}

void DomainShader::SetShader(BOOL CleanUp) const
{
     DeviceKeeper::GetDeviceContext()->DSSetShader((CleanUp) ? NULL : ds, NULL, 0);
}

void DomainShader::SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const
{
    DeviceKeeper::GetDeviceContext()->DSSetConstantBuffers(Register, 1, &ConstantBuffer);
}

void DomainShader::SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const
{
    DeviceKeeper::GetDeviceContext()->DSSetSamplers(Register, 1, &SamplerState);
}

void HullShader::Construct(const HullShader &Val)
{
    Shader::Construct(Val);
    
    if(hs)
        hs->Release();

    Val.hs->AddRef();

    hs = Val.hs;
}

void HullShader::ConstructAsRef(const HullShader &Val)
{
    Shader::Construct(Val);

    if(hs)
        hs->Release();

    Val.hs->AddRef();

    hs = Val.hs;
}

HullShader::HullShader(const HullShader &Val)
{
    Construct(Val);
}

HullShader &HullShader::operator= (const HullShader &Val)
{
    Construct(Val);
    return *this;
}

HullShader::~HullShader()
{
    ReleaseCOM(hs);
}

void HullShader::Load(const std::wstring &FileName, const std::string &EntryPoint) throw (Exception)
{
    Utils::AutoCOM<ID3D10Blob> buffer = CompileShader(FileName, EntryPoint, "hs_5_0");

    HR(DeviceKeeper::GetDevice()->CreateHullShader(
        buffer->GetBufferPointer(),
        buffer->GetBufferSize(),
        NULL,
        &hs));
}

void HullShader::SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const
{
    DeviceKeeper::GetDeviceContext()->HSSetShaderResources(Register, 1, &SRW);
}

void HullShader::SetShader(BOOL CleanUp) const
{
     DeviceKeeper::GetDeviceContext()->HSSetShader((CleanUp) ? NULL : hs, NULL, 0);
}

void HullShader::SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const
{
    DeviceKeeper::GetDeviceContext()->HSSetConstantBuffers(Register, 1, &ConstantBuffer);
}

void HullShader::SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const
{
    DeviceKeeper::GetDeviceContext()->HSSetSamplers(Register, 1, &SamplerState);
}

void GeometryShader::Construct(const GeometryShader &Val)
{
    Shader::Construct(Val);
    
    if(gs)
        gs->Release();

    Val.gs->AddRef();

    gs = Val.gs;
}

void GeometryShader::ConstructAsRef(const GeometryShader &Val)
{
    Shader::ConstructAsRef(Val);
    
    if(gs)
        gs->Release();

    Val.gs->AddRef();

    gs = Val.gs;
}

GeometryShader::GeometryShader(const GeometryShader &Val)
{
    Construct(Val);
}

GeometryShader &GeometryShader::operator= (const GeometryShader &Val)
{
    Construct(Val);
    return *this;
}

GeometryShader::~GeometryShader()
{
    ReleaseCOM(gs);
}

void GeometryShader::Load(const std::wstring &FileName, const std::string &EntryPoint, const StreamOutMetadata &InputSOMetadata) throw (Exception)
{
    Utils::AutoCOM<ID3D10Blob> buffer = CompileShader(FileName, EntryPoint, "gs_5_0");
    
    HR(DeviceKeeper::GetDevice()->CreateGeometryShaderWithStreamOutput(
        buffer->GetBufferPointer(),
        buffer->GetBufferSize(),
        &InputSOMetadata[0],
        InputSOMetadata.size(),
        NULL, 
        0,
        0,
        NULL,
        &gs));
}

void GeometryShader::Load(const std::wstring &FileName, const std::string &EntryPoint) throw (Exception)
{
    Utils::AutoCOM<ID3D10Blob> buffer = CompileShader(FileName, EntryPoint, "gs_5_0");

    HR(DeviceKeeper::GetDevice()->CreateGeometryShader(
        buffer->GetBufferPointer(),
        buffer->GetBufferSize(),
        NULL,
        &gs));
}

void GeometryShader::SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const
{
    DeviceKeeper::GetDeviceContext()->GSSetShaderResources(Register, 1, &SRW);
}

void GeometryShader::SetShader(BOOL CleanUp) const
{
     DeviceKeeper::GetDeviceContext()->GSSetShader((CleanUp) ? NULL : gs, NULL, 0);
}

void GeometryShader::SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const
{
    DeviceKeeper::GetDeviceContext()->GSSetConstantBuffers(Register, 1, &ConstantBuffer);
}

void GeometryShader::SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const
{
    DeviceKeeper::GetDeviceContext()->GSSetSamplers(Register, 1, &SamplerState);
}

}
