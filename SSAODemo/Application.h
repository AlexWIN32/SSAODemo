/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <windows.h>
#include <Exception.h>
#include <Timer.h>
#include <Vector2.h>
#include <Meshes.h>
#include <Camera.h>
#include <SceneManagement.h>
#include <PostProcess.h>
#include "OptionsMenu.h"
#include "SSAODrawer.h"
#include "PointLight.h"

namespace Demo
{

class Application
{
private:
    Camera::EyeCamera eyeCamera;
    Scene::DrawingContainer drawingContainer;
    Meshes::MeshId hallMeshId = 0;
    Meshes::MeshesContainer meshes;
    Scene::Object hallObject;
    Texture::RenderTarget ndRt, ssaoRt;
    PostProcess::DefaultScreenQuad screenQuad; 
    PostProcess::Blur blur;
    GUI::Label *fpsLabel = NULL, *helpLabel = NULL;
    ID3D11ShaderResourceView *kernelOffsetsSRV;
    Time::Timer timer;
    SSAODrawer ssaoDrawer;
    PointLight pointLight; 
    OptionsMenu *optionsMenu = NULL;
    BOOL newFullscreenState = false;
    BOOL newResolution = false;
    SizeUS KernelOffsetsTexSize = {4, 4};
    static Application *instance;
    Application(){}
    ~Application(){Stop();}
    void CreateRenderStates();
    void LoadGUIManager(bool showLogMessage = true) throw (Exception);
    void CreateGUIComponents();
    void LoadResources() throw (Exception);
    void ReleaseGUI();
    void Invalidate(FLOAT Tf);
    void Draw();
    void CalculateSSAO();
    void DrawObjects();
    void OnChangeResolution();
public:
    static Application *GetInstance()
    {
        if(!instance)
            instance = new Application();
        return instance;
    }
    static void ReleaseInstance()
    {
        delete instance;
        instance = NULL;
    }   
    Application(Application &) = delete;
    Application & operator=(Application &) = delete;
    void Load() throw (Exception);
    void Run();
    void Stop();
    bool ProcessMessage(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam);
    void ChangeResolution();
    void ChangeFullscreenMode();
    void SetCursorVisibleState(BOOL IsVisible);
    void ChangeOcclusionRadius(FLOAT NewRadius);
    void ChangeHarshness(FLOAT NewHarshness);
    void SetSsaoMode(bool Mode);
    void SetPointLightMode(bool Mode);
};

};