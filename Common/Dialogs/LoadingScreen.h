/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Exception.h>
#include <windows.h>
#include <map>
#include <mutex>

namespace Dialogs
{

DECLARE_EXCEPTION(LoadingScreenException)

class ILoadingScreen
{
public:
    typedef std::map<std::string, std::string> MessagesStorage;
    virtual ~ILoadingScreen(){}
    virtual void SetProgress(FLOAT NewProgress, const MessagesStorage &Messages = MessagesStorage()) = 0;
    virtual void Invalidate(FLOAT Tf){}
};

class SyncLoadingScreen : public ILoadingScreen
{
protected:
    virtual void ProcessNewProgress(FLOAT NewProgress, const MessagesStorage &Messages) = 0;
    virtual void Render() = 0;
public:
    virtual ~SyncLoadingScreen(){}
    virtual void SetProgress(FLOAT NewProgress, const MessagesStorage &Messages = MessagesStorage());
};

class AsyncLoadingScreen : public ILoadingScreen
{
protected:
    std::mutex mutex;
    virtual void ProcessNewProgress(FLOAT NewProgress, const MessagesStorage &Messages) = 0;
    virtual void DrawObjects() = 0;
public:
    virtual ~AsyncLoadingScreen(){}
    virtual void Draw()
    {
        std::lock_guard<std::mutex> lock(mutex);
        DrawObjects();
    }
    virtual void SetProgress(FLOAT NewProgress, const MessagesStorage &Messages = MessagesStorage())
    {
        std::lock_guard<std::mutex> lock(mutex);
        ProcessNewProgress(NewProgress, Messages);
    }
};

}