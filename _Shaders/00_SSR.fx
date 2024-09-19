DepthStencilState SSR_DSS;
BlendState SSR_BS;

Texture2D<float4> SSR_HDRTex;
Texture2D SSR_DepthTex;

struct SSRPSConstant_Desc
{
    float4x4 ProjMatrix;
    
    float cb_zThickness; // thickness to ascribe to each pixel in the depth buffer
    float cb_nearPlaneZ; // the camera's near z plane
    float cb_stride; // step in horizontal or vertical pixels between samples
    float cb_maxSteps; // maximum number of iterations. higher gives better images but may be slow
    
    float cb_maxDistance; // maximum camear- space distance to trace befor returning a miss
    float cb_strideZCutoff; // more distant pixels are smaller in screen space
    // this value tells at what point to start relaxing the stride to give higher quality reflection
    // for objects far from the camera      
    float cb_numMips; // the number of mip levels in the convolved color buffer
    float cb_fadeStart; // determines hwere to start screen edge fading of effect
    
    float cb_fadeEnd; // determince where to end screen edge fading of effect
    float padding;
    float2 TexSize; // dimesions of the z-buffer
    float4 PerspectiveValues;
};

cbuffer SSReflectionPSConstants
{
    SSRPSConstant_Desc ssrPSDesc;
};

float ConvertZToLinearDepth(float depth)
{
    float linearDepth = ssrPSDesc.PerspectiveValues.z /
    (depth + ssrPSDesc.PerspectiveValues.w);
    return linearDepth;
}

float3 CalcViewPos(float2 csPos, float depth)
{
    float3 position;    
    position.xy = csPos.xy * ssrPSDesc.PerspectiveValues.xy * depth;
    position.z = depth;
    return position;
}

float GetDepth(float2 uv)
{
    float depth = SSR_DepthTex.Sample(PointSampler, uv).x;
    return ConvertZToLinearDepth(depth);
}

float linearDepthTexelFetch(int2 hitPixel)
{
    // load returns 0 for any value accessed out of bounds   
    return ConvertZToLinearDepth(SSR_DepthTex.Load(int3(hitPixel, 0)).r);

}

float3 GetPositionFromUV(float2 uv)
{
    float2 csPos = (uv - 0.5) * float2(2, -2);
    return CalcViewPos(csPos, GetDepth(uv));
}

float distanceSqured(float2 a, float2 b)
{
    a -= b;
    return dot(a, a);
}

// 탐색 거리가 늘어날수록 검사 두께를 늘려 검사한다
bool intersectsDepthBuffer(float z, float minZ, float maxZ)
{
    /*
     * Base on haw far away from the camera the depth is,
     * adding a bit of extra thickness can help imporve some
     * artifacts, driving this value up too high can cause
     * artifacts of its own.
    */
    float depthScale = min(1.0f, z * ssrPSDesc.cb_strideZCutoff);
    z += ssrPSDesc.cb_zThickness + lerp(0.0f, 2.0f, depthScale);
    return (maxZ >= z) && (minZ - ssrPSDesc.cb_zThickness <= z);
}

void swap(inout float a, inout float b)
{
    float t = a;
    a = b;
    b = t;
}

float2 GetUVFromClip(float2 cs)
{
    return cs * float2(0.5, -0.5) + 0.5;
}

float PerspectiveLerp(float z1, float z2, float s)
{
    return z1 * z2 / (z2 + s * (z1 - z2));
}

bool IntersectCheck (float z1, float z2, float s, float2 pixel)
{
    float sampleDepth = linearDepthTexelFetch(int2(pixel));
    float rayDepth = PerspectiveLerp(z1, z2, s);
    
    return
    (
    rayDepth > sampleDepth   
    &&
    abs(rayDepth - sampleDepth) < ssrPSDesc.cb_strideZCutoff
    )
    ? 1 : 0;
}

