#include "Framework.h"
#include "TrailEffect.h"

TrailEffect::TrailEffect()
	:Renderer(L"119_TrailEffect.fx")
{
	Topology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	Reset();
}

TrailEffect::~TrailEffect()
{
	
}

void TrailEffect::Reset()
{
	currentTime = 0.0f;
	lastAddTime = Time::Get()->Running();
	gpuCount = leadCount = activeCount = deactiveCount = 0;

	SafeDeleteArray(vertices);
	SafeDelete(vertexBuffer);

	// 정점 만들기
	vertices = new VertexParticle[MaxParticles];
	vertexBuffer = new VertexBuffer(vertices, MaxParticles, sizeof(VertexParticle), 0, true);
}

void TrailEffect::Add(Vector3 & position, Vector3 & endPosition)
{
	//if (Time::Get()->Running() - lastAddTime < 60.0f / 1000.0f)
	//	return;

	lastAddTime = Time::Get()->Running();


	UINT count = leadCount + 1;

	// 순환큐
	if (count >= MaxParticles)
	{
		count = 0;
	}

	if (count == deactiveCount)
		return;

	vertices[leadCount].Position = position;
	vertices[leadCount].EndPosition = endPosition;
	vertices[leadCount].PrevPosition = prevStart;
	vertices[leadCount].PrevEndPosition = prevEnd;
	vertices[leadCount].Time = currentTime; // 자기가 추가된 시간 ( readyTime 이후 active()에서 활성화 시간으로 변경함


	prevStart = position;
	prevEnd = endPosition;
	// 카운트 증가
	leadCount = count;
}

void TrailEffect::Update()
{
	Super::Update();
	currentTime += Time::Delta();

	MapVertices();
	Activate();
	Deactivate();

	if (activeCount == leadCount)
		currentTime = 0.0f;
}

void TrailEffect::Render()
{
	Super::Render();
	if (leadCount == activeCount)
		return;

	//shader->Draw(0, Pass(), MaxParticles);
	//return;

	if (leadCount > activeCount)
	{
		// tech , pass , draw count , draw start index pos
		shader->Draw(0, Pass(), (leadCount - activeCount), activeCount);
	}
	else
	{
		shader->Draw(0, Pass(), (MaxParticles - activeCount), activeCount);

		if (leadCount > 0)
			shader->Draw(0, Pass(), leadCount);
	}
}



void TrailEffect::MapVertices()
{
	// 추가된게 있을때만 Map을 수행하다 연산량이 많이 든다.
	if (gpuCount == leadCount) return;

	D3D11_MAPPED_SUBRESOURCE subResource;

	// 순환 큐이기 때문에 2가지 경우가 발생한다.
	if (leadCount > gpuCount)
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_map
		// Write : Cpu_Access_Write 일때만 접근, Discard랑 유사하게 덮어씌움
		// Write_Discard : 이전에 쓰여 있는 데이터에 덮어 씌움
		// Write_No_Overwrite : 이전에 씌어 있는 상태에서 지우지 않고 지정한 위치부터 써줌
		// This flag is only valid on vertex and index buffers.
		D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subResource);
		{
			UINT start = gpuCount;
			UINT size = (leadCount - gpuCount) * sizeof(VertexParticle);
			UINT offset = gpuCount * sizeof(VertexParticle);// 이어서 쓸 위치 구하기

			BYTE* p = (BYTE *)subResource.pData + offset;
			memcpy(p, vertices + start, size);
		}
		D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
	}
	else
	{
		D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subResource);
		{
			UINT start = gpuCount;
			UINT size = (MaxParticles - gpuCount) * sizeof(VertexParticle);
			UINT offset = gpuCount * sizeof(VertexParticle);// 이어서 쓸 위치 구하기

			BYTE* p = (BYTE *)subResource.pData + offset;
			memcpy(p, vertices + start, size);
		}
		if (leadCount > 0)
		{
			UINT size = leadCount * sizeof(VertexParticle);

			memcpy(subResource.pData, vertices, size);
		}
		D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
	}

	gpuCount = leadCount; // 갱신이 완료됨
}

void TrailEffect::Activate()
{
	while (activeCount != gpuCount)
	{
		// 추간된 이후 경과한 시간
		float age = currentTime - vertices[activeCount].Time;

		// 추가된 이후 경과한 시간 활성화 시간 전까지라면 return
		if (age < ReadyTime)
			return;
		// 활성화
		vertices[activeCount].Time = currentTime; // 활성화된 시간
		activeCount++;

		if (activeCount >= MaxParticles)
			activeCount = 0;
	}
}

void TrailEffect::Deactivate()
{

	while (deactiveCount != activeCount)
	{
		// 활성화 이후 경과하 시간.
		float age = currentTime - vertices[deactiveCount].Time;

		// 활성화 이후 경과한 시간이
		if (age > ReadyTime)
			return;

		deactiveCount++;

		if (deactiveCount >= MaxParticles)
		{
			deactiveCount = 0;
		}
	}
}

void TrailEffect::ReadFile(wstring file)
{
}
