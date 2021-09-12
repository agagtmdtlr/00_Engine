#include "stdafx.h"
#include "Meshes/PBRMeshRender.h"
#include "PBR/LightMeshRender.h"
#include "PBR/MaterialPBR.h"
#include "PBR/PostFX.h"
#include "Objects/GBufferPBR.h"
#include "Objects/ShadowCube.h"
#include "Objects/ShadowSpot.h"
#include "ScreenSpaceFX/SSRManager.h"
#include "PBRDefferedDemo.h"	

void PBRDefferedDemo::Initialize()
{
	// static texture initialize 
	MaterialPBR::BRDFMap(L"BRDF/BRDF2.dds");
	sky = new CubeSky(L"Environment/Arches_E_PineTree/Arches_E_PineTree_8k_Environment.dds");

	Context::Get()->GetCamera()->RotationDegree(30, -347, 0);
	//Context::Get()->GetCamera()->Position(0, 4, -10);
	Context::Get()->GetCamera()->Position(-21, 67, -140);
	((Freedom*)Context::Get()->GetCamera())->Speed(40);

	shader = new Shader(L"128_PBR_IBL.fxo");
	testShader = new Shader(L"122_SpotLight.fxo");

	Vector3 r = Vector3(10, 0, 0);
	r = Math::ToRadian(r);
	Quaternion q = Math::ToQuaternion(r);
	r = Math::ToEuler(q);
	r = Math::ToDegree(r);
	
	
	DirectionalLighting();
	PointLighting();
	SpotLighting();

	UINT cnt =  Lighting::Get()->PointLightCount();
	// shadow cube create by point light counts;
	shadowCube = new ShadowCube(shader, 1024, 1024,
		DXGI_FORMAT_R8G8B8A8_UNORM, false,2);
	shadowSpot = new ShadowSpot(shader, 512, 512,
		DXGI_FORMAT_R8G8B8A8_UNORM, false, 2);

	LightMesh();
	Mesh();
	Material();
	Model();
	Animator();

	CameraSetting();

	IBL();
	ChangeIBL();
	UINT size = 1024;
	shadow = new Shadow(shader, Vector3(0, 0, 0), 65, size, size);

	gbuffer = new GBufferPBR(shader);
	gbuffer->DrawDebug(false);
	postfx = new PostFX(L"128_PostFX.fx");	

	ssrManager = new SSRManager(shader);
}

void PBRDefferedDemo::Destroy()
{

	SafeDelete(shadowSpot);
	SafeDelete(shadowCube);
	SafeDelete(shadow);
	SafeDelete(gbuffer);
	SafeDelete(postfx);
}

void PBRDefferedDemo::Update()
{
	LightManage();
	ModelManage();
	CameraManage();

	for (auto sph : sphere)
	{
		sph->Update();
		sph->UpdateTransforms();
	}

	BigSphere->Update();
	BigSphere->UpdateTransforms();

	cube->Update();
	cube->UpdateTransforms();
	ssaoCube->Update();
	ssaoCube->UpdateTransforms();

	for (int i = 0; i < 2; i++)
	{
		grid[i]->Update();
		grid[i]->UpdateTransforms();
	}
	
	cylinder->Update();
	cylinder->UpdateTransforms();

	if (model)
	{
		model->Update();
		//model->UpdateTransforms();
	}

	if (animator)
	{
		animator->Update();
		//animator->UpdateTransforms();
	}
	gbuffer->Update();
	
	ssrManager->Update();

	postfx->Update();

	sky->Update();

	lightMesh->Update();
}

