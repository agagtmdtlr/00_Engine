#include "Framework.h"
#include "Reflection.h"
#include "Viewer/Fixity.h"

// 반사 받을 transform
Reflection::Reflection(Shader * shader, Transform * transform, UINT width, UINT height)
	: shader(shader), transform(transform)
{
	this->width = width > 0 ? width : (UINT)D3D::Width();
	this->height = height > 0 ? height : (UINT)D3D::Height();

	// 렌더타겟 따기 위함
	camera = new Fixity();
	renderTarget = new RenderTarget(this->width, this->height);
	depthStencil = new DepthStencil(this->width, this->height);
	viewport = new Viewport((float)this->width, (float)this->height);

	D3DXMatrixIdentity(&matrix);


	sReflection = shader->AsSRV("ReflectionMap");
	sMatrix = shader->AsMatrix("Reflection");
}

Reflection::~Reflection()
{
	SafeDelete(camera);
	SafeDelete(renderTarget);
	SafeDelete(depthStencil);
	SafeDelete(viewport);
}

void Reflection::Update()
{
	// reflection camera는
	// 반사각과 평행하 방향에서 찍은 모습이어야 합니다
	// 기존 시약각 x축 회전 아래로 돌리기 // 아래에서 반사각 벡터 방향으로 보는것
	// 아래니간 y축위치를 반대 방향 -y축방향으로

	Vector3 R, T;
	Context::Get()->GetCamera()->Rotation(&R);
	Context::Get()->GetCamera()->Position(&T);

	R.x *= -1.0f;
	camera->Rotation(R);

	//T.y *= -1.0f;
	Vector3 position; //물체의 위치를 기준으로 반사시켜야 된다.
	transform->Position(&position);


	// * 2.0f를 곱하는 이유는 보정값이다. fov 45도에서는 가장 정확한 보정값이다.
	T.y = (position.y * 2.0f) - T.y;
	//T.y = -abs(position.y - T.y); // 잘린 부분이 나옴....


	
	camera->Position(T);

	camera->GetMatrix(&matrix);
}

void Reflection::PreRender()
{
	renderTarget->PreRender(depthStencil);
	viewport->RSSetViewport();

	sMatrix->SetMatrix(matrix);
}

void Reflection::Render()
{
	sReflection->SetResource(renderTarget->SRV());
}
