#pragma once

class Frustum
{
public:
	Frustum(Camera* camera = NULL, Perspective* perspective = NULL);
	~Frustum();

	void Update();
	void Planes(Plane* planes);

	bool CheckPoint(Vector3& position);
	bool CheckCube(Vector3& center, Vector3& size);
	bool CheckCube(Vector3& center, float radius);

private:
	Plane planes[6];

	// 가상의 영역을 만들기 위함.. 그리기 위한 한정된 공간다.
	Camera* camera;
	Perspective* perspective;
};