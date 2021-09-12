#include "stdafx.h"
#include "Meshes/PBRMeshRender.h"
#include "PBRBasicDemo.h"	
#include "PBR/MaterialPBR.h"

void PBRBasicDemo::Initialize()
{
	//Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(0, 4, -10);

	pbrShader = new Shader(L"126_PBRMaterial.fx");

	//shader = new Shader(L"111_Projector.fxo");
	shader = new Shader(L"124_HDR.fx");
	skycube = new MeshRender(shader, new MeshSphere(0.5));


	Transform * transform = skycube->AddTransform();
	transform->Position(0, 0, 0);
	transform->Scale(1000, 1000, 1000);
	skycube->UpdateTransforms();
	//mesh->Pass(1);

	
	//texture = new Texture(L"Environment/Ice_Lake_Env.hdr");
	texture = new Texture(L"Environment/Ice_Lake_Ref.hdr");
	shader->AsSRV("HDRMap")->SetResource(texture->SRV());


	render2D = new Render2D();
	render2D->SRV(texture->SRV());
	render2D->GetTransform()->Position(D3D::Width()/ 4, D3D::Height() / 4, 0);
	render2D->GetTransform()->Scale(D3D::Width() / 4, D3D::Height() / 4, 0);
	render2D->Pass(1);

	sphere = new PBRMeshRender(pbrShader, new MeshSphere(2.5,64,64));

	sphere2 = new MeshRender(shader, new MeshSphere(0.5));
	//sphere2->Pass(1);

	Mesh();
	Material();
}

void PBRBasicDemo::Destroy()
{
}

void PBRBasicDemo::Update()
{
	shader->AsSRV("HDRMap")->SetResource(texture->SRV());

	Vector3 p;
	Context::Get()->GetCamera()->Position(&p);
	p.y += 0.3f;
	skycube->GetTransform(0)->Position(p);
	skycube->UpdateTransforms();
	skycube->Update();
	render2D->Update();
	
	sphere->Update();
	//sphere2->Update();

	static float dist = -100.0f;

	ImGui::SliderFloat("dist", &dist, 10, 1000);
	pbrShader->AsScalar("PLightDist")->SetFloat(-dist);

	sphere->UpdateTransforms();

}

void PBRBasicDemo::PreRender()
{
}

void PBRBasicDemo::Render()
{
	for (MaterialPBR* material : materials)
	{
		material->Render();
	}
	skycube->Render();
	sphere->Render();
	//sphere2->Render();
}

void PBRBasicDemo::PostRender()
{
	render2D->Render();
}

void PBRBasicDemo::Mesh()
{
	float pstep = 6;
	float mstep = (float)1.0f / 7.0f;
	float rstep = (1 - 0.04f) / 7.0f;

	for (int i = 0; i < 7; i++)
	{
		float metal = Math::Clamp((float)i * mstep,0,1);

		for (int j = 0; j < 7; j++)
		{
			float rough = Math::Clamp(0.04f + (float)j * rstep, 0.04f,1);
			Transform * transform = sphere->AddTransform();
			transform->Position(pstep * j, pstep * i, 0);
			UINT index = (UINT)(7 * i) + j;
		
		}
	}

	sphere->UpdateTransforms();
}

void PBRBasicDemo::Material()
{
	MaterialPBR* material = new MaterialPBR(pbrShader);
	material->AlbedoMap(L"rustediron/rustediron2_basecolor.png");
	material->MetallicMap(L"rustediron/rustediron2_metallic.png");
	material->RoughnessMap(L"rustediron/rustediron2_roughness.png");
	material->NormalMap(L"rustediron/rustediron2_normal.png");

	materials.push_back(material);

	material = new MaterialPBR(pbrShader);
	material->AlbedoMap(L"wornfactory/worn-factory-siding_albedo.png");
	material->MetallicMap(L"wornfactory/worn-factory-siding_metallic.png");
	material->RoughnessMap(L"wornfactory/worn-factory-siding_roughness.png");
	material->NormalMap(L"wornfactory/worn-factory-siding_normal-dx.png");
	material->AOMap(L"wornfactory/worn-factory-siding_ao.png");

	materials.push_back(material);
}
