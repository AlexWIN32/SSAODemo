/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <D3DHeaders.h>
#include <vector>
#include <string>
#include <map>
#include <Exception.h>

namespace Utils
{

namespace DirectX
{

DECLARE_EXCEPTION(SemanticNotFoundException);
DECLARE_EXCEPTION(InvalidSemanticIndexException);
DECLARE_EXCEPTION(InvalidDataException);
DECLARE_EXCEPTION(IndexOutOfRangeException);

class VertexArray final
{
public:
    struct ElementDescription
    {
        std::string semanticName;
        UINT index = 0;
        UINT size = 0;
        ElementDescription(const std::string &SemanticName, UINT Index, UINT Size)
        :semanticName(SemanticName), index(Index), size(Size)
        {}
    };
    typedef std::vector<ElementDescription> ElementsStorage;
    typedef std::vector<std::string> SemanticNamesStorage;
private:
    struct Element
    {
        UINT offset = 0;
        UINT size = 0;
    };
    class PackedData
    {
    private:
        std::vector<char> rawData;
        UINT dataSize = 0;
        template <class TVar>
        void Init(const TVar &Var)
        {
             dataSize = sizeof(TVar);
             rawData.resize(dataSize);

             const char *ptr = reinterpret_cast<const char*>(&Var);

             std::copy(ptr, ptr + dataSize, rawData.begin());
        }
    public:
        PackedData(FLOAT Float){Init(Float);}
        PackedData(const D3DXVECTOR2 &Vec2){Init(Vec2);}
        PackedData(const D3DXVECTOR3 &Vec3){Init(Vec3);}
        PackedData(const D3DXVECTOR4 &Vec4){Init(Vec4);}
        PackedData(const D3DXCOLOR &Col){Init(Col);}
        const char* GetData() const {return &rawData[0];}
        UINT GetDataSize() const {return dataSize;}
    };
    typedef std::map<std::string, Element> VertexElementsStorage;
    VertexElementsStorage vertexElements;
    mutable std::vector<char> rawData;
    UINT vertexSize = 0, verticesCount = 0;
    void SetElementRawData(const std::string &SemanticName, UINT Index, const char *Data) throw (Exception);
    char *GetElementRawData(const std::string &SemanticName, UINT Index) const throw (Exception);
public:
    void Init(const ElementsStorage &Elements, UINT VerticesCount = 0) throw (Exception);
    template<class TData>
    TData Get(const std::string &SemanticName, UINT Index) const throw (Exception)
    {
        const char* rawData = GetElementRawData(SemanticName, Index);
        return *reinterpret_cast<const TData*>(rawData);
    }
    template<class TData>
    TData &Get(const std::string &SemanticName, UINT Index) throw (Exception)
    {
        char* rawData = GetElementRawData(SemanticName, Index);
        return *reinterpret_cast<TData*>(rawData);
    }
    template<class TData>
    void Set(const std::string &SemanticName, UINT Index, const TData &Data) throw (Exception)
    {
        const char* rawData = reinterpret_cast<const char*>(&Data);
        SetElementRawData(SemanticName, Index, rawData);
    }
    template<typename... Args>
    void Set(const SemanticNamesStorage &SemanticNames, UINT Index, const Args&... Data) throw (Exception)
    {
        std::vector<PackedData> packedData = {Data...};

        if(packedData.size() != SemanticNames.size())
            throw InvalidDataException("Inconsistent data");

        for(size_t i = 0; i < SemanticNames.size(); i++)
            SetElementRawData(SemanticNames[i], Index, packedData[i].GetData());
    }
    const char* GetRawData() const {return &rawData[0];}
    UINT GetVerticesCount() const {return verticesCount;}
    UINT GetVertixSize() const {return vertexSize;}
    void ChangeCount(UINT NewCount);
    void Clear();
};

}

}