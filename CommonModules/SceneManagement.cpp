/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <SceneManagement.h>
#include <CommonParams.h>
#include <Utils.h>
#include <Camera.h>
#include <Matrix3x3.h>
#include <MathHelpers.h>
#include <Vector2.h>
#include <sstream>
#include <functional>
#include <RenderStatesManager.h>
#include <Utils/ToString.h>
#include <Meshes.h>

namespace Scene
{

void IObject::SetMaterial(UINT Subset, const Meshes::MaterialData &Material)
{
    materials[Subset] = Material;
}

bool IObject::FindMaterial(UINT Subset, Meshes::MaterialData &Material) const
{
    auto it = materials.find(Subset);
    if(it == materials.end())
        return false;

    Material = it->second;
    return true;
}

void DrawingContainer::SetMesh(const IObject *Object, const Meshes::IMesh *Mesh) throw (DrawingContainerException)
{
    if(objectsToMeshes.find(Object) == objectsToMeshes.end())
        throw DrawingContainerException("object not found");

    RemoveObject(Object);
    AddObject(Object, Mesh);
}

void DrawingContainer::AddObject(const IObject *Object, const Meshes::IMesh *Mesh) throw (DrawingContainerException)
{
	if(Mesh == NULL)
		throw DrawingContainerException("Invalid mesh");

    auto it = meshesToDrawingManagers.find(Mesh);
    if(it == meshesToDrawingManagers.end())
        throw DrawingContainerException("Drawing manager not found");

    objectsToMeshes[Object] = Mesh;

    it->second.objectsCount++;
}

void DrawingContainer::SetDrawingManager(const Meshes::IMesh *Mesh, IMeshDrawManager *DrawingManager) throw (DrawingContainerException)
{
	if(DrawingManager == NULL)
		throw DrawingContainerException("Invalid drawing manager");

    meshesToDrawingManagers[Mesh].drawingManager = DrawingManager;
}

void DrawingContainer::RemoveObject(const IObject *Object, BOOL ClearMesh)
{
	auto it = objectsToMeshes.find(Object);

    if(it == objectsToMeshes.end())
        return;

    auto mIt = meshesToDrawingManagers.find(it->second);

    if(mIt != meshesToDrawingManagers.end()){
            
        mIt->second.objectsCount--;

        if(mIt->second.objectsCount == 0 && ClearMesh)
            meshesToDrawingManagers.erase(mIt);
    }

    objectsToMeshes.erase(it);

}

void DrawingContainer::ClearObjects(BOOL ClearMeshes)
{
    objectsToMeshes.clear();

    if(ClearMeshes)
        meshesToDrawingManagers.clear();
    else
        for(auto &pair : meshesToDrawingManagers)
            pair.second.objectsCount = 0;
}

static void DrawObject(const IObject *Object, const Meshes::IMesh *Mesh, IMeshDrawManager *DrawManager, const Camera::ICamera *Camera)
{
    DrawManager->BeginDraw(Object, Mesh, Camera);

    for(INT s = 0; s < Mesh->GetSubsetCount(); s++){

        Meshes::MaterialData material;
        if(Object && Object->FindMaterial(s, material))
            DrawManager->ProcessMaterial(Object, material);
        else
            DrawManager->ProcessMaterial(Object, Mesh->GetSubsetMaterial(s));

        Mesh->Draw(s);
    }

    DrawManager->EndDraw(Object, Mesh);
}

void DrawingContainer::Draw(const Camera::ICamera * Camera, IMeshDrawManager* CommonManager)
{
	if(CommonManager){
		CommonManager->PrepareForDrawing(Camera);

        for(auto pair : objectsToMeshes)
            DrawObject(pair.first, pair.second, CommonManager, Camera);

		CommonManager->StopDrawing();
    }else{

        for(auto pair : meshesToDrawingManagers)
            pair.second.drawingManager->PrepareForDrawing(Camera);

        for(auto pair : objectsToMeshes){

            const Meshes::IMesh *mesh = pair.second;
            IMeshDrawManager *drawManager = meshesToDrawingManagers[mesh].drawingManager;

            DrawObject(pair.first, pair.second, drawManager, Camera);
        }

        for(auto pair : meshesToDrawingManagers)
            pair.second.drawingManager->StopDrawing();
    }
}

void DrawingContainer::ForEachSpecificObject(const ObjectsGroup &SpecificObjects, const Camera::ICamera * Camera, ProcessFunction Function)
{
    for(IObject *obj : SpecificObjects){
        auto oIt = objectsToMeshes.find(obj);
        if(oIt != objectsToMeshes.end()){

            const Meshes::IMesh *mesh = oIt->second;

            auto dIt = meshesToDrawingManagers.find(mesh);

            if(dIt != meshesToDrawingManagers.end())
                Function(obj, mesh, dIt->second.drawingManager, Camera);
        }
    }
}

void DrawingContainer::ForEachSpecificMesh(const MeshesGroup &SpecificMeshes, const Camera::ICamera * Camera, ProcessFunction Function)
{
    for(const Meshes::IMesh *mesh : SpecificMeshes){
        auto mIt = meshesToDrawingManagers.find(mesh);
        if(mIt != meshesToDrawingManagers.end())
            Function(NULL, mesh, mIt->second.drawingManager, Camera);
    }
}

void DrawingContainer::Draw(const ObjectsGroup &SpecificObjects, const Camera::ICamera * Camera, IMeshDrawManager* CommonManager)
{
    if(CommonManager){
        CommonManager->PrepareForDrawing(Camera);

        ForEachSpecificObject(SpecificObjects, Camera, 
        [&](const IObject *Object, const Meshes::IMesh *Mesh, IMeshDrawManager *DrawManager, const Camera::ICamera *Camera)
        {
            DrawObject(Object, Mesh, CommonManager, Camera);
        });

        CommonManager->StopDrawing();
    }else{
        std::vector<IMeshDrawManager*> drawingManagers;

        ForEachSpecificObject(SpecificObjects, Camera, 
        [&](const IObject *Object, const Meshes::IMesh *Mesh, IMeshDrawManager *DrawManager, const Camera::ICamera *Camera)
        {
            drawingManagers.push_back(DrawManager);
        });

        for(IMeshDrawManager *manager : drawingManagers)
            manager->PrepareForDrawing(Camera);

        ForEachSpecificObject(SpecificObjects, Camera, DrawObject);

        for(IMeshDrawManager *manager : drawingManagers)
            manager->StopDrawing();
    }
}

void DrawingContainer::Draw(const MeshesGroup &SpecificMeshes, const Camera::ICamera *Camera, IMeshDrawManager *CommonManager)
{
    if(CommonManager){
        CommonManager->PrepareForDrawing(Camera);

        ForEachSpecificMesh(SpecificMeshes, Camera, 
        [&](const IObject *Object, const Meshes::IMesh *Mesh, IMeshDrawManager *DrawManager, const Camera::ICamera *Camera)
        {
            DrawObject(Object, Mesh, CommonManager, Camera);
        });

        CommonManager->StopDrawing();
    }else{
        std::vector<IMeshDrawManager*> drawingManagers;

        ForEachSpecificMesh(SpecificMeshes, Camera, 
        [&](const IObject *Object, const Meshes::IMesh *Mesh, IMeshDrawManager *DrawManager, const Camera::ICamera *Camera)
        {
            drawingManagers.push_back(DrawManager);
        });

        ForEachSpecificMesh(SpecificMeshes, Camera, DrawObject);

        for(IMeshDrawManager *manager : drawingManagers)
            manager->StopDrawing();
    }
}

Object::Object() : GenericObject<D3DXVECTOR3>()
{
    scalling = {1.0f, 1.0f, 1.0f};
    rotation = {0.0f, 0.0f, 0.0f};
    position = {0.0f, 0.0f, 0.0f};
}

void Object::CalculateMatrix()
{
    D3DXMATRIX mTrans, mRot, mScl;

    D3DXMatrixTranslation(&mTrans, position.x, position.y, position.z);
    D3DXMatrixRotationYawPitchRoll(&mRot, rotation.y, rotation.x, rotation.z);
    D3DXMatrixScaling(&mScl, scalling.x, scalling.y, scalling.z);

    matWorld = mScl * mRot * mTrans;
}

static D3DXMATRIX Get2DTransformMatrix(const D3DXVECTOR2 &Scalling, const D3DXVECTOR2 &Position, FLOAT Rotation)
{
    Math::Matrix3x3 s = Math::Matrix3x3::Scalling(Cast<SizeF>(Scalling));
    Math::Matrix3x3 r = Math::Matrix3x3::Rotation(Rotation);
    Math::Matrix3x3 t = Math::Matrix3x3::Translation(Cast<Point2F>(Position));

    Math::Matrix3x3 v;
    v(0, 0) = CommonParams::GetHeightOverWidth();

    Math::Matrix3x3 p;
    p(0, 0) = 2.0f;
    p(2, 0) = -1.0f;
    p(1, 1) = -2.0f;
    p(2, 1) = 1.0f;

    Math::Matrix3x3 m = s * r * v * t * p;

    D3DXMATRIX matWorld;

    matWorld(0, 0) = m(0,0);
    matWorld(1, 0) = m(1,0);
    matWorld(2, 0) = m(2,0);

    matWorld(0, 1) = m(0,1);
    matWorld(1, 1) = m(1,1);
    matWorld(2, 1) = m(2,1);

    matWorld(0, 2) = m(0,2);
    matWorld(1, 2) = m(1,2);
    matWorld(2, 2) = m(2,2);

    return matWorld;
}

Object2D::Object2D() : GenericObject<D3DXVECTOR2, FLOAT>()
{
    scalling = {1.0f, 1.0f};
    rotation = 0.0f;
    position = {0.0f, 0.0f};
}

void Object2D::CalculateMatrix()
{
    matWorld = Get2DTransformMatrix(scalling, position, rotation);
}

Shape2D::Shape2D() : GenericObject<D3DXVECTOR2, FLOAT>()
{
    scalling = {1.0f, 1.0f};
    rotation = 0.0f;
    position = {0.0f, 0.0f};
}

void Shape2D::CalculateMatrix()
{
    matWorld = Get2DTransformMatrix(scalling, position, rotation);
}

template<class TObjectData>
static void Draw2DObjectsGroup(const TObjectData *FirstObject,
                               UINT ObjectsCnt,
                               ID3D11Buffer *vertexBuffer)
{
    D3D11_MAPPED_SUBRESOURCE rawData;
    HR(DeviceKeeper::GetDeviceContext()->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &rawData));

