/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/Manager.h>
#include <RenderStatesManager.h>
#include <CommonParams.h>
#include <DirectInput.h>
#include <Meshes.h>
#include <Texture.h>
#include <Utils.h>

namespace GUI
{

Manager *Manager::instance = NULL;

void Manager::Init()
{
    D3D11_INPUT_ELEMENT_DESC desc[4] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},        
        {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    Meshes::VertexMetadata vertexMetadata(desc, desc + 4);

    vs.Load(L"../Resources/Shaders/gui.vs", "ProcessVertex", vertexMetadata);
    ps.Load(L"../Resources/Shaders/gui.ps", "ProcessPixel");
    gs.Load(L"../Resources/Shaders/gui.gs", "ProcessVertex");

    ps.CreateSamplerState(0, {D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP}, "DefaultSamplerState");

    dataVb = Utils::DirectX::CreateBuffer(sizeof(TransformedRect) * MaxControlsCount, 
                                          D3D11_BIND_VERTEX_BUFFER, 
                                          D3D11_USAGE_DYNAMIC, 
                                          D3D11_CPU_ACCESS_WRITE);

    RenderStatesManager::DepthStencilDescription depthStencilDesc;
    depthStencilDesc.stencilDescription.DepthEnable = false;
    RenderStatesManager::GetInstance()->CreateRenderState("DisableDepthForGUI", depthStencilDesc);

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    FLOAT blendFactor[4] = {};
    RenderStatesManager::GetInstance()->CreateRenderState("StandardAlfaBlendForGUI", RenderStatesManager::BlendStateDescription(blendDesc, blendFactor, 0xffffffff));
}

ID3D11ShaderResourceView *Manager::GetTexture(const std::string &FileName) throw (Exception)
{
    std::hash<std::string> hash;
    StringHash texHash = hash(FileName);

    TexturesStorage::const_iterator ci = textures.find(texHash);
    if(ci != textures.end())
        return ci->second;

    ID3D11ShaderResourceView * newTex = Texture::LoadTexture2DFromFile("../Resources/Textures/" + FileName);
    textures.insert(std::make_pair(texHash, newTex));
    return newTex;
}

void Manager::AddExternalTheme(const Theme &NewTheme) throw (Exception)
{
    std::hash<std::string> hash;
    StringHash strHash = hash(NewTheme.GetName());
    
    if(themes.find(strHash) != themes.end())
        throw ThemeException("theme " + NewTheme.GetName() + " already exsists");

    themes.insert(std::make_pair(strHash, NewTheme));
}

const Theme &Manager::GetTheme(const std::string &ThemeName) throw (Exception)
{
    if(ThemeName == ""){
        if(currentTheme == 0)
            throw ThemeException("theme not set");
        return themes[currentTheme];
    }

    std::hash<std::string> hash;
    StringHash strHash = hash(ThemeName);

    ThemesStorage::const_iterator ci = themes.find(strHash);
    if(ci == themes.end())
        throw ThemeException("theme " + ThemeName + " not found");
    
    return ci->second;
}

void Manager::SetTheme(const std::string &NewThemeName) throw (Exception)
{
    std::hash<std::string> hash;
    StringHash strHash = hash(NewThemeName);

    ThemesStorage::const_iterator ci = themes.find(strHash);
    if(ci == themes.end())
        throw ThemeException("theme " + NewThemeName + " not found");

    for(auto control : controls)
        control->OnThemeChange(ci->second);
    
    currentTheme = strHash;
}

void Manager::AddControl(Control *NewControl)
{
    controls.push_back(NewControl);
}

void Manager::RemoveControl(Control *ControlToRemove)
{    
    Utils::RemoveFromVector(controls, ControlToRemove, [](Control *Cntrl) {/* delete Cntrl;*/});
}

struct RectsGroupData
{
    INT count = 0, offset = 0;
    ID3D11ShaderResourceView *texture = NULL;
    ID3D11SamplerState *samplerState = NULL;
};

void Manager::Draw(const ControlsStorage &Controls) throw (Exception)
{

    SortedDrawingObjectsStorage drawingObjects;

    for(Control *cntrl : Controls)
        CollectDrawingObjects(cntrl, drawingObjects, DP_LOW);

    if(!drawingObjects.size())
        return;

    D3D11_MAPPED_SUBRESOURCE rawData;
    HR(DeviceKeeper::GetDeviceContext()->Map(dataVb, 0, D3D11_MAP_WRITE_DISCARD, 0, &rawData));

    TransformedRect *rects = reinterpret_cast<TransformedRect*>(rawData.pData);

    int rInd = 0;

    typedef std::vector<RectsGroupData> RectsGroupsStorage;
    RectsGroupsStorage rectsGroups;

    RectsGroupData lastGroup;
    
    for(const auto &pair : drawingObjects){
        for(const DrawingObject *object : pair.second){

            if(rInd >= MaxControlsCount)
                break;

            if(object->GetTexture() != lastGroup.texture || 
               object->GetSamplerState() != lastGroup.samplerState){

                if(lastGroup.count != 0)
                    rectsGroups.push_back(lastGroup);

                lastGroup.count = 1;
                lastGroup.offset = rInd;
                lastGroup.texture = object->GetTexture();
                lastGroup.samplerState = object->GetSamplerState();

            }else
                lastGroup.count++;

            object->Transform(rects[rInd++]);
        }
    }    

    rectsGroups.push_back(lastGroup);

    DeviceKeeper::GetDeviceContext()->Unmap(dataVb, 0);

    vs.Apply();
    ps.Apply();
    gs.Apply();

    UINT offset = 0;
    UINT stride = sizeof(TransformedRect);

    DeviceKeeper::GetDeviceContext()->IASetVertexBuffers(0, 1, &dataVb, &stride, &offset);
    DeviceKeeper::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    
    RenderStatesManager::GetInstance()->ProcessWithStates({"DisableDepthForGUI", "StandardAlfaBlendForGUI"}, [&]()
    {
        bool defSmpStateSet = false;

        for(RectsGroupData &grp : rectsGroups){
            ps.SetResource(0, grp.texture);

            if(grp.samplerState != NULL)
                ps.ChangeSamplerState(0, grp.samplerState);
            else if(!defSmpStateSet){
                ps.ChangeSamplerState(0, "DefaultSamplerState");
                defSmpStateSet = true;
            }

            DeviceKeeper::GetDeviceContext()->Draw(grp.count, grp.offset);
        }
    });

    gs.CleanUp();
}

void Manager::CollectDrawingObjects(const Control *Cntrl, SortedDrawingObjectsStorage &AllObjects, DrawingPriority ParentPriority )
{
    if(!Cntrl->IsVisible())
        return;

    DrawingPriority priority = DP_LOW;

    if(Cntrl == lockedControl)
        priority = DP_HIGH;
    else{
        auto it = std::find(drawPriorityControls.begin(), drawPriorityControls.end(), Cntrl);
        if(it != drawPriorityControls.end())
            priority = DP_MEDIUM;
    }

    if(priority < ParentPriority)
        priority = ParentPriority;

    for(const DrawingObject *dObj : Cntrl->GetDrawingObjects())
        AllObjects[priority].push_back(dObj);

    for(const Control *childCntrl : Cntrl->childControls)
        CollectDrawingObjects(childCntrl, AllObjects, priority);
}

void Manager::Invalidate(float Tf, const ControlsStorage &Controls)
{
    POINT cursorPs = DirectInput::GetInsance()->GetCursorPos();

    D3DXVECTOR2 cursorPos;
    cursorPos.x = (FLOAT)cursorPs.x / CommonParams::GetScreenWidth();
    cursorPos.y = (FLOAT)cursorPs.y / CommonParams::GetScreenHeight();
        
    cursorDelta = cursorPos - prevCursorPos;

    ControlsStorage::const_reverse_iterator ci;
    for(ci = Controls.rbegin(); ci != Controls.rend(); ++ci)
        if((*ci)->IsVisible())
            (*ci)->Invalidate(cursorPos, Tf);

    prevCursorPos = cursorPos;
}

void Manager::Invalidate(float Tf)
{
    Invalidate(Tf, controls);
}

void Manager::Draw() throw (Exception)
{
    Draw(controls);
}

Manager::~Manager()
{
    controls.clear();

    for(auto pair : textures)
        ReleaseCOM(pair.second);

    ReleaseCOM(dataVb);

    RenderStatesManager::GetInstance()->RemoveRenderState("DisableDepthForGUI");
    RenderStatesManager::GetInstance()->RemoveRenderState("StandardAlfaBlendForGUI");
}

BOOL Manager::TryToLockControl(Control *ControlToLock)
{
    if(!lockedControl)
        lockedControl = ControlToLock;

    return lockedControl == ControlToLock;
}

void Manager::UnlockControl(Control *ControlToUnlock)
{
    if(lockedControl == ControlToUnlock)
        lockedControl = NULL;
}

void Manager::ResetContext()
{
    for(Control *cntrl : controls)
        cntrl->OnManagerResetContext();

    lockedControl = NULL;
    drawPriorityControls.clear();
}

void Manager::RiseDrawPriority(Control *ControlToRise)
{
    if(ControlToRise == lockedControl)
        return;

    auto it = std::find(drawPriorityControls.begin(), drawPriorityControls.end(), ControlToRise);
    if(it == drawPriorityControls.end())
        drawPriorityControls.push_back(ControlToRise); 
}

void Manager::LowDrawPriority(Control *ControlToLow)
{
    auto it = std::find(drawPriorityControls.begin(), drawPriorityControls.end(), ControlToLow);
    if(it != drawPriorityControls.end())
        drawPriorityControls.erase(it); 
}

}
