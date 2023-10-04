#include "Framework.h"
#include "Camera.h"

Camera::Camera()
{
	D3DXMatrixIdentity(&matRotation);
	D3DXMatrixIdentity(&matView);

	Rotation();
	Move();
}

Camera::~Camera()
{

}

void Camera::Position(float x, float y, float z)
{
	Vector3 v(x, y, z);
	Position(v);
}

void Camera::Position(Vector3 & vec)
{
	position = vec;

	Move();
}

void Camera::Position(Vector3 * vec)
{
	*vec = position;
}

void Camera::Rotation(float x, float y, float z)
{
	Vector3 r(x, y, z);
	Rotation(r);
}

void Camera::Rotation(Vector3 & vec)
{
	rotation = vec;

	Rotation();
}

void Camera::Rotation(Vector3 * vec)
{
	*vec = rotation;
}

void Camera::RotationDegree(float x, float y, float z)
{
	Vector3 r(x, y, z);
	RotationDegree(r);
}

void Camera::RotationDegree(Vector3 & vec)
{
	//rotation = vec * Math::PI / 180.0f;
	rotation = vec * 0.01745328f;

	Rotation();
}

void Camera::RotationDegree(Vector3 * vec)
{
	//*vec = rotation * 180.0f / Math::PI;
	*vec = rotation * 57.29577957f; // constant : 180.0f / Math::PI;
}

void Camera::GetMatrix(Matrix * matrix)
{
	//*matrix = matView;
	memcpy(matrix, &matView, sizeof(Matrix));
}

void Camera::Rotation()
{
	Matrix X, Y, Z;
	D3DXMatrixRotationX(&X, rotation.x);
	D3DXMatrixRotationY(&Y, rotation.y);
	D3DXMatrixRotationZ(&Z, rotation.z);

	matRotation = X * Y * Z;


	Vector3 f(0, 0, 1);
	Vector3 u(0, 1, 0);
	Vector3 r(1, 0, 0);
	D3DXVec3TransformNormal(&forward, &f, &matRotation);
	D3DXVec3TransformNormal(&up, &u, &matRotation);
	D3DXVec3TransformNormal(&right, &r, &matRotation);
}

void Camera::Move()
{
	View();
}

void Camera::View()
{
	Vector3 at(position + forward);
	D3DXMatrixLookAtLH(&matView, &position, &at, &up);
}
