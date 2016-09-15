/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

cbuffer Data : register(b0)
{
    float4 weights[11];
    float2 texFactors;
    float2 padding;    
};

cbuffer Data2 : register(b1)
{
    int isVertical;
    float3 padding2;
};