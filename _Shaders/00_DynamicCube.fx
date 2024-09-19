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
// �� ��� �ﰢ���� 6������ �������ϹǷ� maxvertexcount�� 18���� ����
[maxvertexcount(18)]
void GS_PreRender(triangle MeshOutput input[3], inout TriangleStream<MeshGeometryOutput> stream)
{
    int vertex = 0;
    MeshGeometryOutput output;
    
    // ť��� 6�� �׸���
    [unroll(6)]
    for (int i = 0; i < 6; i++)
    {
        // ��� ����Ÿ�ٿ� �׸� ������ ������ �ε���
        // �ش� ��� ������Ʈ�� ���̴����� �����Ѵ�.
        output.TargetIndex = i; // ��� ť��鿡 ���� ����
        
        [unroll(3)] // �ߺ����� �����Ϸ���
        for (vertex = 0; vertex < 3; vertex++)
        {
            // �� ť��鿡 ���ߴ� ī�޶�� ���߱�
            output.Position = mul(float4(input[vertex].wPosition, 1), CubeViews[i]);
            // ť��ʿ� �´� projection
            output.Position = mul(output.Position, CubeProjection);
            
            // VS���� �̹� ó���� ������ �׳� �Ѱ��ֱ�
            output.oPosition = input[vertex].oPosition;
            output.wPosition = input[vertex].wPosition;
            output.Normal = input[vertex].Normal;
            output.Tangent = input[vertex].Tangent;
            output.Uv = input[vertex].Uv;
            
            stream.Append(output);
        }

        stream.RestartStrip(); // ���� �ﰢ�� �׷��ֱ�
    }
}

// �ϴõ� ť�� ���ο� ���Ե� ���̹Ƿ� ���̴��� �ϴ� �������� �����ϰڽ��ϴ�.
float4 PS_DynamicCube_PreRender_Sky(MeshGeometryOutput input) : SV_Target
{
    return PS_Sky(ConvertMeshOutput(input));
}

float4 PS_DynamicCube_PreRender(MeshGeometryOutput input) : SV_Target
{
    return PS_AllLight(ConvertMeshOutput(input));
}

TextureCube DynamicCubeMap;
float RefractionAmount = 0.2f; // ������ ���� ������ 
float RefractionAlpha = 0.75f;

float CubeMapAmount = 1.0f;
float CubeMapBias = 1.0f;
float CubeMapScale = 1.0f;

// ���� ������
//float materialTransmission

// ������ n1, n2
// 

// not GS
float4 PS_DynamicCubeMap(MeshOutput input) : SV_Target
{
    float4 color = 0;
    
    float3 view = normalize(input.wPosition - ViewPosition());
    float3 normal = normalize(input.Normal);
    
    // Reflect(�Ի簢, ��ֺ���)
    float3 reflection = reflect(view, normal);
    // sin(v)n1 = sin(r)n2
    // n2 = refractionAmount
    // n1 = 1;
    float3 refraction = refract(view, normal, RefractionAmount);
    
    float4 diffuse = 0;
    
    if (CubeRenderType == 0) //
    {
        color = DynamicCubeMap.Sample(LinearSampler, input.oPosition); // ������ �����鼭 �浹�ϴ� �ȼ�
    }
    else if (CubeRenderType == 1) // Reflect
    {
        color = DynamicCubeMap.Sample(LinearSampler, reflection);
    }
    else if (CubeRenderType == 2)
    {
        // �ܰ��� ����°� ����
        color = DynamicCubeMap.Sample(LinearSampler, -refraction);
        color.a = RefractionAlpha;
    }
    else if (CubeRenderType == 3)
    {
        diffuse = PS_AllLight(input);
        color = DynamicCubeMap.Sample(LinearSampler, reflection);
        // ��������� �����ѰŴ� �����ϱ� // Fresnel �������� ������̴�.
        // �ݻ緮 * diffuse * ������       
        color.rgb *= (0.15f + diffuse * 0.95f);
    }
    else if (CubeRenderType == 4) // Fresnel ������
    {
        // ������ ������
        // �ݻ�� ������ ������ ���ٴ� �����Ͽ� ������ ������
        diffuse = PS_AllLight(input);
        
        // �Ի簢�� ���� ������ ���� ����ϱ�
        color = DynamicCubeMap.Sample(LinearSampler, reflection);
        
        float4 fresnel = CubeMapBias + (1.0f - CubeMapScale) * pow(abs(1.0f - dot(view, normal)), CubeMapAmount);
        // �ݻ���,�������� ���� (
        color = CubeMapAmount * diffuse + lerp(diffuse, color, fresnel);
        color.a = 1.0f;
    }
    
    
    return color;
}