void PBRDefferedDemo::PreRender()
{
	// Shadow Depth Map Render :: Orthological
	{
		shadow->PreRender(); // shadow map bind
		Pass(4, 5, 6);
		for (int i = 0; i < 10; i++)
		{
			sphereMaterials[i]->Render();
			sphere[i]->Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			sphere[i]->Render();
		}
		BigSphereMaterials->Render();
		BigSphere->Render();
		cubeMaterials[0]->Render();
		cube->Render();
		cylinderMaterials[0]->Render();
		cylinder->Render();

		gridMaterials[grid_mat_index]->Render();
		grid[0]->Render();
		gridMaterials[1]->Render();
		grid[1]->Render();

		if (model)
		{
			model->Render();
		}
		if (animator && bRenderAnim)
		{
			animator->Render();
		}
	}
	// Shadow Depth Map Render :: Perspective Point
	{
		Pass(17, 18, 19);
		UINT cnt = Lighting::Get()->PointLightCount();
		for (UINT i = 0; i < 2; i++)
		{
			PointLight& l = Lighting::Get()->GetPointLight(i);
			shadowCube->PreRender(i);
			for (int i = 0; i < 10; i++)
			{
				sphereMaterials[i]->Render();
				sphere[i]->Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				sphere[i]->Render();
			}

			BigSphereMaterials->Render();
			BigSphere->Render();

			gridMaterials[grid_mat_index]->Render();
			grid[0]->Render();
			gridMaterials[1]->Render();
			grid[1]->Render();

			cubeMaterials[0]->Render();
			cube->Render();
			cylinderMaterials[0]->Render();
			cylinder->Render();

			if (model)
			{
				model->Render();
			}
			if (animator && bRenderAnim)
			{
				animator->Render();
			}
		}
	}
	// Shadow Depth Map Render :: SpotLight
	{
		Pass(20, 21, 22);
		UINT cnt = Lighting::Get()->SpotLightPBRCount();
		for (UINT i = 0; i < cnt; i++)
		{
			SpotLightPBR& l = Lighting::Get()->GetSpotLightPBR(i);
			shadowSpot->PreRender(i);
			for (int i = 0; i < 10; i++)
			{
				sphereMaterials[i]->Render();
				sphere[i]->Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				sphere[i]->Render();
			}
			BigSphereMaterials->Render();
			BigSphere->Render();

			cubeMaterials[0]->Render();
			cube->Render();
			cylinderMaterials[0]->Render();
			cylinder->Render();

			gridMaterials[grid_mat_index]->Render();
			grid[0]->Render();
			gridMaterials[1]->Render();
			grid[1]->Render();

			if (model)
			{
				model->Render();
			}
			if (animator && bRenderAnim)
			{
				animator->Render();
			}
		}
	}
	// GBuffer MRT render
	{
		Pass(7, 8, 9);
		gbuffer->PreRender(); // deffered 1 pass;// setting mrt;
		for (int i = 0; i < 10; i++)
		{
			sphereMaterials[i]->Render();			
			sphere[i]->Topology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			sphere[i]->Pass(10);
			sphere[i]->Render();
		}
		BigSphereMaterials->Render();
		BigSphere->Render();

		cubeMaterials[0]->Render();
		cube->Render();

		cubeMaterials[1]->Render();
		ssaoCube->Render();

		cylinderMaterials[0]->Render();
		cylinder->Render();

		// grid
		gridMaterials[grid_mat_index]->Render();
		grid[0]->Render();
		gridMaterials[1]->Render();
		grid[1]->Render();

		if (model)
		{
			model->Render();
		}
		if (animator && bRenderAnim)
		{
			animator->Render();
		}
	}
	postfx->HDRMap(gbuffer->GetResult());
	postfx->GBufferDepthMap(gbuffer->GetDepthSRV());
}

void PBRDefferedDemo::Render()
{
	shadowCube->Render();
	shadowSpot->Render();
	gbuffer->ReadySkyRender();
	sky->Render();

	// screen space ambient occlusion + deffered light render
	gbuffer->ReadyRender();
	gbuffer->Pass(11, 13, 14);
	gbuffer->DebugPass(15, 16);
	gbuffer->Render();

	if (bDrawLight)
	{
		lightMesh->Render();
	}

	// screen space reflection
	// 1pass search ray tracing for reflection data
	ssrManager->Parameter(gbuffer->GetResultRTV(),gbuffer->GetResult(),gbuffer->GetDepthSRV(), gbuffer->GetReadOnlyDSV());
	ssrManager->PreRenderReflection();
	//ssrManager->FullScreenReflection();
	if(ssrManager->IsSSROn() == true)
	{
		Pass(24, 25, 26);
		gridMaterials[grid_mat_index]->Render();
		grid[0]->Render();

		BigSphereMaterials->Render();
		BigSphere->Render();	
	}
	//// 2pass blend hdr and reflection;
	ssrManager->DoReflectionBlend();
}

