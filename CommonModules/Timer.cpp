/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Timer.h>
#include <sstream>

namespace Time
{

Timer::Timer() :
    frameCounter(0),
    fps(0),
    timeDelta(0.0),
    optimalFrameTickTime(0.0),
    timeFactor(0.0f),
    prevTimeStamp(0),
    ticksPerSecond(0)
{}

void Timer::Init(INT OptimalFps)
{
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&ticksPerSecond));
    optimalFrameTickTime = ticksPerSecond / OptimalFps;

    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&prevTimeStamp));
}

void Timer::Invalidate()
{
    frameCounter++;

    LONGLONG timestamp;
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&timestamp));

    LONGLONG eplasedTime = timestamp - prevTimeStamp;

    prevTimeStamp = timestamp;

    timeFactor = (DOUBLE)eplasedTime / optimalFrameTickTime;

    timeDelta += eplasedTime / (DOUBLE)ticksPerSecond;
    if(timeDelta >= 1.0){
        fps = frameCounter;
        timeDelta = 0.0;
        frameCounter = 0;

        //std::ostringstream os_;
        //os_ << fps << std::endl;
        //OutputDebugStringA( os_.str().c_str() );
    }
}
}