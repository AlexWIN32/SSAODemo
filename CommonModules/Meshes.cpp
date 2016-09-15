/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Meshes.h>
#include <Texture.h>
#include <DeviceKeeper.h>
#include <Utils/FileGuard.h>
#include <Utils/ToString.h>
#include <Utils/DirectX.h>
#include <MathHelpers.h>
#include <Vector2.h>
#include <Basis.h>
#include <cstdio>
#include <memory>

namespace Meshes
{

static void AddVertexAdjacency(UINT Index, UINT ExceptIndex, const IndicesStorage &Indices, IndicesStorage &VertexAdjacency)
{
    if(Indices[Index] == ExceptIndex)
        return;

    auto cit = std::find(VertexAdjacency.begin(), VertexAdjacency.end(), Indices[Index]);
    if(cit == VertexAdjacency.end())
        VertexAdjacency.push_back(Indices[Index]);
}

static VertexAdjacency FindAdjacencyForVertex(UINT VIndex,
                                             const IndicesStorage &Indices,
                                             const Utils::DirectX::VertexArray &Vertices)
{
    VertexAdjacency vertexAdjacency;

    D3DXVECTOR3 vertexPos = Vertices.Get<D3DXVECTOR3>("POSITION", VIndex);

    for(UINT i = 0; i < Indices.size(); i+=3)
        for(UINT tI = i; tI < i + 3; tI++){

            BOOL triangleFound = Indices[tI] == VIndex;

            if(!triangleFound){
                D3DXVECTOR3 testVertexPos = Vertices.Get<D3DXVECTOR3>("POSITION", Indices[tI]);
                triangleFound = vertexPos == testVertexPos;
            }

             if(triangleFound){
                AddVertexAdjacency(i + 0, VIndex, Indices, vertexAdjacency.vertices);
                AddVertexAdjacency(i + 1, VIndex, Indices, vertexAdjacency.vertices);
                AddVertexAdjacency(i + 2, VIndex, Indices, vertexAdjacency.vertices);

                vertexAdjacency.indices.push_back(Indices[i + 0]);
                vertexAdjacency.indices.push_back(Indices[i + 1]);
                vertexAdjacency.indices.push_back(Indices[i + 2]);

                break;
            }
        }

    return vertexAdjacency;
}

AdjacencyStorage FindAdjacency(const IVertexAcessableMesh &Mesh)
{
    const IndicesStorage &indices = Mesh.GetIndices();
    const Utils::DirectX::VertexArray &vertices = Mesh.GetVertices();

    AdjacencyStorage adj(vertices.GetVerticesCount());

    for(UINT v = 0; v < adj.size(); v++)
        adj[v] = FindAdjacencyForVertex(v, indices, vertices);

    return adj;
}

MeshesContainer::~MeshesContainer()
{
    for(auto pair : meshes)
        ReleaseCOM(pair.second);
}

MeshId MeshesContainer::LoadMesh(const std::string &MeshFilePath, Meshes::MeshType MeshType) throw (Exception)
{     
    MeshId id = std::hash<std::string>()(MeshFilePath);

    if(meshes.find(id) != meshes.end())
        throw MeshesContainerException("mesh " + MeshFilePath + " already exsists");

    std::unique_ptr<Meshes::IFileMesh> meshPtr;

    if(MeshType == Meshes::MT_COLLADA_BINARY)
        meshPtr.reset(new Meshes::ColladaBinaryMesh());
    else if(MeshType == Meshes::MT_OBJ)
        meshPtr.reset(new Meshes::OBJMesh());

    meshPtr->Load(MeshFilePath);

    meshes.insert({id, meshPtr.release()});

    return id;
}

const Meshes::IMesh * MeshesContainer::GetMesh(MeshId Id) const throw (Exception)
{
    return GetMesh(Id);
}

Meshes::IMesh * MeshesContainer::GetMesh(MeshId Id) throw (Exception)
{
    auto it = meshes.find(Id);

    if(it == meshes.end())
        throw MeshesContainerException("mesh " + Utils::to_string(Id) + " not found");

    return it->second;
}

void MeshesContainer::RemoveMesh(MeshId Id)
{
    auto it = meshes.find(Id);
    if(it != meshes.end()){
        delete it->second;
        meshes.erase(it);
    }
}

static std::vector<std::string> read_obj_file_line(FILE* File, bool &Eof, const std::string &FileName) throw (Exception)
{
	std::vector<std::string> splData;
	std::string token;

	bool skipToEndOfLine = false;
	char c;
	while (true){
		size_t cnt = fread(&c, 1, 1, File);
		if (ferror(File))
			throw MeshException(FileName+": read error");

		if (feof(File) || c == '\n'){
			if (token != ""){
				splData.push_back(token);
				token = "";
			}
			
			Eof = feof(File);
			break;
		}

		if (skipToEndOfLine)
			continue;

		if (c == '#'){
			skipToEndOfLine = true;
			continue;
		}
				
		if (c == ' ' && token != ""){
			splData.push_back(token);
			token = "";
		}
		else
			token += c;
	}

	return splData;
}

static std::vector<std::string> split_str(const std::string &String, char Delimiter)
{
	std::vector<std::string> splData;
	std::string token;

	std::string::const_iterator ci;
	for (ci = String.begin(); ci != String.end(); ++ci)
		if (*ci == Delimiter){
			splData.push_back(token);
			token = "";
		}
		else
			token += *ci;

	splData.push_back(token);
	return splData;
}

static void get_obj_face_data(const std::string &FaceData, INT &PosInd, INT &TCInd, INT &NormInd, const std::string &FileName)  throw (Exception)
{
	std::vector<std::string> splStr = split_str(FaceData, '/');
	if (!splStr.size())
		throw MeshException(FileName + ":Invalid data format");	

	PosInd = atoi(splStr[0].c_str());

	if (splStr.size() > 1){
		if (splStr[1] == "" && splStr.size() == 2)
			throw MeshException(FileName + ":Invalid data format");		

		TCInd = (splStr[1] != "") ? atoi(splStr[1].c_str()) : -1;		
	}
	else
		TCInd = -1;

	if (splStr.size() > 2){
		if (splStr[2] == "")
			throw MeshException(FileName + ":Invalid data format");

		NormInd = atoi(splStr[2].c_str());
	}
	else
		NormInd = -1;			
}

struct OBJVerticesData
{
	struct VertexDescription
	{
		INT posIndex, normIndex, tcIndex;
	};

