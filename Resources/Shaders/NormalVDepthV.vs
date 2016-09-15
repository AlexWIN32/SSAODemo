/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

cbuffer Data : register(b0)
{
    matrix worldViewProj;
    matrix worldInvTransView;    
    matrix worldView;
};

struct VIn
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD0;
};

struct VOut
{
    float4 posH : SV_POSITION;
    float3 normalV : NORMAL;
    float2 tex : TEXCOORD0;
    float3 posV : TEXCOORD1;
};

VOut ProcessVertex(VIn input)
{
    VOut output;
    output.posH = mul(float4(input.posL, 1.0f), worldViewProj);
    output.normalV = mul(float4(input.normalL, 0.0f), worldInvTransView).xyz;
    output.posV = mul(float4(input.posL, 1.0f), worldView).xyz;
    output.tex = input.tex;
    return output;
}