/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include "2dObjects.fxh"

[maxvertexcount(6)]
void ProcessVertex(point VsToGs InData[1], inout TriangleStream<GsOutPsIn> Stream)
{
    VsToGs In = InData[0];

    float2 v[6];
    v[0] = float2(-In.halfSize.x, -In.halfSize.y);
    v[1] = float2( In.halfSize.x, -In.halfSize.y);
    v[2] = float2( In.halfSize.x, In.halfSize.y);
    v[3] = float2( In.halfSize.x, In.halfSize.y);
    v[4] = float2(-In.halfSize.x, In.halfSize.y);
    v[5] = float2(-In.halfSize.x, -In.halfSize.y);

    float2 tc[6];
    tc[0] = In.sidesTexCoords.xy;
    tc[1] = In.sidesTexCoords.zy;
    tc[2] = In.sidesTexCoords.zw;
    tc[3] = In.sidesTexCoords.zw;
    tc[4] = In.sidesTexCoords.xw;
    tc[5] = In.sidesTexCoords.xy;

    float3x3 transform = float3x3(In.transformRow1, In.transformRow2, In.transformRow3);

    [unroll]
    for(int i = 0; i < 6; ++i){
        GsOutPsIn newData;

        newData.posN = float4(mul(float3(v[i], 1.0f), transform), 1.0f);
        newData.posN.z = 0.0f;

        newData.color = In.color;
        newData.texCoord = tc[i];
        Stream.Append(newData);
    }
    Stream.RestartStrip();
}