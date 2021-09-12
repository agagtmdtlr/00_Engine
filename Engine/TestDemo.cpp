#include "stdafx.h"
#include "Meshes/PBRMeshRender.h"
#include "TestDemo.h"	

void TestDemo::Initialize()
{
	Context::Get()->GetCamera()->Position(0, 4, -10);

	shader = new Shader(L"00_Test.fx");
	texture = new Texture(L"Environment/Ice_Lake_HiRes_TMap.jpg");

	shader->AsSRV("TestMap")->SetResource(texture->SRV());

	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.ArraySize = 1;
		desc.Width = 512;
		desc.Height = 512;
	}


	render2D = new Render2D();
	render2D->SRV(srv);
	render2D->GetTransform()->Position(D3D::Width()/ 4, D3D::Height() / 4, 0);
	render2D->GetTransform()->Scale(D3D::Width() / 4, D3D::Height() / 4, 0);
	render2D->Pass(1);






}

void TestDemo::Destroy()
{
}

void TestDemo::Update()
{
	render2D->Update();
}

void TestDemo::PreRender()
{
}

void TestDemo::Render()
{
}

void TestDemo::PostRender()
{
	render2D->Render();
}

