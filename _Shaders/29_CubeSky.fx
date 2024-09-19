#include "00_Global.fx"
#include "00_Light.fx"
#include "00_Render.fx"

DepthStencilState Sky_DSS;


//struct VertexOutput
//{
//    float4 Position : SV_Position; //position 0번으로 취급된다.
//    float3 oPosition : Position1;
//};

//VertexOutput VS(Vertex input)
//{
//    VertexOutput output;
    
//    output.oPosition = input.Position;
    
//    output.Position = WorldPosition(input.Position);
//    output.Position = ViewProjection(output.Position);

//    return output;
//}

//float4 PS(VertexOutput input) : SV_Target
//{
//    return SkyCubeMap.Sample(LinearSampler, input.oPosition);
//}

cbuffer MipMapLevel
{
    float lod;
    float padding[3];
};

float4 PS(MeshOutput input) : SV_Target
{
    float4 color = SkyCubeMap.SampleLevel(LinearSampler, input.oPosition, lod);
    color = pow(color, 2.2f);
    color.a = 1;
    return color;
}



MeshOutput VS_Sky_Mesh(VertexMesh input)
{
    MeshOutput output;
    
    SetMeshWorld(World, input);
    
    VS_GENERATE
    output.Position.xyz = output.oPosition;
    output.Position.w = 1;
    //output.Position = WorldPosition(output.Position);
    output.Position = ViewProjection(output.Position);
    
    return output;
}



float4 PS_Mip(MeshOutput input) : SV_Target
{
    float4 color = SkyCubeMap.SampleLevel(MipLinearSampler, input.oPosition, lod);
    color.a = 1;
    return color;
}


technique11 T0
{
    //P_RS_DSS_BS_VP(P0, FrontCounterClockwise_True, DepthEnable_False, AdditiveBlend,VS_Mesh, PS)
    P_RS_DSS_VP(P0, FrontCounterClockwise_True, Sky_DSS, VS_Mesh, PS)
    P_RS_DSS_VP(P1, FrontCounterClockwise_True, DepthEnable_False, VS_Mesh,PS_Mip)
    P_VP(P2, VS_Sky_Mesh, PS_Mip)
}