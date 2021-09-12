#include "Framework.h"
#include "PBRMeshRender.h"

PBRMeshRender::PBRMeshRender(Shader * shader, Mesh * mesh)
	: mesh(mesh)
{
	Pass(0);
	mesh->SetShader(shader);

	// instance data ininitalize
	for (UINT i = 0; i < MAX_MESH_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]);

	// instance buffer create :: cpu write true
	instanceBuffer = new VertexBuffer(worlds, MAX_MESH_INSTANCE, sizeof(Matrix), 1, true);
}

PBRMeshRender::~PBRMeshRender()
{
	for (Transform* transform : transforms)
		SafeDelete(transform);

	SafeDelete(instanceBuffer);
	SafeDelete(mesh);
}

void PBRMeshRender::Update()
{
	mesh->Update();
}

void PBRMeshRender::Render()
{
	instanceBuffer->Render();

	// ���� �׷��� ����
	mesh->Render(transforms.size());
}

void PBRMeshRender::UpdateTransform(UINT instanceId, Transform & transform)
{
	transforms[instanceId]->World(transform.World());
}

Transform * PBRMeshRender::AddTransform()
{
	Transform* transform = new Transform();
	transforms.push_back(transform); // instance data �� �־��ش� 

	return transform;
}

void PBRMeshRender::UpdateTransforms()
{
	// ���۵����Ϳ� �������ش�.
	for (UINT i = 0; i < transforms.size(); i++)
	{
		memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix));
	}

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, worlds, sizeof(Matrix) * MAX_MESH_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
}