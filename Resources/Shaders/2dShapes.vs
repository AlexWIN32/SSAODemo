/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include "2dShapes.fxh"

cbuffer buff1 : register(b0)
{
    matrix transform;
}

VsToPs ProcessVertex(ToVs Input)
{
    float3 posN = mul(float3(Input.posL, 1.0f), (float3x3)transform);
    posN.z = 0.0f;

    VsToPs output;
    output.posN = float4(posN, 1.0f);
    output.texCoord = Input.texCoord;

    return output;
}