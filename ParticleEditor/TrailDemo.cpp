#include "stdafx.h"
#include "TrailDemo.h"
#include "Utilities/Xml.h"

void TrailDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(1, 36, -85);
	((Freedom*)Context::Get()->GetCamera())->Speed(50, 2);

	shader = new Shader(L"55_Render.fxo");
	sky = new CubeSky(L"Environment/GrassCube1024.dds");

	Kachujin();
	Mesh();
	trail = new TrailEffect();
	trailTransform = new Transform();
}

void TrailDemo::Destroy()
{


}

void TrailDemo::Update()
{
	sky->Update();
	
	grid->Update();

	kachujin->Update();

	Matrix worlds[MAX_MODEL_TRANSFORMS];
	{
		kachujin->GetAttachTransform(0, worlds);
		
		Vector3 sp;
		Vector3 ep;
		trailTransform->World(worlds[1]);
		trailTransform->Position(&sp);
		trailTransform->World(worlds[40]);
		trailTransform->Position(&ep);

		trail->Add(sp, ep);
		trail->Update();
	}

}

void TrailDemo::Render()
{
	Pass(0, 1, 2);

	sky->Render();

	floor->Render();
	grid->Render();

	kachujin->Render();

	trail->Render();
}

void TrailDemo::Mesh()
{
	floor = new Material(shader);
	floor->DiffuseMap("Floor.png");
	floor->Specular(1, 1, 1, 20);
	floor->SpecularMap("Floor_Specular.png");
	floor->NormalMap("Floor_Normal.png");

	Transform* transform = NULL;

	grid = new MeshRender(shader, new MeshGrid(5, 5));
	transform = grid->AddTransform();
	transform->Position(0, 0, 0);
	transform->Scale(12, 1, 12);
	grid->UpdateTransforms();

	meshes.push_back(grid);
}



void TrailDemo::Kachujin()
{
	kachujin = new ModelAnimator(shader);
	kachujin->ReadMesh(L"Kachujin/Mesh");
	kachujin->ReadMaterial(L"Kachujin/Mesh");
	kachujin->ReadClip(L"Kachujin/Sword And Shield Slash");

	Transform* transform = NULL;

	transform = kachujin->AddTransform();
	transform->Position(0, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayTweenMode(0, 0, 1.0f);

	kachujin->UpdateTransforms();

	animators.push_back(kachujin);
}

void TrailDemo::KachujinCollider()
{
	colliderInitTransforms = new Transform();
	colliderInitTransforms->Position(0, 0, 0);

	for (UINT i = 0; i < 2; i++)
	{
		colliders[i] = new ColliderObject();

		colliders[i]->Transform = new Transform();
		colliders[i]->Collider = new Collider(colliders[i]->Transform, colliderInitTransforms);
	}
}

void TrailDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);

	for (ModelRender* temp : models)
		temp->Pass(model);

	for (ModelAnimator* temp : animators)
		temp->Pass(anim);
}
