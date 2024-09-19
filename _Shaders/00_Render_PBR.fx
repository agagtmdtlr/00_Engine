// MehsOutput : 00_Global.fx
// Reflection : 00_Glabal.fx
// Material resource : 00_PBR_Light.fx
// Shadow : 00_PBR_Light.fx
#include "00_Global.fx"
#include "00_PBR.fx"


struct VertexMesh
{
    float4 Position : Position;
    float2 Uv : Uv;
    float3 Normal : Normal;
    float3 Tangent : Tangent;
    
    matrix Transform : Inst1_Transform;
};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
struct DepthOutput
{
    float4 Position : SV_Position;
    // 라이팅 방향에서의 wvp를 변환한 값이 저장될 값 ( 빛의 방향에서의 투영이니깐 )
    float4 sPosition : Position1;
    float3 wPosition : Position2;
};

struct DepthSpotOuput_PS
{
    float4 RT : SV_Target;
    float depth : SV_Depth;
};

///////////////////////////////////////////////////////////////////////////////////
// 여기는 우리가 눈으로 보기위해서 RTV에 직접 깊이를 그려주는 것이다.
// DSV는 PS를 거치지 않고 바로 OM으로 넘어가 그려진다.
float4 PS_Depth(DepthOutput input) : SV_Target
{
    // NDC 공간으로 변환하기 위해서 z값을 동차로 나눈다.
    float depth = input.Position.z / input.Position.w;
    
    return float4(depth, depth, depth, 1.0f);
}

DepthSpotOuput_PS PS_Depth_Spot(DepthOutput input) : SV_Target
{
    DepthSpotOuput_PS output;
    float depth = saturate(length(input.wPosition - ShadowSpotPosition) / ShadowSpotRange);
    output.RT = float4(depth, depth, depth, 1.0f);
    output.depth = depth;
    return output;
}

#define VS_DEPTH_GENERATE \
output.Position = WorldPosition(input.Position); \
output.wPosition = output.Position.xyz; \
output.Position = mul(output.Position, ShadowView); \
output.Position = mul(output.Position, ShadowProjection); \
\
output.sPosition = output.Position;

#define VS_DEPTH_SPOT_GENERATE \
output.Position = WorldPosition(input.Position); \
output.wPosition = output.Position.xyz; \
output.Position = mul(output.Position, ShadowSpotGenMatrix); \
output.sPosition = output.Position;

///////////////////////////////////////////////////////////////////////////////

#define VS_GENERATE \
output.oPosition = input.Position.xyz; \
output.Position = WorldPosition(input.Position); \
output.wPosition = output.Position.xyz; \
output.gPosition = output.Position; \
\
output.Position = ViewProjection(output.Position); \
output.wvpPosition = output.Position; \
output.wvpPosition_Sub = output.Position; \
\
output.sPosition = WorldPosition(input.Position); \
output.sPosition = mul(output.sPosition, ShadowView); \
output.sPosition = mul(output.sPosition, ShadowProjection); \
\
output.Normal = WorldNormal(input.Normal); \
output.Tangent = WorldTangent(input.Tangent); \
output.Uv = input.Uv; \
\
output.Culling.x = mul(float4(output.wPosition, 1), Culling[0]); \
output.Culling.y = mul(float4(output.wPosition, 1), Culling[1]); \
output.Culling.z = mul(float4(output.wPosition, 1), Culling[2]); \
output.Culling.w = mul(float4(output.wPosition, 1), Culling[3]); \
\
output.Clipping.x = mul(float4(output.wPosition, 1), Clipping); \
output.Clipping.y = 0.0f; \
output.Clipping.z = 0.0f; \
output.Clipping.w = 0.0f;

