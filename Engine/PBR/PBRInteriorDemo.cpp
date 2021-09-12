#include "stdafx.h"
#include "Meshes/PBRMeshRender.h"
#include "PBR/LightMeshRender.h"
#include "PBR/StaticModelRender.h"
#include "PBR/MaterialPBR.h"
#include "PBR/PostFX.h"
#include "Objects/GBufferPBR.h"
#include "Objects/ShadowCube.h"
#include "Objects/ShadowSpot.h"
#include "ScreenSpaceFX/SSRManager.h"
#include "PBRInteriorDemo.h"	
void PBRInteriorDemo::Initialize()
{
	// static texture initialize 
	MaterialPBR::BRDFMap(L"BRDF/BRDF2.dds");
	sky = new CubeSky(L"Environment/Arches_E_PineTree/Arches_E_PineTree_8k_Environment.dds");

	Context::Get()->GetCamera()->RotationDegree(30, -347, 0);
	//Context::Get()->GetCamera()->Position(0, 4, -10);
	Context::Get()->GetCamera()->Position(-21, 67, -140);
	((Freedom*)Context::Get()->GetCamera())->Speed(20);

	camera = new Spin();
	Spin* spin = dynamic_cast<Spin*>(camera);
	spin->Speed(20, 3.0f);
	spin->Center(Vector3(0, 10, 0));
	Context::Get()->SetCamera(camera);

	shader = new Shader(L"128_PBR_IBL.fxo");
	testShader = new Shader(L"122_SpotLight.fxo");

	DirectionalLighting();
	//PointLighting();
	//SpotLighting();

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
	StaticMesh();
	Animator();

	IBL();
	ChangeIBL();
	UINT size = 1024;
	shadow = new Shadow(shader, Vector3(0, 0, 0), 65, size, size);
	render2D = new Render2D();
	render2D->GetTransform()->Position(150, D3D::Height() - 150, 0);
	render2D->GetTransform()->Scale(300, 300, 1);
	render2D->SRV(shadow->SRV());
	//render2D->SRV(shadowSpot->GetRenderTargetSRV());

	gbuffer = new GBufferPBR(shader);
	gbuffer->DrawDebug(false);
	postfx = new PostFX(L"128_PostFX.fx");	

	ssrManager = new SSRManager(shader);
}

void PBRInteriorDemo::Destroy()
{

	SafeDelete(shadowSpot);
	SafeDelete(shadowCube);
	SafeDelete(shadow);
	SafeDelete(render2D);
	SafeDelete(gbuffer);
	SafeDelete(postfx);
}

void PBRInteriorDemo::Update()
{
	//bool deomon = true;
	//ImGui::ShowDemoWindow(&deomon);

	LightManage();

	if (model)
	{
		//Transform* transform = model->GetTransform(0);
		//Vector3 r;
		//transform->RotationDegree(&r);
		//r.y += 5.0 * Time::Delta();

		//if (Keyboard::Get()->Press(VK_LEFT))
		//	r.y += 10.0 * Time::Delta();
		//if (Keyboard::Get()->Press(VK_RIGHT))
		//	r.y -= 20.0 * Time::Delta();


		//transform->RotationDegree(r);
		model->Update();
		model->UpdateTransforms();
	}

	

	staticModel->Update();
	staticModel->UpdateTransforms();

	animator->Update();

	render2D->Update();
	
	gbuffer->Update();
	
	ssrManager->Update();

	postfx->Update();

	sky->Update();

	lightMesh->Update();
}