	typedef std::vector<VertexDescription> Face;

	struct FacesGroup
	{
		std::string  materialName;
		std::vector<Face> faces;
	};

	std::vector<FacesGroup> facesGroups;
	FacesGroup correntGroup;
	std::vector<D3DXVECTOR3> points, normals;
	std::vector<D3DXVECTOR2> texcoords;
	std::string materialFileName;
};

inline const std::string &get_obj_one_value(const std::vector<std::string> &splLine, const std::string &FileName)
{
	if (splLine.size() != 2)
		throw MeshException(FileName + ":Invalid data format");

	return splLine[1];
}

inline D3DXVECTOR3 get_obj_vector3(const std::vector<std::string> &splLine, const std::string &FileName)
{
	if (splLine.size() != 4)
		throw MeshException(FileName + ":Invalid data format");

	float x = atof(splLine[1].c_str());
	float y = atof(splLine[2].c_str());
	float z = atof(splLine[3].c_str());

	return D3DXVECTOR3(x, y, z);
}

inline D3DXVECTOR2 get_obj_vector2(const std::vector<std::string> &splLine, const std::string &FileName)
{
	if (splLine.size() != 3)
		throw MeshException(FileName + ":Invalid data format");

	float x = atof(splLine[1].c_str());
	float y = atof(splLine[2].c_str());

	return D3DXVECTOR2(x, y);
}

static OBJVerticesData load_obj_vertices(const std::string &FileName) throw (Exception)
{	
	FILE* file = fopen(FileName.c_str(), "r");

	if (!file)
		throw MeshException("cant open " + FileName);

    Utils::FileGuard guard(file);

	OBJVerticesData data;
	OBJVerticesData::FacesGroup correntGroup;

	while (true){
		bool eof = false;
		std::vector<std::string> splLine = read_obj_file_line(file, eof, FileName);

		if (eof && !splLine.size())
			break;

		if (!splLine.size())
			continue;

		const std::string dataType = splLine[0];
		if (dataType == "mtllib"){			

			data.materialFileName = get_obj_one_value(splLine, FileName);
		}
		else if (dataType == "v" || dataType == "vn"){
			
			(dataType == "v" ? data.points : data.normals).push_back(get_obj_vector3(splLine, FileName));
		}
		else if (dataType == "vt"){
			
			data.texcoords.push_back(get_obj_vector2(splLine, FileName));
		}
		else if (dataType == "usemtl"){
			
			if (correntGroup.faces.size())
				data.facesGroups.push_back(correntGroup);

			correntGroup.faces.clear();
			correntGroup.materialName = get_obj_one_value(splLine, FileName);
		}
		else if (dataType == "f"){
			if (splLine.size() < 3)
				throw MeshException(FileName + ":Invalid data format");

			OBJVerticesData::Face face;
			for (size_t i = 1; i < splLine.size(); i++){
				OBJVerticesData::VertexDescription vd;
				get_obj_face_data(splLine[i], vd.posIndex, vd.tcIndex, vd.normIndex, FileName);
				vd.posIndex--;
				vd.tcIndex--;
				vd.normIndex--;
				face.push_back(vd);
			}
			correntGroup.faces.push_back(face);
		}
	}

	if (correntGroup.faces.size())
		data.facesGroups.push_back(correntGroup);

	return data;
}

struct OBJMaterial
{
	std::string name;
	MaterialData material;
};

struct OBJVertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 norm;
	D3DXVECTOR2 tc;
};

