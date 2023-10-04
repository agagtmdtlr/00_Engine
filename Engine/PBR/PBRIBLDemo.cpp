#include "stdafx.h"
#include "Meshes/PBRMeshRender.h"
#include "PBR/MaterialPBR.h"
#include "PBRIBLDemo.h"	

void PBRIBLDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(5, 11, 0);
	//Context::Get()->GetCamera()->Position(0, 4, -10);
	Context::Get()->GetCamera()->Position(0, 0, -5);

	shader = new Shader(L"128_PBR_IBL.fx");

	Mesh();
	Material();
	//Model();

	UINT size = 1024;
	Vector3 v(0, 0, 0);
	shadow = new Shadow(shader,v, 65, size, size);
	render2D = new Render2D();
	render2D->GetTransform()->Position(150, D3D::Height() - 150, 0);
	render2D->GetTransform()->Scale(300, 300, 1);
	render2D->SRV(shadow->SRV());
	

	
}

void PBRIBLDemo::Destroy()
{
}

void PBRIBLDemo::Update()
{
	
	for (auto sph : sphere)
	{
		sph->Update();
		sph->UpdateTransforms();
	}
	cube->Update();
	cube->UpdateTransforms();
	grid->Update();
	grid->UpdateTransforms();
	cylinder->Update();
	cylinder->UpdateTransforms();

	if (model)
	{
		model->Update();
		model->UpdateTransforms();
	}

	sky->Update();
	render2D->Update();
}

void PBRIBLDemo::PreRender()
{
	shadow->PreRender(); // shadow map bind

	Pass(4, 5, 6);
	for (int i = 0; i < 10; i++)
	{
		sphereMaterials[i]->Render();
		sphere[i]->Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		sphere[i]->Render();
	}
	cubeMaterials[0]->Render();
	cube->Render();
	cylinderMaterials[0]->Render();
	cylinder->Render();
	gridMaterials[0]->Render();
	grid->Render();

	if (model)
	{
		model->Render();
	}
}

void PBRIBLDemo::Render()
{
	Pass(0, 1, 2);


	sky->Render();

	for (int i = 0; i < 10; i++)
	{
		sphereMaterials[i]->Render();
		sphere[i]->Topology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		sphere[i]->Pass(3);
		sphere[i]->Render();
	}
	
	cubeMaterials[0]->Render();
	cube->Render();
	cylinderMaterials[0]->Render();
	cylinder->Render();
	gridMaterials[0]->Render();
	grid->Render();

	if (model)
	{
		model->Render();
	}
	
}

void PBRIBLDemo::PostRender()
{
	render2D->Render();
}

void PBRIBLDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (PBRMeshRender* m : meshes)
		m->Pass(mesh);
	for (ModelRenderPBR* m : models)
		m->Pass(model);
}

void PBRIBLDemo::Mesh()
{
	{
		Transform* transform = NULL;

		cube = new PBRMeshRender(shader, new MeshCube());
		transform = cube->AddTransform();
		transform->Position(0, 5, 0);
		transform->Scale(20, 10, 20);

		grid = new PBRMeshRender(shader, new MeshGrid(15, 15));
		transform = grid->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(20, 10, 20);

		cylinder = new PBRMeshRender(shader, new MeshCylinder(0.5f, 3.0f, 20, 20));

		for (UINT i = 0; i < 10; i++)
		{
			sphere[i] = new PBRMeshRender(shader, new MeshSphere(0.5));
			sphere[i]->Topology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			sphere[i]->Pass(3);
			transform = sphere[i]->AddTransform();
			transform->Position((float)(i%2) ? -30 : 30.0f, 15.5f, -15.0f + (float)(i/2) * 15.0f);
			transform->Scale(5, 5, 5);

			sphere[i]->UpdateTransforms();

			meshes.push_back(sphere[i]);
		}

		for (UINT i = 0; i < 5; i++)
		{
			transform = cylinder->AddTransform();
			transform->Position(-30, 6, -15.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);

			transform = cylinder->AddTransform();
			transform->Position(30, 6, -15.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);
		}
	}
	cylinder->UpdateTransforms();
	cube->UpdateTransforms();
	grid->UpdateTransforms();

	meshes.push_back(cylinder);
	meshes.push_back(cube);
	meshes.push_back(grid);

}