#define VS_SSR_GENERATE \
output.Position = WorldPosition(input.Position); \
output.ViewPosition = mul(output.Position, View); \
output.Position = ViewProjection(output.Position); \
\
output.ViewNormal = WorldNormal(input.Normal); \
output.ViewNormal = mul(output.ViewNormal, (float3x3)View); \
output.ViewTangent= WorldTangent(input.Tangent); \
output.ViewTangent = mul(output.ViewTangent, (float3x3)View); \
output.csPos = output.Position.xyz / output.Position.w; 
// culling 각면에 대한 d 값

#define VSTS_GENERATE \
output.Position = input.Position; \
\
output.Normal = input.Normal; \
output.Tangent = input.Tangent; \
output.Uv = input.Uv; \
output.Transform = input.Transform;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SetMeshWorld(inout matrix world, VertexMesh input)
{
    world = input.Transform;
}


MeshOutput VS_Mesh(VertexMesh input)
{
    MeshOutput output;   
    SetMeshWorld(World, input);        
    VS_GENERATE
    
    return output;
}


MeshOutput VS_PreRender_Reflection_Mesh(VertexMesh input)
{
    MeshOutput output = VS_Mesh(input);
    
    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, Reflection);
    output.Position = mul(output.Position, Projection);
    
    //output.wvpPosition_Sub = output.Position;
    
    return output;
}

SSR_MeshOutput VS_SSR_Mesh(VertexMesh input)
{
    SSR_MeshOutput output;
    SetMeshWorld(World, input);
    VS_SSR_GENERATE
    
    return output;
}

DepthOutput VS_Depth_Mesh(VertexMesh input)
{
    DepthOutput output;
    
    SetMeshWorld(World, input);    
    VS_DEPTH_GENERATE
    
    return output;
}

DepthOutput VS_Depth_Spot_Mesh(VertexMesh input)
{
    DepthOutput output;
    SetMeshWorld(World, input);
    VS_DEPTH_SPOT_GENERATE
    
    return output;
}

MeshTessVertexOutput VS_Tess_Mesh(VertexMesh input)
{
    MeshTessVertexOutput output;    
    VSTS_GENERATE
    return output;
}

