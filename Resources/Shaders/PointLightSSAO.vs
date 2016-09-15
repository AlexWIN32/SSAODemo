/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

cbuffer buff1 : register(b0)
{
    matrix worldViewProj;
    matrix worldInvTrans;
    matrix world;
}

struct VIn
{
    float4 posL : POSITION;
    float2 tex : TEXCOORD0;
    float3 normalL : NORMAL;
};

struct VOut
{
    float4 posS : SV_POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD0;
    float4 posH : TEXCOORD1; 
    float3 posW : TEXCOORD2;
};

VOut ProcessVertex(VIn input)
{
    VOut output;
    output.posS = output.posH = mul(input.posL, worldViewProj);
    output.posW = mul(float4(input.posL.xyz, 1.0f), world).xyz;
    output.normalL = mul(float4(input.normalL.xyz, 0.0f), worldInvTrans).xyz;
    output.tex = input.tex;
    return output;
}
