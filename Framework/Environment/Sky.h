#pragma once

class Sky
{
public:
	Sky(Shader* shader);
	~Sky();

	void ScatteringPass(UINT val);
	void Pass(UINT domePass, UINT moonPass, UINT cloudPass);

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	
	void RealTime(bool val, float theta, float timeFactor = 1.0f);

private:
	struct ScatterDesc
	{
		// 짥은 파장이 더 산란이 많이된다.
		Vector3 WaveLength = Vector3(0.65f, 0.57f, 0.475f);
		float Padding;

		Vector3 InvWaveLength; // 1 / pow(wavelength , 4)
		int SampleCount = 8;

		Vector3 WaveLengthMie; // pow(wavelength , -0.84)
		float Padding2;
	} scatterDesc;

	struct CloudDesc
	{
		float Tiles = 4.5f;
		float Cover = -0.035f;
		//float Sharpness = 0.26f;
		float Sharpness = 0.02f;
		float Speed = 0.13f;
	} cloudDesc;

private:
	Shader* shader;

	bool bRealTime = false;

	float timeFactor = 1.0f;
	float theta = 2.682f;

	class Scattering* scattering;
	ConstantBuffer* scatterBuffer;
	ID3DX11EffectConstantBuffer* sScatterBuffer;

	ID3DX11EffectShaderResourceVariable* sRayleighMap;
	ID3DX11EffectShaderResourceVariable* sMieMap;

	class Dome* dome;
	class Moon* moon;

	class Cloud* cloud;
	ConstantBuffer* cloudBuffer;
	ID3DX11EffectConstantBuffer* sCloudBuffer;
};