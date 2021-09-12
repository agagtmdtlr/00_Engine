#include "stdafx.h"
#include "Meshes/PBRMeshRender.h"
#include "PBR/MaterialPBR.h"
#include "PBRIrradianceDemo.h"	

void PBRIrradianceDemo::Initialize()
{
	//Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(0, 4, -10);

	//shader = new Shader(L"125_PBR.fx");
	shader = new Shader(L"128_PBR_Irradiance.fx");


	//texture = new Texture(L"Environment/Ice_Lake_HiRes_TMap_cube.dds");
	texture = new Texture(L"Environment/Ice_Lake_HiRes_TMap_Irradiance.dds");




	sphere = new PBRMeshRender(shader, new MeshSphere(2.5,64,64));


	Mesh();
	Material();
}

void PBRIrradianceDemo::Destroy()
{
}

void PBRIrradianceDemo::Update()
{

	sphere->Update();
	sphere->UpdateTransforms();
	sky->Update();
}

void PBRIrradianceDemo::PreRender()
{
}

void PBRIrradianceDemo::Render()
{
	sky->Render();
	materials[0]->Render();
	sphere->Render();
}

void PBRIrradianceDemo::PostRender()
{
}

void PBRIrradianceDemo::Mesh()
{
	float pstep = 6;
	float mstep = (float)1.0f / 7.0f;
	float rstep = (1.0f - 0.04f) / 6.0f;

	for (int i = 0; i < 7; i++)
	{
		float metal = Math::Clamp((float)i * mstep,0,1);

		for (int j = 0; j < 7; j++)
		{
			float rough = Math::Clamp(0.04f + (float)j * rstep, 0.04f,1.0f);
			Transform * transform = sphere->AddTransform();
			transform->Position(pstep * j, pstep * i, 0);
			UINT index = (UINT)(7 * i) + j;
			
		}
	}

	

	sphere->UpdateTransforms();
}

void PBRIrradianceDemo::Material()
{
	//MaterialPBR* material = new MaterialPBR(pbrShader);
	//material->AlbedoMap(L"rustediron/rustediron2_basecolor.png");
	//material->MetallicMap(L"rustediron/rustediron2_metallic.png");
	//material->RoughnessMap(L"rustediron/rustediron2_roughness.png");
	//material->NormalMap(L"rustediron/rustediron2_normal.png");
	//
	//materials.push_back(material);

	//material = new MaterialPBR(pbrShader);
	//material->AlbedoMap(L"wornfactory/worn-factory-siding_albedo.png");
	//material->MetallicMap(L"wornfactory/worn-factory-siding_metallic.png");
	//material->RoughnessMap(L"wornfactory/worn-factory-siding_roughness.png");
	//material->NormalMap(L"wornfactory/worn-factory-siding_normal-dx.png");
	//material->AOMap(L"wornfactory/worn-factory-siding_ao.png");
	//
	//materials.push_back(material);

	MaterialPBR* material = new MaterialPBR(shader);
	
	//material->AlbedoMap(L"rustediron/rustediron2_basecolor.png");
	//material->MetallicMap(L"rustediron/rustediron2_metallic.png");
	//material->RoughnessMap(L"rustediron/rustediron2_roughness.png");
	//material->NormalMap(L"rustediron/rustediron2_normal.png");
	//material->IrradianceCube(L"Environment/HDR_040_Field/HDR_040_Field_Env_cube.dds");
	material->IrradianceCube(L"Environment/Ice_Lake_Env_cube.dds");

	materials.push_back(material);

	//sky = new CubeSky(L"Environment/HDR_040_Field/HDR_040_Field_cube.dds");
	sky = new CubeSky(L"Environment/Ice_Lake_HiRes_TMap_cube.dds");


}
