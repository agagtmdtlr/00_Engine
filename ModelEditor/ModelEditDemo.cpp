#include "stdafx.h"
#include "ModelEditDemo.h"

void ModelEditDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(1, 36, -85);


	shader = new Shader(L"00_ModelEdit.fxo");

	sky = new CubeSky(L"Environment/GrassCube1024.dds");

	Kachujin();
	KachujinCollider();
	BoneSphere();


}

void ModelEditDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(sky);

}

void ModelEditDemo::Update()
{
	sky->Update();

	kachujin->Update();



	Matrix worlds[MAX_MODEL_TRANSFORMS];
	for (UINT i = 0; i < kachujin->GetTransformCount(); i++)
	{
		// 특정 인스턴스의 전체 계산된 본을 가져온다.
		kachujin->GetAttachTransform(i, worlds);
		// 계산된 본들중 40번 본의 정보를 가져온다.
		colliders[i]->Collider->GetTransform()->World(worlds[40]);
		colliders[i]->Collider->Update();
	}

	{
		kachujin->GetAttachTransform(0, worlds);
		for (UINT i = 0; i < boneSphere->GetTransformCount(); i++)
		{
			Vector3 t;
			Quaternion r;
			Vector3 s;			

			Matrix T;
			Matrix S;
			Matrix R;
			Matrix W;

			D3DXMatrixDecompose(&s, &r, &t, &worlds[i]);

			s *= 0.5f;
			D3DXMatrixScaling(&S, s.x, s.y, s.z);
			D3DXMatrixRotationQuaternion(&R, &r);
			D3DXMatrixTranslation(&T, t.x, t.y, t.z);
			W = S * R * T;

			D3DXMatrixScaling(&S, 4, 4, 4);

			W *= S;
			boneSphere->GetTransform(i)->World(W);			
		}
	}

	boneSphere->Update();
	boneSphere->UpdateTransforms();

	CheckIntersection();


	Camera * camera = Context::Get()->GetCamera();

	Vector3 p;
	camera->Position(&p);
	Vector3 f = camera->Forward();
	Vector3 u = camera->Up();
	Vector3 r = camera->Right();

	Vector3 s = p + f * 100.0f + u * -15.0f + r * 40.0f;
	

	DebugLine::Get()->RenderLine(s, p + Vector3(0, 0, 0.01f), Color(0, 0, 1, 1));
	DebugLine::Get()->RenderLine(s, p + Vector3(0, 0.01f, 0), Color(0, 1, 0, 1));
	DebugLine::Get()->RenderLine(s, p + Vector3(0.01f, 0, 0), Color(1, 0, 0, 1));
}

void ModelEditDemo::Render()
{
	Pass(0, 1, 2);

	sky->Render();

	
	kachujin->Render();

	// 충돌체 렌더
	for (UINT i = 0; i < kachujin->GetTransformCount(); i++)
	{
		if ((int)i == collisionIndex)
			colliders[i]->Collider->Render(Color(1, 0, 0, 1));
		else
			colliders[i]->Collider->Render();
	}

	white->Render();

	boneSphere->Pass(4);
	boneSphere->Render();
}





void ModelEditDemo::Kachujin()
{
	kachujin = new ModelAnimator(shader);
	kachujin->ReadMesh(L"Kachujin/Mesh");
	kachujin->ReadMaterial(L"Kachujin/Mesh");
	kachujin->ReadClip(L"Kachujin/Sword And Shield Idle");
	kachujin->ReadClip(L"Kachujin/Sword And Shield Walk");
	kachujin->ReadClip(L"Kachujin/Sword And Shield Run");
	kachujin->ReadClip(L"Kachujin/Sword And Shield Slash");
	kachujin->ReadClip(L"Kachujin/Salsa Dancing");


	Transform* transform = NULL;

	transform = kachujin->AddTransform();
	transform->Position(0, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayTweenMode(0, 0, 1.0f);	

	kachujin->UpdateTransforms();

	animators.push_back(kachujin);
}

void ModelEditDemo::KachujinCollider()
{
	UINT count = kachujin->GetTransformCount();
	colliders = new ColliderObject*[count];

	colliderInitTransforms = new Transform();
	colliderInitTransforms->Position(0, 0, 0);
	colliderInitTransforms->Scale(0.25, 0.25, 0.25);

	for (UINT i = 0; i < count; i++)
	{
		colliders[i] = new ColliderObject();

		colliders[i]->Transform = new Transform();
		colliders[i]->Collider = new Collider(colliders[i]->Transform, colliderInitTransforms);
	}
}

void ModelEditDemo::BoneSphere()
{
	UINT count = kachujin->GetModel()->BoneCount();

	boneSphere = new MeshRender(shader,new MeshSphere(5));

	Transform* transform = NULL;

	for (UINT i = 0; i < count; i++)
	{
		boneSphere->AddTransform();
	}

	boneSphere->UpdateTransforms();

	meshes.push_back(boneSphere);

	{
		white = new Material(shader);
		white->DiffuseMap("White.png");
	}
}

void ModelEditDemo::CheckIntersection()
{
	if (Mouse::Get()->Down(0) == false) return;


	/////////////////// Unproject mouse position////////////////////////
	Matrix V = Context::Get()->View();
	Matrix P = Context::Get()->Projection();
	Viewport* Vp = Context::Get()->GetViewport();

	Vector3 mouse = Mouse::Get()->GetPosition();


	Matrix world;
	D3DXMatrixIdentity(&world);

	Vector3 n, f;
	mouse.z = 0.0f;
	Vp->Unproject(&n, mouse, world, V, P);

	mouse.z = 1.0f;
	Vp->Unproject(&f, mouse, world, V, P);

	Ray ray;
	ray.Position = n;
	ray.Direction = f - n;
	////////////////////////////

	float distance = 0.0f;
	UINT count = kachujin->GetTransformCount();

	bool check = false;
	for (UINT i = 0; i < count; i++)
	{
		if (colliders[i]->Collider->Intersection(ray, &distance))
		{
			collisionIndex = (int)i;
			check = true;

			break;
		}
	}

	if (check == false)
		collisionIndex = -1;


}

void ModelEditDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);

	for (ModelRender* temp : models)
		temp->Pass(model);

	for (ModelAnimator* temp : animators)
		temp->Pass(anim);
}