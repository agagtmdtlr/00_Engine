#pragma once

class TrailEffect : Renderer
{
public:
	// _Textures/Particles/ .xml file (�ش� ���Ϸ� ����Ѵ�.)
	TrailEffect();
	~TrailEffect();

	void Reset(); // ������ ������ �ٲ�� �� ó���ϱ� ����
	void Add(Vector3& position, Vector3 & endPosition); // �ܺο��� ��ƼŬ �߰�


public:
	void Update();
	void Render();
private:
	// leadCount�� gpuCount�� ���󰡸� �ΰ��� ���̸�ŭ Map�� �����Ѵ�.
	void MapVertices(); // �߰��� ������ gpu�� �Ѱ��ִ� �Լ�
	void Activate();
	void Deactivate();

private:
	void ReadFile(wstring file);

private:
	struct VertexParticle
	{
		Vector3 Position;
		Vector3 EndPosition;
		Vector3 PrevPosition;
		Vector3 PrevEndPosition;
		Vector2 Uv;
		float Time; 
	};
private:

	Texture* map = NULL;
	ID3DX11EffectShaderResourceVariable* sMap;

	VertexParticle* vertices = NULL;
	UINT* indices = NULL;

	float currentTime = 0.0f;
	float lastAddTime = 0.0f;

	// ��� ��ȯ ťó�� ��ȯ�ϴ� ������
	UINT leadCount = 0; // ���� �ռ��� ����
	UINT gpuCount = 0; // leadCount�� ���󰡸鼭 GPU�� ������ ����
	UINT activeCount = 0; // activeCount�� ���󰡸� Ȱ��ȭ�� ����
	UINT deactiveCount = 0; // activeCount�� ���󰡸�, ��Ȱ��ȭ ��ų ����
	UINT MaxParticles = 10000; // �ִ밳��
	float ReadyTime = 1.0f;

	Vector3 prevStart{ 0,0,0 };
	Vector3 prevEnd{ 0,0,0 };

	float prevU = 0;
	float prevV = 0;

private:
	Performance * perfo;
};