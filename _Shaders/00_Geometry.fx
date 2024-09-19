float G1 (float k, float x)
{
    return x / (x * (1 - k) + k);
}


///////////////////////////////////////////////////////////////////////

float GeometrySchlickGGX(float val, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    
    float nom = val;
    float denom = val * (1.0f - k) + k;
    
    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

float GeomtrySchlicGGXIBL(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0f;
    
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    
    return nom / denom;
}

float GeometrySmithIBL(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2 = GeomtrySchlicGGXIBL(NdotV, roughness);
    float ggx1 = GeomtrySchlicGGXIBL(NdotL, roughness);
    
    return ggx1 * ggx2;
}
