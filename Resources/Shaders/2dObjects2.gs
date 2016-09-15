/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include "2dObjects2.fxh"

[maxvertexcount(6)]
void ProcessVertex(point VsToGs InData[1], inout TriangleStream<GsOutPsIn> Stream)
{
    VsToGs In = InData[0];

    float2 v[6];
    v[0] = In.pt1pt2.xy;
    v[1] = In.pt1pt2.zw;
    v[2] = In.pt3pt4.xy;
    v[3] = In.pt3pt4.xy;
    v[4] = In.pt3pt4.zw;
    v[5] = In.pt1pt2.xy;

    float2 tc[6];
    tc[0] = In.sidesTexCoords.xy;
    tc[1] = In.sidesTexCoords.zy;
    tc[2] = In.sidesTexCoords.zw;
    tc[3] = In.sidesTexCoords.zw;
    tc[4] = In.sidesTexCoords.xw;
    tc[5] = In.sidesTexCoords.xy;

    [unroll]
    for(int i = 0; i < 6; ++i){
        GsOutPsIn newData;

        newData.posN = float4(v[i], 0.0f, 1.0f);
        newData.color = In.color;
        newData.texCoord = tc[i];

        Stream.Append(newData);
    }
    Stream.RestartStrip();
}