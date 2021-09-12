#include "Framework.h"
#include "Reflection.h"
#include "Viewer/Fixity.h"

// �ݻ� ���� transform
Reflection::Reflection(Shader * shader, Transform * transform, UINT width, UINT height)
	: shader(shader), transform(transform)
{
	this->width = width > 0 ? width : (UINT)D3D::Width();
	this->height = height > 0 ? height : (UINT)D3D::Height();

	// ����Ÿ�� ���� ����
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
	// reflection camera��
	// �ݻ簢�� ������ ���⿡�� ���� ����̾�� �մϴ�
	// ���� �þఢ x�� ȸ�� �Ʒ��� ������ // �Ʒ����� �ݻ簢 ���� �������� ���°�
	// �Ʒ��ϰ� y����ġ�� �ݴ� ���� -y���������

	Vector3 R, T;
	Context::Get()->GetCamera()->Rotation(&R);
	Context::Get()->GetCamera()->Position(&T);

	R.x *= -1.0f;
	camera->Rotation(R);

	//T.y *= -1.0f;
	Vector3 position; //��ü�� ��ġ�� �������� �ݻ���Ѿ� �ȴ�.
	transform->Position(&position);


	// * 2.0f�� ���ϴ� ������ �������̴�. fov 45�������� ���� ��Ȯ�� �������̴�.
	T.y = (position.y * 2.0f) - T.y;
	//T.y = -abs(position.y - T.y); // �߸� �κ��� ����....


	
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
