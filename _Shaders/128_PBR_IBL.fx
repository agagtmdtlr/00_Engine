//#include "00_Global.fx"
//#include "00_PBR.fx"
//#include "00_Render_PBR.fx"
#include "00_Deffered_PBR.fx"
#include "00_DynamicCube_PBR.fx"
#include "00_SSR.fx"
cbuffer ToneMapDesc
{
    float MiddleGrey;
    float LumWhite;
    float2 ProjectionValues;
    uint ToneMapType;
};
StructuredBuffer<float> AvgLum;

#include "00_ToneMap.fx"

float4 PS_Forward(MeshOutput input) : SV_Target
{
    input.Normal = normalize(input.Normal);
    input.Tangent = normalize(input.Tangent);
    input.Tangent = input.Tangent - dot(input.Tangent, input.Normal) * input.Normal;
    float3 BiNormal = cross(input.Normal, input.Tangent);
    // transposed TBN
    float3x3 tTBN = float3x3(input.Tangent, BiNormal, input.Normal);
    tTBN = transpose(tTBN);
    float3 TangentLightPos;
    float3 viewTS = mul(ViewPosition(), tTBN);
    float3 worldTS = mul(input.wPosition, tTBN);
    // offset texture coordinates with Parallax Mapping
    float3 viewDir = normalize(viewTS - worldTS);
    viewDir.y *= -1;    
    // if parallax get new texture coords; else use origin texture coords;
    float2 uv = (mapDesc.parallax ? ParallaxOcclusionMapping(input.Uv, viewDir) : input.Uv);
    // then texture sampling with new texture coords
    //albedo = (mapDesc.albedo ? pow(AlbedoMap.Sample(LinearSampler, uv).rgb,2.2) : float3(0.5, 0.5, 0.5));
	albedo = float3(0.5, 0.5, 0.5);
    metallic = (mapDesc.metal ? MetallicMap.Sample(LinearSampler, uv).r : 0.0f);
    roughness = (mapDesc.rough ? RoughnessMap.Sample(LinearSampler, uv).r : 0.0f);
    ao = (mapDesc.ao ? AOMap.Sample(LinearSampler, uv).r : 1.0f);
    
    
    float3 N = (mapDesc.normal ? NormalMapping(input.Normal, input.Tangent, uv) : input.Normal);
    float3 V = normalize(ViewPosition() - input.wPosition);
    float3 R = reflect(-V, N);
    
    float4 result = 0;
    // preceed with lighting code    
    // direct LIght
    result += DirectionLight(N, V, R);
    //result += ComputePointLight(N, V, R, input.wPosition);
    
    ///////////////////////////////////////////////////////////////////////
    // ambient Light; with Image based light
    ////////////////////////////////////////////////////////////
    irradiance = IrradianceCube.Sample(LinearSampler, N).rgb; //
    
    float3 color;
    float3 diffuse;
    float3 specular;
    float3 kS;
    float3 kD;
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    
    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor;
    float2 brdf;
    
    if (mapDesc.irradiance == 1)
    {
        fresnel = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        kS = fresnel;
        kD = 1.0 - kS;
        kD *= 1.0 - metallic;
        
        diffuse = irradiance * albedo;
        
        prefilteredColor = ReflectionCube.SampleLevel(
        MipLinearSampler, R, roughness * MAX_REFLECTION_LOD).rgb;
        brdf = BRDFMap.Sample(LinearSampler, float2(max(dot(N, V), 0.0), roughness)).rg;
        specular = prefilteredColor * (fresnel * brdf.x + brdf.y);
        diffuse = 0;
        ambient = (kD * diffuse + specular) * ao;
    }
    if (mapDesc.irradiance == 0)
    {
        ambient = 0.03 * albedo * ao;
    }
    
    result *= Shadow_Factor(input.sPosition);
    result = result + float4(ambient.xyz, 1);
    
    
    ReinhardToneMapping(result.xyz);
    //UnchartedToneMapping(result);
    //EAToneMapping(result);
    result = pow(result, 1 / 2.2f);
    result.a = 1;
    return result;
}

float4 PS_Test(MeshOutput input) : SV_Target
{
    return float4(1, 1, 1, 1);
}

float4 PS_Deffered_Test(VertexOutput_Directional input) : SV_Target
{
    return float4(1, 1, 1, 1);    
}

