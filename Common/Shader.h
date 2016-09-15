/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once 
#include <Exception.h>
#include <D3DHeaders.h>
#include <DeviceKeeper.h>
#include <Utils/DirectX.h>
#include <map>
#include <vector>
#include <sstream>

namespace Shaders
{

DECLARE_EXCEPTION(ShaderException)
DECLARE_CHILD_EXCEPTION(ShaderCompilationException, ShaderException)
DECLARE_CHILD_EXCEPTION(ShaderVariableException, ShaderException)
DECLARE_CHILD_EXCEPTION(ShaderSamplerStateException, ShaderException)

typedef INT RegisterType;

typedef std::vector<D3D11_INPUT_ELEMENT_DESC> VertexMetadata;
typedef std::vector<D3D11_SO_DECLARATION_ENTRY> StreamOutMetadata;

class Shader
{
private:	
    typedef std::map<RegisterType, ID3D11Buffer*> ConstantBuffersStorage;
    typedef std::map<std::string, ID3D11SamplerState*> SamplerStatesContainer;
    typedef std::map<RegisterType, SamplerStatesContainer> SamplerStatesStorage;
	ConstantBuffersStorage constantBuffers;	
	SamplerStatesStorage samplerStates;
    typedef INT VariableId;
    struct VariableDescriptionKey
    {
        RegisterType regId = 0;
        INT pos = 0;
        VariableId varId = 0;
        bool operator< (const VariableDescriptionKey &V) const;
    };
    typedef std::map<VariableDescriptionKey, std::vector<char>> VariablesData;
    typedef std::vector<RegisterType> ResourcesToClearStorage;
    typedef std::map<std::string, RegisterType> SmpStatesToRegsMappingStorage;
    VariablesData vars;
    ResourcesToClearStorage resourcesToClear;
    SmpStatesToRegsMappingStorage smpStatesToRegsMapping;
    void SetBufferData(RegisterType RegId, const char *Data, INT DataSize);
    void SetVariableData(const std::string &VarName, const char *Data) throw (Exception);
protected:
	const SamplerStatesStorage &GetSamplerStates() const { return samplerStates; }
	const ConstantBuffersStorage &GetConstantBuffers() const { return constantBuffers; }    
    void Construct(const Shader &Val);
    void ConstructAsRef(const Shader &Val);
    void FreeData();
	virtual void SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const = 0;
    virtual void SetShader(BOOL CleanUp = false) const = 0;
    virtual void SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const = 0;
    virtual void SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const = 0;
public:
	virtual ~Shader();
    void CreateRawVariable(const std::string &VarName, RegisterType RegId, INT Pos, INT SizeInBytes) throw (Exception);
    template<class TVar>
    void CreateVariable(const std::string &VarName, RegisterType RegId, INT Pos, const TVar &DefaultValue = TVar()) throw (Exception)
    {
        CreateRawVariable(VarName, RegId, Pos, sizeof(TVar));
        UpdateVariable(VarName, DefaultValue);
    }
    template<class TVar>
    void CreateVariable(const std::string &VarName, RegisterType RegId, INT Pos, const std::vector<TVar> &Value) throw (Exception)
    {
        CreateRawVariable(VarName, RegId, Pos, sizeof(TVar) * Value.size());
        UpdateVariable(VarName, Value);
    }
	void CreateConstantBuffer(RegisterType Register, UINT BufferSize) throw (Exception); //DEPRECATED
    void CreateSamplerState(RegisterType Register,
                            const Utils::DirectX::SamplerStateDescription &Description,
                            const std::string &StateName = "") throw (Exception);//DEPRECATED
    void CreateSamplerState(const std::string &VarName,
                            RegisterType Register,
                            const Utils::DirectX::SamplerStateDescription &Description,
                            const std::string &StateName = "") throw (Exception);
    void ApplyVariables() throw (Exception);
	template<class TBuffer>
	void UpdateConstantBuffer(RegisterType Register, const TBuffer &BufferData) throw (Exception) //DEPRECATED
	{
        const char* ptr = reinterpret_cast<const char*>(&BufferData);
        SetBufferData(Register, ptr, sizeof(TBuffer));
	}
    template<class TVar>
    void UpdateVariable(const std::string &VarName, const TVar &VariableData) throw (Exception)
    {
        SetVariableData(VarName, reinterpret_cast<const char*>(&VariableData));
    }    
    template<class TVar>
    void UpdateVariable(const std::string &VarName, const std::vector<TVar> &VariableData) throw (Exception)
    {
        SetVariableData(VarName, reinterpret_cast<const char*>(&VariableData[0]));
    }
    void UpdateVariable(const std::string &VarName, const D3DXMATRIX &VariableData) throw (Exception);
	virtual void Apply() const;
    void SetResource(RegisterType Register, ID3D11ShaderResourceView *SRW); //DEPRECATED
    void SetResource(const std::string &VarName, ID3D11ShaderResourceView *SRW) throw (Exception);
    void ChangeSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState); //DEPRECATED
    void ChangeSamplerState(const std::string &VarName, ID3D11SamplerState *SamplerState) throw (Exception);
    void ChangeSamplerState(RegisterType Register, const std::string &StateName) throw (Exception); //DEPRECATED
    void ChangeSamplerState(const std::string &VarName, const std::string &StateName) throw (Exception);
    void ResetResources();
    void CleanUp();
};

