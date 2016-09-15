/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

struct ToVs
{
    float2 posL : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct VsToPs
{
    float4 posN : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};