    TObjectData *objPtr = reinterpret_cast<TObjectData*>(rawData.pData);
    memcpy(objPtr, FirstObject, sizeof(TObjectData) * ObjectsCnt);

    DeviceKeeper::GetDeviceContext()->Unmap(vertexBuffer, 0);

    UINT offset = 0, stride = sizeof(TObjectData);

    DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    DeviceKeeper::GetDeviceContext()->Draw(ObjectsCnt, 0);
}

template<class TObjectData>
static void Draw2DObjects(const std::map<ID3D11ShaderResourceView*, std::vector<TObjectData>> &Objects,
                          Shaders::ShadersSet &Shaders,
                          UINT MaxVertsCount,
                          ID3D11Buffer *VertexBuffer)
{
    for(auto pair : Objects){
        Shaders.ps.SetResource("colorTex", pair.first);

        Shaders.ps.UpdateVariable<INT>("useColorTex", pair.first != NULL);
        Shaders.ps.ApplyVariables();
        Shaders.ps.Apply();

        const std::vector<TObjectData> &objects = pair.second;
        INT grpsCnt = objects.size() / MaxVertsCount;
            
        for(size_t i = 0; i < grpsCnt; i++)
            Draw2DObjectsGroup(&objects[0] + MaxVertsCount * i, MaxVertsCount, VertexBuffer);

        INT leftover = objects.size() % MaxVertsCount;
        if(leftover > 0)
            Draw2DObjectsGroup(&objects[0] + MaxVertsCount * grpsCnt, leftover, VertexBuffer);
    }
}