class PixelShader final : public Shader
{
private:
	mutable ID3D11PixelShader *ps;
protected:
    void Construct(const PixelShader &Val);
	virtual void SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const;
    virtual void SetShader(BOOL CleanUp = false) const;
    virtual void SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const;
    virtual void SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const;
public:
    void ConstructAsRef(const PixelShader &Val);
    PixelShader(const PixelShader &Val);
    PixelShader &operator= (const PixelShader &Val);
	virtual ~PixelShader();
	PixelShader() : ps(NULL){}
	void Load(const std::wstring &FileName, const std::string &EntryPoint) throw (Exception);
};

class VertexShader final : public Shader
{
private:
	mutable ID3D11VertexShader *vs;
    ID3D11InputLayout *layout;
protected:
    void Construct(const VertexShader &Val);
    void ResetRefs(const VertexShader &Val);
	virtual void SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const;
    virtual void SetShader(BOOL CleanUp = false) const;
    virtual void SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const;
    virtual void SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const;
public:
    void ConstructAsRef(const VertexShader &Val);
    VertexShader(const VertexShader &Val);
    VertexShader &operator= (const VertexShader &Val);
	virtual ~VertexShader();
	VertexShader() : vs(NULL), layout(NULL){}
	void Load(const std::wstring &FileName, const std::string &EntryPoint, const VertexMetadata &InputLayout) throw (Exception);
    virtual void Apply() const;
};

class DomainShader final : public Shader
{
private:
    mutable ID3D11DomainShader *ds;
protected:
    void Construct(const DomainShader &Val);
    virtual void SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const;
    virtual void SetShader(BOOL CleanUp = false) const;
    virtual void SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const;
    virtual void SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const;
public:
    void ConstructAsRef(const DomainShader &Val);
    DomainShader(const DomainShader &Val);
    DomainShader &operator= (const DomainShader &Val);
    virtual ~DomainShader();
    DomainShader() : ds(NULL){}
    void Load(const std::wstring &FileName, const std::string &EntryPoint) throw (Exception);
};

class HullShader final : public Shader
{
private:
    mutable ID3D11HullShader *hs;
protected:
    void Construct(const HullShader &Val);
    virtual void SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const;
    virtual void SetShader(BOOL CleanUp = false) const;
    virtual void SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const;
    virtual void SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const;
public:
    void ConstructAsRef(const HullShader &Val);
    HullShader(const HullShader &Val);
    HullShader &operator= (const HullShader &Val);
    virtual ~HullShader();
    HullShader() : hs(NULL){}
    void Load(const std::wstring &FileName, const std::string &EntryPoint) throw (Exception);
};

class GeometryShader : public Shader
{
private:
    mutable ID3D11GeometryShader *gs;
protected:
    void Construct(const GeometryShader &Val);
    virtual void SetShaderResourceView(RegisterType Register, ID3D11ShaderResourceView *SRW) const;
    virtual void SetShader(BOOL CleanUp = false) const;
    virtual void SetConstantBuffer(RegisterType Register, ID3D11Buffer *ConstantBuffer) const;
    virtual void SetSamplerState(RegisterType Register, ID3D11SamplerState *SamplerState) const;
public:
    void ConstructAsRef(const GeometryShader &Val);
    GeometryShader(const GeometryShader &Val);
    GeometryShader &operator= (const GeometryShader &Val);
    virtual ~GeometryShader();
    GeometryShader() : gs(NULL){}
    void Load(const std::wstring &FileName, const std::string &EntryPoint) throw (Exception);
    void Load(const std::wstring &FileName, const std::string &EntryPoint, const StreamOutMetadata &InputSOMetadata) throw (Exception);
};

struct ShadersSet
{
    VertexShader vs;
    PixelShader ps;
    GeometryShader gs;
    HullShader hs;
    DomainShader ds;
};

};
