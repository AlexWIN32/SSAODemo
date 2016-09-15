/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/


struct VIn
{
    float4 posH : POSITION;
    float2 tex : TEXCOORD0;	
};

struct VOut
{
    float4 posH : SV_POSITION;
    float2 tex : TEXCOORD0;  
};

VOut ProcessVertex(VIn input)
{
    VOut output;
    output.posH = input.posH; 
    output.tex = input.tex;
    return output;
}