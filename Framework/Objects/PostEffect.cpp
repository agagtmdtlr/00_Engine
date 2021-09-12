#include "Framework.h"
#include "PostEffect.h"

PostEffect::PostEffect(wstring shaderFile)
	: Renderer(shaderFile)
{
	// -1 뒤에서 이미지를 바라보기
	//D3DXMatrixLookAtLH(&desc.View, &Vector3(0, 0, -1), &Vector3(0, 0, 0), &Vector3(0, 1, 0));
	//D3DXMatrixOrthoOffCenterLH(&desc.Projection, 0, D3D::Width(), 0, D3D::Height(), -1, +1);

	//buffer = new ConstantBuffer(&desc, sizeof(Desc));
	//shader->AsConstantBuffer("CB_PostEffect")->SetConstantBuffer(buffer->Buffer());

	// NDC 좌표계만큼
	Vertex vertices[6];
	vertices[0].Position = Vector3(-1.0f, -1.0f, 0.0f);
	vertices[1].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[2].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[3].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[4].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[5].Position = Vector3(+1.0f, +1.0f, 0.0f);

	vertexBuffer = new VertexBuffer(vertices, 6, sizeof(Vertex));
	sDiffuseMap = shader->AsSRV("DiffuseMap");

	transform->Scale(D3D::Width(), D3D::Height(), 1);
	transform->Position(D3D::Width() * 0.5f, D3D::Height() * 0.5f, 0);

}

PostEffect::~PostEffect()
{
	//SafeDelete(buffer);
}

void PostEffect::Update()
{
	Super::Update();
}

void PostEffect::Render()
{
	Super::Render();

	//buffer->Render();
	shader->Draw(0, Pass(), 6);
}

void PostEffect::SRV(ID3D11ShaderResourceView * srv)
{
	sDiffuseMap->SetResource(srv);
}