/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <DirectInput.h>

DirectInput *DirectInput::instance = NULL;

void DirectInput::Init(HINSTANCE AppInstance, HWND Hwnd) throw (Exception)
{
	hwnd = Hwnd;

	IDirectInput8* DInput;
	if (FAILED(DirectInput8Create(AppInstance, 
		DIRECTINPUT_VERSION, 
		IID_IDirectInput8, 
		reinterpret_cast<void**>(&DInput), 0)))
		throw DirectInputException("cant create direct input");
		

	if (FAILED(DInput->CreateDevice(GUID_SysKeyboard, &Keyboard, 0)))
		throw DirectInputException("cant create keyboard device");

	Keyboard->SetDataFormat(&c_dfDIKeyboard);
	Keyboard->SetCooperativeLevel(Hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	Keyboard->Acquire();

	if (FAILED(DInput->CreateDevice(GUID_SysMouse, &Mouse, 0)))
		throw DirectInputException("cant create mouse device");

	Mouse->SetDataFormat(&c_dfDIMouse2);
	Mouse->SetCooperativeLevel(Hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	Mouse->Acquire();

    DIPROPDWORD dipDw = {};
    dipDw.diph.dwSize = sizeof(DIPROPDWORD);
    dipDw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipDw.diph.dwHow = DIPH_BYOFFSET;
 
    dipDw.diph.dwObj = DIMOFS_X;
    if(FAILED(Mouse->GetProperty(DIPROP_GRANULARITY, &dipDw.diph)))
        throw DirectInputException("cant get mouse property");
    mouseOffset.x = dipDw.dwData;

    dipDw.diph.dwObj = DIMOFS_Y;
    if(FAILED(Mouse->GetProperty(DIPROP_GRANULARITY, &dipDw.diph)))
        throw DirectInputException("cant get mouse property");
    mouseOffset.y = dipDw.dwData;

    dipDw.diph.dwObj = DIMOFS_Z;
    if(FAILED(Mouse->GetProperty(DIPROP_GRANULARITY, &dipDw.diph)))
        throw DirectInputException("cant get mouse property");
    mouseOffset.z = dipDw.dwData;

	DInput->Release();
}

void DirectInput::Poll()
{	
	::GetCursorPos(&mouseCursorPos);
	ScreenToClient(hwnd, &mouseCursorPos);

	char keyboardState[256] = { 0 };
	if (FAILED(Keyboard->GetDeviceState(sizeof(keyboardState), reinterpret_cast<void**>(&keyboardState))))
		Keyboard->Acquire();

	DIMOUSESTATE2 mouseState = { 0 };
	if (FAILED(Mouse->GetDeviceState(sizeof(DIMOUSESTATE2), reinterpret_cast<void**>(&mouseState))))
		Mouse->Acquire();

	for (int i = 0; i < 256; i++)
		if (keyboardState[i] & 0x80)
			kbBtnsDownState[i] = true;					
		else if (kbBtnsDownState[i]){
			kbBtnsDownState[i] = false;
			kbBtnsPressState[i] = true;
		}
		else
			kbBtnsPressState[i] = false;

    mouseDelta.x = mouseState.lX / mouseOffset.x;
    mouseDelta.y = mouseState.lY / mouseOffset.y;
    mouseDelta.z = mouseState.lZ / mouseOffset.z;

	for (int i = 0; i < 8; i++)
		if (mouseState.rgbButtons[i] & 0x80)
			msBtnsDownState[i] = true;
		else if (msBtnsDownState[i]){
			msBtnsDownState[i] = false;
			msBtnsPressState[i] = true;
		}
		else
			msBtnsPressState[i] = false;
}

