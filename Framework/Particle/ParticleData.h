#pragma once
#include "Framework.h"

//입자들의 이동과 회전, 색상들을 운영할 값들을 저장한 구조체
struct ParticleData
{
	enum class BlendType
	{
		Opaque = 0, Additive, AlphaBlend
	} Type = BlendType::Opaque;

	bool bLoop = false;

	wstring TextureFile = L""; // 입자의 텍스처 파일

	UINT MaxParticles = 100; // 최대개수

	float ReadyTime = 1.0f;
	float ReadyRandomTime = 0; // 불규칙한 생성의 딜레이를 주기 위해서

	float StartVelocity = 1; // 시작의 가속도
	float EndVelocity = 1; // 끝의 가속도

	// 수평으로의 점점 진행될값
	float MinHorizontalVelocity = 0;
	float MaxHorizontalVelocity = 0;

	// 수직으로의 점점 진행될값 ( z값으로 생각하면 된다 )
	float MinVerticalVelocity = 0;
	float MaxVerticalVelocity = 0;

	// 중력
	Vector3 Gravity = Vector3(0, 0, 0);

	float ColorAmount = 1.0f;
	Color MinColor = Color(1, 1, 1, 1);
	Color MaxColor = Color(1, 1, 1, 1);

	float MinRotateSpeed = 0;
	float MaxRotateSpeed = 0;

	float MinStartSize = 100;
	float MaxStartSize = 100;

	float MinEndSize = 100;
	float MaxEndSize = 100;
};
