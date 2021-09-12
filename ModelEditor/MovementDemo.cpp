#include "stdafx.h"
#include "MovementDemo.h"
#include "Converter.h"

void MovementDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(17, 0, 0);
	Context::Get()->GetCamera()->Position(0, 6, -41);

	shader = new Shader(L"45_Animation.fx");

	Kachujin();
}

void MovementDemo::Destroy()
{
	SafeDelete(shader);
	
	SafeDelete(kachujin);
}

void MovementDemo::Update()
{
	if (kachujin != NULL)
	{
		kachujin->Update();
	}

	if (Keyboard::Get()->Press(VK_UP))
	{
	}
	if (Keyboard::Get()->Press(VK_UP))
	{
	}
	if (Keyboard::Get()->Press(VK_UP))
	{
	}
	if (Keyboard::Get()->Press(VK_UP))
	{
	}
}

void MovementDemo::Render()
{
	static float speed = 1.0f;
	ImGui::SliderFloat("Speed", &speed, 0.1f, 5.0f);

	static float takeTime = 1.0f;
	ImGui::SliderFloat("TakeTime", &takeTime, 0.1f, 5.0f);

	static int clip = 0;
	if (Keyboard::Get()->Down(VK_SPACE))
	{
		clip++;
		clip %= 4;

		kachujin->PlayTweenMode((UINT)clip,speed, takeTime);
	}

	ImGui::SliderFloat3("Direction", direction, -1, +1);
	shader->AsVector("Direction")->SetFloatVector(direction);

	static int pass = 0;
	ImGui::InputInt("Pass2", &pass);
	pass %= 2;

	if (kachujin != NULL)
	{
		kachujin->Pass(pass);
		kachujin->Render();
	}

}



void MovementDemo::Kachujin()
{
	kachujin = new ModelAnimator(shader);
	// export했던 경로에서 읽어오면 된다.
	kachujin->ReadMesh(L"Kachujin/Mesh");
	kachujin->ReadMaterial(L"Kachujin/Mesh");

	kachujin->ReadClip(L"Kachujin/Sword And Shield Idle");
	kachujin->ReadClip(L"Kachujin/Sword And Shield Run");
	kachujin->ReadClip(L"Kachujin/Sword And Shield Slash");
	kachujin->ReadClip(L"Kachujin/Salsa Dancing");

	//kachujin->GetTransform()->Position(0, 0, -30);
	//kachujin->GetTransform()->Rotation(0, Math::PI, 0);
	//kachujin->GetTransform()->Scale(0.025f, 0.025f, 0.025f);

}