std::vector<OBJMaterial> load_obj_materials(const std::string &FileName) throw (Exception)
{
	std::string path = FileName.substr(0, FileName.find_last_of('/'));

    Utils::FileGuard file(FileName, "r");

	std::vector<OBJMaterial> data;
	OBJMaterial currentMaterial;
    FLOAT transparency = 0.0f;

	while (true){
		bool eof = false;
		std::vector<std::string> splLine = read_obj_file_line(file.get(), eof, FileName);

		if (eof)
			break;

		if (!splLine.size())
			continue;
		
		const std::string &dataType = splLine[0];

		if (dataType == "newmtl"){			

			if (currentMaterial.name != ""){
                Meshes::MaterialData &mtl = currentMaterial.material;
                mtl.ambientColor.w = mtl.diffuseColor.w =mtl.specularColor.w = transparency;

				data.push_back(currentMaterial);

                transparency = 0.0f;
            }

			currentMaterial = OBJMaterial();
			currentMaterial.name = get_obj_one_value(splLine, FileName);
		}
		else if (dataType == "Ns"){			

			currentMaterial.material.specularColor.w = atof(get_obj_one_value(splLine, FileName).c_str());
		}
		else if (dataType == "Kd"){
            currentMaterial.material.diffuseColor = Cast<D3DXVECTOR4>(get_obj_vector3(splLine, FileName));
		}
		else if (dataType == "Ks"){

            currentMaterial.material.specularColor = Cast<D3DXVECTOR4>(get_obj_vector3(splLine, FileName));
		}
		else if (dataType == "Ka"){

            currentMaterial.material.ambientColor = Cast<D3DXVECTOR4>(get_obj_vector3(splLine, FileName));
		}
		else if (dataType == "d" || dataType == "Tr"){

            transparency = atof(get_obj_one_value(splLine, FileName).c_str());
		}	
		else if (dataType == "map_Kd"){
			
            currentMaterial.material.colorSRV = Texture::LoadTexture2DFromFile(path + "/" + get_obj_one_value(splLine, FileName));
		}
		else if (dataType == "map_bump" || dataType == "bump"){

            currentMaterial.material.normalSRV = Texture::LoadTexture2DFromFile(path + "/" + get_obj_one_value(splLine, FileName));
		}
	}

	if (currentMaterial.name != ""){
        Meshes::MaterialData &mtl = currentMaterial.material;
        mtl.ambientColor.w = mtl.diffuseColor.w =mtl.specularColor.w = transparency;

		data.push_back(currentMaterial);

        transparency = 0.0f;
    }
	
	return data;
}

