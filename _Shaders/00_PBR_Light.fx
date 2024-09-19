Texture2D AlbedoMap;
Texture2D MetallicMap;
Texture2D RoughnessMap;
Texture2D AOMap;
Texture2D NormalMap;
Texture2D DisplacementMap;
Texture2D ParallaxMap;

Texture2D SSAOMap;

Texture2DArray AlbedoMapArray;
Texture2DArray MetallicMapArray;
Texture2DArray RoughnessMapArray;
Texture2DArray AOMapArray;
Texture2DArray NormalMapArray;
Texture2DArray DisplacementMapArray;
Texture2DArray ParallaxMapArray;

// IBL
TextureCube IrradianceCube;
TextureCube ReflectionCube;
Texture2D BRDFMap;

// Dynamic reflectionMap :: use to water
Texture2D ReflectionMap;
Texture2D RefractionMap;

TextureCube SkyCubeMap;

cbuffer Material
{
    float3 albedo;
    float metallic;
    //
    float roughness;
    float ao;
    float pad;
    uint reverse;
    //
    float3 ambient;
    float padding;
    //
    float3 fresnel;
    float padding3;
    
    float3 irradiance;
    float pad2;
};

float PLightDist;

struct MapDesc
{
    uint albedo;
    uint metal;
    uint rough;
    uint ao;
    uint normal;
    uint irradiance;
    uint reflection;
    uint displacement;
    uint parallax;
    float height_scale;
    float2 texOffset;
};

cbuffer CB_MapDesc
{
    MapDesc mapDesc;
};

////////////////////////////////////////////////////////////////////////////////////
// Directional Light Desc
////////////////////////////////////////////////////////////////////////////////////
struct LightDesc
{
    float4 Ambient;
    float4 Specular;
    float3 Direction;
    float Intensity;
    float3 Position;
};

cbuffer CB_Light
{
    LightDesc GlobalLight;
};

/////////////////////////////////////////////////////////////////////////////
// PointLightDesc
/////////////////////////////////////////////////////////////////////////
#define MAX_POINT_LIGHTS 256
#define MAX_POINT_LIGHTS_SHADOW 4

struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;
    
    float3 Position; //point light의 위치
    float Range; // point light의 위치
    
    float Intensity;
    float3 Padding;
};
cbuffer CB_PointLights
{
    uint PointLightCount;
    float3 CB_PointLights_Padding;
    
    PointLight PointLights[MAX_POINT_LIGHTS];
};
///////////////////////////////////////////////////////////////////////////////////////
// Spot Light Desc
/////////////////////////////////////////////////////////////////////////////
#define MAX_SPOT_LIGHTS 256
struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;
    
    float3 Position; 
    float Range;
    
    float3 Direction;
    float Angle;
    
    float Intensity;
    float3 Padding;
};
cbuffer CB_SpotLights
{
    uint SpotLightCount;
    float3 CB_SpotLights_Padding;
    
    SpotLight SpotLights[MAX_SPOT_LIGHTS];
};

struct SpotLightPBR // USE SPOT LIGHT STRUCTURE FOR PBR
{
    float3 Position;
    float Range;
    
    float3 Direction;
    float OuterAngle;
    
    float3 color;
    float InnerAngle;
    
    float Intensity;
    float3 pad;
};

cbuffer CB_SpotLights_PBR
{
    uint SpotLightPBRCount;
    float3 CB_SpotLightsPBR_Padding;
    
    SpotLightPBR SpotLightPBRs[MAX_SPOT_LIGHTS];
};

