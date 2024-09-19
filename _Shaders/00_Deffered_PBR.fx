#include "00_Render_PBR.fx"

RasterizerState Deffered_Rasterizer_State;
DepthStencilState Deffered_DepthStencil_State;
BlendState Deffered_Blend_State;

cbuffer CB_Deffered_Desc
{
    float4 defferedProjectValues;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Pack GBuffer
/////////////////////////////////////////////////////////////////////////////////////////////
struct PixelOutuput_GBuffer
{
    float4 Albedo : SV_Target0;
    float4 Metallic : SV_Target1;
    float4 Roughness : SV_Target2;
    float4 AO : SV_Target3;    
    float4 Normal : SV_Target4; 
    float4 Tangent : SV_Target5;
};

PixelOutuput_GBuffer PS_PackGBuffer(MeshOutput input)
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
    viewDir.y = -viewDir.y;
    // if parallax get new texture coords; else use origin texture coords;
    float2 uv = (mapDesc.parallax ? ParallaxOcclusionMapping(input.Uv, viewDir) : input.Uv);
    // then texture sampling with new texture coords
    //albedo = (mapDesc.albedo ? pow(abs(AlbedoMap.Sample(LinearSampler, uv).xyz), 2.2) : float3(0.5, 0.5, 0.5));
	albedo = float3(0.5, 0.5, 0.5);

    metallic = (mapDesc.metal ? MetallicMap.Sample(LinearSampler, uv).r : 0.0f);
    roughness = clamp((mapDesc.rough ? RoughnessMap.Sample(LinearSampler, uv).r : 1.0f), 0.04, 1.0);
    
    ao = (mapDesc.ao ? AOMap.Sample(LinearSampler, uv).r : 1.0f);
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic); // 표면 반사율 구하기
    
    PixelOutuput_GBuffer output;
    output.Albedo = float4(albedo, 1);
    output.Metallic = float4(metallic, F0);
    output.Roughness = float4(roughness, roughness, roughness, roughness);
    output.AO = float4(ao, ao, ao, ao);
    
    //float3 N = (mapDesc.normal ? NormalMapping(input.Normal, input.Tangent, uv) : input.Normal);
	float3 N = input.Normal;
    float3 T = normalize(input.Tangent + dot(input.Tangent, N) * N);
    output.Normal = float4(N, 1);
    output.Tangent = float4(T, 1);
    
    //output.Tangent = float4(T, 1);
    
    return output;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Unpack GBuffer ( albedo, metallic, roughness, ao
/////////////////////////////////////////////////////////////////////////////////////////////
Texture2D GBufferMaps[7];
void UnpackGBuffer(inout float4 position, in float2 screen, out float3 normal, out float3 tangent)
{
    albedo = GBufferMaps[1].Load(int3(position.xy, 0)).xyz;
    metallic = GBufferMaps[2].Load(int3(position.xy, 0)).r;
    roughness = GBufferMaps[3].Load(int3(position.xy, 0)).r;
    ao = GBufferMaps[4].Load(int3(position.xy, 0)).r;
    
    normal = GBufferMaps[5].Load(int3(position.xy, 0)).xyz;
    tangent = GBufferMaps[6].Load(int3(position.xy, 0)).xyz;
    
    // get projection value for calculate world position
    float2 xy = defferedProjectValues.xy;
    float z = defferedProjectValues.z;
    float w = defferedProjectValues.w;
    
    // D24S8==R24G8_TYPELESS :: transform linear depth
    float depth = GBufferMaps[0].Load(int3(position.xy, 0)).r;
    float linearDepth = z / (depth + w);
    
    //get world position;
    position.xy = screen.xy * xy * linearDepth;
    position.z = linearDepth;
    position.w = 1.0f;
    // 깊이 버퍼에 저장된 값은 
    // 월드 공간 위치 값을 행렬의 세번째 열과 내적시킨 결과다.
    // 깊이 버퍼를 선형 깊이로 다시 변환하려면 
    // 전반사 정도를 계산했던 방법을 거꾸로 수행하기만 하면 된다.
    position = mul(position, ViewInverse);    
    
}
/////////////////////////////////////////////////////////////////////////////////////////////
// DirectionalLight
/////////////////////////////////////////////////////////////////////////////////////////////
// cpu draw call index가 아닌 정점을 가지고 드로우콜하는데 
//이때 정점의 아이디 0,1,2,3을가지고 접근활용한다.
// vertex Id를 가지고 사용활용한다.
static const float2 NDC[4] = 
    { 
    float2(-1, +1), 
    float2(+1, +1), 
    float2(-1, -1), 
    float2(+1, -1) };

struct VertexOutput_Directional
{
    float4 Position : SV_Position;
    float2 Screen : Position1;
    float2 UV : UV;
};
// deffered vertex shader :: 단순한 id로 사용 vertexbuffer를 실제로 사용하지 않는다.
VertexOutput_Directional VS_Directional(uint id : SV_VertexID)
{
    //Point List   
    VertexOutput_Directional output;
    
    output.Position = float4(NDC[id], 0, 1);
    output.Screen = output.Position.xy;
    output.UV = output.Screen * float2(0.5, -0.5) + 0.5;
    return output;
}

float4 PS_Deffered_Directional(VertexOutput_Directional input) : SV_Target
{
    float4 position = input.Position;
    
    float3 N;
    float3 T;    
    UnpackGBuffer(position, input.Screen, N, T);    
    float3 V = normalize(ViewPosition() - position.xyz);
    float3 R = reflect(-V, N);
    
    float4 sPosition = mul(position, ShadowView);
    sPosition = mul(sPosition, ShadowProjection);   
    
    float4 result = 0;
    
    //result += DirectionLight(N, V, R) * Shadow_Factor_PCF(sPosition);
	result += DirectionLight(N, V, R) * Shadow_Factor(sPosition);

    
    result.a = 1;    
    return result;    
}

///////////////////////////////////////////////////////////////////////
// Ambient Light; with Image based light
////////////////////////////////////////////////////////////
float4 PS_IBLAmibeintLight(VertexOutput_Directional input) : SV_Target
{
    float ssao = SSAOMap.Sample(LinearSampler, input.UV).r;
    
    float4 position = input.Position;
    float3 N;
    float3 T;
    UnpackGBuffer(position, input.Screen, N, T);    
    float3 V = normalize(ViewPosition() - position.xyz);
    float3 R = reflect(-V, N);
   
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
        
        //prefilteredColor = ReflectionCube.SampleLevel(
        //LinearSampler, R,  MAX_REFLECTION_LOD).rgb;
        
        brdf = BRDFMap.Sample(LinearSampler, float2(max(dot(N, V), 0.0), roughness)).rg;
        specular = prefilteredColor * (fresnel * brdf.x + brdf.y);
        diffuse = 0;
        ambient = (kD * diffuse + specular) * min(ao, ssao);
    }
    if (mapDesc.irradiance == 0)
    {
        ambient = 0.03 * albedo * min(ao, ssao);
    }
    
    return float4(ambient.xyz, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// PointLighting
/////////////////////////////////////////////////////////////////////////////////////////////
cbuffer CB_Deffered_PointLight
{
    float PointLight_TessFactor;
    float3 CB_Deffered_PointLight_Padding;
    
    matrix ShadowCubeProjection[MAX_POINT_LIGHTS];
    
    matrix PointLight_Projection[MAX_POINT_LIGHTS];
    PointLight PointLight_Deffered[MAX_POINT_LIGHTS];
};

float4 VS_PointLights() : Position
{
    return float4(0, 0, 0, 1);
}

struct CHullOutput_PointLights
{
    float Edge[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

CHullOutput_PointLights CHS_PointLights()
{
    CHullOutput_PointLights output;
    output.Edge[0] = output.Edge[1] = output.Edge[2] = output.Edge[3] = PointLight_TessFactor;
    output.inside[0] = output.inside[1] = PointLight_TessFactor;
    return output;
}

struct HullOutput_PointLights
{
    float4 Direction : Position;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CHS_PointLights")]
HullOutput_PointLights HS_PointLights(uint id : SV_PrimitiveID) // SV_PrimitiveID
{
    // 구를 만들 방향을 정의
    // 반구 모양에 상응하는 양의 평타도 상수 값
    float4 direction[2] = { float4(1, 1, 1, 1), float4(-1, 1, -1, 1) };
    HullOutput_PointLights output;
    output.Direction = direction[id % 2];   
    // 네개의 제어 지점에 대한 패치를 생성한다.
    return output;
}

struct DomainOutput_PointLights
{
    float4 Position : SV_Position;
    float4 oPosition : Position1;
    float2 Screen : Uv;
    uint PrimitiveID : Id;
};

[domain("quad")]
DomainOutput_PointLights DS_PointLights(CHullOutput_PointLights input, float2 uv : SV_DomainLocation,
    const OutputPatch<HullOutput_PointLights, 4> quad, uint id : SV_PrimitiveID)
{
    // uv -> [-1,1]의 NDC공간상의 좌표로 변혼한
    float2 clipSpace = uv.xy * 2.0f - 1.0f;
    
    // 중심에서 가장 먼 지점의 절댓값 거리 계싼
    float2 clipSpaceAbs = abs(clipSpace.xy);
    // 두 값중 큰걸 구의 넓이로 사용하겠다.
    float maxLength = max(clipSpaceAbs.x, clipSpaceAbs.y);
    
    // (maxLength - 1.0f) 가운데에서 가장 크가 -1
    float3 direction = normalize(float3(clipSpace.xy, (maxLength - 1.0f)) * quad[0].Direction.xyz);
    float4 position = float4(direction, 1.0f);

    DomainOutput_PointLights output;
    output.oPosition = position;
    output.Position = mul(position, PointLight_Projection[id / 2]);
    output.Screen = output.Position.xy / output.Position.w;
    output.PrimitiveID = id / 2;
    
    return output;
}
// point light mesh check
float4 PS_PointLights_Debug(DomainOutput_PointLights input) : SV_Target
{
    return float4(0, 1, 0, 1);
}



float4 ComputeDefferedPointLight(float3 N, float3 V, float3 R, float3 wPosition, uint id)
{
    // result
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    // BRDF Factor
    
    PointLight desc = PointLight_Deffered[id];
    
    float distance = length(desc.Position.xyz - wPosition);
    [flatten]
    if(distance > desc.Range  )
        return float4(0, 0, 0, 0);
    
    float3 L = normalize(desc.Position.xyz - wPosition);
    
    float3 H = normalize(V + L);
    
    const float costantTerm = 1.0f;
    const float linearTerm = 0.022;
    const float quadraticTerm = 0.0019;
    //float attenuation = 1.0f / (distance * distance) * desc.Intensity;
    
    float attenuation = 1.0f / (costantTerm + linearTerm * distance +
    quadraticTerm * (distance * distance) * 4 * PI) * desc.Intensity;
    
    //float attenuation = (1.0f / (distance * distance)) * desc.Intensity;
    
    float LightRadiusMask = pow( 
    saturate( 1 - pow((distance * distance) / (desc.Range * desc.Range) , 2) ), 2);
    
    float3 radiance = desc.Diffuse.xyz * attenuation ;
    
    Lo += PBR_Light(N, V, R, L, radiance);            
   
    float4 result = float4(Lo, 1);
    
    return result;
}


float ShadowCube_Factor(float4 position, uint index)
{
    
    
    PointLight desc = PointLight_Deffered[index];
    float3 dir = position.xyz - desc.Position;
    
    float z = ShadowCubeProjection[index]._43;
    float w = -ShadowCubeProjection[index]._33;
    
    // D24S8==R24G8_TYPELESS :: transform linear depth
    float depth;
    depth = ShadowCube.Sample(LinearSampler, float4(dir, index)).r;
    //float linearDepth = z / (depth + w);
    
    float currentDepth = length(dir);
    float shadowDepth = depth * desc.Range; // range is far plane
    float bias = -0.04;
    
    currentDepth = length(dir) / desc.Range;
    return ShadowCube.SampleCmpLevelZero(ShadowSampler, float4(dir, index), currentDepth).r;
    
    float factor = (float) (shadowDepth >= currentDepth - bias);
    
    return factor;   
}

float4 PS_Deffered_PointLights(DomainOutput_PointLights input) : SV_Target
{
    float4 position = input.Position;
    
    float3 N;
    float3 T;
    UnpackGBuffer(position, input.Screen, N, T);
    
    float3 V = normalize(ViewPosition() - position.xyz);
    float3 R = reflect(-V, N);
    
    float4 result = 0;
    if (input.PrimitiveID < 2) // max point light shadow map count is 2;
        result += ComputeDefferedPointLight(N, V, R, position.xyz, input.PrimitiveID) * ShadowCube_Factor(position, input.PrimitiveID);
    else
        result += ComputeDefferedPointLight(N, V, R, position.xyz, input.PrimitiveID);
    
    result.a = 1;
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPOT LiGHT DEFFERED
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
cbuffer CB_Deffered_SpotLight
{
    float SpotLight_TessFactor;
    float SpotLight_Bias;
    float2 CB_Deffered_SpotLight_Padding;
    
    float4 SpotLight_Angle[MAX_SPOT_LIGHTS];
    matrix SpotLight_Projection[MAX_SPOT_LIGHTS];
    SpotLightPBR SpotLight_Deffered[MAX_SPOT_LIGHTS];
};

cbuffer CB_Deffered_SpotLight_PS
{
    matrix SpotLight_ShadowMap[MAX_SPOT_LIGHTS];    
};

float4 VS_SpotLights() : Position
{
    return float4(0, 0, 0, 1);
}

struct ConstantHullOutput_SpotLights
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

ConstantHullOutput_SpotLights ConstantHS_SpotLights()
{
    ConstantHullOutput_SpotLights output;
    
    output.Edges[0] = output.Edges[1] = output.Edges[2] = output.Edges[3] = SpotLight_TessFactor;
    output.Inside[0] = output.Inside[1] = SpotLight_TessFactor;
   
    return output;
}

struct HullOutput_SpotLights
{
    float4 Position : Position;
};

// horn 하나만 그린다.
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS_SpotLights")]
HullOutput_SpotLights HS_SpotLights()
{
    HullOutput_SpotLights output;

    output.Position = float4(0, 0, 0, 1);

    return output;
}

struct DomainOutput_SpotLights
{
    float4 Position : SV_Position;
    float2 Screen : Uv;
    uint PrimitiveID : Id;
};

[domain("quad")]
DomainOutput_SpotLights DS_SpotLights(ConstantHullOutput_SpotLights input, float2 uv : SV_DomainLocation,
    const OutputPatch<HullOutput_SpotLights, 4> quad, uint id : SV_PrimitiveID)
{
    float c = SpotLight_Angle[id].x;
    float s = SpotLight_Angle[id].y;
    
    // uv를 클립 공간으로 변환
    float2 clipSpace = uv.xy * float2(2, -2) + float2(-1, 1);
    
    // uv에 대한 버텍스 오프셋 계산 : 밑에 둘레
    float2 clipSpaceAbs = abs(clipSpace.xy);
    float maxLength = max(clipSpaceAbs.x, clipSpaceAbs.y);
    
    // 스포트라이트 메쉬에 필요한 상수값
    float cylinder = 0.2f;
    float expentAmount = (1.0f + cylinder); // 부피
    
    // 원뿔 버텍스를 메시 엣지로 강제 변환
    float2 clipSpaceCylAbs = saturate(clipSpaceAbs * expentAmount);
    float maxLengthCapsule = max(clipSpaceCylAbs.x, clipSpaceCylAbs.y);
    float2 clipSpaceCyl = sign(clipSpace.xy) * clipSpaceCylAbs; // 방향 만들기
    
    // 반구 위치를 가장자리의 원뿔 버텍스로 변화
    float3 halfSpherePosition = normalize(float3(clipSpaceCyl.xy, 1.0f - maxLengthCapsule));
    // 구를 원뿔 밑면 크기로 스케일 조정
    halfSpherePosition = normalize(float3(halfSpherePosition.xy * s, c));
    
    // 월뿐 버텍스의 오프셋 계산 원뿔 밑면은 0
    float cylOffsetZ = saturate((maxLength * expentAmount - 1.0f) / cylinder);
    
    // 원뿔 버텍스를 최종 위치로 오프셋
    float4 position = 0;
    position.xy = halfSpherePosition.xy * (1.0f - cylOffsetZ);
    position.z = halfSpherePosition.z - cylOffsetZ * c;
    position.w = 1.0f;
    
    // 투영 변환 및 uv좌표값 생성
    DomainOutput_SpotLights output;
    output.Position = mul(position, SpotLight_Projection[id]);
    output.Screen = output.Position.xy / output.Position.w;
    output.PrimitiveID = id;
    
    return output;
}

float4 ComputeDefferedSpotLight(float3 N, float3 V, float3 R, float3 wPosition, uint id)
{
   
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    
    float outCos = SpotLight_Angle[id].x;
    float attRange = SpotLight_Angle[id].z;
    SpotLightPBR desc = SpotLight_Deffered[id];   
    float3 L = normalize(desc.Position.xyz - wPosition);
    float3 H = normalize(V + L);
    float cosAng = dot(-L, desc.Direction);
    float conAtt = saturate((cosAng - outCos) / attRange);
    conAtt *= conAtt;
    float distance = length(desc.Position.xyz - wPosition);
    
    float DistToLightNorm = 1.0f - saturate(distance / desc.Range);
    float Attn = DistToLightNorm * DistToLightNorm;
    
    float3 radiance = desc.color.xyz * Attn * conAtt * desc.Intensity;  
    
    if(cosAng < outCos)
        return float4(0, 0, 0, 1);
    
    Lo += PBR_Light(N, V, R, L, radiance);
    
    
    float4 result = float4(Lo, 1);
    
    return result;
}

// Shadow PCF calculation helper function
float ShadowSpot_Factor(float3 wPosition,uint id)
{
    // Transform the world position to shadow projected space;    
    float4 posShadowMap = mul(float4(wPosition, 1.0), SpotLight_ShadowMap[id]);
    
    // Transform the position to shadow clip space;
    float3 UVD = posShadowMap.xyz / posShadowMap.w;
    // Convert to shadow Map UV values;
    UVD.xy = UVD.xy * float2(0.5, -0.5) + 0.5;
	// Compute the hardware PCF value
    
    return ShadowSpot.SampleCmpLevelZero(ShadowSampler, float3(UVD.xy, id), UVD.z).r;
}

float4 PS_Deffered_SpotLights_Debug(DomainOutput_SpotLights input) : SV_Target
{
    return float4(1, 0, 0, 1);
}
float4 PS_Deffered_SpotLights(DomainOutput_SpotLights input) : SV_Target
{
    float4 position = input.Position;
    
    float3 N;
    float3 T;
    UnpackGBuffer(position, input.Screen, N, T);
    float3 V = normalize(ViewPosition() - position.xyz);
    float3 R = reflect(-V, N);   
    
    float4 result = 0;
    result += ComputeDefferedSpotLight(N, V, R, position.xyz, input.PrimitiveID) * ShadowSpot_Factor(position.xyz, input.PrimitiveID);
    //result += ComputeDefferedSpotLight(N, V, R, position.xyz, input.PrimitiveID);
    
    result.a = 1;
    return result;
}
