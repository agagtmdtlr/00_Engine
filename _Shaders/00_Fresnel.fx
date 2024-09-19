
////////////////////////////////////////'
// opengl tutorial
////////////////////////////////////////'
float3 fresnelSchlick(float cosTheta, float3 F0)
{
    float f90 = saturate(50.0 * dot(F0, 0.33));
    return F0 + (1.0f - F0) * pow(max(1.0f - cosTheta, 0.0f), 5.0f);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{    
    float r = 1.0f - roughness;
    return F0 + (max(float3(r, r, r), F0) - F0) * pow(max(1.0f - cosTheta, 0.0f), 5.0f);
}

