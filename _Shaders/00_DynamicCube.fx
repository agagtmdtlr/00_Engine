//////////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic Cube Map
//////////////////////////////////////////////////////////////////////////////////////////////////////////
cbuffer CB_DynamicCube
{
    uint CubeRenderType;
    float3 CB_DynamicCube_Padding;
    
    matrix CubeViews[6];
    matrix CubeProjection;
};
// 각 면당 삼각형을 6면으로 렌더링하므로 maxvertexcount는 18개로 세팅
[maxvertexcount(18)]
void GS_PreRender(triangle MeshOutput input[3], inout TriangleStream<MeshGeometryOutput> stream)
{
    int vertex = 0;
    MeshGeometryOutput output;
    
    // 큐브맵 6면 그리기
    [unroll(6)]
    for (int i = 0; i < 6; i++)
    {
        // 어는 렌더타겟에 그릴 면인지 결정할 인덱스
        // 해당 기능 지오메트리 셰이더엣거 가능한다.
        output.TargetIndex = i; // 어는 큐브면에 대한 건지
        
        [unroll(3)] // 중복선언 방지하려고
        for (vertex = 0; vertex < 3; vertex++)
        {
            // 각 큐브면에 맞추는 카메라로 맞추기
            output.Position = mul(float4(input[vertex].wPosition, 1), CubeViews[i]);
            // 큐브맵에 맞는 projection
            output.Position = mul(output.Position, CubeProjection);
            
            // VS에서 이미 처리된 값들은 그냥 넘격주기
            output.oPosition = input[vertex].oPosition;
            output.wPosition = input[vertex].wPosition;
            output.Normal = input[vertex].Normal;
            output.Tangent = input[vertex].Tangent;
            output.Uv = input[vertex].Uv;
            
            stream.Append(output);
        }

        stream.RestartStrip(); // 각각 삼각형 그려주기
    }
}

// 하늘도 큐브 매핑에 포함될 것이므로 셰이더에 하늘 렌더링도 통합하겠습니다.
float4 PS_DynamicCube_PreRender_Sky(MeshGeometryOutput input) : SV_Target
{
    return PS_Sky(ConvertMeshOutput(input));
}

float4 PS_DynamicCube_PreRender(MeshGeometryOutput input) : SV_Target
{
    return PS_AllLight(ConvertMeshOutput(input));
}

TextureCube DynamicCubeMap;
float RefractionAmount = 0.2f; // 매질에 따른 굴절률 
float RefractionAlpha = 0.75f;

float CubeMapAmount = 1.0f;
float CubeMapBias = 1.0f;
float CubeMapScale = 1.0f;

// 모델의 굴절률
//float materialTransmission

// 굴절률 n1, n2
// 

// not GS
float4 PS_DynamicCubeMap(MeshOutput input) : SV_Target
{
    float4 color = 0;
    
    float3 view = normalize(input.wPosition - ViewPosition());
    float3 normal = normalize(input.Normal);
    
    // Reflect(입사각, 노멀벡터)
    float3 reflection = reflect(view, normal);
    // sin(v)n1 = sin(r)n2
    // n2 = refractionAmount
    // n1 = 1;
    float3 refraction = refract(view, normal, RefractionAmount);
    
    float4 diffuse = 0;
    
    if (CubeRenderType == 0) //
    {
        color = DynamicCubeMap.Sample(LinearSampler, input.oPosition); // 방향을 나가면서 충돌하는 픽셀
    }
    else if (CubeRenderType == 1) // Reflect
    {
        color = DynamicCubeMap.Sample(LinearSampler, reflection);
    }
    else if (CubeRenderType == 2)
    {
        // 외곽선 생기는걸 방지
        color = DynamicCubeMap.Sample(LinearSampler, -refraction);
        color.a = RefractionAlpha;
    }
    else if (CubeRenderType == 3)
    {
        diffuse = PS_AllLight(input);
        color = DynamicCubeMap.Sample(LinearSampler, reflection);
        // 유사식으로 전개한거니 참고하기 // Fresnel 방정식의 유사식이다.
        // 반사량 * diffuse * 굴절량       
        color.rgb *= (0.15f + diffuse * 0.95f);
    }
    else if (CubeRenderType == 4) // Fresnel 방정식
    {
        // 프레넬 방정식
        // 반사와 굴절의 매질이 같다는 전제하에 전개한 방정식
        diffuse = PS_AllLight(input);
        
        // 입사각에 따른 프레넬 공식 사용하기
        color = DynamicCubeMap.Sample(LinearSampler, reflection);
        
        float4 fresnel = CubeMapBias + (1.0f - CubeMapScale) * pow(abs(1.0f - dot(view, normal)), CubeMapAmount);
        // 반사율,투과율에 따른 (
        color = CubeMapAmount * diffuse + lerp(diffuse, color, fresnel);
        color.a = 1.0f;
    }
    
    
    return color;
}