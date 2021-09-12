#include "stdafx.h"
#include "ConvertMap.h"
#include "Converter/EnvironmentMap.h"
#include "Converter/BRDFMap.h"

void ConvertMap::Initialize()
{
	//Context::Get()->GetCamera()->Position(0, 0, -2);
	//Context::Get()->GetCamera()->RotationDegree(0, 0, 0);
	//((Freedom *)Context::Get()->GetCamera())->Speed(5, 1);


	//Context::Get()->GetCamera()->Update();

	bakingMap = new EnvironmentMap();
	envf = bind(&ConvertMap::EnvPath, this, placeholders::_1);
	irrf = bind(&ConvertMap::IrrPath, this, placeholders::_1);
	reff = bind(&ConvertMap::RefPath, this, placeholders::_1);

	//brdfMap = new BRDFMap();
	//brdfMap->BakingBRDF();

	//BakeMaps(
	//	L"Environment/SkyOnly/SkyOnlyHDRI001_8K-HDR.dds",
	//	L"Environment/SkyOnly/SkyOnlyHDRI001_8K-HDR.dds",
	//	L"Environment/SkyOnly/SkyOnlyHDRI001_8K-HDR.dds"
	//);

	//BakeMaps(
	//	L"Environment/Arches_E_PineTree/Arches_E_PineTree_8k.jpg",
	//	L"Environment/Arches_E_PineTree/Arches_E_PineTree_Env.hdr",
	//	//L"Environment/Arches_E_PineTree/Arches_E_PineTree_3k.hdr"
	//	L"Environment/Arches_E_PineTree/Arches_E_PineTree_8k.jpg"
	//);
	//BakeMaps(
	//	L"Environment/Ice_Lake/Ice_Lake_HiRes_TMap.jpg",
	//	L"Environment/Ice_Lake/Ice_Lake_Env.hdr",
	//	L"Environment/Ice_Lake/Ice_Lake_Ref.hdr"
	//);

	//sky = new CubeSky(bakingMap->GetSRVReflection());
	//static Texture* texture = new Texture(L"Environment/Arches_E_PineTree/Arches_E_PineTree_3k_Reflection.dds");
	
	//sky = new CubeSky(texture->SRV());
	//sky->Pass(2);

	//Texture* texture = new Texture(L"Environment/Ice_Lake/Ice_Lake_Env.hdr");
	//render2D = new Render2D();
	//render2D->SRV(texture->SRV());
	//render2D->GetTransform()->Position(D3D::Width() / 2, D3D::Height() / 2, 0);
	//render2D->GetTransform()->Scale(D3D::Width(), D3D::Width(), 0);
	
}

void ConvertMap::Destroy()
{
}

void ConvertMap::Update()
{
	//sky->Update();
	//render2D->Update();
	ImGuiID id;
	ImGui::GetID(&id);
	ImVec2 v;
	v.x = 100;
	v.y = 100;
	bool b = true;
	ImGui::Begin("ConvertMap",&b);
	//ImGui::BeginChildFrame(id, v);
	{
		if (ImGui::Button("Load Env File"))
			Path::OpenFileDialog(L"", Path::ImageFilter, L"../../_Textures/Environment", envf, D3D::GetHandle());
		if (ImGui::Button("Load Irr File"))
			Path::OpenFileDialog(L"", Path::ImageFilter, L"../../_Textures/Environment", irrf, D3D::GetHandle());
		if (ImGui::Button("Load Ref File"))
			Path::OpenFileDialog(L"", Path::ImageFilter, L"../../_Textures/Environment", reff, D3D::GetHandle());
		if (ImGui::Button("Bake Map"))
			ExcuteBake();
	}
	ImGui::End();
	//ImGui::EndChildFrame();

	//render2D->Update();
}

void ConvertMap::PreRender()
{
}

void ConvertMap::Render()
{
	//sky->Render();
	
}

void ConvertMap::PostRender()
{
	//render2D->Render();
	
}

void ConvertMap::BakeMaps(wstring env, wstring irr, wstring ref)
{
	bakingMap->LoadEquirectangularMap(env);
	bakingMap->BakingEnvironeMentMap();
	bakingMap->ConvertEquirectangularMapToCubeMap(irr,512,true,L"irr");
	bakingMap->LoadEquirectangularMap(ref);
	bakingMap->BakingReflectionMap();
}

