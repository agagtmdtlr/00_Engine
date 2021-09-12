#pragma once


class ParticleSystem : public Renderer
{
public:
	// _Textures/Particles/ .xml file (해당 파일로 운용한다.)
	ParticleSystem(wstring file);
	~ParticleSystem();

	void Reset(); // 정점의 개수가 바뀌는 걸 처리하기 위해
	void Add(Vector3& position); // 외부에서 파티클 추가


public:
	void Update();
	void Render();

	ParticleData& GetData() { return data; }
	void SetTexture(wstring file);

private:
	// leadCount를 gpuCount가 따라가며 두개의 차이만큼 Map을 수행한다.
	void MapVertices(); // 추가된 정점을 gpu롷 넘겨주는 함수
	void Activate();
	void Deactivate();

private:
	void ReadFile(wstring file);

private:
	struct VertexParticle
	{
		Vector3 Position;
		Vector3 Velocity;
		Vector4 Random; //x:주기, y:크기, z:회전, w:색상 // 움직임 구현에 사용
		float Time; // 플레이되는 시간
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
		float ReadyTime; // 파티클이 실행되는 시간
		float ReadyRandomTime; // 실행되는 시간을 보정해준다. 값이 커지면 커질수록 실행시간이 늘어난다.

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

	// 모두 순환 큐처럼 순환하는 변수들
	UINT leadCount = 0; // 가장 앞서는 개수
	UINT gpuCount = 0; // leadCount를 따라가면서 GPU에 복사할 개수
	UINT activeCount = 0; // activeCount를 따라가면 활성화할 개수
	UINT deactiveCount = 0; // activeCount를 따라가면, 비활성화 시킬 개수

private:
	Performance * perfo;
};