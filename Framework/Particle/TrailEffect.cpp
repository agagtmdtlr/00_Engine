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

	// ���� �����
	vertices = new VertexParticle[MaxParticles];
	vertexBuffer = new VertexBuffer(vertices, MaxParticles, sizeof(VertexParticle), 0, true);
}

void TrailEffect::Add(Vector3 & position, Vector3 & endPosition)
{
	//if (Time::Get()->Running() - lastAddTime < 60.0f / 1000.0f)
	//	return;

	lastAddTime = Time::Get()->Running();


	UINT count = leadCount + 1;

	// ��ȯť
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
	vertices[leadCount].Time = currentTime; // �ڱⰡ �߰��� �ð� ( readyTime ���� active()���� Ȱ��ȭ �ð����� ������


	prevStart = position;
	prevEnd = endPosition;
	// ī��Ʈ ����
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
	// �߰��Ȱ� �������� Map�� �����ϴ� ���귮�� ���� ���.
	if (gpuCount == leadCount) return;

	D3D11_MAPPED_SUBRESOURCE subResource;

	// ��ȯ ť�̱� ������ 2���� ��찡 �߻��Ѵ�.
	if (leadCount > gpuCount)
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_map
		// Write : Cpu_Access_Write �϶��� ����, Discard�� �����ϰ� �����
		// Write_Discard : ������ ���� �ִ� �����Ϳ� ���� ����
		// Write_No_Overwrite : ������ ���� �ִ� ���¿��� ������ �ʰ� ������ ��ġ���� ����
		// This flag is only valid on vertex and index buffers.
		D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subResource);
		{
			UINT start = gpuCount;
			UINT size = (leadCount - gpuCount) * sizeof(VertexParticle);
			UINT offset = gpuCount * sizeof(VertexParticle);// �̾ �� ��ġ ���ϱ�

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
			UINT offset = gpuCount * sizeof(VertexParticle);// �̾ �� ��ġ ���ϱ�

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

	gpuCount = leadCount; // ������ �Ϸ��
}

void TrailEffect::Activate()
{
	while (activeCount != gpuCount)
	{
		// �߰��� ���� ����� �ð�
		float age = currentTime - vertices[activeCount].Time;

		// �߰��� ���� ����� �ð� Ȱ��ȭ �ð� ��������� return
		if (age < ReadyTime)
			return;
		// Ȱ��ȭ
		vertices[activeCount].Time = currentTime; // Ȱ��ȭ�� �ð�
		activeCount++;

		if (activeCount >= MaxParticles)
			activeCount = 0;
	}
}

void TrailEffect::Deactivate()
{

	while (deactiveCount != activeCount)
	{
		// Ȱ��ȭ ���� ����� �ð�.
		float age = currentTime - vertices[deactiveCount].Time;

		// Ȱ��ȭ ���� ����� �ð���
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
