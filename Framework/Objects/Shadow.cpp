#include "Framework.h"
#include "Shadow.h"

Shadow::Shadow(Shader * shader, Vector3 & position, float radius, UINT width, UINT height)
	: shader(shader),
	// light lookat point �Ϲ�������( 0,0,0) ������ ���ϰ� ���ߴ�)
	position(position),
	// ������ü ���� ������ ���� ( �׸��ڰ� �� �ݰ� )
	radius(radius),
	// shadow map texture size
	width(width),
	height(height)
{
	// shadow map rtv,dsv, viewport ����
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
		// D3D11_FILTER_MIN_MAG_MIP_LINEAR �ܼ� ��
		// ���ø� �� ���� �񱳸� ���� ���� �ɼ�. ������ ���� ���̳ʷ� ����Ƽ ��
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; //
		// �� ���
		// depth >= z ���̺�
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

// ������ ���⿡�� ������ ���̷� �������Ѵ�.
// ���̸� ���ϸ鼭 ���� ��ü�� �������Ѵ�.
void Shadow::CalcViewProjection() 
{
	Vector3 up = Vector3(0, 1, 0);
	Vector3 direction = Context::Get()->Direction(); // ���� ����
	Vector3 position = direction * radius * -2.0f;

	// ����� ����� ( ���� ���ߴ� ������ ����)
	D3DXMatrixLookAtLH(&desc.View, &position, &this->position, &up);

	// ������ ��������� �̵���Ű��...
	Vector3 origin;
	D3DXVec3TransformCoord(&origin, &this->position, &desc.View);

	// orthto ����
	// ������� ����
	float l = origin.x - radius;	
	float b = origin.y - radius;
	float n = origin.z - radius;

	float r = origin.x + radius;
	float t = origin.y + radius;
	float f = origin.z + radius;

	// ortho ���Ϳ� ���̴� ������ ���� �߽����� �ȴ�.
	D3DXMatrixOrthoLH(&desc.Projection, r - l, t - b, n, f);


}
