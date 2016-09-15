/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Exception.h>
#include <D3DHeaders.h>
#include <vector>

namespace Meshes
{

DECLARE_EXCEPTION(MeshException);
DECLARE_EXCEPTION(MeshesContainerException);

enum MeshType
{
    MT_COLLADA_BINARY,
    MT_OBJ
};

struct MaterialData
{
    ID3D11ShaderResourceView *colorSRV = NULL, *normalSRV = NULL;
    D3DXVECTOR4 ambientColor = {0.0f, 0.0f, 0.0f, 0.0f};
    D3DXVECTOR4 diffuseColor = {1.0f, 1.0f, 1.0f, 1.0f};
    D3DXVECTOR4 specularColor = {1.0f, 1.0f, 1.0f, 1.0f};
    FLOAT specularPower = 10.0f;
};

struct SubsetData
{
    MaterialData material;
    INT startIndex = 0, indicesCnt = 0;
};

typedef std::vector<SubsetData> SubsetsStorage;

typedef std::vector<D3D11_INPUT_ELEMENT_DESC> VertexMetadata;

typedef std::vector<UINT> IndicesStorage;

struct VertexAdjacency
{
    IndicesStorage vertices;
    IndicesStorage indices;
};

typedef std::vector<VertexAdjacency> AdjacencyStorage;

typedef LONG MeshId;

}