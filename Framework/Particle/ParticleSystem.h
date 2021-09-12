#pragma once


class ParticleSystem : public Renderer
{
public:
	// _Textures/Particles/ .xml file (�ش� ���Ϸ� ����Ѵ�.)
	ParticleSystem(wstring file);
	~ParticleSystem();

	void Reset(); // ������ ������ �ٲ�� �� ó���ϱ� ����
	void Add(Vector3& position); // �ܺο��� ��ƼŬ �߰�


public:
	void Update();
	void Render();

	ParticleData& GetData() { return data; }
	void SetTexture(wstring file);

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
		Vector3 Velocity;
		Vector4 Random; //x:�ֱ�, y:ũ��, z:ȸ��, w:���� // ������ ������ ���
		float Time; // �÷��̵Ǵ� �ð�
	};

private:
	struct Desc
	{
		Color MinColor;
		Color MaxColor;

		Vector3 Gravity;
		float EndVelocity;

		Vector2 StartSize;
		Vector2 EndSize;

		Vector2 RotateSpeed;
		float ReadyTime; // ��ƼŬ�� ����Ǵ� �ð�
		float ReadyRandomTime; // ����Ǵ� �ð��� �������ش�. ���� Ŀ���� Ŀ������ ����ð��� �þ��.

		float ColorAmount;
		float CurrentTime;
		float Padding[2];
	} desc;

private:

	ParticleData data;

	Texture* map = NULL;
	ID3DX11EffectShaderResourceVariable* sMap;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	VertexParticle* vertices = NULL;
	UINT* indices = NULL;
	
	float currentTime = 0.0f;
	float lastAddTime = 0.0f;

	// ��� ��ȯ ťó�� ��ȯ�ϴ� ������
	UINT leadCount = 0; // ���� �ռ��� ����
	UINT gpuCount = 0; // leadCount�� ���󰡸鼭 GPU�� ������ ����
	UINT activeCount = 0; // activeCount�� ���󰡸� Ȱ��ȭ�� ����
	UINT deactiveCount = 0; // activeCount�� ���󰡸�, ��Ȱ��ȭ ��ų ����

private:
	Performance * perfo;
};