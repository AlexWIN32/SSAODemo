/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <windows.h>

namespace Time
{

class Timer final
{
private:
    UINT frameCounter, fps;
    DOUBLE timeDelta, optimalFrameTickTime;
    FLOAT timeFactor;
    LONGLONG prevTimeStamp, ticksPerSecond;
public:
    Timer();
    UINT GetFps() const {return fps;}
    DOUBLE GetTimeFactor() const {return timeFactor;}
    void Init(INT OptimalFps);
    void Invalidate();
};

}