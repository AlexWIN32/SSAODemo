/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <map>
#include <vector>
#include <functional>
#include <D3DHeaders.h>
#include <Exception.h>
#include <MeshesFwd.h>
#include <Shader.h>
#include <Matrix3x3.h>
#include <Basis.h>

namespace Camera
{
    class ICamera;
};   

namespace Meshes
{
    class IMesh;
};

namespace Scene
{

class IObject
{
private:
    typedef std::map<UINT, Meshes::MaterialData> MaterialsStorage;
    MaterialsStorage materials;
public:
    void SetMaterial(UINT Subest, const Meshes::MaterialData &Material);
    bool FindMaterial(UINT Subest, Meshes::MaterialData &Material) const;
    virtual ~IObject(){}
    virtual const D3DXMATRIX &GetWorldMatrix() const = 0;
};

class IMeshDrawManager
{
protected:
    IMeshDrawManager(){}
public:
    virtual ~IMeshDrawManager(){}
    virtual void PrepareForDrawing(const Camera::ICamera * Camera){}
    virtual void BeginDraw(const IObject *Object, const Meshes::IMesh *Mesh, const Camera::ICamera * Camera){}
    virtual void ProcessMaterial(const IObject *Object, const Meshes::MaterialData &Material){}
    virtual void EndDraw(const IObject *Object, const Meshes::IMesh *Mesh){}
    virtual void StopDrawing(){}
};

DECLARE_EXCEPTION(DrawingContainerException);

typedef std::vector<IObject*> ObjectsGroup;
typedef std::vector<const Meshes::IMesh*> MeshesGroup;

class DrawingContainer
{
private:
    struct DrawingManagerData
    {
        IMeshDrawManager* drawingManager = NULL;
        INT objectsCount = 0;
    };
    typedef std::map<const Meshes::IMesh*, DrawingManagerData> MeshesToDrawingManagersStorage;
    typedef std::map<const IObject*, const Meshes::IMesh*> ObjectsToMeshesStorage;
    typedef std::function<void(const IObject *Object, 
                      const Meshes::IMesh *Mesh, 
                      IMeshDrawManager *DrawManager, 
                      const Camera::ICamera *Camera)> ProcessFunction;
    MeshesToDrawingManagersStorage meshesToDrawingManagers;
    ObjectsToMeshesStorage objectsToMeshes;
    void ForEachSpecificObject(const ObjectsGroup &SpecificObjects, const Camera::ICamera * Camera, ProcessFunction Function);
    void ForEachSpecificMesh(const MeshesGroup &SpecificMeshes, const Camera::ICamera * Camera, ProcessFunction Function);
public:
    void SetDrawingManager(const Meshes::IMesh *Mesh, IMeshDrawManager *DrawingManager) throw (DrawingContainerException);
    void SetMesh(const IObject *Object, const Meshes::IMesh *Mesh) throw (DrawingContainerException);
    void AddObject(const IObject *Object, const Meshes::IMesh *Mesh) throw (DrawingContainerException);
    void RemoveObject(const IObject *Object, BOOL ClearMesh = true);
    void ClearObjects(BOOL ClearMeshes = true);
    void Draw(const Camera::ICamera * Camera, IMeshDrawManager* CommonManager = NULL);
    void Draw(const ObjectsGroup &SpecificObjects, const Camera::ICamera * Camera, IMeshDrawManager* CommonManager = NULL);
    void Draw(const MeshesGroup &SpecificMeshes, const Camera::ICamera *Camera, IMeshDrawManager *CommonManager = NULL);
};

template<class TVector, class TRotation = TVector>
class GenericObject : public IObject
{
protected:
    TVector scalling, position;
    TRotation rotation;
    D3DXMATRIX matWorld;
    virtual void CalculateMatrix() = 0;
public:
    virtual ~GenericObject(){}
    GenericObject() {D3DXMatrixIdentity(&matWorld);}
    virtual void SetScalling(const TVector &NewScalling)
    {
        if(scalling != NewScalling){
            scalling = NewScalling;
            CalculateMatrix();
        }
    }
    virtual const TVector &GetScalling() const {return scalling;}
    virtual void SetRotation(const TRotation &NewRotation)
    {
        if(rotation != NewRotation){
            rotation = NewRotation;
            CalculateMatrix();
        }
    }
    virtual const TRotation &GetRotation() const {return rotation;}
    virtual void SetPos(const TVector &NewPos)
    {
        if(position != NewPos){
            position = NewPos;
            CalculateMatrix();
        }
    }
    virtual const TVector &GetPos() const { return position; }
    virtual const D3DXMATRIX &GetWorldMatrix() const {return matWorld; };
};

class Object : public GenericObject<D3DXVECTOR3>
{
protected:
    virtual void CalculateMatrix();
public:
    virtual ~Object(){}
    Object();
};

class UVNBasisObject : public Scene::IObject, public Basis::UVNBasis
{
private:
    const D3DXMATRIX &GetMatrix() const {return UVNBasis::GetMatrix();}
public:
    virtual const D3DXMATRIX &GetWorldMatrix() const { return UVNBasis::GetMatrix();}
};

class Shape2D : public GenericObject<D3DXVECTOR2, FLOAT>
{
protected:
    virtual void CalculateMatrix();
public:
    virtual ~Shape2D(){}
    Shape2D();
};

typedef std::vector<Shape2D*> Shapes2DGroup;

class Object2D : public GenericObject<D3DXVECTOR2, FLOAT>
{
protected:
    virtual void CalculateMatrix();
private:
    D3DXVECTOR2 size = {0.0f, 0.0f};
    ID3D11ShaderResourceView *texture = NULL;
    D3DXCOLOR color = {1.0f, 1.0f, 1.0f, 1.0f};
    D3DXVECTOR4 sidesTexCoords = {0.0f, 0.0f, 1.0f, 1.0f};
public:
    virtual ~Object2D(){}
    Object2D();
    const D3DXVECTOR2 &GetSize() const {return size;}
    void SetSize(const D3DXVECTOR2 &Size) {size = Size;}
    ID3D11ShaderResourceView *GetTexture() const {return texture;}
    void SetTexture(ID3D11ShaderResourceView *Texture){texture = Texture;}
    const D3DXCOLOR &GetColor() const {return color;}
    void SetColor(const D3DXCOLOR &NewColor){color = NewColor;}
    void SetSidesTexCoords(const D3DXVECTOR4 SidesTexCoords){sidesTexCoords = SidesTexCoords;}
    const D3DXVECTOR4 &GetSidesTexCoords() const {return sidesTexCoords;}
};

typedef std::vector<Object2D*> Objects2DGroup;

class Object2DDrawingContainer1 final
{
protected:
    typedef std::vector<const Object2D*> ObjectsStorage;
    ObjectsStorage objects;
    static const UINT MaxVertsCount = 100;
    ID3D11Buffer *vertexBuffer = NULL;
    mutable Shaders::ShadersSet shaders;
public:
    void Init() throw (Exception);
    void AddObject(const Object2D *Object) { objects.push_back(Object);}
    void RemoveObject(const Object2D *Object);
    void Clear(){objects.clear();}
    void Draw() const;
};

class Object2DDrawingContainer2 final
{
protected:
    typedef std::vector<const Object2D*> ObjectsStorage;
    ObjectsStorage objects;
    static const UINT MaxVertsCount = 100;
    ID3D11Buffer *vertexBuffer = NULL;
    mutable Shaders::ShadersSet shaders;
public:
    void Init() throw (Exception);
    void AddObject(const Object2D *Object) { objects.push_back(Object);}
    void RemoveObject(const Object2D *Object);
    void Clear(){objects.clear();}
    void Draw() const;
};

typedef Object2DDrawingContainer1 Object2DDrawingContainer;

class Shapes2DDrawManager : public IMeshDrawManager
{
private:
    Shaders::ShadersSet shaders;
public:
    void Init() throw (Exception);
    virtual void BeginDraw(const IObject *Object, const Meshes::IMesh *Mesh, const Camera::ICamera * Camera);
    virtual void ProcessMaterial(const IObject *Object, const Meshes::MaterialData &Material);
};

}