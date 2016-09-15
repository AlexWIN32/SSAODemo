/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <windows.h>
#include <functional>
#include <map>

namespace Utils
{

typedef UINT EventId;

template<class TFunction>
class EventsStorage
{
public:
    typedef typename std::function<TFunction> Event;
protected:
    typedef std::map<EventId, Event> EventsData;
    EventsData events;
    EventId eventsCnt;
public:
    EventsStorage() : eventsCnt(0){}
    EventId AddEvent(Event NewEvent)
    {
        INT id = eventsCnt;
        events.insert(std::make_pair(eventsCnt, NewEvent));
        eventsCnt++;
        return id;
    }
    void RemoveEvent(EventId EventToRemove)
    {
        EventsData::iterator it = events.find(EventToRemove);
        if(it != events.end())
            events.erase(it);
    }
    auto begin() -> decltype(events.begin()) const {return events.begin();}
    auto end() -> decltype(events.end()) const {return events.end();}
};

}