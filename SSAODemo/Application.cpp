/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <RenderStatesManager.h>
#include <AdapterManager.h>
#include <CommonParams.h>
#include <DirectInput.h>
#include <InitFunctions.h>
#include <Utils/ToString.h>
#include <MathHelpers.h>
#include <algorithm>
#include "Application.h"
#include "LoadingScreen.h"

namespace Demo
{

class LoadingProcess
{
private:
    typedef std::function<void()> Procedure;
    struct Stage
    {
        Procedure procedure;
        int number = 0;
    };
    typedef std::vector<Stage> StagesStorage;
    StagesStorage stages;
    int counter = 0;
public:
    void AddStage(Procedure Proc)
    {
        Stage newStage;
        newStage.number = ++counter;
        newStage.procedure = Proc;

        stages.push_back(newStage);
    }
    void Excecute() throw (Exception)
    {
        Demo::LoadingScreen::GetInstance()->SetProgress(0.0f);
        for(const Stage &stage : stages){
            stage.procedure();
            Demo::LoadingScreen::GetInstance()->SetProgress((float)stage.number / (float)counter);
        }
    }
};

Application *Application::instance = NULL;

static void DrawPreloadingMessage(const std::wstring &Message) throw (Exception)
{
    float color[4] = {0.9,0.9,0.9,0};

    DeviceKeeper::GetDeviceContext()->ClearRenderTargetView(DeviceKeeper::GetRenderTargetView(), color);
    DeviceKeeper::GetDeviceContext()->ClearDepthStencilView(DeviceKeeper::GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    HR(DeviceKeeper::GetSwapChain()->Present(0, 0), []{return D3DException("swap chain present failed");});
        
    HDC wdc = GetWindowDC(CommonParams::GetWindow());

    SetBkMode(wdc,TRANSPARENT);
    SetTextColor(wdc, 0);

    TextOut(wdc, 8, 30, Message.c_str(), Message.size());

    DeleteDC(wdc);
}

static std::vector<D3DXVECTOR4> CreateKernel(UINT KernelSize)
{
    std::vector<D3DXVECTOR4> kernel(KernelSize);

    int i = 0;

    for(D3DXVECTOR4 &k : kernel){
        k.x = Math::RandSNorm();
        k.y = Math::RandSNorm();
        k.z = Math::RandSNorm();
        k.w = 0.0f;

        D3DXVec4Normalize(&k, &k);

        FLOAT factor = (float)i / KernelSize;

        k *= Math::Lerp(0.1f, 0.9f, factor);

        i++;
    }

    return kernel;
}

void Application::CreateRenderStates()
{
    DrawPreloadingMessage(L"Creating render states");

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FillMode = D3D11_FILL_SOLID;

    RenderStatesManager::GetInstance()->CreateRenderState("NotCull", rasterDesc);
    RenderStatesManager::GetInstance()->ApplyState("NotCull");

}

void Application::LoadGUIManager(bool ShowLogMessage) throw (Exception)
{
    if(ShowLogMessage)
        DrawPreloadingMessage(L"Loading GUI");

    GUI::Theme theme;
    theme.Load("GuiTheme1.xml");

    GUI::Manager::GetInstance()->Init();
    GUI::Manager::GetInstance()->AddExternalTheme(theme);
    GUI::Manager::GetInstance()->SetTheme("GuiTheme1");

    SetCursorVisibleState(false);
}

void Application::CreateGUIComponents()
{
    fpsLabel = new GUI::Label();
    helpLabel = new GUI::Label();

    const GUI::Font *fnt = &GUI::Manager::GetInstance()->GetTheme().GetFont();
    fpsLabel->Init(fnt);
    GUI::Manager::GetInstance()->AddControl(fpsLabel);

    helpLabel->Init(fnt);
    helpLabel->SetPos({0.0f, fnt->GetLineScreenHeight()});
    helpLabel->SetCaption(L"Press F1");
    GUI::Manager::GetInstance()->AddControl(helpLabel);

    optionsMenu = new OptionsMenu();
    optionsMenu->Init();
}

void Application::LoadResources() throw (Exception)
{
    Demo::LoadingScreen::GetInstance()->Init();

    LoadingProcess ldPrc;
    ldPrc.AddStage([this]()
    {
        eyeCamera.SetFlyingMode(true);
        eyeCamera.SetDir(Math::Normalize(D3DXVECTOR3(0.934, 0.059, -0.350)));    
        eyeCamera.SetPos(D3DXVECTOR3(24.30f, 3.694f, 2.95f));    
        eyeCamera.SetProjMatrix(Math::PerspectiveFovLH(0.25f * D3DX_PI, 0.1f, 1000.0f, CommonParams::GetWidthOverHeight()));
    });
    ldPrc.AddStage([this]()
    {
        ndRt.Init(DXGI_FORMAT_R32G32B32A32_FLOAT);
        ssaoRt.Init(DXGI_FORMAT_R32G32B32A32_FLOAT);

        kernelOffsetsSRV = Texture::CreateTexture2D(KernelOffsetsTexSize, DXGI_FORMAT_R8G8B8A8_UNORM,
        [](UCHAR *Px, const POINT &Pt)
        {
            Px[0] = rand() % 255; //b
            Px[1] = rand() % 255; //g
            Px[2] = rand() % 255; //r
            Px[3] = rand() % 255; //a
        });
    });
    ldPrc.AddStage([this]()
    {
        screenQuad.Init();
        hallMeshId = meshes.LoadMesh("../Resources/Meshes/CryTecHall/hall.bin", Meshes::MT_COLLADA_BINARY);

        Meshes::MaterialData material = meshes.GetMesh(hallMeshId)->GetSubsetMaterial(0);
        material.diffuseColor = material.ambientColor = {0.5f, 0.5f, 0.5f, 1.0f};
        material.specularColor = {0.1f, 0.1f, 0.1f, 1.0f};
        meshes.GetMesh(hallMeshId)->SetSubsetMaterial(0, material);

    });
    ldPrc.AddStage([this]()
    {
        Shaders::ShadersSet ssao;
        ssao.vs.Load(L"../Resources/Shaders/SSAOv3.vs", "ProcessVertex", screenQuad.GetVertexMetadata());
        ssao.ps.Load(L"../Resources/Shaders/SSAOv3.ps", "ProcessPixel");
    
        Shaders::ShadersSet nd;
        nd.vs.Load(L"../Resources/Shaders/NormalVDepthV.vs", "ProcessVertex", meshes.GetMesh(hallMeshId)->GetVertexMetadata());
        nd.ps.Load(L"../Resources/Shaders/NormalVDepthV.ps", "ProcessPixel");

        Shaders::ShadersSet drawBlurRes;
        drawBlurRes.vs.Load(L"../Resources/Shaders/SimplePostProcess.vs", "ProcessVertex", screenQuad.GetVertexMetadata());
        drawBlurRes.ps.Load(L"../Resources/Shaders/SimplePostProcess.ps", "ProcessPixel");
    
        drawBlurRes.ps.CreateSamplerState(0, {D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP});

        ssao.ps.CreateSamplerState(0, {D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP});
        ssao.ps.CreateSamplerState(1, {D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP});
        
        const UINT kernelSize = 16;

        ssao.ps.CreateVariable("kernel", 0, 0, CreateKernel(kernelSize));
        ssao.ps.CreateVariable("proj", 0, 1, eyeCamera.GetProjMatrix());
        ssao.ps.CreateVariable("invProj", 0, 2, Math::Inverse(eyeCamera.GetProjMatrix()));
        ssao.ps.CreateVariable<float>("occlusionRadius", 0, 3, 0.8f);
        ssao.ps.CreateVariable("rndTexFactor", 0, 4, D3DXVECTOR2(CommonParams::GetScreenWidth() / (float)KernelOffsetsTexSize.width, CommonParams::GetScreenHeight() / (float)KernelOffsetsTexSize.height));
        ssao.ps.CreateVariable<float>("harshness", 0, 5, 1.5f);
        ssao.ps.ApplyVariables();

        nd.vs.CreateVariable<D3DXMATRIX>("worldViewProj", 0, 0);
        nd.vs.CreateVariable<D3DXMATRIX>("worldInvTransView", 0, 1);
        nd.vs.CreateVariable<D3DXMATRIX>("worldView", 0, 2);

        ssaoDrawer.Init(nd, ssao, drawBlurRes, ndRt, ssaoRt, kernelOffsetsSRV);
    });
    ldPrc.AddStage([this]()
    {
        Shaders::ShadersSet pl;
        pl.vs.Load(L"../Resources/Shaders/PointLightSSAO.vs", "ProcessVertex", meshes.GetMesh(hallMeshId)->GetVertexMetadata());
        pl.ps.Load(L"../Resources/Shaders/PointLightSSAO.ps", "ProcessPixel");

        pl.vs.CreateVariable<D3DXMATRIX>("worldViewProj", 0, 0);
        pl.vs.CreateVariable<D3DXMATRIX>("worldInvTrans", 0, 1);
        pl.vs.CreateVariable<D3DXMATRIX>("world", 0, 2);
        
        pl.ps.CreateVariable<PointLight::Material>("pointLightMaterial", 0, 0);

        pl.ps.CreateVariable<D3DXVECTOR3>("eyeW", 1, 0);
        pl.ps.CreateVariable<FLOAT>("padding", 1, 1);
        pl.ps.CreateVariable<D3DXVECTOR3>("pointLightPosW", 1, 2);
        pl.ps.CreateVariable<FLOAT>("padding2", 1, 3);

        pl.ps.CreateVariable<D3DXVECTOR4>("ambient", 2, 0);
        pl.ps.CreateVariable<D3DXVECTOR4>("diffuse", 2, 1);
        pl.ps.CreateVariable<D3DXVECTOR4>("specularWithPower", 2, 2);

        pl.ps.CreateVariable<INT>("useSsao", 3, 0, true);
        pl.ps.CreateVariable<INT>("useColorTexture", 3, 1, false);
        pl.ps.CreateVariable<D3DXVECTOR2>("padding3", 3, 2);

        pl.ps.CreateSamplerState(0, {D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP});
        pl.ps.CreateSamplerState(1, {D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP});

        pointLight.Init(pl, ssaoRt);
    });
    ldPrc.AddStage([this]()
    {
        std::wstring blurVsPath = L"../Resources/Shaders/SimplePostProcess.vs";
        std::wstring blurPsPath = L"../Resources/Shaders/EdgeSavingBlur.ps";

        blur.Init(blurVsPath, blurPsPath, &screenQuad, ssaoRt);
        blur.SetKernel(PostProcess::Blur::GetGaussianKernel(5.0f));

        blur.GetPixelShader().CreateSamplerState(1, {D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP});
    });
    ldPrc.AddStage([this]()
    {
        drawingContainer.SetDrawingManager(meshes.GetMesh(hallMeshId), &ssaoDrawer);
        drawingContainer.SetDrawingManager(&screenQuad, &ssaoDrawer);
        drawingContainer.AddObject(&hallObject, meshes.GetMesh(hallMeshId));
    });
    ldPrc.AddStage([this]()
    {
        CreateGUIComponents();
        optionsMenu->SetSsaoMode(true);
    });
    ldPrc.Excecute();

    Demo::LoadingScreen::ReleaseInstance();
}

void Application::Load() throw (Exception)
{
    DisplaySettings::AdapterManager::GetInstance()->Init();

    CreateRenderStates();

    LoadGUIManager();

    LoadResources();

    timer.Init(60);
}

void Application::Invalidate(float Tf)
{
    if(optionsMenu->GetState() == Dialogs::MENU_STATE_CLOSED){
        eyeCamera.Invalidate(Tf);

        fpsLabel->SetCaption(Utils::to_wstring(timer.GetFps()));

        if(optionsMenu->GetPointLightMode()){
            D3DXVECTOR3 plPos = pointLight.GetPos();

            if(DirectInput::GetInsance()->IsKeyboardDown(DIK_UP))
                plPos.z += Tf * 0.1;

            if(DirectInput::GetInsance()->IsKeyboardDown(DIK_DOWN))
                plPos.z -= Tf * 0.1;

            if(DirectInput::GetInsance()->IsKeyboardDown(DIK_LEFT))
                plPos.x += Tf * 0.1;

            if(DirectInput::GetInsance()->IsKeyboardDown(DIK_RIGHT))
                plPos.x -= Tf * 0.1;

            if(DirectInput::GetInsance()->IsKeyboardDown(DIK_Q))
                plPos.y += Tf * 0.1;

            if(DirectInput::GetInsance()->IsKeyboardDown(DIK_E))
                plPos.y -= Tf * 0.1;
    
            pointLight.SetPos(plPos);
        }
    }

    optionsMenu->Invalidate(Tf);

    GUI::Manager::GetInstance()->Invalidate(Tf);

    if(newResolution){
        OnChangeResolution();
        newResolution = false;
    }

    if(newFullscreenState){

        DeviceKeeper::GetSwapChain()->SetFullscreenState(optionsMenu->GetFullscreenMode(), NULL);

        SizeUS res(CommonParams::GetScreenWidth(), CommonParams::GetScreenHeight());
        DisplaySettings::AdapterManager::GetInstance()->ChangeResolution(res);

        optionsMenu->SetFullscreenMode(optionsMenu->GetFullscreenMode());

        newFullscreenState = false;
    }
}

void Application::OnChangeResolution()
{
    SizeUS newRes = optionsMenu->GetResolution();
    bool isFullscreen = optionsMenu->GetFullscreenMode();
    FLOAT occlusionRadius = optionsMenu->GetOcclusionRadius();
    FLOAT harshness = optionsMenu->GetHarshness();
    bool pointLightMode = optionsMenu->GetPointLightMode();
    bool ssaoMode = optionsMenu->GetSsaoMode();

    ReleaseGUI();

    CommonParams::SetScreenSize(newRes.width, newRes.height);

    eyeCamera.SetProjMatrix(Math::PerspectiveFovLH(0.25f * D3DX_PI, 0.1f, 1000.0f, CommonParams::GetWidthOverHeight()));

    LoadGUIManager(false);

    Demo::LoadingScreen::GetInstance()->Init();

    LoadingProcess ldPrc;
    ldPrc.AddStage([&, this]()
    {
        ssaoDrawer.GetSSAOSHadersSet().ps.UpdateVariable("rndTexFactor", D3DXVECTOR2(CommonParams::GetScreenWidth() / (float)KernelOffsetsTexSize.width, CommonParams::GetScreenHeight() / (float)KernelOffsetsTexSize.height));
        ssaoDrawer.GetSSAOSHadersSet().ps.UpdateVariable("proj", eyeCamera.GetProjMatrix());
        ssaoDrawer.GetSSAOSHadersSet().ps.UpdateVariable("invProj", Math::Inverse(eyeCamera.GetProjMatrix()));
        ssaoDrawer.GetSSAOSHadersSet().ps.ApplyVariables();
    });
    ldPrc.AddStage([&, this]()
    {
        CreateGUIComponents();

        optionsMenu->SetResolution(newRes);
        optionsMenu->SetFullscreenMode(isFullscreen);
        optionsMenu->SetOcclusionRadius(occlusionRadius);
        optionsMenu->SetHarshness(harshness);
        optionsMenu->SetPointLightMode(pointLightMode);
        optionsMenu->SetSsaoMode(ssaoMode);
        
    });
    ldPrc.AddStage([&, this]()
    {
        Texture::RenderTarget newNdRt, newSsaoRt;

        newNdRt.Init(DXGI_FORMAT_R32G32B32A32_FLOAT, (USHORT)CommonParams::GetScreenWidth(), (USHORT)CommonParams::GetScreenHeight());
        newSsaoRt.Init(DXGI_FORMAT_R32G32B32A32_FLOAT, (USHORT)CommonParams::GetScreenWidth(), (USHORT)CommonParams::GetScreenHeight());

        blur.OnResolutionChanged();
        blur.SetDataRenderTarget(newSsaoRt);

        ssaoDrawer.SetNewRenderTargets(newNdRt, newSsaoRt);
        pointLight.SetNewSSAORenderTarget(newSsaoRt);
        
        ndRt = newNdRt;
        ssaoRt = newSsaoRt;
    });
    ldPrc.Excecute();

    Demo::LoadingScreen::ReleaseInstance();

    DisplaySettings::AdapterManager::GetInstance()->ChangeResolution(newRes);
}

void Application::CalculateSSAO()
{
    ssaoDrawer.SetPass(SSAODrawer::PASS_DRAW_DEPTH);

    {
        PostProcess::RenderPass pass(ndRt.GetRenderTargetView());
        drawingContainer.Draw({&hallObject}, &eyeCamera);
    }

    ssaoDrawer.SetPass(SSAODrawer::PASS_DRAW_SSAO);

    {
        PostProcess::RenderPass pass(ssaoRt.GetRenderTargetView());
        drawingContainer.Draw({&screenQuad}, &eyeCamera);
    }

    blur.GetPixelShader().SetResource(1, ndRt.GetSahderResourceView());
    blur.Draw();
    blur.GetPixelShader().SetResource(1, NULL);
}

void Application::DrawObjects()
{

    if(optionsMenu->GetPointLightMode())
        drawingContainer.Draw({&hallObject}, &eyeCamera, &pointLight);
    else if(optionsMenu->GetSsaoMode()){
        ssaoDrawer.SetPass(SSAODrawer::PASS_DRAW_BLURRED_RESULT);
        drawingContainer.Draw({&screenQuad}, &eyeCamera);
    }

    GUI::Manager::GetInstance()->Draw();
}

void Application::Draw()
{
    float color[4];
    color[0] = 0.9;//Red
    color[1] = 0.9;//Green
    color[2] = 0.9;//Blue
    color[3] = 0;//Alpha

    DeviceKeeper::GetDeviceContext()->ClearRenderTargetView(DeviceKeeper::GetRenderTargetView(), color);
    DeviceKeeper::GetDeviceContext()->ClearDepthStencilView(DeviceKeeper::GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if(optionsMenu->GetSsaoMode())
        CalculateSSAO();

    if(optionsMenu->GetState() != Dialogs::MENU_STATE_CLOSED){

        PostProcess::RenderPass pass(optionsMenu->GetRenderTargetView());

        DrawObjects();

        optionsMenu->Draw();
    }else
        DrawObjects();

    HR(DeviceKeeper::GetSwapChain()->Present(0, 0));
}

void Application::Run()
{
    timer.Invalidate();

    Invalidate(timer.GetTimeFactor());

    if(DirectInput::GetInsance()->IsKeyboardPress(DIK_ESCAPE)){
        PostQuitMessage(0);
        return;
    }

    Draw();
}

void Application::ReleaseGUI()
{
    GUI::Manager::ReleaseInstance();
    delete optionsMenu;
    delete fpsLabel;
    delete helpLabel;
}

void Application::Stop()
{
    RenderStatesManager::ReleaseInstance();
    DisplaySettings::AdapterManager::ReleaseInstance();

    ReleaseGUI();
}

bool Application::ProcessMessage(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
    if(DisplaySettings::AdapterManager::GetInstance()->OnResolutionChangeResizing(Hwnd,Msg,WParam,LParam)){
        RenderStatesManager::GetInstance()->ApplyState("NotCull");
        return true;
    }

    return false;
}

void Application::ChangeResolution()
{
    newResolution = true;
}

void Application::ChangeFullscreenMode()
{
    newFullscreenState = true;
}

void Application::SetCursorVisibleState(BOOL IsVisible)
{
    ShowCursor(IsVisible);
}

void Application::SetSsaoMode(bool Mode)
{
    pointLight.GetShaders().ps.UpdateVariable("useSsao", static_cast<INT>(Mode));
    pointLight.GetShaders().ps.ApplyVariables();
}

void Application::ChangeOcclusionRadius(FLOAT NewRadius)
{
    ssaoDrawer.GetSSAOSHadersSet().ps.UpdateVariable("occlusionRadius", NewRadius);
    ssaoDrawer.GetSSAOSHadersSet().ps.ApplyVariables();
}

void Application::ChangeHarshness(FLOAT NewHarshness)
{
    ssaoDrawer.GetSSAOSHadersSet().ps.UpdateVariable("harshness", NewHarshness);
    ssaoDrawer.GetSSAOSHadersSet().ps.ApplyVariables();
}

void Application::SetPointLightMode(bool Mode)
{
    D3DXCOLOR newColor = (Mode) ? D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f) : D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
    fpsLabel->SetColor(newColor);
    helpLabel->SetColor(newColor);
}

}