void OBJMesh::Load(const std::string &FileName) throw (Exception)
{	
	OBJVerticesData verticesData = load_obj_vertices(FileName);

	if (!verticesData.points.size())
		throw MeshException("Empty points for " + FileName);

	if (!verticesData.normals.size() || !verticesData.texcoords.size())
		throw MeshException("Invalid vertex format for " + FileName + ": must be pos, texCoords and normals");

	
	std::string path = FileName.substr(0, FileName.find_last_of('/'));
	std::vector<OBJMaterial> materials;
	if (verticesData.materialFileName != "")
		materials = load_obj_materials(path + "/" +verticesData.materialFileName);

	D3D11_INPUT_ELEMENT_DESC layout;
	memset(&layout, 0, sizeof(layout));

	layout.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	layout.SemanticName = "POSITION";
	layout.Format = DXGI_FORMAT_R32G32B32_FLOAT;	
	vertexMetadata.push_back(layout);
	
	layout.SemanticName = "NORMAL";
	layout.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	layout.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexMetadata.push_back(layout);

	layout.SemanticName = "TEXCOORD";
	layout.Format = DXGI_FORMAT_R32G32_FLOAT;
	layout.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;		
	vertexMetadata.push_back(layout);

	std::vector<OBJVertex> vertices;
	std::vector<UINT> indices;

	std::vector<OBJVerticesData::FacesGroup>::const_iterator ci;
	for (ci = verticesData.facesGroups.begin(); ci != verticesData.facesGroups.end(); ++ci){
		const OBJVerticesData::FacesGroup &fg = *ci;

		const std::vector<OBJVerticesData::Face> &faces = fg.faces;
		if (!faces.size())
			continue;

		UINT subsetStrtInd = indices.size();

		std::vector<OBJVerticesData::Face>::const_iterator fci;
		for (fci = faces.begin(); fci != faces.end(); ++fci){
			const OBJVerticesData::Face &face = *fci;

			if (face.size() > 4)
				throw MeshException("Invalid data for " + FileName + ": face with more than 4 vertices not supported");

            UINT faceStrtInd = vertices.size();

			OBJVerticesData::Face::const_iterator vci;
			for (vci = face.begin(); vci != face.end(); ++vci){
				const OBJVerticesData::VertexDescription &vDesc = *vci;

				if (vDesc.normIndex < 0 || vDesc.posIndex < 0 || vDesc.tcIndex < 0)
					throw MeshException("Invalid vertex format for " + FileName + ": negative face indices not supported");
				
				if (vDesc.normIndex >= verticesData.normals.size() || 
					vDesc.posIndex >= verticesData.points.size() || 
					vDesc.tcIndex >= verticesData.texcoords.size())
					throw MeshException("Invalid data for " + FileName + ": face index out of data range");

				OBJVertex vtx;
				vtx.pos = verticesData.points[vDesc.posIndex];
				vtx.norm = verticesData.normals[vDesc.normIndex];
				vtx.tc = verticesData.texcoords[vDesc.tcIndex];

				vertices.push_back(vtx);
			}
			
			if (face.size() == 3){
				indices.push_back(faceStrtInd + 0);
				indices.push_back(faceStrtInd + 1);
				indices.push_back(faceStrtInd + 2);
			}
			else{
				indices.push_back(faceStrtInd + 0);
				indices.push_back(faceStrtInd + 1);
				indices.push_back(faceStrtInd + 2);
				indices.push_back(faceStrtInd + 2);
				indices.push_back(faceStrtInd + 3);
				indices.push_back(faceStrtInd + 0);
			}
						
		}				

		MaterialData subsetMaterial;

		std::vector<OBJMaterial>::const_iterator mci;
		for (mci = materials.begin(); mci != materials.end(); ++mci)
			if (mci->name == fg.materialName){
				subsetMaterial = mci->material;
				break;
			}
		
		if (mci == materials.end())
			throw MeshException("Invalid face group for " + FileName + ": material " + fg.materialName + " not found");

		SubsetData subset;
		subset.startIndex = subsetStrtInd;
		subset.indicesCnt = indices.size() - subsetStrtInd;
		subset.material = subsetMaterial;
		subsets.push_back(subset);
	}
	
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(OBJVertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
    memset(&vInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
	vInitData.pSysMem = &vertices[0];
	HR(DeviceKeeper::GetDevice()->CreateBuffer(&vbd, &vInitData, &vertexBuffer));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
    memset(&iInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
	iInitData.pSysMem = &indices[0];
	HR(DeviceKeeper::GetDevice()->CreateBuffer(&ibd, &iInitData, &indexBuffer));
}

void OBJMesh::Release()
{
	if (vertexBuffer)
		ReleaseCOM(vertexBuffer);

	if (indexBuffer)
		ReleaseCOM(indexBuffer);

    for (auto it : subsets){
        ReleaseCOM(it.material.colorSRV);
        ReleaseCOM(it.material.normalSRV);
	}
		
	subsets.clear();
	vertexMetadata.clear();
}

void OBJMesh::Draw(INT SubsetNumber) const throw (Exception)
{
	UINT stride = sizeof(OBJVertex);
	UINT offset = 0;

	DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	DeviceKeeper::GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (SubsetNumber == -1){
		SubsetsStorage::const_iterator ci;
		for (ci = subsets.begin(); ci != subsets.end(); ++ci){
			const SubsetData &subset = *ci;
			DeviceKeeper::GetDeviceContext()->DrawIndexed(subset.indicesCnt, subset.startIndex, 0);
		}
	}
	else{
		if (SubsetNumber < 0 || SubsetNumber >= subsets.size())
			throw MeshException("Invalid subset number");

		const SubsetData &subset = subsets[SubsetNumber];
		DeviceKeeper::GetDeviceContext()->DrawIndexed(subset.indicesCnt, subset.startIndex, 0);
	}
}

const MaterialData &OBJMesh::GetSubsetMaterial(INT SubsetNumber) const throw (Exception)
{
	if (SubsetNumber < 0 || SubsetNumber >= subsets.size())
		throw MeshException("Invalid subset number");

	return subsets[SubsetNumber].material;
}

void OBJMesh::SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception)
{
	if (SubsetNumber < 0 || SubsetNumber >= subsets.size())
		throw MeshException("Invalid subset number");

	subsets[SubsetNumber].material = Material;
}

typedef OBJVertex ColladaVertex;

template<class TNum> 
TNum ReadNumber(FILE *File) throw (Exception)
{
    TNum num;
    if(fread(&num, sizeof(TNum), 1, File) != 1)
        throw MeshException("cant read from file");

    return num;
}

void ColladaBinaryMesh::Release()
{
    SubsetsStorage::iterator it;
    for(it = subsets.begin(); it != subsets.end(); ++it){
        ReleaseCOM(it->vertexBuffer);
        ReleaseCOM(it->indexBuffer);
    }

    subsets.clear();
    vertexMetadata.clear();
}

void ColladaBinaryMesh::Load(const std::string &FilePath) throw (Exception)
{
    D3D11_INPUT_ELEMENT_DESC desc[3] = 
    {
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    vertexMetadata = VertexMetadata(desc, desc + 3);

    Utils::FileGuard file(FilePath, "rb");

    size_t meshesCnt = ReadNumber<size_t>(file.get());

    for(size_t m = 0; m < meshesCnt; m++){
        size_t subsetsCnt = ReadNumber<size_t>(file.get());
        
        for(size_t s = 0; s < subsetsCnt; s++){
            
            size_t vertsCnt = ReadNumber<size_t>(file.get());

            SubsetData newSubset;
            newSubset.verticesCnt = vertsCnt;

            std::vector<ColladaVertex> vertices(vertsCnt);
            std::vector<UINT> indices(vertsCnt);

            for(size_t v = 0; v < vertsCnt; v++){
                ColladaVertex vertex;

                vertex.pos.x = ReadNumber<float>(file.get());
                vertex.pos.y = ReadNumber<float>(file.get());
                vertex.pos.z = ReadNumber<float>(file.get());
                vertex.norm.x = ReadNumber<float>(file.get());
                vertex.norm.y = ReadNumber<float>(file.get());
                vertex.norm.z = ReadNumber<float>(file.get());
                vertex.tc.x = ReadNumber<float>(file.get());
                vertex.tc.y = ReadNumber<float>(file.get());

                vertices[v] = vertex;
                indices[v] = v;
            }

            D3D11_BUFFER_DESC vbd = {};
	        vbd.Usage = D3D11_USAGE_DEFAULT;
	        vbd.ByteWidth = sizeof(ColladaVertex) * vertices.size();
	        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	        

            D3D11_SUBRESOURCE_DATA vInitData = {};            
	        vInitData.pSysMem = &vertices[0];
	        HR(DeviceKeeper::GetDevice()->CreateBuffer(&vbd, &vInitData, &newSubset.vertexBuffer));

            D3D11_BUFFER_DESC ibd = {};
	        ibd.Usage = D3D11_USAGE_DEFAULT;
	        ibd.ByteWidth = sizeof(UINT) * indices.size();
	        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	        

            D3D11_SUBRESOURCE_DATA iInitData = {};            
	        iInitData.pSysMem = &indices[0];
	        HR(DeviceKeeper::GetDevice()->CreateBuffer(&ibd, &iInitData, &newSubset.indexBuffer));

            subsets.push_back(newSubset);
        }                
    }
}

void ColladaBinaryMesh::Draw(INT SubsetNumber) const throw (Exception)
{    
    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if(SubsetNumber == -1){
        SubsetsStorage::const_iterator ci;
        for(ci = subsets.begin(); ci != subsets.end(); ++ci){
            const SubsetData &subset = *ci;

            UINT offset = 0, stride = sizeof(ColladaVertex);
            DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &subset.vertexBuffer, &stride, &offset);
	        DeviceKeeper::GetDeviceContext()->IASetIndexBuffer(subset.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

            DeviceKeeper::GetDeviceContext()->DrawIndexed(subset.verticesCnt, 0, 0);
        }
    }else{
        if (SubsetNumber < 0 || SubsetNumber >= subsets.size())
			throw MeshException("Invalid subset number");

		const SubsetData &subset = subsets[SubsetNumber];

        UINT offset = 0, stride = sizeof(ColladaVertex);
        DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &subset.vertexBuffer, &stride, &offset);
	    DeviceKeeper::GetDeviceContext()->IASetIndexBuffer(subset.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        DeviceKeeper::GetDeviceContext()->DrawIndexed(subset.verticesCnt, 0, 0);

    }
}

void SimpleCone::Init(FLOAT Height, FLOAT Radius, UINT SlicesCount, const Vector3 &Dir) throw (Exception)
{
    Basis::UVNBasis basis;
    basis.SetPos(Cast<D3DXVECTOR3>(-Dir * Height * 0.5f));
    basis.SetDir(Cast<D3DXVECTOR3>(Dir));
    basis.SetInvertUp(true);

    basis.GetMatrix();

    vertexMetadata = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    int vertsCnt = SlicesCount + 2;
    int indsCnt = SlicesCount * 6;

    vertices.Init({{"POSITION", 0, 12}, {"NORMAL", 1, 12}}, vertsCnt);

    const D3DXVECTOR3 &bottomPos = basis.GetPos();
    D3DXVECTOR3 topPos = bottomPos + Cast<D3DXVECTOR3>(Dir) * Height;
    
    vertices.Set({"POSITION", "NORMAL"}, 0, bottomPos, Math::Normalize(bottomPos));
    vertices.Set({"POSITION", "NORMAL"}, 1, topPos, Math::Normalize(topPos));

    float step = (2.0f * Pi) / (float)SlicesCount;

    for(int i = 0; i < SlicesCount; i++){
        float a = step * (float)i;

        D3DXVECTOR3 offset = basis.GetRight() * cosf(a) + basis.GetUp() * sinf(a);
        D3DXVECTOR3 pos = basis.GetPos() + offset * Radius;
    
        vertices.Set({"POSITION", "NORMAL"}, i + 2, pos, Math::Normalize(offset));
    }

    vertexBuffer = Utils::DirectX::CreateBuffer(vertices);

    indices.resize(indsCnt);

    int startVInd = 2;
    for(int i = 0; i < SlicesCount; i++){
        int vInd = startVInd + i;

        int nextVInd = (i == SlicesCount - 1) ? startVInd : vInd + 1;

        int indOffset = i * 6;

        indices[indOffset + 0] = vInd;
        indices[indOffset + 1] = 0;
        indices[indOffset + 2] = nextVInd;
        indices[indOffset + 3] = vInd;
        indices[indOffset + 4] = 1;
        indices[indOffset + 5] = nextVInd;
    }

    indexBuffer = Utils::DirectX::CreateBuffer(indices, D3D11_BIND_INDEX_BUFFER);
}

void SimpleCone::Release()
{
    ReleaseCOM(vertexBuffer);
    ReleaseCOM(indexBuffer);

    vertexMetadata.clear();
}

void SimpleCone::Draw(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber != -1 && SubsetNumber != 0)
        throw MeshException("Invalid subset number");

    UINT offset = 0, vertexSize = vertices.GetVertixSize();

    DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexSize, &offset);
    DeviceKeeper::GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DeviceKeeper::GetDeviceContext()->DrawIndexed(indices.size(), 0, 0);
}

const MaterialData &SimpleCone::GetSubsetMaterial(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber < 0 && SubsetNumber >= 1)
        throw MeshException("Invalid subset number");

    return material;
}

