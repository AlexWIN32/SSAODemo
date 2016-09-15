/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <GUI.h>
#include <Dialogs.h>

namespace Demo
{

class LoadingScreen : public Dialogs::SyncLoadingScreen
{
private:
    GUI::ProgressBar pb;
    GUI::Label lbl;
    static LoadingScreen *instance;
    LoadingScreen(){}
public:
    static LoadingScreen *GetInstance()
    {
        if(!instance)
            instance = new LoadingScreen();
        return instance;
    }
    static void ReleaseInstance()
    {
        delete instance;
        instance = NULL;
    }
    LoadingScreen(LoadingScreen &) = delete;
    LoadingScreen & operator=(LoadingScreen &) = delete;
    void Init() throw (Exception);
    virtual void ProcessNewProgress(FLOAT NewProgress, const MessagesStorage &Messages);
    virtual void Render();
};

}