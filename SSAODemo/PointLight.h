/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <SceneManagement.h>
#include <Shader.h>
#include <Texture.h>

namespace Demo
{

class PointLight : public Scene::IMeshDrawManager
{
public:
    struct Material
    {
        D3DXVECTOR4 ambient = {1.0f, 1.0f, 1.0f, 1.0f};
        D3DXVECTOR4 diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
        D3DXVECTOR4 specular = {1.0f, 1.0f, 1.0f, 1.0f};
        FLOAT intencity = 0.5f;
        FLOAT fadeDisstance = 12.0f;
        FLOAT range = 12.0f;
        FLOAT padding = 0.0f;
    };
private:
    Material material;
    D3DXVECTOR3 pos;
    Shaders::ShadersSet shaders;
    Texture::RenderTarget ssaoRt;
public:
    void Init(const Shaders::ShadersSet &Shaders, const Texture::RenderTarget &SsaoRt);
    void SetPos(const D3DXVECTOR3 &NewPos){pos = NewPos;}
    const D3DXVECTOR3 &GetPos() const {return pos;}
    void SetMaterial(const Material &NewMaterial);
    const Material &GetMaterial() const {return material;}
    Shaders::ShadersSet &GetShaders() {return shaders;}
    virtual void PrepareForDrawing(const Camera::ICamera * Camera);
    virtual void BeginDraw(const Scene::IObject *Object, const Meshes::IMesh *Mesh, const Camera::ICamera * Camera);
    virtual void ProcessMaterial(const Scene::IObject *Object, const Meshes::MaterialData &Material);
    virtual void StopDrawing();
    void SetNewSSAORenderTarget(const Texture::RenderTarget &NewSsoaRt){ssaoRt = NewSsoaRt;}
};

}