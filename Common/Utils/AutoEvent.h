/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <functional>

namespace Utils
{

class AutoEvent
{
private:
	typedef std::function<void()> EventObject;
	EventObject evnt;
public:
    AutoEvent(const AutoEvent &) = delete;
    AutoEvent &operator= (const AutoEvent &) = delete;
	AutoEvent(EventObject Event):evnt(Event){}
	~AutoEvent(){evnt();}
};

}