void SimpleCone::SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception)
{
    if(SubsetNumber < 0 && SubsetNumber >= 1)
        throw MeshException("Invalid subset number");

    material = Material;
}

void SimpleSphere::Init(FLOAT Radius, UINT XSlices, UINT YSlices) throw (Exception)
{
    RangeF xAngle(0.0f, D3DX_PI * 2.0f);
    RangeF yAngle(0.0f, D3DX_PI);

    Init(Radius, XSlices, YSlices, xAngle, yAngle);
}

void SimpleSphere::Init(FLOAT Radius, UINT XSlices, UINT YSlices, const RangeF &XAngle, const RangeF &YAngle) throw (Exception)
{
    vertexMetadata = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    UINT xSlicesExt = XSlices + 1;
    UINT ySlicesExt = YSlices + 1;

    UINT vertsCnt = xSlicesExt * ySlicesExt;

    vertices.Init({{"POSITION", 0, 12}, {"NORMAL", 1, 12}}, vertsCnt);

    Utils::DirectX::VertexArray::SemanticNamesStorage semanticsNames = {"POSITION", "NORMAL"};

    FLOAT xStep = (XAngle.maxVal - XAngle.minVal) / (FLOAT)XSlices;
    FLOAT yStep = (YAngle.maxVal - YAngle.minVal) / (FLOAT)YSlices;

    for(UINT y = 0; y < ySlicesExt; y++)
        for(UINT x = 0; x < xSlicesExt; x++){
            FLOAT angX = XAngle.minVal + xStep * (FLOAT)x;
            FLOAT angY = YAngle.minVal + yStep * (FLOAT)y;

            D3DXVECTOR3 pos = Cast<D3DXVECTOR3>(Math::SphericalToDec(angX, angY, Radius));

            UINT ind = y * xSlicesExt + x;
            vertices.Set(semanticsNames, ind, pos, Math::Normalize(pos));
        }
    
    vertexBuffer = Utils::DirectX::CreateBuffer(vertices);

    indices.resize(XSlices * YSlices * 6);

    for(UINT y = 0; y < YSlices; y++)
        for(UINT x = 0; x < XSlices; x++){

            UINT row = y * xSlicesExt + x;
            UINT nextRow = (y + 1) * xSlicesExt + x;

            UINT indOffset = (y * XSlices + x) * 6;

            indices[indOffset + 0] = row;
            indices[indOffset + 1] = row + 1;
            indices[indOffset + 2] = nextRow + 1;
            indices[indOffset + 3] = nextRow + 1;
            indices[indOffset + 4] = nextRow;
            indices[indOffset + 5] = row;
        }

    indexBuffer = Utils::DirectX::CreateBuffer(indices, D3D11_BIND_INDEX_BUFFER);
}

