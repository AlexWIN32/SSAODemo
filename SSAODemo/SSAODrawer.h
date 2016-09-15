/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <SceneManagement.h>
#include <Shader.h>
#include <PostProcess.h>
#include <Texture.h>

namespace Demo
{

class SSAODrawer : public Scene::IMeshDrawManager
{
public:
    enum Pass
    {
        PASS_DRAW_DEPTH,
        PASS_DRAW_SSAO,
        PASS_DRAW_BLURRED_RESULT
    };
private:
    Pass pass = PASS_DRAW_DEPTH;
    Shaders::ShadersSet drawDepth;
    Shaders::ShadersSet drawSsao;
    Shaders::ShadersSet drawBlurResult;
    Texture::RenderTarget ndRt, ssaoRt;
    ID3D11ShaderResourceView *kernelOffsetsSRV = NULL;
public:
    void Init(const Shaders::ShadersSet &DrawDepth, 
              const Shaders::ShadersSet &DrawSsao, 
              const Shaders::ShadersSet &DrawBlurResult,
              const Texture::RenderTarget &NdRt,
              const Texture::RenderTarget &SsaoRt,
              ID3D11ShaderResourceView *KernelOffsetsSRV);

    virtual void BeginDraw(const Scene::IObject *Object, const Meshes::IMesh *Mesh, const Camera::ICamera * Camera);
    virtual void EndDraw(const Scene::IObject *Object, const Meshes::IMesh *Mesh);
    void SetPass(Pass NewPass) {pass = NewPass;}
    Shaders::ShadersSet &GetSSAOSHadersSet(){return drawSsao;}
    void SetNewRenderTargets(const Texture::RenderTarget &NdRt, const Texture::RenderTarget &SsaoRt)
    {
        ndRt = NdRt;
        ssaoRt = SsaoRt;
    }
};

}