bool IntersectCheckWithThickness(float z1, float z2, float s, float2 pixel)
{
    float sampleDepth = linearDepthTexelFetch(int2(pixel));
    float rayDepth = PerspectiveLerp(z1, z2, s);
    
    return
    (
    rayDepth > sampleDepth
    &&
    abs(rayDepth - sampleDepth) < ssrPSDesc.cb_zThickness
    )
    ? 1 : 0;
}

float4 SSRRelfectionPS(SSR_MeshOutput In) : SV_Target
{
    float4 positionVS = In.ViewPosition;
    
    float3 normalVS = normalize(In.ViewNormal);
    float3 viewRayVS = normalize(positionVS.xyz);
    float3 rayVS = normalize(reflect(viewRayVS, normalVS));
    
    float RdotN = dot(rayVS, normalVS);
    float correctDistance = 10.0f * RdotN;
    
    float3 originVS = positionVS.xyz;
    float3 endVS = originVS + rayVS * (ssrPSDesc.cb_maxDistance + correctDistance);
    
    float4 P0 = mul(float4(originVS, 1), ssrPSDesc.ProjMatrix);
    P0.xyz /= P0.w; // clip space;
    float2 U0 = GetUVFromClip(P0.xy); // uv coord
    float2 T0 = U0 * ssrPSDesc.TexSize; // pixel coord
    float4 P1 = mul(float4(endVS, 1), ssrPSDesc.ProjMatrix);
    P1.xyz /= P1.w;
    float2 U1 = GetUVFromClip(P1.xy);
    float2 T1 = U1 * ssrPSDesc.TexSize;
    
    float2 rayUV = U1 - U0; 
    float2 rayPixel = T1 - T0;
    
    float2 prevPixel;
    float2 curPixel;
    
    float z1 = originVS.z;
    float z2 = endVS.z;
    float s = ssrPSDesc.cb_zThickness;
    
    float delta = max(abs(rayPixel.x), abs(rayPixel.y));    
    float offs = 1.0f / delta;
    //float offs = (delta < ssrPSDesc.cb_stride ? 1 / delta : ssrPSDesc.cb_stride / delta);
    //float limit = (delta < ssrPSDesc.cb_stride ? delta : delta / ssrPSDesc.cb_stride);
    
    prevPixel = T0;
    curPixel = T0 + offs * rayPixel;
    
    float i = 1;
    
    float2 finalPixel = float2(-1, -1);
    for (; i <= delta && !IntersectCheckWithThickness(z1, z2, offs * i, curPixel); i++)
    {
        prevPixel = curPixel;
        curPixel = T0 + (rayPixel * offs * i);
    }
    
    finalPixel = curPixel;
    finalPixel /= ssrPSDesc.TexSize;
    
    float4 color = SSR_HDRTex.Sample(PointSampler, finalPixel) ;
    color.w = 1 - (offs * i);
    color *= (i <= delta ? 1 : 0);
    return color;
    
}

// d3d_primitive_topology_trianglestrip
static const float2 SSRarrBasePos[4] =
{
    float2(-1, +1),
    float2(+1, +1),
    float2(-1, -1),
    float2(+1, -1)
};

struct VertexOutput_SSRBlend
{
    float4 Position : SV_Position;
    float2 Screen : Position1;
    float2 UV : UV;
    
};


VertexOutput_SSRBlend ReflectionBlendVS(uint VertexID : SV_VertexID)
{
    VertexOutput_SSRBlend output;
    // 1 / 4 위치 반환
    output.Position = float4(SSRarrBasePos[VertexID].xy, 0.0, 1.0);
    output.Screen = output.Position.xy;
    output.UV = output.Screen * float2(0.5, -0.5) + 0.5;
    return output;
}

float4 ReflectionBlendPS(VertexOutput_SSRBlend input) : SV_Target
{
    float2 texel = 1 / ssrPSDesc.TexSize;
    float4 color = float4(0, 0, 0, 0);
    for (int i = 0; i < 2;i++)
    {
        for (int j = 0; j < 2;j++)
        {
            float2 pixel = input.Position.xy + float2(j * texel.x, i * texel.y);
            color += SSR_HDRTex.Load(int3(pixel, 0));
        }
    }
    
    return color / 4.0f;
}

