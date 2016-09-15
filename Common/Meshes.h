/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <D3DHeaders.h>
#include <Exception.h>
#include <Vector2.h>
#include <MeshesFwd.h>
#include <vector>
#include <map>
#include <Utils/VertexArray.h>

namespace Meshes
{

class IMesh
{
public:
	virtual ~IMesh(){}
	virtual void Release() = 0;
	virtual void Draw(INT SubsetNumber = -1) const throw (Exception) = 0;
	virtual INT GetSubsetCount() const = 0;
	virtual const MaterialData &GetSubsetMaterial(INT SubsetNumber) const throw (Exception) = 0;
	virtual const VertexMetadata &GetVertexMetadata() const throw (Exception) = 0;
	virtual void SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception) = 0;
};

class IVertexAcessableMesh : public IMesh
{
public:
    virtual ~IVertexAcessableMesh(){}
    virtual const IndicesStorage & GetIndices() const = 0;
    virtual const Utils::DirectX::VertexArray &GetVertices() const = 0;
};

AdjacencyStorage FindAdjacency(const IVertexAcessableMesh &Mesh);

class IFileMesh : public IMesh
{
public:
    virtual ~IFileMesh(){}
    virtual void Load(const std::string &FileName) = 0;
};

class MeshesContainer
{
private:
    typedef std::map<MeshId, Meshes::IMesh *> MeshesStorage;
    MeshesStorage meshes;
public:
    ~MeshesContainer();
    MeshId LoadMesh(const std::string &MeshFilePath, Meshes::MeshType MeshType) throw (Exception);
    const Meshes::IMesh * GetMesh(MeshId Id) const throw (Exception);
    Meshes::IMesh * GetMesh(MeshId Id) throw (Exception);
    void RemoveMesh(MeshId Id);
};

class OBJMesh : public IFileMesh
{
private:
	SubsetsStorage subsets;
	ID3D11Buffer *vertexBuffer, *indexBuffer;
	VertexMetadata vertexMetadata;
public:
    OBJMesh(const OBJMesh &) = delete;
    OBJMesh &operator=(const OBJMesh &) = delete;
	OBJMesh() : vertexBuffer(NULL), indexBuffer(NULL) {}
	virtual ~OBJMesh(){ Release(); }
	virtual void Load(const std::string &FileName) throw (Exception);
	virtual void Release();
	virtual void Draw(INT SubsetNumber = -1) const throw (Exception);
	virtual INT GetSubsetCount() const throw (Exception) { return subsets.size(); }
	virtual const MaterialData &GetSubsetMaterial(INT SubsetNumber) const throw (Exception);
	virtual const VertexMetadata &GetVertexMetadata() const throw (Exception) { return vertexMetadata; }
	virtual void SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception);	
};

class ColladaBinaryMesh : public IFileMesh
{
private:
    struct SubsetData
    {
        MaterialData material;
        ID3D11Buffer *vertexBuffer, *indexBuffer; 
        INT verticesCnt;
    };
    typedef std::vector<SubsetData> SubsetsStorage;
    SubsetsStorage subsets;
    VertexMetadata vertexMetadata;
    MaterialData tmpMaterial;
public:
	ColladaBinaryMesh(){}
	virtual ~ColladaBinaryMesh(){ Release(); }
	virtual void Load(const std::string &FileName) throw (Exception);
	virtual void Release();
	virtual void Draw(INT SubsetNumber = -1) const throw (Exception);
	virtual INT GetSubsetCount() const throw (Exception) { return subsets.size(); }
    virtual const MaterialData &GetSubsetMaterial(INT SubsetNumber) const throw (Exception){return tmpMaterial;}
	virtual const VertexMetadata &GetVertexMetadata() const throw (Exception) { return vertexMetadata; }
    virtual void SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception){tmpMaterial = Material;}
};

class SimpleCone : public Meshes::IVertexAcessableMesh
{
private:
    VertexMetadata vertexMetadata;
    ID3D11Buffer *vertexBuffer = NULL, *indexBuffer = NULL;
    MaterialData material;
    Utils::DirectX::VertexArray vertices;
    IndicesStorage indices;
public:
    virtual ~SimpleCone(){Release();}
    void Init(FLOAT Height, FLOAT Radius, UINT SlicesCount, const Vector3 &Dir = {0.0f, 1.0f, 0.0f}) throw (Exception);
    virtual void Release();
    virtual void Draw(INT SubsetNumber = -1) const throw (Exception);
    virtual INT GetSubsetCount() const {return 1;}
    virtual const MaterialData &GetSubsetMaterial(INT SubsetNumber) const throw (Exception);
    virtual const VertexMetadata &GetVertexMetadata() const throw (Exception){return vertexMetadata;}
    virtual void SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception);
    virtual const IndicesStorage & GetIndices() const {return indices;}
    virtual const Utils::DirectX::VertexArray &GetVertices() const {return vertices;}
};

