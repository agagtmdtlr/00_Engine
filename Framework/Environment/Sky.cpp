#include "Framework.h"
#include "Sky.h"
#include "Scattering.h"
#include "Dome.h"
#include "Moon.h"
#include "Cloud.h"

Sky::Sky(Shader * shader)
	: shader(shader)
{
	scatterDesc.InvWaveLength.x = 1.0f / powf(scatterDesc.WaveLength.x, 4.0f);
	scatterDesc.InvWaveLength.y = 1.0f / powf(scatterDesc.WaveLength.y, 4.0f);
	scatterDesc.InvWaveLength.z = 1.0f / powf(scatterDesc.WaveLength.z, 4.0f);

	scatterDesc.WaveLengthMie.x = powf(scatterDesc.WaveLength.x, -0.84f);
	scatterDesc.WaveLengthMie.y = powf(scatterDesc.WaveLength.y, -0.84f);
	scatterDesc.WaveLengthMie.z = powf(scatterDesc.WaveLength.z, -0.84f);

	scattering = new Scattering(shader);
	scatterBuffer = new ConstantBuffer(&scatterDesc, sizeof(ScatterDesc));
	sScatterBuffer = shader->AsConstantBuffer("CB_Scattering");


	sRayleighMap = shader->AsSRV("RayleighMap");
	sMieMap = shader->AsSRV("MieMap");

	dome = new Dome(shader, Vector3(0, 50, 0), Vector3(400, 400, 400));
	moon = new Moon(shader);

	cloud = new Cloud(shader);
	cloudBuffer = new ConstantBuffer(&cloudDesc, sizeof(CloudDesc));
	sCloudBuffer = shader->AsConstantBuffer("CB_Cloud");
}

Sky::~Sky()
{
	SafeDelete(scattering);
	SafeDelete(scatterBuffer);

	SafeDelete(dome);
	SafeDelete(moon);

	SafeDelete(cloud);
	SafeDelete(cloudBuffer);
}

void Sky::ScatteringPass(UINT val)
{
	scattering->Pass(val);
}

void Sky::Pass(UINT domePass, UINT moonPass, UINT cloudPass)
{
	dome->Pass(domePass);
	moon->Pass(moonPass);
	cloud->Pass(cloudPass);
}

void Sky::Update()
{
	//ImGui::SliderFloat("Tile", &cloudDesc.Tiles, 0.0f, 10.0f);
	//ImGui::SliderFloat("Cover", &cloudDesc.Cover, -1.0f, 1.0f);
	//ImGui::SliderFloat("Sharpness", &cloudDesc.Sharpness, -1.0f, 1.0f);
	//ImGui::SliderFloat("Speed", &cloudDesc.Speed, 0.0f, 1.0f);

	if (bRealTime == true)
	{
		theta += Time::Delta() * timeFactor;

		if (theta > Math::PI)
			theta -= Math::PI * 2.0f;


		float x = sinf(theta);
		float y = cosf(theta);

		Context::Get()->Direction() = Vector3(x, y, 0.0f);
	}
	else // global light Direction을 바꾸는 방법 sky와 연동하기 위함
	{
		ImGui::SliderFloat("Theta", &theta, -Math::PI, Math::PI);

		float x = sinf(theta);
		float y = cosf(theta);
		// 전체 조명과 하늘을 연동하는 방법 [ 원래는 라이트의 포지션을 넘겨주는거도 있다.
		Context::Get()->Direction() = Vector3(x, y, 0.0f);
	}

	scattering->Update();
	dome->Update();
	moon->Update();
	cloud->Update();
}

void Sky::PreRender()
{
	scatterBuffer->Render();
	sScatterBuffer->SetConstantBuffer(scatterBuffer->Buffer());
	// MRT setting
	scattering->PreRender();
}

void Sky::Render()
{
	//Dome
	{
		sRayleighMap->SetResource(scattering->RayleighRTV()->SRV());
		sMieMap->SetResource(scattering->MieRTV()->SRV());

		dome->Render();
	}

	//Moon
	{
		Vector3 p;
		
		dome->GetTransform()->Position(&p);
		moon->Render(theta,p, 400);
	}

	//Cloud
	{
		cloudBuffer->Render();
		sCloudBuffer->SetConstantBuffer(cloudBuffer->Buffer());

		//cloud->Render();
	}
}

void Sky::PostRender()
{
	scattering->PostRender();
	cloud->PostRender();
}

void Sky::RealTime(bool val, float theta, float timeFactor)
{
	bRealTime = val;

	this->theta = theta;
	this->timeFactor = timeFactor;
}