void PBRIBLDemo::Model()
{
	model = new ModelRenderPBR(shader);
	model->ReadMesh(L"SpaceSuit/SpaceSuit");

	Performance p;
	p.Start();
	model->ReadMaterial(L"Spacesuit/SpaceSuit");
	wstring s = to_wstring(p.End());
	//MessageBox(D3D::GetHandle(), s.c_str(), L"",MB_OK);
	Transform * transform = model->AddTransform();
	transform->Scale(15 / 185.0f, 15 / 185.0f, 15 / 185.0f);
	transform->Position(0, 0, -30);

	model->Update();


	models.push_back(model);
}

void PBRIBLDemo::Animator()
{
	animator = new ModelAnimatorPBR(shader);
	animator->ReadMesh(L"SpaceSuit/SpaceSuit");
	animator->ReadMaterial(L"Spacesuit/SpaceSuit");
}


void PBRIBLDemo::Material()
{
	MaterialPBR* material = new MaterialPBR(shader);
	//sphere
	{
		//1
		material->AlbedoMap(L"rustediron/rustediron2_basecolor.png");
		material->MetallicMap(L"rustediron/rustediron2_metallic.png");
		material->RoughnessMap(L"rustediron/rustediron2_roughness.png");
		material->NormalMap(L"rustediron/rustediron2_normal.png");
		sphereMaterials.push_back(material);
		//2
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"wornfactory/worn-factory-siding_albedo.png");
		material->MetallicMap(L"wornfactory/worn-factory-siding_metallic.png");
		material->RoughnessMap(L"wornfactory/worn-factory-siding_roughness.png");
		material->NormalMap(L"wornfactory/worn-factory-siding_normal-dx.png");
		material->AOMap(L"wornfactory/worn-factory-siding_ao.png");
		material->DisplacementMap(L"wornfactory/worn-factory-siding_height.png");
		sphereMaterials.push_back(material);
		//3
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/Fabric048_1K-PNG/Fabric048_1K_Color.png");
		material->RoughnessMap(L"PBR/Fabric048_1K-PNG/Fabric048_1K_Roughness.png");
		material->NormalMap(L"PBR/Fabric048_1K-PNG/Fabric048_1K_Normal.png");
		material->AOMap(L"PBR/Fabric048_1K-PNG/Fabric048_1K_AmbientOcclusion.png");
		material->DisplacementMap(L"PBR/Fabric048_1K-PNG/Fabric048_1K_Displacement.png");
		sphereMaterials.push_back(material);
		//4
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/Marble016_1K-PNG/Marble016_1K_Color.png");
		material->RoughnessMap(L"PBR/Marble016_1K-PNG/Marble016_1K_Roughness.png");
		material->NormalMap(L"PBR/Marble016_1K-PNG/Marble016_1K_Normal.png");
		material->DisplacementMap(L"PBR/Marble016_1K-PNG/Marble016_1K_Displacement.png");
		sphereMaterials.push_back(material);
		//5
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/Metal034_1K-PNG/Metal034_1K_Color.png");
		material->MetallicMap(L"PBR/Metal034_1K-PNG/Metal034_1K_Metalness.png");
		material->NormalMap(L"PBR/Metal034_1K-PNG/Metal034_1K_Normal.png");
		material->RoughnessMap(L"PBR/Metal034_1K-PNG/Metal034_1K_Roughness.png");
		material->DisplacementMap(L"PBR/Metal034_1K-PNG/Metal034_1K_Displacement.png");
		sphereMaterials.push_back(material);
		//6
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/Metal035_1K-PNG/Metal035_1K_Color.png");
		material->MetallicMap(L"PBR/Metal035_1K-PNG/Metal035_1K_Metalness.png");
		material->NormalMap(L"PBR//Metal035_1K-PNG/Metal035_1K_Normal.png");
		material->RoughnessMap(L"PBR//Metal035_1K-PNG/Metal035_1K_Roughness.png");
		material->DisplacementMap(L"PBR//Metal035_1K-PNG/Metal035_1K_Displacement.png");
		sphereMaterials.push_back(material);
		//7
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/PaintedBricks001_1K-PNG/PaintedBricks001_1K_Color.png");
		material->RoughnessMap(L"PBR/PaintedBricks001_1K-PNG/PaintedBricks001_1K_Roughness.png");
		material->NormalMap(L"PBR/PaintedBricks001_1K-PNG/PaintedBricks001_1K_Normal.png");
		material->AOMap(L"PBR/PaintedBricks001_1K-PNG/PaintedBricks001_1K_AmbientOcclusion.png");
		material->DisplacementMap(L"PBR/PaintedBricks001_1K-PNG/PaintedBricks001_1K_Displacement.png");
		sphereMaterials.push_back(material);
		//8
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/PaintedMetal017_1K-JPG/PaintedMetal017_1K_Color.jpg");
		material->MetallicMap(L"PBR/PaintedMetal017_1K-JPG/PaintedMetal017_1K_Metalness.jpg");
		material->RoughnessMap(L"PBR/PaintedMetal017_1K-JPG/PaintedMetal017_1K_Roughness.jpg");
		material->NormalMap(L"PBR/PaintedMetal017_1K-JPG/PaintedMetal017_1K_Normal.jpg");
		material->AOMap(L"PBR/PaintedMetal017_1K-JPG/PaintedMetal017_1K_AmbientOcclusion.jpg");
		material->DisplacementMap(L"PBR/PaintedMetal017_1K-JPG/PaintedMetal017_1K_Displacement.jpg");
		sphereMaterials.push_back(material);
		//9
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/Tiles099_1K-JPG/Tiles099_1K_Color.jpg");
		material->RoughnessMap(L"PBR/Tiles099_1K-JPG/Tiles099_1K_Roughness.jpg");
		material->NormalMap(L"PBR/Tiles099_1K-JPG/Tiles099_1K_Normal.jpg");
		material->AOMap(L"PBR/Tiles099_1K-JPG/Tiles099_1K_AmbientOcclusion.jpg");
		material->DisplacementMap(L"PBR/Tiles099_1K-JPG/Tiles099_1K_Displacement.jpg");
		sphereMaterials.push_back(material);
		//10
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/Tiles106_1K-JPG/Tiles106_1K_Color.jpg");
		material->RoughnessMap(L"PBR/Tiles106_1K-JPG/Tiles106_1K_Roughness.jpg");
		material->NormalMap(L"PBR/Tiles106_1K-JPG/Tiles106_1K_Normal.jpg");
		material->AOMap(L"PBR/Tiles106_1K-JPG/Tiles106_1K_AmbientOcclusion.jpg");
		material->DisplacementMap(L"PBR/Tiles106_1K-JPG/Tiles106_1K_Displacement.jpg");
		sphereMaterials.push_back(material);
	}
	

	//grid
	{
		material = new MaterialPBR(shader);
		//material->AlbedoMap(L"PBR/PavingStones075_1K-PNG/PavingStones075_1K_Color.png");
		//material->RoughnessMap(L"PBR/PavingStones075_1K-PNG/PavingStones075_1K_Roughness.png");
		//material->AOMap(L"PBR/PavingStones075_1K-PNG/PavingStones075_1K_AmbientOcclusion.png");
		//material->NormalMap(L"PBR/PavingStones075_1K-PNG/PavingStones075_1K_Normal.png");
		//material->DisplacementMap(L"PBR/PavingStones075_1K-PNG/PavingStones075_1K_Displacement.png");

		material->AlbedoMap(L"PBR/Tiles093_1K-JPG/Tiles093_1K_Color.jpg");
		material->RoughnessMap(L"PBR/Tiles093_1K-JPG/Tiles093_1K_Roughness.jpg");
		material->AOMap(L"PBR/Tiles093_1K-JPG/Tiles093_1K_AmbientOcclusion.jpg");
		material->NormalMap(L"PBR/Tiles093_1K-JPG/Tiles093_1K_Normal.jpg");
		material->ParallaxMap(L"PBR/Tiles093_1K-JPG/Tiles093_1K_Displacement.jpg");

		material->HeightSacle(0.07f);
		gridMaterials.push_back(material);
	}

	//cube
	{
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/PavingStones070_1K-PNG/PavingStones070_1K_Color.png");
		material->RoughnessMap(L"PBR/PavingStones070_1K-PNG/PavingStones070_1K_Roughness.png");
		material->NormalMap(L"PBR/PavingStones070_1K-PNG/PavingStones070_1K_Normal.png");
		material->AOMap(L"PBR/PavingStones070_1K-PNG/PavingStones070_1K_AmbientOcclusion.png");
		material->DisplacementMap(L"PBR/PavingStones070_1K-PNG/PavingStones070_1K_Displacement.png");
		cubeMaterials.push_back(material);
	}
	// cylinder
	{
		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/Wood069_1K-PNG/Wood069_1K_Color.png");
		material->NormalMap(L"PBR/Wood069_1K-PNG/Wood069_1K_Normal.png");
		material->RoughnessMap(L"PBR/Wood069_1K-PNG/Wood069_1K_Roughness.png");
		cylinderMaterials.push_back(material);
	}
	

	//sky = new CubeSky(L"Environment/Ice_Lake_HiRes_TMap_cube.dds");
	//MaterialPBR::IrradianceCube(L"Environment/Ice_Lake_Env_cube.dds");
	//MaterialPBR::ReflectionCube(L"Environment/Ice_Lake_HiRes_TMap_Reflection.dds");
	
	sky = new CubeSky(L"Environment/Arches_E_PineTree/Arches_E_PineTree_8k_Environment.dds");
	MaterialPBR::IrradianceCube(L"Environment/Arches_E_PineTree/Arches_E_PineTree_Env_irrcube.dds");
	MaterialPBR::ReflectionCube(L"Environment/Arches_E_PineTree/Arches_E_PineTree_3k_Reflection.dds");
	
	//sky = new CubeSky(L"Environment/Circus_Backstage/Circus_Backstage_8k_Environment.dds");
	//MaterialPBR::IrradianceCube(L"Environment/Circus_Backstage/Circus_Backstage_Env_irrcube.dds");
	//MaterialPBR::ReflectionCube(L"Environment/Circus_Backstage/Circus_Backstage_3k_Reflection.dds");
	
	//sky = new CubeSky(L"Environment/Milkyway/Milkyway_BG_Environment.dds");
	//MaterialPBR::IrradianceCube(L"Environment/Milkyway/Milkyway_Light_irrcube.dds");
	//MaterialPBR::ReflectionCube(L"Environment/Milkyway/Milkyway_small_Reflection.dds");



	Vector2 uv = { 0.583750f, 0.365000f };
	// -0.25 ~ 0.75 start
	uv.x -= 0.25; // -0.5 ~ 0.5
	uv.x /= 0.5; // -1 ~ 1
	uv *=  Math::PI; 

	Vector3 dir = {
		sinf(uv.y)*cosf(uv.x),
		cosf(uv.y),
		sinf(uv.y)*sinf(uv.x),
	};
	D3DXVec3Normalize(&dir, &dir);

	Vector3& L = Context::Get()->Direction();
	L = -dir;


}

