#pragma once

class Water : public Renderer
{
public:
	Water(Shader* shader, float radius, UINT width = 0, UINT height = 0);
	~Water();

	void Update();

	void PreRender_Reflection();
	void PreRender_Refraction();
	void Render();

private:
	struct Desc
	{
		Color RefractionColor = Color(0.2f, 0.3f, 1.0f, 1.0f);

		Vector2 NormalMapTile = Vector2(0.1f, 0.2f);
		float WaveTranslation = 0.0f; // 물의 속도
		float WaveScale = 0.05f; // 물결의 크기 normal * wavesacle

		float Shininess = 30.0f; // 물에 빛 비침!! specular
		float Alpha = 0.5f; // 투명도
		float WaveSpeed = 0.5f;

		float Padding;
	} desc;

private:
	float radius; // 정사각형의 판
	UINT width, height; // 해상도

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;
	// 물의 표면을 표현할 텍스처 맵
	Reflection* reflection;
	Refraction* refraction;
};