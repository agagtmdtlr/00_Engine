#include "Framework.h"
#include "Render2D.h"

Render2D::Render2D()
	: Renderer(L"62_Render2D.fx")
{
	// -1 뒤에서 이미지를 바라보기
	Vector3 eye(0, 0, -1);
	Vector3 at(0, 0, 0);
	Vector3 up(0, 1, 0);
	D3DXMatrixLookAtLH(&desc.View, &eye, &at, &up);
	
	// 직교 투영 : UI
	// Ortho  중앙을 잡아서 직교 투영
	D3DXMatrixOrthoOffCenterLH(&desc.Projection, 0, D3D::Width(), 0, D3D::Height(), -1, +1);


	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	shader->AsConstantBuffer("CB_Render2D")->SetConstantBuffer(buffer->Buffer());

	VertexTexture vertices[6];
	vertices[0].Position = Vector3(-0.5f, -0.5f, 0.0f);
	vertices[1].Position = Vector3(-0.5f, +0.5f, 0.0f);
	vertices[2].Position = Vector3(+0.5f, -0.5f, 0.0f);
	vertices[3].Position = Vector3(+0.5f, -0.5f, 0.0f);
	vertices[4].Position = Vector3(-0.5f, +0.5f, 0.0f);
	vertices[5].Position = Vector3(+0.5f, +0.5f, 0.0f);

	vertices[0].Uv = Vector2(0, 1);
	vertices[1].Uv = Vector2(0, 0);
	vertices[2].Uv = Vector2(1, 1);
	vertices[3].Uv = Vector2(1, 1);
	vertices[4].Uv = Vector2(0, 0);
	vertices[5].Uv = Vector2(1, 0);

	vertexBuffer = new VertexBuffer(vertices, 6, sizeof(VertexTexture));
	sDiffuseMap = shader->AsSRV("DiffuseMap");
	sDiffuseMap2 = shader->AsSRV("DiffuseMap2");

}

Render2D::~Render2D()
{
	SafeDelete(buffer);
}

void Render2D::Update()
{
	Super::Update();
}

void Render2D::Render()
{
	Super::Render();

	buffer->Render();
	shader->Draw(0, Pass(), 6);
}

void Render2D::SRV(ID3D11ShaderResourceView * srv)
{
	sDiffuseMap->SetResource(srv);
}

void Render2D::SRV2(ID3D11ShaderResourceView * srv)
{
	if (srv == NULL)
		Pass(0);
	else
		Pass(1);
	sDiffuseMap2->SetResource(srv);
}
