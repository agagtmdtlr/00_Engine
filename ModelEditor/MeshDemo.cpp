#include "stdafx.h"
#include "MeshDemo.h"
#include "Converter.h"

void MeshDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(1, 36, -85);

	shader = new Shader(L"51_InstancingMesh.fxo");

	gridMaterial = new Material(shader);
	gridMaterial->DiffuseMap("White.png");

	grid = new MeshRender(shader , new MeshGrid());
	grid->AddTransform()->Scale(10, 1, 2); 
	// 추가후에는 업데이트를 반드시해줘야함
	grid->UpdateTransforms();

	cubeMaterial = new Material(shader);
	cubeMaterial->DiffuseMap(L"Box.png");

	cube = new MeshRender(shader, new MeshCube());
	for(float x = -50; x <= 50.0f; x += 2.5f)
	{
		Transform * transform = cube->AddTransform();
		transform->Scale(0.25f, 0.25f, 0.25f);
		transform->Position(Vector3(x, 0.25f * 0.5f, 0.0f));
		transform->RotationDegree(0, Math::Random(-180, 180), 0);
	}
	cube->UpdateTransforms();

}

void MeshDemo::Destroy()
{
	SafeDelete(shader);
	
	SafeDelete(gridMaterial);
	SafeDelete(grid);

	SafeDelete(cubeMaterial);
	SafeDelete(cube);
	
}

void MeshDemo::Update()
{
	grid->Update();
	cube->Update();
	
}

void MeshDemo::Render()
{
	gridMaterial->Render();
	grid->Render();

	cubeMaterial->Render();
	cube->Render();
}


