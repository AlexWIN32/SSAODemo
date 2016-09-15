/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <D3DHeaders.h>

class CommonParams
{
private:
    static FLOAT screenWidth, screenHeight;
    static FLOAT widthOverHeight, heightOverWidth;
    static D3DXVECTOR2 pixelSize;
    static HWND window;
public:
    CommonParams(CommonParams &) = delete;
    CommonParams & operator=(CommonParams &) = delete;
    static void SetScreenSize(UINT Width, UINT Height)
    {
        screenWidth = (float)Width;
        screenHeight = (float)Height;

        widthOverHeight = screenWidth / screenHeight;
        heightOverWidth = screenHeight / screenWidth;

        pixelSize.x = 1.0f / screenWidth;
        pixelSize.y = 1.0f / screenHeight;
    }
    static void SetWindow(HWND Window)
    {
        window = Window;
    }
    static HWND GetWindow() 
    {
        return window;
    }
    static FLOAT GetScreenWidth()
    {
        return screenWidth;
    }
    static FLOAT GetScreenHeight()
    {
        return screenHeight;
    }
    static FLOAT GetWidthOverHeight()
    {
        return widthOverHeight;
    }
    static FLOAT GetHeightOverWidth()
    {
        return heightOverWidth;
    }
    static D3DXVECTOR2 &GetPixelSize() 
    {
        return pixelSize;
    } 
};