void SimpleSphere::Release()
{
    ReleaseCOM(vertexBuffer);
    ReleaseCOM(indexBuffer);
}

void SimpleSphere::Draw(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber != -1 && SubsetNumber != 0)
        throw MeshException("Invalid subset number");

    UINT offset = 0, vertexSize = vertices.GetVertixSize();

    DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexSize, &offset);
    DeviceKeeper::GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DeviceKeeper::GetDeviceContext()->DrawIndexed(indices.size(), 0, 0);
}

const MaterialData &SimpleSphere::GetSubsetMaterial(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber < 0 && SubsetNumber >= 1)
        throw MeshException("Invalid subset number");

    return material;
}

void SimpleSphere::SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception)
{
    if(SubsetNumber < 0 && SubsetNumber >= 1)
        throw MeshException("Invalid subset number");

    material = Material;
}

void Triangle::Init(const VertexDefinition &A, const VertexDefinition &B, const VertexDefinition &C) throw (Exception)
{
    D3D11_INPUT_ELEMENT_DESC desc[3] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    vertexMetadata = VertexMetadata(desc, desc + 3);

    D3DXVECTOR3 d1 = A.pos - B.pos, d2 = C.pos - B.pos;

    D3DXVECTOR3 normal = Math::Normalize(Math::Cross(d1, d2));

    Utils::DirectX::VertexArray verts;
    verts.Init({{"POSITION", 0, 12}, {"NORMAL", 1, 12}, {"COLOR", 2, 16}}, 3);

    verts.Set({"POSITION", "NORMAL", "COLOR"}, 0, A.pos, A.color, normal);
    verts.Set({"POSITION", "NORMAL", "COLOR"}, 1, B.pos, B.color, normal);
    verts.Set({"POSITION", "NORMAL", "COLOR"}, 2, C.pos, C.color, normal);

    vertexBuffer = Utils::DirectX::CreateBuffer(verts);

    vertexSize = verts.GetVertixSize();
}

