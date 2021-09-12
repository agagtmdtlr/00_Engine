#include "Framework.h"
#include "Shadow.h"

Shadow::Shadow(Shader * shader, Vector3 & position, float radius, UINT width, UINT height)
	: shader(shader),
	// light lookat point 일반적으로( 0,0,0) 지점을 향하게 비추는)
	position(position),
	// 직육면체 조명 구역을 지정 ( 그림자가 들어갈 반경 )
	radius(radius),
	// shadow map texture size
	width(width),
	height(height)
{
	// shadow map rtv,dsv, viewport 생성
	renderTarget = new RenderTarget(width, height);
	depthStencil = new DepthStencil(width, height);
	viewport = new Viewport((float)width, (float)height);
	// Blur shadow map pixel size;
	desc.MapSize = Vector2((float)width, (float)height);

	buffer = new ConstantBuffer(&desc, sizeof(Desc));	
	sBuffer = shader->AsConstantBuffer("CB_Shadow");
	// 2pass rendering shadow map depth buffer dsv
	sShadowMap = shader->AsSRV("ShadowMap");

	// Create Sampler Desc
	{
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));
		// D3D11_FILTER_MIN_MAG_MIP_LINEAR 단순 비교
		// 샘플링 시 깊이 비교를 위한 필터 옵션. 포인터 보더 라이너로 퀄리티 없
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; //
		// 비교 방식
		// depth >= z 깊이비교
		desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

		// MSAA antialiasing
		desc.MaxAnisotropy = 1;
		desc.MaxLOD = FLT_MAX;

		Check(D3D::GetDevice()->CreateSamplerState(&desc, &pcfSampler));
		sPcfSampler = shader->AsSampler("ShadowSampler"); // samplerstate;
	}

}

Shadow::~Shadow()
{
	SafeDelete(renderTarget);
	SafeDelete(depthStencil);
	SafeDelete(viewport);

	SafeDelete(buffer);
}

void Shadow::PreRender()
{
	// 2pass
	ImGui::InputInt("Quality", (int *)&desc.Quality);
	desc.Quality %= 3;

	//1Pass
	//ImGui::SliderFloat3("Light Direction", Context::Get()->Direction(), -1, +1);
	// 2pass
	//ImGui::SliderFloat("Bias", &desc.Bias, -0.0001f, +0.01f, "%.4f");
	
	renderTarget->PreRender(depthStencil);
	viewport->RSSetViewport();

	CalcViewProjection();

	buffer->Render();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	sShadowMap->SetResource(depthStencil->SRV());
	sPcfSampler->SetSampler(0, pcfSampler);
}

// 조명의 방향에서 공간을 깊이로 렌더링한다.
// 깊이를 비교하면서 실제 물체를 렌더링한다.
void Shadow::CalcViewProjection() 
{
	Vector3 up = Vector3(0, 1, 0);
	Vector3 direction = Context::Get()->Direction(); // 빛의 방향
	Vector3 position = direction * radius * -2.0f;

	// 뷰공간 만들기 ( 빛이 비추는 방향의 공간)
	D3DXMatrixLookAtLH(&desc.View, &position, &this->position, &up);

	// 원점을 뷰공간으로 이동시키기...
	Vector3 origin;
	D3DXVec3TransformCoord(&origin, &this->position, &desc.View);

	// orthto 공간
	// 뷰공간의 왼쪽
	float l = origin.x - radius;	
	float b = origin.y - radius;
	float n = origin.z - radius;

	float r = origin.x + radius;
	float t = origin.y + radius;
	float f = origin.z + radius;

	// ortho 센터와 차이는 공간의 반이 중심점이 된다.
	D3DXMatrixOrthoLH(&desc.Projection, r - l, t - b, n, f);


}
