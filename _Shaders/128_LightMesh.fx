#include "00_Global.fx"
#include "00_PBR_Light.fx"

struct VertexMesh
{
    float4 Position : Position;
    float2 Uv : Uv;
    float3 Normal : Normal;
    float3 Tangent : Tangent;
    
    matrix Transform : Inst1_Transform;
};

struct VertexOutput
{
    float4 Position : SV_Position;    
    uint id : ID;
};

VertexOutput VS_Mesh(VertexMesh input , uint id : SV_InstanceID)
{
    VertexOutput output;
    World = input.Transform;
    output.Position = WorldPosition(input.Position);
    output.Position = ViewProjection(output.Position);
    output.id = id;
    return output;
}

float4 PS_Point(VertexOutput input) : SV_Target
{
    return PointLights[input.id].Diffuse;
}

float4 PS_Spot(VertexOutput input) : SV_Target
{
    return SpotLights[input.id].Diffuse;
}

DepthStencilState lightMeshDSS;

technique11 T0
{
    P_DSS_VP(P0, lightMeshDSS, VS_Mesh, PS_Point)
    P_DSS_VP(P1, lightMeshDSS, VS_Mesh, PS_Spot)
   
}