CHullOutput_Mesh CHS_Mesh(InputPatch<MeshTessVertexOutput, 3> input)
{
    CHullOutput_Mesh output;
    output.Edge[0] = 64;
    output.Edge[1] = 64;
    output.Edge[2] = 64;
    output.Inside = 64;
    return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CHS_Mesh")]
HullOutput_Mesh HS_Mesh(InputPatch<MeshTessVertexOutput, 3> input, uint pointID : SV_OutputControlPointID)
{
    HullOutput_Mesh output;
    output.Position = input[pointID].Position;
    output.Normal = input[pointID].Normal;
    output.Tangent = input[pointID].Tangent;
    output.Uv = input[pointID].Uv;
    output.Transform = input[pointID].Transform;
    return output;
}

float3 GetTrianglePoint(float3 p0,float3 p1, float3 p2 , float3 uvw)
{
    return p0 * uvw.x + p1 * uvw.y + p2 * uvw.z;
}
float2 GetTrianglePoint(float2 p0, float2 p1, float2 p2, float3 uvw)
{
    return p0 * uvw.x + p1 * uvw.y + p2 * uvw.z;
}


[domain("tri")]
MeshOutput DS_Mesh(
CHullOutput_Mesh chinput, 
const OutputPatch<HullOutput_Mesh , 3> patch, 
float3 uvw : SV_DomainLocation)
{
    VertexMesh input;
    MeshOutput output;
    
    float4 origin;
    float3 position = uvw.x * patch[0].Position + uvw.y * patch[1].Position + uvw.z * patch[2].Position;    
    input.Position = float4(position, 1);
    input.Position.xyz = normalize(input.Position.xyz) * 0.5;
    //input.Position = float4(GetTrianglePoint(patch[0].Position.xyz, patch[1].Position.xyz, patch[2].Position.xyz, uvw), 1);
   
    origin = input.Position;
    input.Normal = GetTrianglePoint(patch[0].Normal, patch[1].Normal, patch[2].Normal, uvw);
    input.Tangent = GetTrianglePoint(patch[0].Tangent, patch[1].Tangent, patch[2].Tangent, uvw);
    input.Uv = GetTrianglePoint(patch[0].Uv, patch[1].Uv, patch[2].Uv, uvw);
    
    if (mapDesc.displacement)
    {
        input.Position.xyz += (DisplacementMap.Load(
        int3(input.Uv.x * mapDesc.texOffset.x, input.Uv.y * mapDesc.texOffset.y, 0)).r) * input.Normal
        * mapDesc.height_scale;
    }
    //float4 r1 = float4(uvw.x, uvw.x, uvw.x);
    World = patch[1].Transform;
    VS_GENERATE
    return output;    
}

[domain("tri")]
MeshOutput DS_Mesh_Flat(
CHullOutput_Mesh chinput,
const OutputPatch<HullOutput_Mesh, 3> patch,
float3 uvw : SV_DomainLocation)
{
    VertexMesh input;
    MeshOutput output;
    
    float4 origin;
    float3 position = uvw.x * patch[0].Position + uvw.y * patch[1].Position + uvw.z * patch[2].Position;
    input.Position = float4(position, 1);
    input.Position.xyz = input.Position.xyz * 0.5;
    //input.Position = float4(GetTrianglePoint(patch[0].Position.xyz, patch[1].Position.xyz, patch[2].Position.xyz, uvw), 1);
   
    origin = input.Position;
    input.Normal = GetTrianglePoint(patch[0].Normal, patch[1].Normal, patch[2].Normal, uvw);
    input.Tangent = GetTrianglePoint(patch[0].Tangent, patch[1].Tangent, patch[2].Tangent, uvw);
    input.Uv = GetTrianglePoint(patch[0].Uv, patch[1].Uv, patch[2].Uv, uvw);
    
    if (mapDesc.displacement)
    {
        input.Position.xyz += (DisplacementMap.Load(
        int3(input.Uv.x * mapDesc.texOffset.x, input.Uv.y * mapDesc.texOffset.y, 0)).r) * input.Normal
        * mapDesc.height_scale;
    }
    //float4 r1 = float4(uvw.x, uvw.x, uvw.x);
    World = patch[1].Transform;
    VS_GENERATE
    return output;
}


///////////////////////////////////////////////////////////////////////////////

struct VertexModel
{
    float4 Position : Position;
    float2 Uv : Uv;
    float3 Normal : Normal;
    float3 Tangent : Tangent;
    float4 BlendIndices : BlendIndices;
    float4 BlendWeights : BlendWeights;
    
    uint InstanceID : SV_InstanceID;
    
    matrix Transform : Inst1_Transform;
    //float4 Color : Inst2_Color;
};

Texture2DArray TransformsMap;
#define MAX_MODEL_TRANSFORMS 250

cbuffer CB_Bone // 해당 mesh의 본 트랜스폼을 참조할때 사용한다.
{
    uint BoneIndex;
};

void SetModelWorld(inout matrix world, VertexModel input)
{
    //world = mul(BoneTransforms[BoneIndex], world);
    
    float4 m0 = TransformsMap.Load(int4(BoneIndex * 4 + 0, input.InstanceID, 0, 0));
    float4 m1 = TransformsMap.Load(int4(BoneIndex * 4 + 1, input.InstanceID, 0, 0));
    float4 m2 = TransformsMap.Load(int4(BoneIndex * 4 + 2, input.InstanceID, 0, 0));
    float4 m3 = TransformsMap.Load(int4(BoneIndex * 4 + 3, input.InstanceID, 0, 0));
    
    matrix transform = matrix(m0, m1, m2, m3);
    world = mul(transform, input.Transform);
}

void SetSkinningModelWorld(inout matrix world, VertexModel input)
{
    matrix transform = 0;
    
    float indices[4] = { input.BlendIndices.x, input.BlendIndices.y, input.BlendIndices.z, input.BlendIndices.w };
    float weights[4] = { input.BlendWeights.x, input.BlendWeights.y, input.BlendWeights.z, input.BlendWeights.w };
    
    
    for (int i = 0; i < 4;i++)
    {
        float4 m0 = TransformsMap.Load(int4(indices[i] * 4 + 0, input.InstanceID, 0, 0));
        float4 m1 = TransformsMap.Load(int4(indices[i] * 4 + 1, input.InstanceID, 0, 0));
        float4 m2 = TransformsMap.Load(int4(indices[i] * 4 + 2, input.InstanceID, 0, 0));
        float4 m3 = TransformsMap.Load(int4(indices[i] * 4 + 3, input.InstanceID, 0, 0));
        
        matrix mat = matrix(m0, m1, m2, m3);
        
        transform += mul(weights[i], mat);
    }    
    world = mul(transform, input.Transform);    
}

MeshOutput VS_Model(VertexModel input)
{
    MeshOutput output;
    
    SetModelWorld(World, input);   
   
    VS_GENERATE
    
    return output;
}

SSR_MeshOutput VS_SSR_Model(VertexModel input)
{
    SSR_MeshOutput output;
    SetModelWorld(World, input);
    VS_SSR_GENERATE
    
    return output;
}

MeshOutput VS_Model_Skinning(VertexModel input)
{
    MeshOutput output;
    
    SetSkinningModelWorld(World, input);
    
    VS_GENERATE
    
    return output;
}


MeshOutput VS_PreRender_Reflection_Model(VertexModel input)
{
    MeshOutput output = VS_Model(input);
    
    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, Reflection);
    output.Position = mul(output.Position, Projection);
    
    return output;
}

