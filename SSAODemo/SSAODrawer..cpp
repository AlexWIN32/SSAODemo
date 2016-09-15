/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include "SSAODrawer.h"
#include <Camera.h>
#include <MathHelpers.h>

namespace Demo
{

void SSAODrawer::Init(const Shaders::ShadersSet &DrawDepth,
            const Shaders::ShadersSet &DrawSsao,
            const Shaders::ShadersSet &DrawBlurResult,
            const Texture::RenderTarget &NdRt,
            const Texture::RenderTarget &SsaoRt,
            ID3D11ShaderResourceView *KernelOffsetsSRV)
{
    drawDepth.vs.ConstructAsRef(DrawDepth.vs);
    drawDepth.ps.ConstructAsRef(DrawDepth.ps);

    drawSsao.vs.ConstructAsRef(DrawSsao.vs);
    drawSsao.ps.ConstructAsRef(DrawSsao.ps);

    drawBlurResult.vs.ConstructAsRef(DrawBlurResult.vs);
    drawBlurResult.ps.ConstructAsRef(DrawBlurResult.ps);

    ndRt = NdRt;
    ssaoRt = SsaoRt;

    kernelOffsetsSRV = KernelOffsetsSRV;
}

void SSAODrawer::BeginDraw(const Scene::IObject *Object, const Meshes::IMesh *Mesh, const Camera::ICamera * Camera)
{
    if(pass == PASS_DRAW_DEPTH){

        const D3DXMATRIX &worldMatrix = Object->GetWorldMatrix();

        drawDepth.vs.UpdateVariable("worldViewProj", worldMatrix * Camera->GetViewMatrix() * Camera->GetProjMatrix());
        drawDepth.vs.UpdateVariable("worldView", worldMatrix * Camera->GetViewMatrix());
        drawDepth.vs.UpdateVariable("worldInvTransView", Math::Transpose(Math::Inverse(worldMatrix)) * Camera->GetViewMatrix());

        drawDepth.vs.ApplyVariables();

        drawDepth.vs.Apply();
        drawDepth.ps.Apply();

    }else if(pass == PASS_DRAW_SSAO){
        drawSsao.ps.SetResource(0, ndRt.GetSahderResourceView());
        drawSsao.ps.SetResource(1, kernelOffsetsSRV);
        
        drawSsao.vs.Apply();
        drawSsao.ps.Apply();
    }else if(pass == PASS_DRAW_BLURRED_RESULT){
    
        drawBlurResult.ps.SetResource(0, ssaoRt.GetSahderResourceView());

        drawBlurResult.ps.Apply();
        drawBlurResult.vs.Apply();
    }
}

void SSAODrawer::EndDraw(const Scene::IObject *Object, const Meshes::IMesh *Mesh)
{
    if(pass == PASS_DRAW_SSAO)
        drawSsao.ps.ResetResources();
    else if(pass == PASS_DRAW_BLURRED_RESULT)
        drawBlurResult.ps.ResetResources();
}

}