#pragma once

class TrailEffect : Renderer
{
public:
	// _Textures/Particles/ .xml file (해당 파일로 운용한다.)
	TrailEffect();
	~TrailEffect();

	void Reset(); // 정점의 개수가 바뀌는 걸 처리하기 위해
	void Add(Vector3& position, Vector3 & endPosition); // 외부에서 파티클 추가


public:
	void Update();
	void Render();
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

	// 모두 순환 큐처럼 순환하는 변수들
	UINT leadCount = 0; // 가장 앞서는 개수
	UINT gpuCount = 0; // leadCount를 따라가면서 GPU에 복사할 개수
	UINT activeCount = 0; // activeCount를 따라가면 활성화할 개수
	UINT deactiveCount = 0; // activeCount를 따라가면, 비활성화 시킬 개수
	UINT MaxParticles = 10000; // 최대개수
	float ReadyTime = 1.0f;

	Vector3 prevStart{ 0,0,0 };
	Vector3 prevEnd{ 0,0,0 };

	float prevU = 0;
	float prevV = 0;

private:
	Performance * perfo;
};