#include "Framework.h"
#include "Utilities/BinaryFile.h"
#include "Utilities/Xml.h"
#include "Meshes/PBRMeshRender.h"
#include "LightMeshRender.h"

LightMeshRender::LightMeshRender()
{
	shader = new Shader(L"128_LightMesh.fxo");

	pointLightMesh = new PBRMeshRender(shader, new MeshSphere(0.5f));
	spotLightMesh = new PBRMeshRender(shader, new MeshSphere(0.5f));

	UINT count = Lighting::Get()->PointLightCount();
	for (UINT i = 0; i < count; i++)
	{
		PointLight& light = Lighting::Get()->GetPointLight(i);
		Transform* transform = pointLightMesh->AddTransform();
		transform->Position(light.Position);
	}
	count = Lighting::Get()->SpotLightPBRCount();
	for (UINT i = 0; i < count; i++)
	{
		SpotLightPBR& light = Lighting::Get()->GetSpotLightPBR(i);
		Transform* transform = spotLightMesh->AddTransform();
		transform->Position(light.Position);
	}	
	UpdateTransforms();
}

LightMeshRender::~LightMeshRender()
{
	SafeDelete(pointLightMesh);
	SafeDelete(spotLightMesh);

	SafeRelease(dss);

	SafeDelete(shader);
}

void LightMeshRender::Update()
{
	pointLightMesh->Update();
	spotLightMesh->Update();

	UINT count = Lighting::Get()->PointLightCount();
	for (UINT i = 0; i < count; i++)
	{
		PointLight& light = Lighting::Get()->GetPointLight(i);
		Transform* transform = pointLightMesh->GetTransform(i);
		transform->Position(light.Position);
	}
	count = Lighting::Get()->SpotLightPBRCount();
	for (UINT i = 0; i < count; i++)
	{
		SpotLightPBR& light = Lighting::Get()->GetSpotLightPBR(i);
		Transform* transform = spotLightMesh->GetTransform(i);
		transform->Position(light.Position);
	}
	UpdateTransforms();
}

void LightMeshRender::Render()
{
	shader->AsDepthStencil("lightMeshDSS")->GetDepthStencilState(1, &dss);
	pointLightMesh->Render();
	spotLightMesh->Render();
}

void LightMeshRender::Pass(UINT pass)
{
	pointLightMesh->Pass(pass);
	spotLightMesh->Pass(pass);
}

void LightMeshRender::Topology(D3D11_PRIMITIVE_TOPOLOGY t)
{
	pointLightMesh->Topology(t);
	spotLightMesh->Topology(t);
}

void LightMeshRender::UpdatePointTransform(UINT instanceid, Transform & transform)
{
	pointLightMesh->UpdateTransform(instanceid, transform);
}

void LightMeshRender::UpdateSpotTransform(UINT instanceid, Transform & transform)
{
	spotLightMesh->UpdateTransform(instanceid, transform);
}

Transform * LightMeshRender::AddPointTransform()
{
	return pointLightMesh->AddTransform();
}

Transform * LightMeshRender::AddSpotTransform()
{
	return spotLightMesh->AddTransform();
}

// UpdateInstanceBuffer
void LightMeshRender::UpdateTransforms()
{
	pointLightMesh->UpdateTransforms();
	spotLightMesh->UpdateTransforms();
}

void LightMeshRender::CreatDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	desc.DepthEnable = true;
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.StencilEnable = false;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &dss));
}


