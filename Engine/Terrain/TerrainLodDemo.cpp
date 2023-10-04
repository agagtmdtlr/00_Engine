#include "stdafx.h"
#include "TerrainLodDemo.h"
#include "Environment/Terrain.h"
#include "Environment/Ocean.h"

void TerrainLodDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(17, 100, 0);
	Context::Get()->GetCamera()->Position(-245, 140, -2);
	//Context::Get()->GetCamera()->RotationDegree(-1, 3, 0);
	//Context::Get()->GetCamera()->Position(-81, 27, -620);
	((Freedom *)Context::Get()->GetCamera())->Speed(100, 1);


	shader = new Shader(L"119_TerrainLod.fxo");

	//Terrain
	{
		TerrainLod::InitializeDesc desc =
		{
			shader,
			L"Terrain/Thumbnails/Terrain_Alpha (7).png",
			//L"Terrain/Ue4.png",
			// 한칸의 크기, patch정점, height 비율
		    1.0f, 16, 5
		};

		terrain = new TerrainLod(desc);
		terrain->BaseMap(L"Terrain/Snow.jpg");		
		//terrain->BaseMap(L"Terrain/snow01n.tga");
		//terrain->LayerMap(L"Terrain/Grass (Lawn).jpg", L"Terrain/Thumbnails/Terrain_Alpha (8).jpg");
		terrain->LayerMap(L"Terrain/rock01d.tga", L"Terrain/Thumbnails/Terrain_Alpha (7).png");
		terrain->NormalMap(L"Terrain/rock01n.tga");
		//terrain->Pass(1);
		terrain->Pass(4);
	}


	//Billboard
	{
		grassShader = new Shader(L"96_Billboard.fxo");
		grass = new Billboard(grassShader);
		grass->AddTexture(L"Terrain/grass_01.tga");
		grass->AddTexture(L"Terrain/grass_03.tga");
		grass->AddTexture(L"Terrain/grass_06.tga");

		for (int i = 0; i < 1000; i++)
		{
			Vector2 r = Math::RandomVec2(-50, 50);	

			Vector3 v(r.x, 5, r.y + 50);
			Vector2 rand2(Math::RandomVec2(3, 5));
			grass->Add(  v , rand2, 0);
			r = Math::RandomVec2(-50, 50);

			v = Vector3(r.x, 5, r.y + 50);
			rand2 = Math::RandomVec2(3, 5);
			grass->Add( v ,rand2 , 1);
			r = Math::RandomVec2(-50, 50);

			v = Vector3(r.x, 5, r.y + 50);
			rand2 = Math::RandomVec2(3, 5);
			grass->Add( v, rand2, 2);
		}

		grass->Pass(5);
	}

	sky = new Sky(shader);
	sky->ScatteringPass(5);
	sky->Pass(6, 9, 9);

	oceanShader = new Shader(L"123_Ocean.fx");
	Ocean::InitializeDesc oceanDesc =
	{
		oceanShader, 512, 512, 1
	};
	ocean = new Ocean(oceanDesc);
	ocean->GetTransform()->Position(0, 5, 0);

	Texture * texture = new Texture(L"Environment/Ice_Lake_Ref.hdr");
	D3D11_TEXTURE2D_DESC desc;
	texture->GetTexture()->GetDesc(&desc);

	int a = 0;
	
	
}

void TerrainLodDemo::Destroy()
{
	SafeDelete(terrain);
}

void TerrainLodDemo::Update()
{
	terrain->Update();
	grass->Update();
	sky->Update();
	ocean->Update();

	static float sp = 1.0f;
	ImGui::SliderFloat("sp", &sp,1.0,100);
	oceanShader->AsScalar("specp")->SetFloat(sp);
}

void TerrainLodDemo::PreRender()
{
	terrain->Pass(10);
	terrain->PreRender();
	sky->PreRender();
}

void TerrainLodDemo::Render()
{
	sky->Pass(6, 7, 8);
	sky->Render();
	terrain->Pass(4);
	terrain->Render();
	//grass->Render();

	//ImGui::ShowDemoWindow();
	ocean->Render();
}

void TerrainLodDemo::PostRender()
{
	//terrain->PostRender();
	sky->PostRender();
}
