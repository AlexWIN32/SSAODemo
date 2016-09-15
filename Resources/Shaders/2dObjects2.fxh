/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

struct VsToGs
{
    float4 pt1pt2 : POSITION;
    float4 pt3pt4 : TEXCOORD0;
    float4 sidesTexCoords : TEXCOORD1;
    float4 color : COLOR;
};

struct GsOutPsIn
{
    float4 posN : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};