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
		float WaveTranslation = 0.0f; // ���� �ӵ�
		float WaveScale = 0.05f; // ������ ũ�� normal * wavesacle

		float Shininess = 30.0f; // ���� �� ��ħ!! specular
		float Alpha = 0.5f; // ������
		float WaveSpeed = 0.5f;

		float Padding;
	} desc;

private:
	float radius; // ���簢���� ��
	UINT width, height; // �ػ�

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;
	// ���� ǥ���� ǥ���� �ؽ�ó ��
	Reflection* reflection;
	Refraction* refraction;
};