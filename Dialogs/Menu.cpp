/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Dialogs/Menu.h>
#include <DirectInput.h>

namespace Dialogs
{

void BlurredMenu::Init() throw (Exception)
{
    screenQuad.Init();
    backgroundRt.Init(DXGI_FORMAT_R32G32B32A32_FLOAT);

    std::wstring vertexShaderPath = L"../Resources/Shaders/SimplePostProcess.vs";
    std::wstring pixelShaderPath = L"../Resources/Shaders/Blur.ps";
    blur.Init(vertexShaderPath, pixelShaderPath, &screenQuad, backgroundRt);

    showBlurVs.Load(L"../Resources/Shaders/SimplePostProcess.vs", "ProcessVertex", screenQuad.GetVertexMetadata());
    showBlurPs.Load(L"../Resources/Shaders/SimplePostProcess.ps", "ProcessPixel");
    
    showBlurPs.CreateSamplerState(0, Utils::DirectX::SamplerStateDescription(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP));
}

void BlurredMenu::Show()
{
    if(state != MENU_STATE_OPEN)
        state = MENU_STATE_OPENING;
}

void BlurredMenu::Hide()
{
    if(state != MENU_STATE_CLOSED)
        state = MENU_STATE_CLOSING;
}

void BlurredMenu::Draw()
{
    blur.Draw();

    showBlurPs.SetResource(0, backgroundRt.GetSahderResourceView());

    showBlurPs.Apply();
    showBlurVs.Apply();

    screenQuad.Draw();

    showBlurPs.ResetResources();

    if(state == MENU_STATE_OPEN)
        DrawObjects();
}

void BlurredMenu::Invalidate(float Tf)
{
    InvalidateObjects(Tf);

    if(DirectInput::GetInsance()->IsKeyboardPress(DIK_F1)){
        if(state == MENU_STATE_OPENING || state == MENU_STATE_OPEN)
            state = MENU_STATE_CLOSING;
        else
            state = MENU_STATE_OPENING;
    }

    if(state == MENU_STATE_OPENING || state == MENU_STATE_CLOSING){

        static float timeout = 7.8f;

        allTime += Tf;

        float deviation = (state == MENU_STATE_OPENING) ? allTime : timeout - allTime;
        blur.SetKernel(PostProcess::Blur::GetGaussianKernel(5, deviation));

        if(allTime >= timeout){
            state = (state == MENU_STATE_OPENING) ? MENU_STATE_OPEN : MENU_STATE_CLOSED;
            allTime = 0.0f;

            ((state == MENU_STATE_OPEN) ? OnShow() : OnHide());

        }
    }
}

}