void Triangle::Release()
{
    ReleaseCOM(vertexBuffer);
}

void Triangle::Draw(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber != -1 && SubsetNumber != 0)
        throw MeshException("Invalid subset number");

    UINT offset = 0;

    DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexSize, &offset);
    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DeviceKeeper::GetDeviceContext()->Draw(3, 0);
}

const MaterialData &Triangle::GetSubsetMaterial(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber != 0)
        throw MeshException("Invalid subset number");

    return material;
}

void Triangle::SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception)
{
    if(SubsetNumber != 0)
        throw MeshException("Invalid subset number");

    material = Material;
}

void Fan::Init(const D3DXVECTOR3 &Up, const D3DXVECTOR3 &Right, FLOAT Height, FLOAT Radius, UINT Slices) throw (Exception)
{
    vertexMetadata = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    vertices.Init({{"POSITION", 0, 12}, {"NORMAL", 1, 12}}, Slices + 1);

    D3DXVECTOR3 up2 = Math::Normalize(Math::Cross(Right, Up));
    vertices.Set<D3DXVECTOR3, D3DXVECTOR3>({"POSITION", "NORMAL"}, 0, up2 * Height, up2);

    float step  = (D3DX_PI * 2.0f) / (float)Slices;

    for(UINT i = 0; i < Slices; i++){
        float angle = step * (float)i;

        D3DXVECTOR3 pos = (Up * cosf(angle) + Right * sinf(angle)) * Radius;
        D3DXVECTOR3 norm = pos;

        D3DXVec3Normalize(&norm, &norm);

        vertices.Set({"POSITION", "NORMAL"}, i + 1, pos, norm);
    }

    vertexBuffer = Utils::DirectX::CreateBuffer(vertices);

    indices.resize(Slices * 3);

    int startVInd = 1;
    for(int i = 0; i < Slices; i++){
        int vInd = startVInd + i;

        int nextVInd = (i == Slices - 1) ? startVInd : vInd + 1;

        int indOffset = i * 3;

        indices[indOffset + 0] = vInd;
        indices[indOffset + 1] = 0;
        indices[indOffset + 2] = nextVInd;
    }

    indexBuffer = Utils::DirectX::CreateBuffer(indices, D3D11_BIND_INDEX_BUFFER);
}

void Fan::Release()
{
    ReleaseCOM(vertexBuffer);
    ReleaseCOM(indexBuffer);
}

void Fan::Draw(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber != -1 && SubsetNumber != 0)
        throw Meshes::MeshException("Invalid subset number");

    UINT offset = 0, vertexSize = vertices.GetVertixSize();

    DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexSize, &offset);
    DeviceKeeper::GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DeviceKeeper::GetDeviceContext()->DrawIndexed(indices.size(), 0, 0);
}

const Meshes::MaterialData &Fan::GetSubsetMaterial(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber != 0)
        throw Meshes::MeshException("Invalid subset number");

    return material;
}

void Fan::SetSubsetMaterial(INT SubsetNumber, const Meshes::MaterialData &Material) throw (Exception)
{
    if(SubsetNumber != 0)
        throw Meshes::MeshException("Invalid subset number");

    material = Material;
}

void Torus::Init(FLOAT InnerRadius, FLOAT OuterRadius, UINT SliceSteps, UINT Steps)
{
    if(InnerRadius <= 0.0f)
        throw Meshes::MeshException("Invalid inner radius");

    if(OuterRadius <= 0.0f)
        throw Meshes::MeshException("Invalid outer radius");

    if(InnerRadius >= OuterRadius)
        throw Meshes::MeshException("Inner radius is greater than outer radius");

    vertexMetadata = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    float sliceRadius = (OuterRadius - InnerRadius) * 0.5f;

    float stepRadius = InnerRadius + sliceRadius;

    vertices.Init({{"POSITION", 0, 12}, {"NORMAL", 1, 12}}, SliceSteps * Steps);

    float step = (D3DX_PI * 2.0f) / (float)Steps, sliceStep = (D3DX_PI * 2.0f) / (float)SliceSteps;

    D3DXVECTOR3 dir(1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f), up(0.0f, 1.0f, 0.0f);

    for(UINT i = 0; i < Steps; i++){
        float angle = step * (float)i;

        D3DXVECTOR3 pos = (dir * cosf(angle) + right * sinf(angle)) * stepRadius;

        D3DXVECTOR3 norm = Math::Normalize(pos);

        D3DXVECTOR3 sliceAxis = Math::Normalize(Math::Cross(up, norm));

        for(UINT e = 0; e < SliceSteps; e++){

            D3DXVECTOR3 vec = Math::RotationAxis(norm, sliceAxis, sliceStep * (float)e);
            
            D3DXVECTOR3 normal = Math::Normalize(vec);

            D3DXVECTOR3 newPos = pos + normal * sliceRadius;

            vertices.Set({"POSITION", "NORMAL"}, i * SliceSteps + e, newPos, normal);
        }
    }

    vertexBuffer = Utils::DirectX::CreateBuffer(vertices);

    indices.resize(SliceSteps * Steps * 6);

   for(UINT i = 0; i < Steps; i++){
        UINT startOfSlice = i * SliceSteps;

        for(UINT e = 0; e < SliceSteps; e++){

            UINT faceInd = (startOfSlice + e) * 6;

            UINT c1 = startOfSlice + e;

            UINT c2  = c1 + 1;
            if((c2 % SliceSteps) == 0)
                c2 = startOfSlice;

            UINT c3 = c1 + SliceSteps;
            if(c3 >= vertices.GetVerticesCount())
                c3 -= vertices.GetVerticesCount();

            UINT c4 = c2 + SliceSteps;
            if(c4 >= vertices.GetVerticesCount())
                c4 -= vertices.GetVerticesCount();

            indices[faceInd + 0] = c1;
            indices[faceInd + 1] = c3;
            indices[faceInd + 2] = c4;
            indices[faceInd + 3] = c4;
            indices[faceInd + 4] = c2;
            indices[faceInd + 5] = c1;
        }
    }

    indexBuffer = Utils::DirectX::CreateBuffer(indices, D3D11_BIND_INDEX_BUFFER);
}

