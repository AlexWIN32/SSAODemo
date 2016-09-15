/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Utils/VertexArray.h>
#include <Utils/ToString.h>
#include <numeric>

namespace Utils
{

namespace DirectX
{

void VertexArray::SetElementRawData(const std::string &SemanticName, UINT Index, const char *Data) throw (Exception)
{
    if(Index >= verticesCount)
        throw IndexOutOfRangeException("index " + Utils::to_string(Index) + " is out of range");

    auto it = vertexElements.find(SemanticName);
    if(it == vertexElements.end())
        throw SemanticNotFoundException(SemanticName +" semantic not found");

    const Element &elemData = it->second;

    int offset = Index * vertexSize + elemData.offset;

    memcpy(&rawData[offset], Data, elemData.size);
}

char *VertexArray::GetElementRawData(const std::string &SemanticName, UINT Index) const throw (Exception)
{
    if(Index >= verticesCount)
        throw IndexOutOfRangeException("index " + Utils::to_string(Index) + " is out of range");

    auto it = vertexElements.find(SemanticName);
    if(it == vertexElements.end())
        throw SemanticNotFoundException(SemanticName +" semantic not found");

    return &rawData[Index * vertexSize + it->second.offset];
}

void VertexArray::Init(const ElementsStorage &Elements, UINT VerticesCount) throw (Exception)
{
    verticesCount = VerticesCount;

    std::vector<UINT> byteSizes(Elements.size());

    for(const ElementDescription &descr : Elements){
        if(descr.index >= Elements.size())
            throw InvalidSemanticIndexException("invalid index for semantic " + descr.semanticName);

        byteSizes[descr.index] = descr.size;

        vertexSize += descr.size;
    }

    std::vector<UINT> buteOffsets;

    for(auto it = byteSizes.begin(); it != byteSizes.end(); ++it){

        UINT offset = std::accumulate(byteSizes.begin(), it, 0);
        buteOffsets.push_back(offset);
    }

    for(const ElementDescription &descr : Elements){

        Element newElem;
        newElem.offset = buteOffsets[descr.index];
        newElem.size = descr.size;
        
        vertexElements.insert({descr.semanticName, newElem});
    }

    rawData.resize(vertexSize * verticesCount);
}

void VertexArray::Clear()
{
    rawData.clear();
    verticesCount = 0;
}

void VertexArray::ChangeCount(UINT NewCount)
{
    rawData.resize(vertexSize * NewCount);
    verticesCount = NewCount;
}

}

}