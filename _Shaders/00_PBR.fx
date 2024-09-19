#include "00_PBR_Light.fx"
#include "00_BRDF.fx"





float3 PBR_Light(float3 N, float3 V, float3 R, float3 L, float3 radiance)
{
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic); // 표면 반사율 구하기
    
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    float NDF;
    float G;
    float3 F; // specular ratio
    float3 kS;
    float3 kD;
    
    float3 H = normalize(V + L);
    
    float NdotL = saturate(dot(N, L));
    float NdotH = saturate(dot(N, H));
    float NdotV = saturate(dot(N, V));
    float VdotH = saturate(dot(V, H));
    float LdotH = saturate(dot(L, H));
        
    // Cook-Torrance BRDF
    NDF = DistributionGGX(N, H, roughness);
    G = GeometrySmith(N, V, L, roughness);
    F = fresnelSchlick(VdotH, F0);
    //F = fresnelSchlick(LdotH, F0);
    
        
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001f;
    float3 specular = numerator / denominator;
        
    kS = F;
    kD = (float3(1.0, 1.0, 1.0) - kS);
    kD *= 1.0f - metallic;
    
    Lo += clamp((kD * albedo / PI + specular) * radiance * NdotL, 0, radiance * NdotL * 15.0f);
    //Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    //Lo += specular * radiance * NdotL;
    //Lo += (kD * albedo / PI) * radiance * NdotL;

    return Lo;
}

float4 DirectionLight(float3 N, float3 V, float3 R)
{
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic); // 표면 반사율 구하기
  
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    // Calculate Light
    float3 L = -GlobalLight.Direction;
    float3 H = normalize(V + L);
    float3 radiance = GlobalLight.Specular.xyz * GlobalLight.Intensity;       
    Lo += PBR_Light(N, V, R, L, radiance);
    
    float4 result = float4(Lo, 1);
    
    return result;
}

float ForwadPointLightShadow(int id, float3 position)
{
    float3 dir = position - PointLights[id].Position;
    float depth;
    float currentDepth = length(dir) / PointLights[id].Range;
    
    return ShadowCube.SampleCmpLevelZero(ShadowSampler, float4(dir, id), currentDepth).r;
}

float3 FowardPointLight(float3 N, float3 V, float3 R, float3 wPosition)
{
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    
    // reflectance equation
    float3 Lo = float3(0.0f, 0.0f, 0.0f);  
    
    // attenuation constat
    const float costantTerm = 1.0f;
    const float linearTerm = 0.022;
    const float quadraticTerm = 0.0019;
    
    for (int i = 0; i < PointLightCount; i++)
    {
        // calculate per-light radiance;               
        float3 L = normalize(PointLights[i].Position.xyz - wPosition);
        float3 H = normalize(V + L);
        float distance = length(PointLights[i].Position.xyz - wPosition);
        // 1/ (c + dist * linearAtt + sqr(dist) * quadAtt) -> point attenuation;
        float attenuation = 1.0f / (costantTerm + linearTerm * distance +
        quadraticTerm * (distance * distance) * 4 * PI) * PointLights[i].Intensity;
        float3 radiance = PointLights[i].Diffuse.xyz * attenuation;      
        // add luminance
        Lo += PBR_Light(N, V, R, L, radiance) * (i < 2 ? ForwadPointLightShadow(i, wPosition) : 1);
    }
    
    float3 result = Lo;    
    return result;
}

float FowardSpotLightShadow(int id, float3 position)
{
    float4 posShadowMap = mul(float4(position, 1.0f), ForwardSpotShadwProjection[id]);

    float3 UVD = posShadowMap.xyz / posShadowMap.w;
    [flatten] // check in shadow clip space
    if (UVD.x < -1.0f || UVD.x > +1.0f ||
        UVD.y < -1.0f || UVD.y > +1.0f ||
        UVD.z < +0.0f || UVD.z > +1.0f)
    {
        return 1;
    }
    UVD.xy = UVD.xy * float2(0.5, -0.5) + 0.5;
    
    return ShadowSpot.SampleCmpLevelZero(ShadowSampler, float3(UVD.xy, id), UVD.z).r;
}

float3 FowardSpotLight(float3 N, float3 V, float3 R, float3 wPosition)
{
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    
    // reflectance equation
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    // attenuation constat
    const float costantTerm = 1.0f;
    const float linearTerm = 0.022;
    const float quadraticTerm = 0.0019;
    
    for (int i = 0; i < SpotLightCount; i++)
    {
        // calculate per-light radiance;               
        float3 L = normalize(SpotLightPBRs[i].Position.xyz - wPosition);
        float3 H = normalize(V + L);
       
        float distance = length(SpotLightPBRs[i].Position.xyz - wPosition);
        // get angle attenuation
        float outCos = cos(SpotLightPBRs[i].OuterAngle / 180.0f * PI);
        float innerCos = cos(SpotLightPBRs[i].InnerAngle / 180.0f * PI);        
        float attRange = innerCos - outCos;
        
        float cosAngle = dot(-L, SpotLightPBRs[i].Direction);
        float conAtt = saturate((cosAngle - outCos) / attRange);
        conAtt *= conAtt;
        // get distance attenuation
        float DistToLightNorm = 1.0f - saturate(distance / SpotLightPBRs[i].Range);
        float Attn = DistToLightNorm * DistToLightNorm;
        // cone attuation * distance attanuation * intensity * light;
        float3 radiance = SpotLightPBRs[i].color.xyz * Attn * conAtt * SpotLightPBRs[i].Intensity;
        // check angle range
        if (cosAngle < outCos)
            radiance = float3(0,0,0);
        
        // add luminance
        Lo += PBR_Light(N, V, R, L, radiance) * (i < 2 ? FowardSpotLightShadow(i, wPosition) : 1);
    }
    
    float3 result = Lo;
    return result;
}