/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include "PointLight.h"
#include <Camera.h>
#include <MathHelpers.h>

namespace Demo
{

void PointLight::Init(const Shaders::ShadersSet &Shaders, const Texture::RenderTarget &SsaoRt)
{
    shaders.vs.ConstructAsRef(Shaders.vs);
    shaders.ps.ConstructAsRef(Shaders.ps);
    ssaoRt = SsaoRt;
}

void PointLight::SetMaterial(const Material &NewMaterial)
{
    material = NewMaterial;

    shaders.ps.UpdateVariable("PointLightMaterial", material);
    shaders.ps.ApplyVariables();
}

void PointLight::PrepareForDrawing(const Camera::ICamera * Camera)
{
    shaders.ps.UpdateVariable("eyeW", Camera->GetPos());
    shaders.ps.UpdateVariable("pointLightPosW", pos);
    shaders.ps.ApplyVariables();

    shaders.ps.SetResource(1, ssaoRt.GetSahderResourceView());
}

void PointLight::BeginDraw(const Scene::IObject *Object, const Meshes::IMesh *Mesh, const Camera::ICamera * Camera)
{
    const D3DXMATRIX &worldMatrix = Object->GetWorldMatrix();

    shaders.vs.UpdateVariable("worldViewProj", worldMatrix * Camera->GetViewMatrix() * Camera->GetProjMatrix());
    shaders.vs.UpdateVariable("worldInvTrans", Math::Transpose(Math::Inverse(worldMatrix)));
    shaders.vs.UpdateVariable("world", worldMatrix);

    shaders.vs.ApplyVariables();

    shaders.vs.Apply();
    shaders.ps.Apply();
}

void PointLight::ProcessMaterial(const Scene::IObject *Object, const Meshes::MaterialData &Material)
{
    shaders.ps.UpdateVariable("ambient", Material.ambientColor);
    shaders.ps.UpdateVariable("diffuse", Material.diffuseColor);
    shaders.ps.UpdateVariable("specularWithPower", Material.specularColor);
    shaders.ps.ApplyVariables();
}

void PointLight::StopDrawing()
{
    shaders.ps.ResetResources();
}

}