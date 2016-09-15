#include <GUI/ProgressBar.h>
#include <GUI/Manager.h>
#include <memory>

namespace GUI
{

void ProgressBar::Init() throw (Exception)
{
    Init(Manager::GetInstance()->GetTheme().GetControlView("ProgressBar"));
}

void ProgressBar::Init(const ControlView &PBControlView) throw (Exception)
{
    std::string paramData;
    if(PBControlView.FindParameter("bgArea", paramData)){
        std::unique_ptr<Area> area(new Area());

        area->Init(paramData);

        bgControl = area.release();

    }else{
        std::unique_ptr<Image> image(new Image());

        image->Init(PBControlView.GetParameter("bgImage"));
        image->SetPositioning(CNTRL_POS_BOTTOM_RIGHT);

        bgControl = image.release();
    }

    AddControl(bgControl);

    progressArea.Init(PBControlView.GetParameter("progressArea"));
}

void ProgressBar::SetSize(const D3DXVECTOR2 &Size)
{
    D3DXVECTOR2 newBgSize = {Control::CheckPixelErrorForY(Size.x), Control::CheckPixelErrorForY(Size.y)};
    bgControl->SetSize(newBgSize);

    progressArea.SetSize({Control::CheckPixelErrorForY(newBgSize.x * progress), newBgSize.y});

    Control::SetSize(newBgSize);
}

void ProgressBar::SetPos(const D3DXVECTOR2 &Pos)
{
    D3DXVECTOR2 delta = Pos - GetPos();

    bgControl->SetPos(bgControl->GetPos() + delta);
    progressArea.SetPos(progressArea.GetPos() + delta);

    Control::SetSize(Pos);
}

void ProgressBar::SetProgress(FLOAT NewProgress) throw (Exception)
{
    if(NewProgress < 0.0f)
        NewProgress = 0.0f;

    if(NewProgress > 1.0f)
        NewProgress = 1.0f;

    progress = NewProgress;

    if(progress != 0.0f){
        bool found = false;
        for(Control* cntrl : GetControls())
            if(cntrl == &progressArea){
                found = true;
                break;
            }

        if(!found)
            AddControl(&progressArea);
    }

    const D3DXVECTOR2 &bgSize = bgControl->GetSize();

    progressArea.SetSize({Control::CheckPixelErrorForY(bgSize.x * progress), bgSize.y});
}

}