struct ObjectData
{
    D3DXVECTOR2 halfSize = {0.0f, 0.0f};
    D3DXCOLOR color = {1.0f, 1.0f, 1.0f, 1.0f};
    D3DXVECTOR4 sidesTexCoords = {0.0f, 0.0f, 1.0f, 1.0f};
    D3DXVECTOR3 transformRow1 = {1.0f, 0.0f, 0.0f};
    D3DXVECTOR3 transformRow2 = {0.0f, 1.0f, 0.0f};
    D3DXVECTOR3 transformRow3 = {1.0f, 0.0f, 1.0f};
};

void Object2DDrawingContainer1::RemoveObject(const Object2D *Object)
{
    auto it = std::find(objects.begin(), objects.end(), Object);
    if(it != objects.end())
        objects.erase(it);
}

void Object2DDrawingContainer1::Init()
{
    Shaders::VertexMetadata metadata =
    {
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8,  D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 2, DXGI_FORMAT_R32G32B32_FLOAT,    0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 3, DXGI_FORMAT_R32G32B32_FLOAT,    0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 4, DXGI_FORMAT_R32G32B32_FLOAT,    0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    shaders.vs.Load(L"../Resources/Shaders/2dObjects.vs", "ProcessVertex", metadata);
    shaders.ps.Load(L"../Resources/Shaders/2dObjects.ps", "ProcessPixel");
    shaders.gs.Load(L"../Resources/Shaders/2dObjects.gs", "ProcessVertex");

    shaders.ps.CreateSamplerState("colorTex", 0, {D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP});
    shaders.ps.CreateVariable<INT>("useColorTex", 0, 0, true);
    shaders.ps.CreateVariable<D3DXVECTOR3>("padding", 0, 1);

    vertexBuffer = Utils::DirectX::CreateBuffer(sizeof(ObjectData) * MaxVertsCount, 
                                D3D11_BIND_VERTEX_BUFFER,
                                D3D11_USAGE_DYNAMIC,
                                D3D11_CPU_ACCESS_WRITE);

    Utils::DirectX::BlendParameters color = {D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD};
    Utils::DirectX::BlendParameters alpha = {D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD};
    D3D11_BLEND_DESC bDesc = Utils::DirectX::CreateBlendDescription({{color, alpha, true, D3D11_COLOR_WRITE_ENABLE_ALL}});

    FLOAT blendFactor[4] = {};
    RenderStatesManager::GetInstance()->CreateRenderState("AF_" + Utils::to_string(this), RenderStatesManager::BlendStateDescription(bDesc, blendFactor, 0xffffffff));

    RenderStatesManager::DepthStencilDescription depthStencilDesc;
    depthStencilDesc.stencilDescription.DepthEnable = false;
    RenderStatesManager::GetInstance()->CreateRenderState("ND_" + Utils::to_string(this), depthStencilDesc);
}

void Object2DDrawingContainer1::Draw() const
{
    std::map<ID3D11ShaderResourceView*, std::vector<ObjectData>> objectsData;

    for(const Object2D *obj : objects){
        ObjectData objData;

        const D3DXMATRIX m = obj->GetWorldMatrix();

        objData.color = obj->GetColor();
        objData.halfSize = obj->GetSize() * 0.5f;

        objData.sidesTexCoords = obj->GetSidesTexCoords();

        objData.transformRow1 = {m(0,0), m(0,1), m(0,2)};
        objData.transformRow2 = {m(1,0), m(1,1), m(1,2)};
        objData.transformRow3 = {m(2,0), m(2,1), m(2,2)};

        objectsData[obj->GetTexture()].push_back(objData);
    }

    shaders.vs.Apply();
    shaders.ps.Apply();
    shaders.gs.Apply();

    RenderStatesManager::GetInstance()->ProcessWithStates({"AF_" + Utils::to_string(this), "ND_" + Utils::to_string(this)},
    [&]()
    {
        Draw2DObjects(objectsData, shaders, MaxVertsCount, vertexBuffer);
    });

    shaders.gs.CleanUp();
}

struct ObjectData2
{
    D3DXVECTOR4 pt1Pt2 = {0.0f, 0.0f, 0.0f, 0.0f};
    D3DXVECTOR4 pt3Pt4 = {0.0f, 0.0f, 0.0f, 0.0f};
    D3DXVECTOR4 sidesTexCoords = {0.0f, 0.0f, 1.0f, 1.0f};
    D3DXCOLOR color = {1.0f, 1.0f, 1.0f, 1.0f};
};

void Object2DDrawingContainer2::Init() throw (Exception)
{
    Shaders::VertexMetadata metadata =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32,  D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    shaders.vs.Load(L"../Resources/Shaders/2dObjects2.vs", "ProcessVertex", metadata);
    shaders.ps.Load(L"../Resources/Shaders/2dObjects2.ps", "ProcessPixel");
    shaders.gs.Load(L"../Resources/Shaders/2dObjects2.gs", "ProcessVertex");

    shaders.ps.CreateSamplerState("colorTex", 0, {D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP});
    shaders.ps.CreateVariable<INT>("useColorTex", 0, 0, true);
    shaders.ps.CreateVariable<D3DXVECTOR3>("padding", 0, 1);

    vertexBuffer = Utils::DirectX::CreateBuffer(sizeof(ObjectData) * MaxVertsCount, 
                                D3D11_BIND_VERTEX_BUFFER,
                                D3D11_USAGE_DYNAMIC,
                                D3D11_CPU_ACCESS_WRITE);
}

void Object2DDrawingContainer2::RemoveObject(const Object2D *Object)
{
    auto it = std::find(objects.begin(), objects.end(), Object);
    if(it != objects.end())
        objects.erase(it);
}

void Object2DDrawingContainer2::Draw() const
{
    std::map<ID3D11ShaderResourceView*, std::vector<ObjectData2>> objectsData;

    for(const Object2D *obj : objects){

        D3DXVECTOR2 hSize = obj->GetSize() * 0.5f;

        D3DXVECTOR2 ptsL[4] = {
            {-hSize.x, -hSize.y},
            { hSize.x, -hSize.y},
            { hSize.x,  hSize.y},
            {-hSize.x,  hSize.y},
        };

        D3DXVECTOR2 ptsT[4] = {};

        const D3DXVECTOR2 &pos = obj->GetPos();
        const D3DXVECTOR2 &scl = obj->GetScalling();
        FLOAT ang = obj->GetRotation();

        for(int p = 0; p < 4; p++){
            ptsT[p].x = ptsL[p].x * cosf(ang) * scl.x + ptsL[p].y * (-sinf(ang));
            ptsT[p].y = ptsL[p].x * sinf(ang) + ptsL[p].y * cosf(ang) * scl.y;

            ptsT[p].x *= CommonParams::GetHeightOverWidth();

            ptsT[p] += pos;

            ptsT[p].x =  2.0f * ptsT[p].x - 1.0f;
            ptsT[p].y = -2.0f * ptsT[p].y + 1.0f;
        }
        /*
        Math::Matrix3x3 r = Math::Matrix3x3::Rotation(obj->GetRotation());
        Math::Matrix3x3 s = Math::Matrix3x3::Scalling(obj->GetScalling());
        Math::Matrix3x3 t = Math::Matrix3x3::Translation(pos);
        
        Math::Matrix3x3 v;
        v(0, 0) = CommonParams::GetHeightOverWidth();

        Math::Matrix3x3 p;
        p(0,0) = 2.0f;
        p(2,0) = -1.0f;
        p(1,1) = -2.0f;
        p(2,1) = 1.0f;

        Math::Matrix3x3 o = s * r * v * t * p;

        for(int p = 0; p < 4; p++)
            ptsT[p] = Cast<D3DXVECTOR2>(o.Transform({ptsL[p].x, ptsL[p].y, 1.0f}));
        */

        ObjectData2 objData;
        objData.pt1Pt2 = {ptsT[0].x, ptsT[0].y, ptsT[1].x, ptsT[1].y};
        objData.pt3Pt4 = {ptsT[2].x, ptsT[2].y, ptsT[3].x, ptsT[3].y};
        objData.color = obj->GetColor();
        objData.sidesTexCoords = obj->GetSidesTexCoords();

        objectsData[obj->GetTexture()].push_back(objData);
    }

    shaders.vs.Apply();
    shaders.ps.Apply();
    shaders.gs.Apply();

    Draw2DObjects(objectsData, shaders, MaxVertsCount, vertexBuffer);

    shaders.gs.CleanUp();
}

void Shapes2DDrawManager::Init() throw (Exception)
{
    Shaders::VertexMetadata metadata =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    shaders.vs.Load(L"../Resources/Shaders/2dShapes.vs", "ProcessVertex", metadata);
    shaders.ps.Load(L"../Resources/Shaders/2dShapes.ps", "ProcessPixel");

    shaders.vs.CreateVariable<D3DXMATRIX>("transform", 0, 0);

    shaders.ps.CreateSamplerState("colorTex", 0, {D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP});
    shaders.ps.CreateVariable<INT>("useColorTex", 0, 0, true);
    shaders.ps.CreateVariable<D3DXVECTOR3>("padding", 0, 1);
    shaders.ps.CreateVariable<D3DXVECTOR4>("materialColor", 1, 0, {1.0f, 1.0f, 1.0f, 1.0f});
}

void Shapes2DDrawManager::BeginDraw(const IObject *Object, const Meshes::IMesh *Mesh, const Camera::ICamera * Camera)
{
    shaders.vs.UpdateVariable("transform", Object->GetWorldMatrix());

    shaders.vs.ApplyVariables();
    shaders.vs.Apply();
}

void Shapes2DDrawManager::ProcessMaterial(const IObject *Object, const Meshes::MaterialData &Material)
{
    shaders.ps.UpdateVariable<INT>("useColorTex", Material.colorSRV != NULL);
    shaders.ps.UpdateVariable("materialColor", Material.ambientColor);

    shaders.ps.ApplyVariables();
    shaders.ps.Apply();
}

}