void Torus::Release()
{
    ReleaseCOM(vertexBuffer);
    ReleaseCOM(indexBuffer);
}

void Torus::Draw(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber != -1 && SubsetNumber != 0)
        throw Meshes::MeshException("Invalid subset number");

    UINT offset = 0, vertexSize = vertices.GetVertixSize();

    DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexSize, &offset);
    DeviceKeeper::GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DeviceKeeper::GetDeviceContext()->DrawIndexed(indices.size(), 0, 0);
}

const Meshes::MaterialData &Torus::GetSubsetMaterial(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber != 0)
        throw Meshes::MeshException("Invalid subset number");

    return material;
}

void Torus::SetSubsetMaterial(INT SubsetNumber, const Meshes::MaterialData &Material) throw (Exception)
{
    if(SubsetNumber != 0)
        throw Meshes::MeshException("Invalid subset number");

    material = Material;
}

void CustomMesh::Init(const VertexMetadata &VertexMetadata,
            const Utils::DirectX::VertexArray &Vertices,
            const IndicesStorage &Indices,
            const SubsetsStorage &Subsets) throw (Exception)
{
    vertexMetadata = VertexMetadata;

    indexBuffer = Utils::DirectX::CreateBuffer(Indices, D3D11_BIND_INDEX_BUFFER);
    vertexBuffer = Utils::DirectX::CreateBuffer(Vertices);

    vertexSize = Vertices.GetVertixSize();
    subsets = Subsets;
}

void CustomMesh::Init(const VertexMetadata &VertexMetadata)
{
    vertexMetadata = VertexMetadata;
}

void CustomMesh::Release()
{
    ReleaseCOM(vertexBuffer);
    ReleaseCOM(indexBuffer);

    vertexMetadata.clear();
    subsets.clear();
}

void CustomMesh::Draw(INT SubsetNumber) const throw(Exception)
{
    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT offset = 0;
    DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexSize, &offset);
    DeviceKeeper::GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if(SubsetNumber == -1){
        for(const SubsetData &subset : subsets)
            DeviceKeeper::GetDeviceContext()->DrawIndexed(subset.indicesCnt, subset.startIndex, 0);
    }else{
        if (SubsetNumber < 0 || SubsetNumber >= subsets.size())
            throw MeshException("Invalid subset number");

        const SubsetData &subset = subsets[SubsetNumber];

        DeviceKeeper::GetDeviceContext()->DrawIndexed(subset.indicesCnt, subset.startIndex, 0);

    }
}

const MaterialData &CustomMesh::GetSubsetMaterial(INT SubsetNumber) const throw (Exception)
{
    if(SubsetNumber < 0 || SubsetNumber >= subsets.size())
        throw MeshException("Invalid subset number");

    return subsets[SubsetNumber].material;
}

void CustomMesh::SetSubsetMaterial(INT SubsetNumber, const MaterialData &Material) throw (Exception)
{
    if(SubsetNumber < 0 || SubsetNumber >= subsets.size())
        throw MeshException("Invalid subset number");

    subsets[SubsetNumber].material = Material;
}

void CustomMesh::Update(const Utils::DirectX::VertexArray &Vertices, const std::vector<UINT> &Indices) throw (Exception)
{
    ReleaseCOM(vertexBuffer);
    ReleaseCOM(indexBuffer);

    if(vertexSize == 0)
        vertexSize = Vertices.GetVertixSize();
    else if(vertexSize != Vertices.GetVertixSize())
        throw MeshException("Inconsistent vertex size");

    if(Vertices.GetVerticesCount() == 0)
        throw MeshException("No vertices data");

    if(Indices.size() == 0)
        throw MeshException("No indices data");

    indexBuffer = Utils::DirectX::CreateBuffer(Indices, D3D11_BIND_INDEX_BUFFER);
    vertexBuffer = Utils::DirectX::CreateBuffer(Vertices);
}

}