class SimpleSphere : public Meshes::IVertexAcessableMesh
{
private:
    VertexMetadata vertexMetadata;
    ID3D11Buffer *vertexBuffer = NULL, *indexBuffer = NULL;
    MaterialData material;
    Utils::DirectX::VertexArray vertices;
    IndicesStorage indices;
public:
    virtual ~SimpleSphere(){Release();}
    void Init(FLOAT Radius, UINT XSlices, UINT YSlices) throw (Exception);
    void Init(FLOAT Radius, UINT XSlices, UINT YSlices, const RangeF &XAngle, const RangeF &YAngle) throw (Exception);
    virtual void Release();
    virtual void Draw(INT SubsetNumber = -1) const throw (Exception);
    virtual INT GetSubsetCount() const {return 1;}
    virtual const MaterialData &GetSubsetMaterial(INT SubsetNumber) const throw (Exception);
    virtual const VertexMetadata &GetVertexMetadata() const throw (Exception){return vertexMetadata;}
    virtual void SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception);
    virtual const IndicesStorage & GetIndices() const {return indices;}
    virtual const Utils::DirectX::VertexArray &GetVertices() const {return vertices;}
};

class Triangle : public IMesh
{
public:
    struct VertexDefinition
    {
        D3DXVECTOR3 pos = {0.0f, 0.0f, 0.0f};
        D3DXVECTOR4 color = {1.0f, 1.0f, 1.0f, 1.0f};
        VertexDefinition(){}
        VertexDefinition(const D3DXVECTOR3 &Pos, const D3DXVECTOR4 &Color) : pos(Pos), color(Color) {}
    };
private:
    VertexMetadata vertexMetadata;
    ID3D11Buffer *vertexBuffer = NULL;
    MaterialData material;
    UINT vertexSize = 0;
public:
    virtual ~Triangle(){Release();}
    void Init(const VertexDefinition &A, const VertexDefinition &B, const VertexDefinition &C) throw (Exception);
    virtual void Release();
    virtual void Draw(INT SubsetNumber = -1) const throw (Exception);
    virtual INT GetSubsetCount() const {return 1;}
    virtual const VertexMetadata &GetVertexMetadata() const throw (Exception){return vertexMetadata;}
    virtual const MaterialData &GetSubsetMaterial(INT SubsetNumber) const throw (Exception);
    virtual void SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception);
};

class Fan : public Meshes::IVertexAcessableMesh
{
private:
    Meshes::VertexMetadata vertexMetadata;
    ID3D11Buffer *vertexBuffer = NULL, *indexBuffer = NULL;
    Meshes::MaterialData material;
    Utils::DirectX::VertexArray vertices;
    IndicesStorage indices;
public:
    virtual ~Fan(){Release();}
    void Init(const D3DXVECTOR3 &Up, const D3DXVECTOR3 &Right, FLOAT Height, FLOAT Radius, UINT Slices) throw (Exception);
    void Release();
    void Draw(INT SubsetNumber = -1) const throw (Exception);
    virtual INT GetSubsetCount() const {return 1;}
    virtual const Meshes::MaterialData &GetSubsetMaterial(INT SubsetNumber) const throw (Exception);
    virtual const Meshes::VertexMetadata &GetVertexMetadata() const throw (Exception){return vertexMetadata;}
    virtual void SetSubsetMaterial(INT SubsetNumber, const Meshes::MaterialData &Material) throw (Exception);
    virtual const IndicesStorage & GetIndices() const {return indices;}
    virtual const Utils::DirectX::VertexArray &GetVertices() const {return vertices;}
};

class Torus : public Meshes::IVertexAcessableMesh
{
private:
    Meshes::VertexMetadata vertexMetadata;
    ID3D11Buffer *vertexBuffer = NULL, *indexBuffer = NULL;
    Meshes::MaterialData material;
    Utils::DirectX::VertexArray vertices;
    IndicesStorage indices;
public:
    virtual ~Torus(){Release();}
    void Init(FLOAT InnerRadius, FLOAT OuterRadius, UINT SliceSteps, UINT Steps);
    void Release();
    void Draw(INT SubsetNumber = -1) const throw (Exception);
    virtual INT GetSubsetCount() const {return 1;}
    virtual const Meshes::MaterialData &GetSubsetMaterial(INT SubsetNumber) const throw (Exception);
    virtual const Meshes::VertexMetadata &GetVertexMetadata() const throw (Exception){return vertexMetadata;}
    virtual void SetSubsetMaterial(INT SubsetNumber, const Meshes::MaterialData &Material) throw (Exception);
    const IndicesStorage & GetIndices() const {return indices;}
    const Utils::DirectX::VertexArray &GetVertices() const {return vertices;}
};

class CustomMesh : public IMesh
{
private:
    VertexMetadata vertexMetadata;
    ID3D11Buffer *vertexBuffer = NULL, *indexBuffer = NULL;
    UINT vertexSize = 0;
    SubsetsStorage subsets;
public:
    void Init(const VertexMetadata &VertexMetadata,
              const Utils::DirectX::VertexArray &Vertices,
              const IndicesStorage &Indices,
              const SubsetsStorage &Subsets) throw (Exception);
    void Init(const VertexMetadata &VertexMetadata);
    void Update(const Utils::DirectX::VertexArray &Vertices,
                const std::vector<UINT> &Indices) throw (Exception);
    virtual ~CustomMesh(){Release();}
    virtual void Release();
    virtual void Draw(INT SubsetNumber = -1) const throw (Exception);
    virtual INT GetSubsetCount() const {return subsets.size();}
    virtual const VertexMetadata &GetVertexMetadata() const throw (Exception){return vertexMetadata;}
    virtual const MaterialData &GetSubsetMaterial(INT SubsetNumber) const throw (Exception);
    virtual void SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception);
    void SetSubsets(const SubsetsStorage &Subsets) {subsets = Subsets;}

};

}