void PBRDefferedDemo::PostRender()
{
	gbuffer->SetReadOnlyDSV();
	postfx->PostProcessing();
	gbuffer->PostRender();
}

void PBRDefferedDemo::ResizeScreen()
{
	gbuffer->ResizeScreen();
	ssrManager->ResizeScreen();
	postfx->ResizeScreen();
}

void PBRDefferedDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (PBRMeshRender* m : meshes)
		m->Pass(mesh);
	for (ModelRenderPBR* m : models)
		m->Pass(model);
	for (ModelAnimatorPBR* m : animators)
		m->Pass(anim);
}

void PBRDefferedDemo::Mesh()
{
	{
		Transform* transform = NULL;
		// cube
		cube = new PBRMeshRender(shader, new MeshCube());
		transform = cube->AddTransform();
		transform->Position(0, 5, 0);
		transform->Scale(20, 10, 20);
		// ssao cube
		ssaoCube = new PBRMeshRender(shader, new MeshCube());

		for (UINT i = 0; i < 4; i++) // y
		{
			for (UINT j = 0; j < 16; j++) // xz
			{
				transform = ssaoCube->AddTransform();
				float x = (float)(j % 4) * 11.0f - 16.5f;
				float z = (float)(j / 4) * 11.0f - 16.5f + 150.0f;
				float y = (float)i * 12.0f + 5.0f;
				transform->Position(x, y, z);
				transform->Scale(10, 10, 10);
			}
		}
		// grid
		grid[0] = new PBRMeshRender(shader, new MeshGrid(15, 15));
		transform = grid[0]->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(20, 10, 20);
		transform->RotationDegree(0, 45, 0);

		grid[1] = new PBRMeshRender(shader, new MeshGrid(15, 15));
		transform = grid[1]->AddTransform();
		transform->Position(40, 10, 0);
		transform->Scale(20, 10, 20);
		transform->RotationDegree(0, 0, 90);

		cylinder = new PBRMeshRender(shader, new MeshCylinder(0.5f, 3.0f, 20, 20));
		// spheres
		for (UINT i = 0; i < 10; i++)
		{
			sphere[i] = new PBRMeshRender(shader, new MeshSphere(0.5));
			sphere[i]->Topology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			sphere[i]->Pass(3);
			transform = sphere[i]->AddTransform();
			transform->Position((i%2) ? -30.0f : 30.0f, 15.5f, -15.0f + (float)(i/2) * 15.0f);
			transform->Scale(5, 5, 5);

			sphere[i]->UpdateTransforms();

			meshes.push_back(sphere[i]);
		}
		//big sphere
		BigSphere = new PBRMeshRender(shader, new MeshSphere(0.5));
		transform = BigSphere->AddTransform();
		transform->Position(50, 60 ,70);
		transform->Scale(50, 50, 50);
		BigSphere->UpdateTransforms();
		meshes.push_back(BigSphere);
		//cylinder
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
	grid[0]->UpdateTransforms();
	grid[1]->UpdateTransforms();


	meshes.push_back(cylinder);
	meshes.push_back(cube);

	meshes.push_back(grid[0]);
	meshes.push_back(grid[1]);

	meshes.push_back(ssaoCube);

}

void PBRDefferedDemo::Model()
{
	model = new ModelRenderPBR(shader);
	model->ReadMesh(L"SpaceSuit/SpaceSuit3");

	Performance p;
	p.Start();
	model->ReadMaterial(L"Spacesuit/SpaceSuitTest");
	wstring s = to_wstring(p.End());
	//MessageBox(D3D::GetHandle(), s.c_str(), L"",MB_OK);
	Transform * transform = model->AddTransform();
	transform->Scale(15.0f / 185.0f, 15.0f / 185.0f, 15.0f / 185.0f);
	transform->Position(0, 0, -30);
	transform->RotationDegree(90, 0, 0);

	model->Update();
	model->UpdateTransforms();

	models.push_back(model);
}

void PBRDefferedDemo::Animator()
{
	animator = new ModelAnimatorPBR(shader);
	animator->ReadMesh(L"SpaceSuit/SpaceSuit3");
	animator->ReadMaterial(L"Spacesuit/SpaceSuit");
	animator->ReadClip(L"Spacesuit/Swimming");
	animator->ReadClip(L"Spacesuit/Running");
	animator->ReadClip(L"Spacesuit/Walking");
	animator->ReadClip(L"Spacesuit/Silly Dancing2");
	//0
	Transform * transform = animator->AddTransform();
	transform->Scale(15 / 185.0f, 15 / 185.0f, 15 / 185.0f);
	transform->Position(20, 0, -30);
	//1
	transform = animator->AddTransform();
	transform->Scale(15 / 185.0f, 15 / 185.0f, 15 / 185.0f);
	transform->Position(-20, 0, -30);
	//2
	transform = animator->AddTransform();
	transform->Scale(15 / 185.0f, 15 / 185.0f, 15 / 185.0f);
	transform->Position(0, 0, -60);
	//3
	transform = animator->AddTransform();
	transform->Scale(15 / 185.0f, 15 / 185.0f, 15 / 185.0f);
	transform->Position(0, 10, 0);

	animator->PlayTweenMode(0, 0, 1.2f, 1.0f);
	animator->PlayTweenMode(1, 1, 1.0f, 1.0f);
	animator->PlayTweenMode(2, 2, 1.2f, 1.0f);
	animator->PlayTweenMode(3, 3, 1.0f, 1.0f);

	//animator->PlayBlendMode(1, 0, 1, 2);
	//animator->SetBlendAlpha(1, 0.5f);

	animator->UpdateTransforms();
	animator->Update();

	animators.push_back(animator);
}

void PBRDefferedDemo::DirectionalLighting()
{
	Vector2 uv = { 0.583750f, 0.365000f };
	// -0.25 ~ 0.75 start
	uv.x -= 0.25; // -0.5 ~ 0.5
	uv.x /= 0.5; // -1 ~ 1
	uv *= Math::PI;

	Vector3 dir = {
		sinf(uv.y)*cosf(uv.x),
		cosf(uv.y),
		sinf(uv.y)*sinf(uv.x),
	};
	D3DXVec3Normalize(&dir, &dir);

	Vector3& L = Context::Get()->Direction();
	L = -dir;
	Color& c = Context::Get()->Specular();
	c = Color(254 / 255.0f, 241 / 255.0f, 224 / 255.0f, 1);
	c.a = 1;
}

void PBRDefferedDemo::PointLighting()
{
	PointLight light;
	light =
	{
		Color(1.0f, 1.0f, 1.0f, 1.0f), 
		Color(1.0f, 1.0f, 1.0f, 1.0f), 
		Color(1.0f, 1.0f, 1.0f, 1.0f), 
		Color(1.0f, 1.0f, 1.0f, 1.0f), 
		Vector3(-55, 10, -30), 250.0f, 1.0f //position, range, intensity
	};
	Lighting::Get()->AddPointLight(light);

	light =
	{
		Color(1.0f, 1.0f, 1.0f, 1.0f),
		Color(1.0f, 1.0f, 1.0f, 1.0f),
		Color(1.0f, 1.0f, 1.0f, 1.0f),
		Color(1.0f, 1.0f, 1.0f, 1.0f),
		Vector3(15, 10, -30), 50.0f, 15.0f
	};
	Lighting::Get()->AddPointLight(light);

	for (float z = -30; z <= 30; z += 10)
	{
		for (float x = -30; x <= 30; x += 10)
		{
			light =
			{
				Color(0, 0, 0, 1),
				Math::RandomColor3(),
				Color(0, 0, 0, 1),
				Color(0, 0, 0, 1),
				Vector3(x, 2, z), 10, Math::Random(15.0f, 20.0f)
			};
	
			Lighting::Get()->AddPointLight(light);
		}
	}
}

void PBRDefferedDemo::SpotLighting()
{
	SpotLightPBR light =
	{
		Vector3(0,5,-35), //pos
		150.0f, // range
		Vector3(0, 0, -1), // dir
		25.0f, // out angle
		Vector3(1,1,1), // color
		24.0f, // inner angle
		1
	};
	Lighting::Get()->AddSpotLightPBR(light);
	light =
	{
		Vector3(0,40,0), //pos
		150.0f, // range
		Vector3(0, -1, 0), // dir
		15.0f, // out angle
		Vector3(1,0.5,0.5), // color
		12.0f, // inner angle
		5
	};
	Lighting::Get()->AddSpotLightPBR(light);

	SpotLight l =
	{
		Color(1.0f, 0.2f, 0.9f, 1.0f), // ambient
		Color(1.0f, 0.2f, 0.9f, 1.0f), // diffuse
		Color(1.0f, 0.2f, 0.9f, 1.0f), // specular
		Color(1.0f, 0.2f, 0.9f, 1.0f), // emissive
		Vector3(0, 20, -30), // position
		30.0f, // range
		Vector3(0, -1, 0), // directon
		40.0f, // angle
		0.9f // intensity
	};
	Lighting::Get()->AddSpotLight(l);
}

void PBRDefferedDemo::LightMesh()
{
	lightMesh = new LightMeshRender();
}

void PBRDefferedDemo::CameraSetting()
{
	cameras[0] = Context::Get()->GetCamera();
	cameras[1] = new Spin();

	for (UINT i = 0; i < 10; i++)
	{
		sphere[i]->GetTransform(0)->Position(&cameraPositons[i]);
	}

	for (UINT i = 0; i < 4; i++)
	{
		animator->GetTransform(i)->Position(&cameraPositons[i + 10]);
	}
}

void PBRDefferedDemo::LightManage()
{
	if (ImGui::CollapsingHeader("Light Manage"))
	{
		float& sun = Context::Get()->Intensity();
		ImGui::SliderFloat("sun power", &sun, 0.0f, 10.0f);

		ImGui::Checkbox("light on / off", &bDrawLight);
		gbuffer->DrawLight(bDrawLight);

		PointLight& point = Lighting::Get()->GetPointLight(0);
		static float power = point.Intensity;
		ImGui::SliderFloat("point power", &power, 1.0f, 1000.0f);
		point.Intensity = power;

		ImGui::SliderFloat3("point position", (float*)&point.Position, -100, 100);

		SpotLightPBR& spot = Lighting::Get()->GetSpotLightPBR(0);
		static float sp = spot.color.x;
		ImGui::SliderFloat("spot power", &sp, 1.0f, 100.0f);
		spot.Intensity = sp;

		ImGui::SliderFloat("GAdaption", &gAdaption, 0, 1.0f);
		{
			float NormVal = -1.0f;
			float fVal = -1.0f;

			static bool bFirstTime = true;


			if (bFirstTime == true)
			{
				// On the first frame we want to fully adapt the new value so use 0
				AdaptionNorm = 0.0f;
				bFirstTime = false;
			}
			else
			{
				// Normalize the adaptation time with the frame time (all in seconds)
				// Never use a value higher or equal to 1 since that means no adaptation at all (keeps the old value)
				AdaptionNorm = min(gAdaption < 0.0001f ? 1.0f : Time::Delta() * gAdaption, 0.9999f);
			}
			postfx->SetTonemapParameter(AdaptionNorm);

		}

		ImGui::InputInt("Environment INDEX", (int*)&IBL_index);
		{
			IBL_index %= IBLs.size();
		}
		if (ImGui::Button("Environment Change"))
		{
			ChangeIBL();
		}
	}

	D3DXMATRIX rot;

	UINT count = Lighting::Get()->PointLightCount();
	for (UINT i = 2; i < count; i++)
	{
		D3DXMatrixRotationYawPitchRoll(&rot, Math::Random(0.1f, 1.0f) * 1.0f * Time::Delta(), 0, 0);
		PointLight & light = Lighting::Get()->GetPointLight(i);
		Vector3 prevp = light.Position;
		D3DXVec3TransformCoord(&light.Position, &prevp, &rot);
		light.Position.y = 15.0f + sinf(Time::Get()->Running() + float(i)) * 10.0f;
	}
}

void PBRDefferedDemo::ModelManage()
{
	if(ImGui::CollapsingHeader("Model Manage"))
	{
		Transform * transform = BigSphere->GetTransform(0);
		Vector3 p;
		transform->Position(&p);
		ImGui::SliderFloat3("Big Sphere Position", (float*)&p,-100.0f,100.0f);
		transform->Position(p);

		ImGui::InputInt("plane material", (int*)&grid_mat_index);
		grid_mat_index %= gridMaterials.size();
	}



}

void PBRDefferedDemo::CameraManage()
{
	if (ImGui::CollapsingHeader("Camera Manage"))
	{
		UINT prev_mode = camera_mode;
		ImGui::InputInt("Camera Mode", (int*)&camera_mode);
		camera_mode %= 2;

		if (prev_mode != camera_mode)
		{
			Context::Get()->SetCamera(cameras[camera_mode]);

			if(camera_mode == 1)
				((Spin*)cameras[camera_mode])->Center(cameraPositons[camera_index]);

		}

		UINT prev_index = camera_index;
		ImGui::InputInt("Camera Index", (int*)&camera_index);
		camera_index %= 14;

		if (camera_mode == 1)
		{
			if (prev_index != camera_index)
				((Spin*)cameras[camera_mode])->Center(cameraPositons[camera_index]);
		}
	}
}

void PBRDefferedDemo::IBL()
{
	IBLs.assign(2, IBL_Texture());

	IBLs[0].EnvironmentMap = new Texture(L"Environment/Milkyway/Milkyway_BG_Environment.dds");
	IBLs[0].IrradianceMap = new Texture(L"Environment/Milkyway/Milkyway_Light_irrcube.dds");
	IBLs[0].ReflectionMap = new Texture(L"Environment/Milkyway/Milkyway_small_Reflection.dds");
	
	IBLs[1].EnvironmentMap = new Texture(L"Environment/Arches_E_PineTree/Arches_E_PineTree_8k_Environment.dds");
	IBLs[1].IrradianceMap = new Texture(L"Environment/Arches_E_PineTree/Arches_E_PineTree_Env_irrcube.dds");
	IBLs[1].ReflectionMap = new Texture(L"Environment/Arches_E_PineTree/Arches_E_PineTree_3k_Reflection.dds");

}

void PBRDefferedDemo::ChangeIBL()
{
	sky->SetSRV(IBLs[IBL_index].EnvironmentMap->SRV());
	MaterialPBR::IrradianceCube(IBLs[IBL_index].IrradianceMap);
	MaterialPBR::ReflectionCube(IBLs[IBL_index].ReflectionMap);
}


void PBRDefferedDemo::Material()
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
	// big sphere
	BigSphereMaterials = new MaterialPBR(shader);
	BigSphereMaterials->MetallicMap(L"_Textures/White.png");
	BigSphereMaterials->RoughnessMap(L"_Textures/Black.png");
	//grid
	{
		material = new MaterialPBR(shader);
		
		material->AlbedoMap(L"_Textures/White.png");
		material->MetallicMap(L"_Textures/White.png");
		material->RoughnessMap(L"_Textures/Black.png");


		gridMaterials.push_back(material);

		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/Tiles093_1K-JPG/Tiles093_1K_Color.jpg");
		material->RoughnessMap(L"PBR/Tiles093_1K-JPG/Tiles093_1K_Roughness.jpg");
		material->AOMap(L"PBR/Tiles093_1K-JPG/Tiles093_1K_AmbientOcclusion.jpg");
		material->NormalMap(L"PBR/Tiles093_1K-JPG/Tiles093_1K_Normal.jpg");
		material->ParallaxMap(L"PBR/Tiles093_1K-JPG/Tiles093_1K_Displacement.jpg");

		material->HeightSacle(0.07f);
		gridMaterials.push_back(material);

		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/Gravel025_1K-PNG/Gravel025_1K_Color.png");
		material->RoughnessMap(L"PBR/Gravel025_1K-PNG/Gravel025_1K_Roughness.png");
		material->NormalMap(L"PBR/Gravel025_1K-PNG/Gravel025_1K_NormalDX.png");
		material->ParallaxMap(L"PBR/Gravel025_1K-PNG/Gravel025_1K_Displacement.png");
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

		material = new MaterialPBR(shader);
		material->AlbedoMap(L"PBR/Marble021_1K-PNG/Marble021_1K_Color.png");
		material->RoughnessMap(L"PBR/Marble021_1K-PNG/Marble021_1K_Roughness.png");
		material->NormalMap(L"PBR/Marble021_1K-PNG/Marble021_1K_Normal.png");

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

}

