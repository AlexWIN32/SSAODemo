/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Dialogs/LoadingScreen.h>
#include <DeviceKeeper.h>

namespace Dialogs
{

void SyncLoadingScreen::SetProgress(FLOAT NewProgress, const MessagesStorage &Messages)
{
    ProcessNewProgress(NewProgress, Messages);

    float color[4];
    color[0] = 0.9;//Red
    color[1] = 0.9;//Green
    color[2] = 0.9;//Blue
    color[3] = 0;//Alpha

    DeviceKeeper::GetDeviceContext()->ClearRenderTargetView(DeviceKeeper::GetRenderTargetView(), color);
    DeviceKeeper::GetDeviceContext()->ClearDepthStencilView(DeviceKeeper::GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    Render();

    HR(DeviceKeeper::GetSwapChain()->Present(0, 0));
}

}