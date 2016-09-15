/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <dinput.h>
#include <Exception.h>

DECLARE_EXCEPTION(DirectInputException)

class DirectInput
{
public:
	struct MouseState
	{
		LONG x, y, z;
		MouseState() :x(0), y(0), z(0){}
	};
private :	
	IDirectInputDevice8* Keyboard;		 
	IDirectInputDevice8* Mouse;		
	bool kbBtnsDownState[256], kbBtnsPressState[256];
	bool msBtnsDownState[8], msBtnsPressState[8];
	MouseState mouseDelta, mouseOffset;
	POINT mouseCursorPos;
	HWND hwnd;

	DirectInput(const DirectInput&);
	DirectInput& operator= (const DirectInput&);
	DirectInput(){}
	~DirectInput();
	static DirectInput *instance;	
public:
	void Init(HINSTANCE AppInstance, HWND Hwnd) throw (Exception);
	void Poll();
	bool IsKeyboardPress(UINT Code) const { return kbBtnsPressState[Code]; }
	bool IsKeyboardDown(UINT Code) const { return kbBtnsDownState[Code]; }
	bool IsMousePress(UINT Code) const { return msBtnsPressState[Code]; }
	bool IsMouseDown(UINT Code) const { return msBtnsDownState[Code]; }
	const POINT &GetCursorPos() const { return mouseCursorPos; }
	const MouseState &GetMouseDelta() const { return mouseDelta; }
	static DirectInput * GetInsance()
	{
		if (!instance)
			instance = new DirectInput();
		return instance;
	}
	static void ReleaseInstance()
	{
		delete instance;
		instance = NULL;
	}		
};
