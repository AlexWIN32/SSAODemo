/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Shader.h>
#include <PostProcess.h>

namespace Dialogs
{

enum MenuState 
{
    MENU_STATE_OPENING,
    MENU_STATE_OPEN,
    MENU_STATE_CLOSING,
    MENU_STATE_CLOSED,
};

class IMenu
{
public:
    virtual ~IMenu(){};
    virtual MenuState GetState() const = 0;
    virtual void Draw() = 0;
    virtual void Invalidate(float Tf) = 0;
    virtual void Show() = 0;
    virtual void Hide() = 0;
};

class BlurredMenu : public IMenu
{
private:
    PostProcess::Blur blur;
    Shaders::VertexShader showBlurVs;
    Shaders::PixelShader showBlurPs;
    PostProcess::DefaultScreenQuad screenQuad;
    Texture::RenderTarget backgroundRt;
    MenuState state = MENU_STATE_CLOSED;
    float allTime = 0.0f;
    virtual void DrawObjects(){}
    virtual void InvalidateObjects(FLOAT Tf){}
    virtual void OnShow() = 0;
    virtual void OnHide() = 0;
public:
    virtual void Init() throw (Exception);
    virtual MenuState GetState() const {return state;}
    ID3D11RenderTargetView *GetRenderTargetView() const {return backgroundRt.GetRenderTargetView();}   
    virtual void Draw();
    virtual void Invalidate(float Tf);
    virtual void Show();
    virtual void Hide();
};

}
