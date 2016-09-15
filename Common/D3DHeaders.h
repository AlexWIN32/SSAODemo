/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10math.h>
#include <dxerr.h>
#include <Exception.h>

DECLARE_EXCEPTION(D3DException)

inline void HR(HRESULT Hr) throw (Exception)
{
	if (FAILED(Hr))
		throw D3DException(std::string(DXGetErrorStringA(Hr)) + ": " + std::string(DXGetErrorDescriptionA(Hr)));
}

template<class TFunction>
inline void HR(HRESULT Hr, const TFunction &Function) throw (Exception)
{
    if (FAILED(Hr))
        throw Function();
}

template <typename T>
inline void ReleaseCOM(T *&x) 
{
    if(x){ 
        x->Release(); 
        x = 0; 
    } 
}
