#include "Framework.h"
#include "Moon.h"

Moon::Moon(Shader * shader)
	: Renderer(shader)
	, distance(79.0f), glowDistance(78.7f)
{
	moon = new Texture(L"Environment/Moon.png");
	moonGlow = new Texture(L"Environment/MoonGlow.png");

	sMoon = shader->AsSRV("MoonMap");
	sAlpha = shader->AsScalar("MoonAlpha");


	VertexTexture vertices[6];

	vertices[0].Position = Vector3(-1.0f, -1.0f, 0.0f);
	vertices[1].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[2].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[3].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[4].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[5].Position = Vector3(+1.0f, +1.0f, 0.0f);

	vertices[0].Uv = Vector2(0, 1);
	vertices[1].Uv = Vector2(0, 0);
	vertices[2].Uv = Vector2(1, 1);
	vertices[3].Uv = Vector2(1, 1);
	vertices[4].Uv = Vector2(0, 0);
	vertices[5].Uv = Vector2(1, 0);

	vertexBuffer = new VertexBuffer(vertices, 6, sizeof(VertexTexture));
}

Moon::~Moon()
{

}

void Moon::Update()
{
	Super::Update();
}

void Moon::Render(float theta, Vector3 position, float d)
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	sAlpha->SetFloat(GetAlpha(theta));

	//Moon
	{
		Matrix W = GetTransform(theta, position, d);
		transform->World(W);

		Super::Render();

		sMoon->SetResource(moon->SRV());
		shader->Draw(0, Pass(), 6);
	}

	//Glow
	{
		Matrix W = GetGlowTransform(theta, position , d);
		transform->World(W);

		Super::Render();

		sMoon->SetResource(moonGlow->SRV());
		shader->Draw(0, Pass(), 6);
	}
}

float Moon::GetAlpha(float theta)
{
	if (theta < Math::PI * 0.5f || theta > Math::PI * 1.5f)
		return fabsf(sinf(theta + Math::PI / 2.0f));

	return 0.0f;
}

// 카메라 영역 때문에 잘리는 달이 잘 나올수 있게
D3DXMATRIX Moon::GetTransform(float theta, Vector3 position, float d)
{
	//Vector3 position;
	//Context::Get()->GetCamera()->Position(&position);


	Matrix S, R, T, D;
	//D3DXMatrixScaling(&S, 4, 4, 1);
	D3DXMatrixScaling(&S, 50, 50, 1);

	D3DXMatrixRotationYawPitchRoll(&R, Math::PI * 0.5f, theta - (Math::PI * 0.5f), 0);

	D3DXMatrixTranslation(&T, position.z, position.y, position.z);

	Vector3 direction = Context::Get()->Direction();
	D3DXMatrixTranslation
	(
		&D
		, direction.x * d
		, direction.y * d
		, direction.z * d
	);

	return S * R * T * D;
}

D3DXMATRIX Moon::GetGlowTransform(float theta, Vector3 position, float d)
{
	//D3DXVECTOR3 position;
	//Context::Get()->GetCamera()->Position(&position);

	Vector3 b = -Context::Get()->GetCamera()->Forward();

	D3DXMATRIX S, R, T, D;
	D3DXMatrixScaling(&S, 100, 100, 1);
	D3DXMatrixRotationYawPitchRoll(&R, Math::PI * 0.5f, theta - (Math::PI * 0.5f), 0);
	D3DXMatrixTranslation(&T, position.z, position.y, position.z);

	D3DXVECTOR3 direction = Context::Get()->Direction();
	D3DXMatrixTranslation
	(
		&D
		, direction.x * ( d - 5)
		, direction.y * ( d - 5)
		, direction.z * ( d - 5)
	);

	return S * R * T * D;


}