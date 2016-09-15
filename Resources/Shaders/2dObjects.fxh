/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

struct VsToGs
{
    float2 halfSize : TEXCOORD0;
    float4 color : COLOR;
    float4 sidesTexCoords : TEXCOORD1;
    float3 transformRow1 : TEXCOORD2;
    float3 transformRow2 : TEXCOORD3;
    float3 transformRow3 : TEXCOORD4;
};

struct GsOutPsIn
{
    float4 posN : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};