DepthOutput VS_Depth_Model(VertexModel input)
{
    DepthOutput output;
    
    SetModelWorld(World, input);
    VS_DEPTH_GENERATE
    
    return output;
}

DepthOutput VS_Depth_Spot_Model(VertexModel input)
{
    DepthOutput output;
    SetModelWorld(World, input);
    VS_DEPTH_SPOT_GENERATE
    
    return output;
}

///////////////////////////////////////////////////////////////////////////////

#define MAX_MODEL_KEYFRAMES 500
#define MAX_MODEL_INSTANCE 500

struct AnimationFrame
{
    int Clip;

    uint CurrFrame;
    uint NextFrame;

    float Time;
    float Running;

    float3 Padding;
};

struct TweenFrame
{
    float TakeTime;
    float TweenTime;
    float RunningTime;
    float Padding;

    AnimationFrame Curr;
    AnimationFrame Next;
};

cbuffer CB_TweenFrame
{
    TweenFrame TweenFrames[MAX_MODEL_INSTANCE];
};

void SetTweenWorld(inout matrix world, VertexModel input)
{
    float indices[4] = { input.BlendIndices.x, input.BlendIndices.y, input.BlendIndices.z, input.BlendIndices.w };
    float weights[4] = { input.BlendWeights.x, input.BlendWeights.y, input.BlendWeights.z, input.BlendWeights.w };
    
    
    int clip[2];
    int currFrame[2];
    int nextFrame[2];
    float time[2];
    
    // read current instance animation clip data [각 인스턴스의 애니메이션 클립/프레임 정보]
    clip[0] = TweenFrames[input.InstanceID].Curr.Clip;
    currFrame[0] = TweenFrames[input.InstanceID].Curr.CurrFrame;
    nextFrame[0] = TweenFrames[input.InstanceID].Curr.NextFrame;
    time[0] = TweenFrames[input.InstanceID].Curr.Time;
    
    clip[1] = TweenFrames[input.InstanceID].Next.Clip;
    currFrame[1] = TweenFrames[input.InstanceID].Next.CurrFrame;
    nextFrame[1] = TweenFrames[input.InstanceID].Next.NextFrame;
    time[1] = TweenFrames[input.InstanceID].Next.Time;
    
    
    
    float4 c0, c1, c2, c3;
    float4 n0, n1, n2, n3;
    
    matrix curr = 0, next = 0;
    matrix currAnim = 0;
    matrix nextAnim = 0;
    
    matrix transform = 0;
    
    // read animtion transform form TRansformsMap resouce
    // array struct data
    // skinning with four effected weight froom four bones
    [unroll(4)]
    for (int i = 0; i < 4; i++) 
    {
        // interpolation current and next frame
        c0 = TransformsMap.Load(int4(indices[i] * 4 + 0, currFrame[0], clip[0], 0));
        c1 = TransformsMap.Load(int4(indices[i] * 4 + 1, currFrame[0], clip[0], 0));
        c2 = TransformsMap.Load(int4(indices[i] * 4 + 2, currFrame[0], clip[0], 0));
        c3 = TransformsMap.Load(int4(indices[i] * 4 + 3, currFrame[0], clip[0], 0));
        curr = matrix(c0, c1, c2, c3);
        
        n0 = TransformsMap.Load(int4(indices[i] * 4 + 0, nextFrame[0], clip[0], 0));
        n1 = TransformsMap.Load(int4(indices[i] * 4 + 1, nextFrame[0], clip[0], 0));
        n2 = TransformsMap.Load(int4(indices[i] * 4 + 2, nextFrame[0], clip[0], 0));
        n3 = TransformsMap.Load(int4(indices[i] * 4 + 3, nextFrame[0], clip[0], 0));
        next = matrix(n0, n1, n2, n3);
        
        currAnim = lerp(curr, next, time[0]);
        
        // interpolation with next clip
        [flatten]
        if (clip[1] > -1)
        {
            c0 = TransformsMap.Load(int4(indices[i] * 4 + 0, currFrame[1], clip[1], 0));
            c1 = TransformsMap.Load(int4(indices[i] * 4 + 1, currFrame[1], clip[1], 0));
            c2 = TransformsMap.Load(int4(indices[i] * 4 + 2, currFrame[1], clip[1], 0));
            c3 = TransformsMap.Load(int4(indices[i] * 4 + 3, currFrame[1], clip[1], 0));
            curr = matrix(c0, c1, c2, c3);
        
            n0 = TransformsMap.Load(int4(indices[i] * 4 + 0, nextFrame[1], clip[1], 0));
            n1 = TransformsMap.Load(int4(indices[i] * 4 + 1, nextFrame[1], clip[1], 0));
            n2 = TransformsMap.Load(int4(indices[i] * 4 + 2, nextFrame[1], clip[1], 0));
            n3 = TransformsMap.Load(int4(indices[i] * 4 + 3, nextFrame[1], clip[1], 0));
            next = matrix(n0, n1, n2, n3);
        
            nextAnim = lerp(curr, next, time[1]);
            
            currAnim = lerp(currAnim, nextAnim, TweenFrames[input.InstanceID].TweenTime);
        }
        
        
        transform += mul(weights[i], currAnim);
    }
    
    world = mul(transform, input.Transform);
}

