#include "Framework.h"
#include "Frustum.h"

Frustum::Frustum(Camera * camera, Perspective * perspective)
	: camera(camera), perspective(perspective)
{
	if (camera == NULL)
		this->camera = Context::Get()->GetCamera();

	if (perspective == NULL)
		this->perspective = Context::Get()->GetPerspective();
}

Frustum::~Frustum()
{

}

void Frustum::Update()
{
	//http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
	// culling용 view projection 공간
	Matrix V, P;
	camera->GetMatrix(&V);
	perspective->GetMatrix(&P);

	Matrix M = V * P;

	//Left
	planes[0].a = M._14 + M._11;
	planes[0].b = M._24 + M._21;
	planes[0].c = M._34 + M._31;
	planes[0].d = M._44 + M._41;


	//Right
	planes[1].a = M._14 - M._11;
	planes[1].b = M._24 - M._21;
	planes[1].c = M._34 - M._31;
	planes[1].d = M._44 - M._41;


	//Top
	planes[2].a = M._14 + M._12;
	planes[2].b = M._24 + M._22;
	planes[2].c = M._34 + M._32;
	planes[2].d = M._44 + M._42;

	//Bottom
	planes[3].a = M._14 - M._12;
	planes[3].b = M._24 - M._22;
	planes[3].c = M._34 - M._32;
	planes[3].d = M._44 - M._42;


	//Near
	planes[4].a = M._13;
	planes[4].b = M._23;
	planes[4].c = M._33;
	planes[4].d = M._43;

	//Far
	planes[5].a = M._14 - M._13;
	planes[5].b = M._24 - M._23;
	planes[5].c = M._34 - M._33;
	planes[5].d = M._44 - M._43;

	// 방향으로 사용할꺼니깐 정규화 시켜주기 [ a, b, c] == 1
	for (int i = 0; i < 6; i++)
		D3DXPlaneNormalize(&planes[i], &planes[i]);
}

void Frustum::Planes(Plane * planes)
{
	memcpy(planes, this->planes, sizeof(Plane) * 6);
}

bool Frustum::CheckPoint(Vector3 & position) // frustum과 점의 비교
{
	for (int i = 0; i < 6; i++)
	{
		// 평면으로 부터 음의 거리에 있으면 바깥에 존재
		if (D3DXPlaneDotCoord(&planes[i], &position) < 0.0f)
			return false;
	}

	return true;
}

// 직육면체와 비교
// 8개의 점중에 하나라도 들어가 있으면 렌더링한다.
bool Frustum::CheckCube(Vector3 & center, Vector3 & size)
{
	// 절두체의 각6면과 큐브이 8개의 점을 전부 검사한다.
	for (int i = 0; i < 6; i++)
	{
		// 1면과 8개의 점을 비교하는 과정
		// 8개의 점 중 하나라도 ㅇ
		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x - size.x, center.y - size.y, center.z - size.z)) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x + size.x, center.y - size.y, center.z - size.z)) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x - size.x, center.y + size.y, center.z - size.z)) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x - size.x, center.y - size.y, center.z + size.z)) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x + size.x, center.y + size.y, center.z - size.z)) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x + size.x, center.y - size.y, center.z + size.z)) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x - size.x, center.y + size.y, center.z + size.z)) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x + size.x, center.y + size.y, center.z + size.z)) >= 0.0f)
			continue;

		return false;
	}
	// 모든 검사를 통과했다느것은 적어도 하나의 점은 절두체 안에 존재한다는 의미이다.
	return true;
}

// 정육면체와 비교 ( 구나 정육면체의 경우 이런식으로 비교 가능한다. )
bool Frustum::CheckCube(Vector3 & center, float radius)
{
	Vector3 check;

	for (int i = 0; i < 6; i++)
	{
		check.x = center.x - radius;
		check.y = center.y - radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y - radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y + radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y + radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y - radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y - radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y + radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y + radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		return false;
	}
	
	// 모든 점이 내부에 있음
	return true;
}