void PBRInteriorDemo::PreRender()
{
	// Shadow Depth Map Render :: Orthological
	{
		shadow->PreRender(); // shadow map bind
		Pass(4, 5, 6);

		if (model)
		{
			model->Render();
		}

		staticModel->Render();
		//animator->Render();

	}
	// Shadow Depth Map Render :: Perspective Point
	{
		Pass(17, 18, 19);
		UINT cnt = Lighting::Get()->PointLightCount();
		for (UINT i = 0; i < 2; i++)
		{
			PointLight& l = Lighting::Get()->GetPointLight(i);
			shadowCube->PreRender(i);

			if (model)
			{
				model->Render();
			}

			staticModel->Render();
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

			if (model)
			{
				model->Render();
			}

			staticModel->Render();

		}
	}
	
	// GBuffer MRT render
	{
		Pass(7, 8, 9);
		gbuffer->PreRender(); // deffered 1 pass;// setting mrt;

		if (model)
		{
			model->Render();
		}

		staticModel->Render();
		animator->Render();

	}
	postfx->HDRMap(gbuffer->GetResult());
	postfx->GBufferDepthMap(gbuffer->GetDepthSRV());
}

void PBRInteriorDemo::Render()
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

	lightMesh->Render();

	// screen space reflection
	// 1pass search ray tracing for reflection data
	ssrManager->Parameter(gbuffer->GetResultRTV(),gbuffer->GetResult(),gbuffer->GetDepthSRV(), gbuffer->GetReadOnlyDSV());
	ssrManager->PreRenderReflection();
	//ssrManager->FullScreenReflection();
	if(ssrManager->IsSSROn() == true)
	{
		Pass(24, 25, 26);

	}
	//// 2pass blend hdr and reflection;
	ssrManager->DoReflectionBlend();

}

void PBRInteriorDemo::PostRender()
{
	//gbuffer->SaveTexture(L"../result.png");

	gbuffer->SetReadOnlyDSV();
	postfx->PostProcessing();
	gbuffer->PostRender();

	//render2D->Render();

	//ssrManager->PostRender();
}

void PBRInteriorDemo::ResizeScreen()
{
	gbuffer->ResizeScreen();
	ssrManager->ResizeScreen();
	postfx->ResizeScreen();
}

void PBRInteriorDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (PBRMeshRender* m : meshes)
		m->Pass(mesh);

	for (StaticModelRender* m : staticModels)
		m->Pass(mesh);

	for (ModelRenderPBR* m : models)
		m->Pass(model);

	for (ModelAnimatorPBR* m : animators)
		m->Pass(anim);
}

void PBRInteriorDemo::Mesh()
{
}

void PBRInteriorDemo::StaticMesh()
{
	staticModel = new StaticModelRender(shader);

	//staticModel->ReadMesh(L"winter_girl/winter_girl");
	//staticModel->ReadMaterial(L"winter_girl/winter_girl");
	//Transform * transform = staticModel->AddTransform();
	//transform->Scale(10.0f, 10.0f, 10.0f);
	//transform->RotationDegree(90.0f, 0, 0);
	//staticModel->ReadMaterial(L"Soi_Armour/Soi_Armour");

	staticModel->Update();
	staticModels.push_back(staticModel);
}

void PBRInteriorDemo::Model()
{
	model = new ModelRenderPBR(shader);
	//model->ReadMesh(L"retroTV/retroTV");	
	//model->GetModel()->MeshByIndex(0)->MaterialName(L"Cloth");
	//model->GetModel()->MeshByIndex(1)->MaterialName(L"Skin");
	//model->ReadMaterial(L"retroTV/retroTV");
	Transform * transform = model->AddTransform();
	transform->Scale(1/5.0f, 1/5.0f, 1/5.0f);
	//transform->Position(0, 0, -30);
	//transform->RotationDegree(90, 0, 0);

	


	model->Update();
	models.push_back(model);
}

void PBRInteriorDemo::Animator()
{
	animator = new ModelAnimatorPBR(shader);
	animator->ReadMesh(L"SpaceSuit/SpaceSuit3");
	animator->ReadMaterial(L"Spacesuit/SpaceSuit");
	animator->ReadClip(L"Spacesuit/Swimming");
	//0
	Transform * transform = animator->AddTransform();
	transform->Scale(15 / 185.0f, 15 / 185.0f, 15 / 185.0f);
	transform->Position(0, 0, 0);
	transform->RotationDegree(30, 0, 0);
	
	
	animator->PlayTweenMode(0, 0, 1.2f, 1.0f);
	animator->UpdateTransforms();
	animator->Update();

	animators.push_back(animator);
}