struct BlendFrame
{
    uint Mode;
    float Alpha;
    float2 Padding;
    
    AnimationFrame Clip[3];
};

cbuffer CB_BlendFrame
{
    BlendFrame BlendFrames[MAX_MODEL_INSTANCE];
};

void SetBlendWorld(inout matrix world, VertexModel input)
{
    float indices[4] = { input.BlendIndices.x, input.BlendIndices.y, input.BlendIndices.z, input.BlendIndices.w };
    float weights[4] = { input.BlendWeights.x, input.BlendWeights.y, input.BlendWeights.z, input.BlendWeights.w };
    
    
    float4 c0, c1, c2, c3;
    float4 n0, n1, n2, n3;
    
    matrix curr = 0, next = 0;
    matrix currAnim[3];
    matrix anim = 0;
    matrix transform = 0;
    
    BlendFrame frame = BlendFrames[input.InstanceID];
    
    [unroll(4)]
    for (int i = 0; i < 4; i++)
    {
        [unroll(3)]
        for (int k = 0; k < 3; k++)
        {
            c0 = TransformsMap.Load(int4(indices[i] * 4 + 0, frame.Clip[k].CurrFrame, frame.Clip[k].Clip, 0));
            c1 = TransformsMap.Load(int4(indices[i] * 4 + 1, frame.Clip[k].CurrFrame, frame.Clip[k].Clip, 0));
            c2 = TransformsMap.Load(int4(indices[i] * 4 + 2, frame.Clip[k].CurrFrame, frame.Clip[k].Clip, 0));
            c3 = TransformsMap.Load(int4(indices[i] * 4 + 3, frame.Clip[k].CurrFrame, frame.Clip[k].Clip, 0));
            curr = matrix(c0, c1, c2, c3);
        
            n0 = TransformsMap.Load(int4(indices[i] * 4 + 0, frame.Clip[k].NextFrame, frame.Clip[k].Clip, 0));
            n1 = TransformsMap.Load(int4(indices[i] * 4 + 1, frame.Clip[k].NextFrame, frame.Clip[k].Clip, 0));
            n2 = TransformsMap.Load(int4(indices[i] * 4 + 2, frame.Clip[k].NextFrame, frame.Clip[k].Clip, 0));
            n3 = TransformsMap.Load(int4(indices[i] * 4 + 3, frame.Clip[k].NextFrame, frame.Clip[k].Clip, 0));
            next = matrix(n0, n1, n2, n3);
        
            currAnim[k] = lerp(curr, next, frame.Clip[k].Time);
        }
       
        int clipA = (int) frame.Alpha;
        int clipB = clipA + 1;
        
        float alpha = frame.Alpha;
        if (alpha >= 1.0f)
        {
            alpha = frame.Alpha - 1.0f;
            
            if (frame.Alpha >= 2.0f)
            {
                clipA = 1;
                clipB = 2;
            }
        }
        
        anim = lerp(currAnim[clipA], currAnim[clipB], alpha);
        
        transform += mul(weights[i], anim);
    }
    
    world = mul(transform, input.Transform);
}

