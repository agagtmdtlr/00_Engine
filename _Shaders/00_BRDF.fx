
float sqr(float value)
{
    return value * value;
}
#include "00_Distribution.fx"
#include "00_Fresnel.fx"
#include "00_Geometry.fx"

void BRDFCook_Torrance(inout float NDF, inout float G, inout float3 F,
float3 N, float3 H, float3 L,float3 V, float roughness, float3 F0)
{
    NDF = DistributionGGX(N, H, roughness);
    G = GeometrySmith(N, V, L, roughness);
    F = fresnelSchlick(max(dot(H, V), 0.0), F0);
}

void BRDFCook_Torrance_Irradiance(inout float NDF, inout float G, inout float3 F,
float3 N, float3 H, float3 L, float3 V, float roughness, float3 F0)
{
    NDF = DistributionGGX(N, H, roughness);
    G = GeometrySmith(N, V, L, roughness);
    F = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, roughness);
}