technique11 T0
{
    // Forward Rendering
    P_VP(P0, VS_Mesh, PS_Forward)
    P_VP(P1, VS_Model, PS_Forward)
    P_VP(P2, VS_Animation, PS_Forward)
    // tessellation Mesh Render for Displacement mapping
    P_VTP(P3, VS_Tess_Mesh, HS_Mesh, DS_Mesh, PS_Forward)
    //Shadow PreRender for directional light
    P_RS_VP(P4, FrontCounterClockwise_True,VS_Depth_Mesh, PS_Depth)
    P_RS_VP(P5, FrontCounterClockwise_True,VS_Depth_Model, PS_Depth)
    P_RS_VP(P6, FrontCounterClockwise_True,VS_Depth_Animation, PS_Depth)
    // Deffered 1Pass (MultiRenderTarget)
    P_DSS_VP(P7, Deffered_DepthStencil_State, VS_Mesh, PS_PackGBuffer)
    P_DSS_VP(P8, Deffered_DepthStencil_State, VS_Model, PS_PackGBuffer)
    P_DSS_VP(P9, Deffered_DepthStencil_State, VS_Animation, PS_PackGBuffer)
    P_DSS_VTP(P10, Deffered_DepthStencil_State, VS_Tess_Mesh, HS_Mesh, DS_Mesh, PS_PackGBuffer)
    // Deffered 2Pass
    //P_VP(P11, VS_Directional, PS_Deffered_Test)
    P_DSS_VP(P11, Deffered_DepthStencil_State, VS_Directional, PS_Deffered_Directional)
    P_DSS_BS_VP(P12, Deffered_DepthStencil_State ,Deffered_Blend_State, VS_Directional, PS_IBLAmibeintLight)
    P_RS_DSS_BS_VTP(P13, Deffered_Rasterizer_State, Deffered_DepthStencil_State, Deffered_Blend_State, VS_PointLights, HS_PointLights, DS_PointLights, PS_Deffered_PointLights)
    P_RS_DSS_BS_VTP(P14, Deffered_Rasterizer_State, Deffered_DepthStencil_State, Deffered_Blend_State, VS_SpotLights, HS_SpotLights, DS_SpotLights, PS_Deffered_SpotLights)

    // Deffered Debug
    P_RS_VTP(P15, Deffered_Rasterizer_State, VS_PointLights, HS_PointLights, DS_PointLights, PS_PointLights_Debug)
    P_RS_DSS_VTP(P16, Deffered_Rasterizer_State, Deffered_DepthStencil_State, VS_SpotLights, HS_SpotLights, DS_SpotLights, PS_Deffered_SpotLights_Debug)
    
    // point light perspective shadow map texturecube depth rendering
    P_RS_VGP(P17, FrontCounterClockwise_True, VS_Mesh, GS_DynamicCube, PS_DynamicCube_ShadowMap)
    P_RS_VGP(P18, FrontCounterClockwise_True, VS_Model, GS_DynamicCube, PS_DynamicCube_ShadowMap)
    P_RS_VGP(P19, FrontCounterClockwise_True, VS_Animation, GS_DynamicCube, PS_DynamicCube_ShadowMap)
    
    // spot light perspective shadow map texture2d depth rendering
    P_RS_VP(P20, ShadowSpotRasterizerState, VS_Depth_Spot_Mesh, PS_Depth)
    P_RS_VP(P21, ShadowSpotRasterizerState, VS_Depth_Spot_Model, PS_Depth)
    P_RS_VP(P22, ShadowSpotRasterizerState, VS_Depth_Spot_Animation, PS_Depth)

    P_DSS_VTP(P23, Deffered_DepthStencil_State, VS_Tess_Mesh, HS_Mesh, DS_Mesh_Flat, PS_PackGBuffer)
    
    // render reflection mesh by screen space 
    P_DSS_VP(P24, SSR_DSS, VS_SSR_Mesh, SSRRelfectionPS)
    P_DSS_VP(P25, SSR_DSS, VS_SSR_Model, SSRRelfectionPS)
    P_DSS_VP(P26, SSR_DSS, VS_SSR_Animation, SSRRelfectionPS)
    
    P_BS_VP(P27, SSR_BS, ReflectionBlendVS ,ReflectionBlendPS )
}