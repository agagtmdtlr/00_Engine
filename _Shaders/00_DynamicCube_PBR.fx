//////////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic Cube Map
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//DyanamicCube 렌더링할때 사용하는 viewprojection matrix;
cbuffer CB_DynamicCube // viewprojection to 6face
{
    uint CubeRenderType;
    float3 CB_DynamicCube_Padding;
    
    matrix DynamicCubeViews[6];
    matrix DynamicCubeProjection;
};

struct Struct_ShadowDynamicCubeDesc // Linear depth calculation
{
    float3 position;
    float farPlane;
};

cbuffer CB_ShadowDynamicCube
{
    Struct_ShadowDynamicCubeDesc shadowDynamicCubeDesc;
};

TextureCube DynamicCubeMap;


//////////////////////////////////////////////////////////////////////////////////////////////
// DynamicCube Rendering EnvironmentMap,ReflectionMap,Perspective ShadowMap
//////////////////////////////////////////////////////////////////////////////////////////////
[maxvertexcount(18)]
void GS_DynamicCube(triangle MeshOutput input[3], inout TriangleStream<MeshGeometryOutput> stream)
{
    int vertex = 0;
    MeshGeometryOutput output;
    [unroll(6)]
    for (int i = 0; i < 6; i++)
    {
        output.TargetIndex = i;
        [unroll(3)]
        for (vertex = 0; vertex < 3; vertex++)
        {
            output.Position = mul(float4(input[vertex].wPosition, 1), DynamicCubeViews[i]);
            output.Position = mul(output.Position, DynamicCubeProjection);
            output.wvpPosition = input[vertex].wvpPosition;
            output.wvpPosition_Sub = input[vertex].wvpPosition_Sub;
            output.oPosition = input[vertex].oPosition;
            output.wPosition = input[vertex].wPosition;
            output.gPosition = input[vertex].gPosition;
            output.sPosition = input[vertex].sPosition;
            
            output.Normal = input[vertex].Normal;
            output.Tangent = input[vertex].Tangent;
            output.Uv = input[vertex].Uv;      
            
            
            stream.Append(output);
        }

        stream.RestartStrip();
    }

}
struct DynamicCube_ShadowOutput
{
    float4 Color : SV_Target;
    float depth : SV_Depth;
};

//MeshGeometryOutput.TargetIndx
// SV_Depth :: save linear depth
DynamicCube_ShadowOutput PS_DynamicCube_ShadowMap(MeshGeometryOutput input)
{
    DynamicCube_ShadowOutput output;
    float lightDistance = length(shadowDynamicCubeDesc.position - input.wPosition);
    // map to [0;1] range by dividing by far_plane
    float depth = lightDistance / shadowDynamicCubeDesc.farPlane;
    output.Color = float4(lightDistance, lightDistance, lightDistance, 1);
    output.depth = depth;
    return output;
}