void PBRInteriorDemo::DirectionalLighting()
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

void PBRInteriorDemo::PointLighting()
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
				Vector3(x,  Math::Random(15.0f, 20.0f), z), 10, Math::Random(15.0f, 20.0f)
			};
	
			Lighting::Get()->AddPointLight(light);
		}
	}
}

void PBRInteriorDemo::SpotLighting()
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

void PBRInteriorDemo::LightMesh()
{
	lightMesh = new LightMeshRender();
}

void PBRInteriorDemo::LightManage()
{
	if (ImGui::CollapsingHeader("Light Manage"))
	{
		float& sun = Context::Get()->Intensity();
		ImGui::SliderFloat("sun power", &sun, 0.0f, 10.0f);

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
		D3DXMatrixRotationYawPitchRoll(&rot,Math::Random(0.1f,1.0f) * 1.0f * Time::Delta(), 0, 0);
		PointLight & light = Lighting::Get()->GetPointLight(i);
		Vector3 prevp = light.Position;
		D3DXVec3TransformCoord(&light.Position, &prevp, &rot);
		light.Position.y = 15.0f + sinf(Time::Get()->Running() + float(i)) * 10.0f;
	}
}

void PBRInteriorDemo::IBL()
{
	IBLs.assign(5, IBL_Texture());

	IBLs[0].EnvironmentMap = new Texture(L"Environment/Arches_E_PineTree/Arches_E_PineTree_8k_Environment.dds");
	IBLs[0].IrradianceMap = new Texture(L"Environment/Arches_E_PineTree/Arches_E_PineTree_Env_irrcube.dds");
	IBLs[0].ReflectionMap = new Texture(L"Environment/Arches_E_PineTree/Arches_E_PineTree_3k_Reflection.dds");
	// slender_man
	IBLs[1].EnvironmentMap = new Texture(L"Environment/Circus_Backstage/Circus_Backstage_8k_Environment.dds");
	IBLs[1].IrradianceMap = new Texture(L"Environment/Circus_Backstage/Circus_Backstage_Env_irrcube.dds");
	IBLs[1].ReflectionMap = new Texture(L"Environment/Circus_Backstage/Circus_Backstage_3k_Reflection.dds");
	// space_suit
	IBLs[2].EnvironmentMap = new Texture(L"Environment/Milkyway/Milkyway_BG_Environment.dds");
	IBLs[2].IrradianceMap = new Texture(L"Environment/Milkyway/Milkyway_Light_irrcube.dds");
	IBLs[2].ReflectionMap = new Texture(L"Environment/Milkyway/Milkyway_small_Reflection.dds");
	// winter_girl
	IBLs[3].EnvironmentMap = new Texture(L"Environment/Ice_Lake_HiRes_TMap_cube.dds");
	IBLs[3].IrradianceMap = new Texture(L"Environment/Ice_Lake_Env_cube.dds");
	IBLs[3].ReflectionMap = new Texture(L"Environment/Ice_Lake_HiRes_TMap_Reflection.dds");
	// retro_TV
	IBLs[4].EnvironmentMap = new Texture(L"Environment/hdrvfx_0011_zanla/hdrvfx_zanla_1_n1_v01_Bg_Environment.dds");
	IBLs[4].IrradianceMap = new Texture(L"Environment/hdrvfx_0011_zanla/hdrvfx_zanla_1_n1_v01_Env_irrcube.dds");
	IBLs[4].ReflectionMap = new Texture(L"Environment/hdrvfx_0011_zanla/hdrvfx_zanla_1_n1_v01_Ref_Reflection.dds");



}

void PBRInteriorDemo::ChangeIBL()
{
	sky->SetSRV(IBLs[IBL_index].EnvironmentMap->SRV());
	MaterialPBR::IrradianceCube(IBLs[IBL_index].IrradianceMap);
	MaterialPBR::ReflectionCube(IBLs[IBL_index].ReflectionMap);
}


void PBRInteriorDemo::Material()
{
	MaterialPBR* material = new MaterialPBR(shader);
	
}

