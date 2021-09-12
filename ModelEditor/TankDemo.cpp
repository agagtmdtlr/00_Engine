#include "stdafx.h"
#include "TankDemo.h"
#include "Converter.h"

void TankDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(1, 36, -85);

	shader = new Shader(L"38_Model.fx");

	Tank();

	sky = new CubeSky(L"Environment/GrassCube1024.dds");


	gridShader = new Shader(L"25_Mesh.fx");
	//grid = new MeshGrid(gridShader, 6, 6);
	//grid->GetTransform()->Scale(12, 1, 12);
	//grid->DiffuseMap(L"Floor.png");
}

void TankDemo::Destroy()
{
	SafeDelete(shader);
	
	SafeDelete(tank);


	SafeDelete(sky);

	SafeDelete(gridShader);
	SafeDelete(grid);

	
}

void TankDemo::Update()
{
	sky->Update();
	//grid->Update();
	
	if (tank != NULL)
	{
		ModelBone* root = tank->GetModel()->BoneByIndex(0);
		
		// turret bone Index
		ModelBone* bone = tank->GetModel()->BoneByIndex(10);
		// 3 7
		// 5 9
		ModelBone* tire1B = tank->GetModel()->BoneByIndex(5);
		ModelBone* tire2B = tank->GetModel()->BoneByIndex(9);

		ModelMesh* tire1M = tank->GetModel()->MeshByIndex(5);
		ModelMesh* tire2M = tank->GetModel()->MeshByIndex(9);


		Transform transform;

		float rotation = sinf(Time::Get()->Running()) * Math::PI * Time::Delta();
		transform.Rotation(0, rotation, 0);

		//Transform* tankTransform = tank->GetTransform();
		//Vector3 f = tankTransform->Forward();

		//Matrix wheel1 = tire1B->Transform();
		//Matrix wheel2 = tire2B->Transform();

		//Vector3 p;
		//Quaternion q;
		//Vector3 s;


		//

		//Transform movement;
		//float Xrot = 0;

		////f *= 1.0f * Time::Delta();
		//if (Keyboard::Get()->Press(VK_UP))
		//{
		//	Xrot = 2.0f * Time::Delta();			
		//	f *= 2.0f * Time::Delta();
		//	
		//}
		//else if (Keyboard::Get()->Press(VK_DOWN))
		//{
		//	Xrot = -2.0f * Time::Delta();
		//	f *= -2.0f * Time::Delta();
		//	
		//}
		//else
		//{
		//	f *= 0.0f;
		//}


		//{
		//	Matrix S, R, QR, T;
		//	Matrix W;

		//	D3DXMatrixDecompose(&s, &q, &p, &wheel1);

		//	D3DXMatrixScaling(&S, s.x, s.y, s.z);
		//	D3DXMatrixRotationQuaternion(&QR, &q);
		//	D3DXMatrixRotationX(&R, Xrot);
		//	D3DXMatrixTranslation(&T, p.x, p.y, p.z);

		//	W = S * R * QR * T;

		//	tire1B->Transform(W);
		//}

		//{
		//	Matrix S, R, QR, T;
		//	Matrix W;

		//	D3DXMatrixDecompose(&s, &q, &p, &wheel2);

		//	D3DXMatrixScaling(&S, s.x, s.y, s.z);
		//	D3DXMatrixRotationQuaternion(&QR, &q);
		//	D3DXMatrixRotationX(&R, Xrot);
		//	D3DXMatrixTranslation(&T, p.x, p.y, p.z);

		//	W = S * R * QR * T;

		//	tire2B->Transform(W);
		//}

		//Vector3 tp;
		//tank->GetTransform()->Position(&tp);
		//tp += f;
		//tank->GetTransform()->Position(tp);
		//
		//
		//tank->UpdateTransform(bone, transform.World());

		

		tank->Update();
	}
}

void TankDemo::Render()
{
	ImGui::SliderFloat3("TRot", tRot, -5, 5);

	ImGui::SliderFloat3("Direction", direction, -1, +1);
	shader->AsVector("Direction")->SetFloatVector(direction);
	gridShader->AsVector("Direction")->SetFloatVector(direction);

	static int pass = 0;
	ImGui::InputInt("Pass", &pass);
	pass %= 2;

	sky->Render();
	//grid->Render();

	if (tank != NULL)
	{
		tank->Pass(pass);
		tank->Render();
	}


}

void TankDemo::Tank()
{
	tank = new ModelRender(shader);
	// export했던 경로에서 읽어오면 된다.
	tank->ReadMesh(L"Tank/Tank");
	tank->ReadMaterial(L"Tank/Tank");
	//tank->GetTransform()->Position(20, 0, 0);

}

