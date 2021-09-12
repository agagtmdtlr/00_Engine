#pragma once

struct Ray
{
	Ray() : Position(0, 0, 0), Direction(0, 0, 0)
	{

	}

	Ray(Vector3& position, Vector3& direction) : Position(position), Direction(direction)
	{

	}

	Vector3 Position;
	Vector3 Direction;
};

struct ColliderObject
{
	class Transform* Init = NULL;
	class Transform* Transform = NULL;
	class Collider* Collider = NULL;
};

class Collider
{
private:
	struct Bounding;

public:
	Collider(Transform* transform, Transform* init = NULL);
	~Collider();

	void Update();
	void Render(Color color = Color(0, 1, 0, 1));

	Transform* GetTransform() { return transform; }

	bool Intersection(Ray& ray, float* outDistance);
	bool Intersection(Collider* other);
private:
	// �и��� ����� �Լ�
	bool SeperatingPlane(Vector3& position, Vector3& direction, Bounding& box1, Bounding& box2);

	bool Collision(Bounding& box1, Bounding& box2);

	// �����Լ� ( value return�� ���ؼ�)
	Vector3 Cross(Vector3& vec1, Vector3& vec2);

private:
	struct Bounding
	{
		Vector3 Position;

		// �� ��
		Vector3 AxisX;
		Vector3 AxisY;
		Vector3 AxisZ;

		// 0.5 ~ 0.5 �� ũ�� local ���� ����� �����ϱ�
		Vector3 HalfSize;
	} bounding;


private:
	// ������ġ = ���� �ʱ� ��(init) * �̵��� Ʈ������
	Transform* init = NULL;
	Transform* transform;

	Vector3 lines[8];
};