cbuffer CB_ShadowSpotGen // USE SPOT LIGHT SPACE DEPTH RENDER
{
    matrix ShadowSpotGenMatrix;
    float3 ShadowSpotPosition;
    float ShadowSpotRange;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////
float3 NormalMapping(float3 normal, float3 tangent, float2 uv)
{
    float3 bump = NormalMap.Sample(LinearSampler, uv).rgb;    
    bump = bump * 2.0f - 1.0f;
    
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    float3 binormal = normalize(cross(normal, tangent));
    
    float3x3 TBN = float3x3(tangent, binormal, normal);
    normal = mul(bump, TBN);
    
    return normal;
}

float3 UvToVector3(float2 uv)
{
    uv.x -= 0.25; // -0.5 ~ 0.5
    uv.x /= 0.5; // -1 ~ 1
    uv *= PI; // x : [-pi,pi] , y : [0,pi]
    float3 dir =
    {
        sin(uv.y) * cos(uv.x),
		cos(uv.y),
		sin(uv.y) * sin(uv.x),
    };
    return normalize(dir);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parallax Mapping
//////////////////////////////////////////////////////////////////////////////////////////////////////////

float GetParallaxHeight(float2 uv)
{
    return abs(1 - ParallaxMap.Sample(LinearSampler, uv).r);
}
float GetGradParallaxHeight(float2 uv, float2 dx, float2 dy)
{
    return abs(1 - ParallaxMap.SampleGrad(LinearSampler, uv, dx, dy).r);
}


float2 ParallaxMapping(float2 uv, float3 viewDir)
{
    float height = ParallaxMap.Sample(LinearSampler, uv).r;
    float2 p = viewDir.xy / viewDir.z * (height * 0.1);
    
    return uv - p;
}

float2 SteepParallaxMapping(float2 uv, float3 viewDir)
{
    //improve algorithm 
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    
    
    // number of depth layers
    float numLayers = lerp(maxLayers, minLayers, max(dot(float3(0, 0, 1), viewDir), 0.0));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer ( from vector P)
    float2 P = viewDir.xy * mapDesc.height_scale;
    float2 deltaUv = P / numLayers;
    
    // get initial values
    float2 currentUv = uv;
    float currentDepthMapValue = ParallaxMap.Sample(LinearSampler, uv).r;
    //currentDepthMapValue = 1 - currentDepthMapValue;
    [loop]
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentUv -= deltaUv;
        // get depthmap value at current texxture coordinates
        currentDepthMapValue = ParallaxMap.Sample(LinearSampler, currentUv).r;
        // get depth of next layere
        currentLayerDepth += layerDepth;
    }
    
    return currentUv;
}
///////////////////////////////// Parallax Reference////////////////////////////////////////////////
// https://developer.amd.com/wordpress/media/2012/10/I3D2006-Tatarchuk-POM.pdf
//////////////////////////////////////////////////////////////////////////////////////////////////////////
float2 ParallaxOcclusionMapping(float2 uv, float3 viewDir)
{
    //improve algorithm 
    const float minLayers = 60.0;
    const float maxLayers = 60.0;
    
    float2 dx = ddx(uv);
    float2 dy = ddy(uv);  
    
    // number of depth layers 
    // 비스듬할수록 더 많은 교차 샘플링을 시도한다.
    float numLayers = 
    lerp(maxLayers, minLayers, abs(dot(float3(0, 0, 1), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer
    float2 ray =
    viewDir.xy / viewDir.z 
    * mapDesc.height_scale ;
    float2 deltaTexCoords = ray / numLayers; // ray Interval
    
    // get initial values
    float2 currentTexCoords = uv;
    float currentDepthMapValue = GetParallaxHeight(currentTexCoords);
    //currentDepthMapValue = 1 - currentDepthMapValue;
    [unroll(60)]
    while (currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texxture coordinates
        currentDepthMapValue = GetGradParallaxHeight(currentTexCoords, dx, dy);
        // get depth of next layere
        currentLayerDepth += layerDepth;
        
    }
    
    // get texture coordinates before collision ( reverse operations)
    float2 prevTexCoords = currentTexCoords + deltaTexCoords;    
    // get depth after and before collision for linear interpolation
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = GetParallaxHeight(prevTexCoords)
    - currentLayerDepth + layerDepth;    
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    //float2 finalUv = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
    float2 finalUv = 
    currentTexCoords + weight * (prevTexCoords - currentTexCoords);
    return finalUv;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Shadow Rendering
/////////////////////////////////////////////////////////////////////////////////
Texture2D ShadowMap; // 깊이 맵 우리가 볼거
TextureCubeArray ShadowCube; // for point light shadow
Texture2DArray ShadowSpot;
SamplerComparisonState ShadowSampler;
RasterizerState ShadowSpotRasterizerState;

cbuffer CB_Shadow
{
    matrix ShadowView;
    matrix ShadowProjection;
    
    float2 ShadowMapSize;
    float ShadowBias;
    
    uint ShadowQuality;
};

cbuffer CB_ForwardSpotLightShadow
{
    matrix ForwardSpotShadwProjection[2];
};

///////////////////////////////////////////////////////////////////////////////////
// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
///////////////////////////////////////////////////////////////////////////////////
float Shadow_Factor(float4 position)
{
    position.xyz /= position.w; // Light NDC transform
    [flatten]
    if (position.x < -1.0f || position.x > +1.0f ||
        position.y < -1.0f || position.y > +1.0f ||
        position.z < +0.0f || position.z > +1.0f)
    {
        return 1;
    }
    position.x = position.x * 0.5f + 0.5f;
    position.y = -position.y * 0.5f + 0.5f;
    
    // 피터 패닝 보정값 shadowbias 
    float z = position.z - 1.1f; // ShadowBias :: shadow acne problem solution
    float shadow = 0;
    float2 texelSize = 1.0 / ShadowMapSize;

	float pcfDepth = ShadowMap.Sample(LinearSampler, position.xy ).r;
	//shadow += (float)(pcfDepth >= z); //  그림자 o : 0:
	shadow = 1;
    //for (int x = -1; x <= 1;x++)
    //{
    //    for (int y = -1; y <= 1; y++)
    //    {
    //        float pcfDepth = ShadowMap.Sample(LinearSampler, position.xy + float2(x, y) * texelSize).r;
    //        shadow += (float) (pcfDepth >= z); //  그림자 o : 0:
    //    }
    //}
    
    //return shadow / 9.0;    
	return shadow;

}
float Shadow_Factor_PCF(float4 position)
{
    position.xyz /= position.w; // Light NDC transform
    [flatten]
    if (position.x < -1.0f || position.x > +1.0f ||
        position.y < -1.0f || position.y > +1.0f ||
        position.z < +0.0f || position.z > +1.0f)
    {
        return 1;
    }
    position.x = position.x * 0.5f + 0.5f;
    position.y = -position.y * 0.5f + 0.5f;
    
     
    float depth = 0;
    // 피터 패닝 보정값 shadowbias 
    float z = position.z - ShadowBias;
    float factor = 0;
    depth = position.z;
        
    float2 size = 1.0f / ShadowMapSize; // 해상동 한픽셀크기
    float2 offsets[] =
    {
        float2(-size.x, -size.y), float2(0.0f, -size.y), float2(+size.x, -size.y),
            float2(-size.x, 0.0f), float2(0.0f, 0.0f), float2(+size.x, 0.0f),
            float2(-size.x, +size.y), float2(0.0f, +size.y), float2(+size.x, +size.y),
    };
        
    float2 uv = 0;
    float sum = 0;
    //for (int i = 0; i < 9;i++)
    //    sum += ShadowMap.SampleCmpLevelZero(ShadowSampler, position.xy + offsets[i], depth).r;
    //sum /= 9.0f;
	ShadowMap.SampleCmpLevelZero(ShadowSampler, position.xy, depth).r;
    return sum;    
    //return ShadowMap.SampleCmpLevelZero(ShadowSampler, position.xy, depth).r;
}

float LinearizeDepth(float depth , float near_plane, float far_plane)
{
    float z = depth * 2.0f - 1.0f; // Back to NDC;
    return (2.0f * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}


float4 PS_Shadow(float4 position, float4 color) 
{
    // model color :: AllLight    
    // position =  sPosition : Light방향에서 wvp
    
    position.xyz /= position.w; // Light NDC transform
    
    // Light의 NDC 공간을 벗어난 곳이라면 그림자가 없는 공간이다.
    [flatten]
    if (position.x < -1.0f || position.x > +1.0f ||
        position.y < -1.0f || position.y > +1.0f ||
        position.z < +0.0f || position.z > +1.0f)
    {
        return color;
    }
    
    
    // ndc -> uv 좌표로 변화
    position.x = position.x * 0.5f + 0.5f;
    position.y = -position.y * 0.5f + 0.5f;
    
    float depth = 0;
    // 피터 패닝 보정값 shadowbias 
    float z = position.z - ShadowBias;
    float factor = 0;
    
    if (ShadowQuality == 0)
    {
        // detph를 r32로 썻으니깐 r성분을 읽어야 한다.
        depth = ShadowMap.Sample(LinearSampler, position.xy).r;
        // detph >= z 그림자 안지는곳
        // compare with shadow depth and element surface depth
        // 앞에껄 뒤집으면 된다.
        factor = (float) (depth >= z); // 0 or 1 false true  
    }
    else if (ShadowQuality == 1)
    {
        depth = position.z;
        //SampleBias[오차 보정]
        //SampCmp는 migmap이 존재하는 경우에 사용한다.
        factor = ShadowMap.SampleCmpLevelZero(ShadowSampler, position.xy, depth);
    }
    else if (ShadowQuality == 2) //PCF + Blur
    {
        depth = position.z;
        
        float2 size = 1.0f / ShadowMapSize; // 해상동 한픽셀크기
        float2 offsets[] =
        {
            float2(-size.x, -size.y), float2(0.0f, -size.y), float2(+size.x, -size.y),
            float2(-size.x, 0.0f), float2(0.0f, 0.0f), float2(+size.x, 0.0f),
            float2(-size.x, +size.y), float2(0.0f, +size.y), float2(+size.x, +size.y),
        };
        
        float2 uv = 0;
        float sum = 0;
        
        [unroll(9)]
        for (int i = 0; i < 9; i++)
        {
            uv = position.xy + offsets[i];
            sum += ShadowMap.SampleCmpLevelZero(ShadowSampler, uv, depth).r;
        }
        
        factor = sum / 9.0f;
    }
   
    
    // 색이 자연스럽게 들어간다.
    factor = saturate(factor + depth);
    return float4(color.rgb * factor, 1);
}