MeshOutput VS_Animation(VertexModel input)
{
    MeshOutput output;
    
    if (BlendFrames[input.InstanceID].Mode == 0)
        SetTweenWorld(World, input);
    else
        SetBlendWorld(World, input);
    
    VS_GENERATE
    
    return output;
}

SSR_MeshOutput VS_SSR_Animation(VertexModel input)
{
    SSR_MeshOutput output;
    
    if (BlendFrames[input.InstanceID].Mode == 0)
        SetTweenWorld(World, input);
    else
        SetBlendWorld(World, input);
    VS_SSR_GENERATE
    
    return output;
}

MeshOutput VS_PreRender_Reflection_Animation(VertexModel input)
{
    MeshOutput output = VS_Animation(input);
    
    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, Reflection);
    output.Position = mul(output.Position, Projection);
    
    return output;
}


DepthOutput VS_Depth_Animation(VertexModel input)
{
    DepthOutput output;
    
    if (BlendFrames[input.InstanceID].Mode == 0)
        SetTweenWorld(World, input);
    else
        SetBlendWorld(World, input);
    
    VS_DEPTH_GENERATE
    
    return output;
}

DepthOutput VS_Depth_Spot_Animation(VertexModel input)
{
    DepthOutput output;
    
    if (BlendFrames[input.InstanceID].Mode == 0)
        SetTweenWorld(World, input);
    else
        SetBlendWorld(World, input);
    
    VS_DEPTH_SPOT_GENERATE
    
    return output;
}