/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

struct VIn
{
    float3 posN : POSITION;
    float2 tex : TEXCOORD0;	
};

struct VOut
{
    float4 posN : SV_POSITION;
    float2 tex : TEXCOORD0;  
    float4 eyeRayN: TEXCOORD1;
};

VOut ProcessVertex(VIn input)
{    
    VOut output;
    output.posN = float4(input.posN, 1.0f); 
    output.tex = input.tex;
    output.eyeRayN = float4(output.posN.xy, 1.0f, 1.0f);
    return output;
}