void ConvertMap::ExcuteBake()
{
	BakeMaps(envPath, irrPath, refPath);
}

void ConvertMap::EnvPath(wstring path)
{
	envPath = path;
}

void ConvertMap::IrrPath(wstring path)
{
	irrPath = path;
}

void ConvertMap::RefPath(wstring path)
{
	refPath = path;
}

void ConvertMap::Bake1()
{
	BakeMaps(
		L"Environment/HDR_040_Field/HDR_040_Field_Bg.jpg",
		L"Environment/HDR_040_Field/HDR_040_Field_Env.hdr",
		L"Environment/HDR_040_Field/HDR_040_Field_Ref.hdr"
	);

	BakeMaps(
		L"Environment/Arches_E_PineTree/Arches_E_PineTree_8k.jpg",
		L"Environment/Arches_E_PineTree/Arches_E_PineTree_Env.hdr",
		L"Environment/Arches_E_PineTree/Arches_E_PineTree_3k.hdr"
	);

	BakeMaps(
		L"Environment/Circus_Backstage/Circus_Backstage_8k.jpg",
		L"Environment/Circus_Backstage/Circus_Backstage_Env.hdr",
		L"Environment/Circus_Backstage/Circus_Backstage_3k.hdr"
	);

	BakeMaps(
		L"Environment/Milkyway/Milkyway_BG.jpg",
		L"Environment/Milkyway/Milkyway_Light.hdr",
		L"Environment/Milkyway/Milkyway_small.hdr"
	);

	BakeMaps(
		L"Environment/Winter_Forest/WinterForest_8k.jpg",
		L"Environment/Winter_Forest/WinterForest_Env.hdr",
		L"Environment/Winter_Forest/WinterForest_Ref.hdr"
	);

	BakeMaps(
		L"Environment/Factory_Catwalk/Factory_Catwalk_Bg.jpg",
		L"Environment/Factory_Catwalk/Factory_Catwalk_Env.hdr",
		L"Environment/Factory_Catwalk/Factory_Catwalk_2k.hdr"
	);

	BakeMaps(
		L"Environment/GrandCanyon_C_YumaPoint/GCanyon_C_YumaPoint_8k.jpg",
		L"Environment/GrandCanyon_C_YumaPoint/GCanyon_C_YumaPoint_Env.hdr",
		L"Environment/GrandCanyon_C_YumaPoint/GCanyon_C_YumaPoint_3k.hdr"
	);

	//bakingMap->LoadEquirectangularMap(L"Environment/Ice_Lake_HiRes_TMap.jpg");
	//bakingMap->BakingCubeMap();

	//brdfMap = new BRDFMap();
	//brdfMap->BakingBRDF();

	//sky = new CubeSky(bakingMap->GetSRVReflection());

	//bakingMap->ConvertEquirectangularMapToCubeMap(L"Environment/Ice_Lake_HiRes_TMap.jpg", 1024, false);
	//bakingMap->ConvertEquirectangularMapToCubeMap(L"Environment/Ice_Lake_Ref.hdr", 512, true);
	//bakingMap->ConvertEquirectangularMapToCubeMap(L"Environment/Ice_Lake_Env.hdr", 512, true);

	{
		//bakingMap->ConvertEquirectangularMapToCubeMap(L"Environment/HDR_040_Field/HDR_040_Field.hdr",1024,true);
		//bakingMap->ConvertEquirectangularMapToCubeMap(L"Environment/HDR_040_Field/HDR_040_Field_Env.hdr",512,true);
		//bakingMap->ConvertEquirectangularMapToCubeMap(L"Environment/HDR_040_Field/HDR_040_Field_Ref.hdr", 512, true);
	}

	//sky = new CubeSky(bakingMap->GetSRVReflection());
	//static Texture* texture = new Texture(L"Environment/Ice_Lake_HiRes_TMap_Reflection.dds");
	//
	//render2D = new Render2D();
	//render2D->SRV(brdfMap->GetSRVBRDFLUT());
	//render2D->GetTransform()->Position(D3D::Width() / 2, D3D::Height() / 2, 0);
	//render2D->GetTransform()->Scale(500, 500, 0);
	//
	//sky = new CubeSky(texture->SRV());
	//sky->Pass(2);
}
