#include "Framework.h"
#include "ParticleSystem.h"
#include "Utilities/Xml.h"

ParticleSystem::ParticleSystem(wstring file)
	: Renderer(L"96_Particle.fx")
{
	Topology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);


	ReadFile(L"../../_Textures/Particles/" + file + L".xml");

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Particle");

	sMap = shader->AsSRV("ParticleMap");

	Reset();

}

ParticleSystem::~ParticleSystem()
{
	SafeDelete(map);
	SafeDelete(buffer);

	SafeDeleteArray(vertices);
}

void ParticleSystem::Reset()
{
	currentTime = 0.0f;
	lastAddTime = Time::Get()->Running();
	gpuCount = leadCount = activeCount = deactiveCount = 0;


	SafeDeleteArray(vertices);
	SafeDelete(vertexBuffer);

	// ���� �����
	vertices = new VertexParticle[data.MaxParticles];	
	vertexBuffer = new VertexBuffer(vertices, data.MaxParticles, sizeof(VertexParticle), 0, true);
}



void ParticleSystem::Update()
{
	Super::Update();

	currentTime += Time::Delta();

	MapVertices();
	Activate();
	Deactivate();

	if (activeCount == leadCount)
		currentTime = 0.0f;


	desc.MinColor = data.MinColor;
	desc.MaxColor = data.MaxColor;
	
	desc.ColorAmount = data.ColorAmount;

	desc.Gravity = data.Gravity;
	desc.EndVelocity = data.EndVelocity;

	desc.RotateSpeed = Vector2(data.MinRotateSpeed, data.MaxRotateSpeed);
	desc.StartSize = Vector2(data.MinStartSize, data.MaxStartSize);
	desc.EndSize = Vector2(data.MinEndSize, data.MaxEndSize);

	desc.ReadyTime = data.ReadyTime;
	desc.ReadyRandomTime = data.ReadyRandomTime;
}

void ParticleSystem::Render()
{
	Super::Render();

	desc.CurrentTime = currentTime;

	buffer->Render();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	sMap->SetResource(map->SRV());

	if (leadCount == activeCount)
		return;


	UINT pass = (UINT)data.Type; // blendType : pass number
	if (leadCount > activeCount)
	{
		// tech , pass , draw count , draw start index pos
		shader->Draw(0, pass, (leadCount - activeCount), activeCount);
	}
	else
	{
		shader->Draw(0, pass, (data.MaxParticles - activeCount), activeCount);

		if (leadCount > 0)
			shader->Draw(0, pass, leadCount);
	}
}

void ParticleSystem::SetTexture(wstring file)
{
	SafeDelete(map);

	map = new Texture(file);
}

void ParticleSystem::MapVertices()
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
			UINT size = (data.MaxParticles - gpuCount) * sizeof(VertexParticle);
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

// 60�����Ӹ��� �߰��� �� �ֵ��� ������ ����
void ParticleSystem::Add(Vector3 & position)
{
	if (Time::Get()->Running() - lastAddTime < 60.0f / 1000.0f)
		return;

	lastAddTime = Time::Get()->Running();

	
	UINT count = leadCount + 1;

	// ��ȯť
	if (count >= data.MaxParticles)
	{

		if (data.bLoop == true)
		{
			count = 0;
		}
		else
		{
			count = data.MaxParticles;
			return;
		}
	}

	if (count == deactiveCount)
		return;

	Vector3 velocity = Vector3(1, 1, 1); // �⺻ �ӵ�
	velocity *= data.StartVelocity; // ���ۼӵ��� ����

	float horizontalVelocity = Math::Lerp(data.MinHorizontalVelocity, data.MaxHorizontalVelocity, Math::Random(0.0f, 1.0f));
	float horizontalAngle = Math::PI * 2.0f * Math::Random(0.0f, 1.0f); // 0 ~ 360����

	// �ӵ� : �ð���ŭ �̵��������� �̵���ġ sin/cos�� �̿��Ͽ� ȸ�� �̵���Ų��.
	velocity.x += horizontalVelocity * cosf(horizontalAngle);
	velocity.y += horizontalVelocity * sinf(horizontalAngle);
	velocity.z += Math::Lerp(data.MinVerticalVelocity, data.MaxVerticalVelocity, Math::Random(0.0f, 1.0f));

	Vector4 random = Math::RandomVec4(0.0f, 1.0f);

	vertices[leadCount].Position = position;
	vertices[leadCount].Velocity = velocity;
	vertices[leadCount].Random = random;
	vertices[leadCount].Time = currentTime; // �ڱⰡ �߰��� �ð� ( readyTime ���� active()���� Ȱ��ȭ �ð����� ������

	// ī��Ʈ ����
	leadCount = count;
}

void ParticleSystem::Activate()
{
	while (activeCount != gpuCount)
	{
		// �߰��� ���� ����� �ð�
		float age = currentTime - vertices[activeCount].Time;

		// �߰��� ���� ����� �ð� Ȱ��ȭ �ð� ��������� return
		if (age < data.ReadyTime)
			return;
		// Ȱ��ȭ
		vertices[activeCount].Time = currentTime; // Ȱ��ȭ�� �ð�
		activeCount++;

		if (activeCount >= data.MaxParticles)
			activeCount = (data.bLoop == true) ? 0 : data.MaxParticles;
	}
}

void ParticleSystem::Deactivate()
{
	while (deactiveCount != activeCount )
	{
		// Ȱ��ȭ ���� ����� �ð�.
		float age = currentTime - vertices[deactiveCount].Time;

		// Ȱ��ȭ ���� ����� �ð���
		if (age > data.ReadyTime) 
			return;

		deactiveCount++;

		if (deactiveCount >= data.MaxParticles)
		{
			deactiveCount = (data.bLoop == true) ? 0 : data.MaxParticles;			
		}
	}
}

// read xml particle data
void ParticleSystem::ReadFile(wstring file)
{

	Xml::XMLDocument* document = new Xml::XMLDocument();
	Xml::XMLError error = document->LoadFile(String::ToString(file).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement* root = document->FirstChildElement();

	// BlendState
	Xml::XMLElement* node = root->FirstChildElement(); 
	data.Type = (ParticleData::BlendType)node->IntText();

	node = node->NextSiblingElement();
	data.bLoop = node->BoolText();
	//data.bLoop = true;

	// TextureFile
	node = node->NextSiblingElement();
	wstring textureFile = String::ToWString(node->GetText());
	data.TextureFile = L"Particles/" + textureFile;
	map = new Texture(data.TextureFile);

	node = node->NextSiblingElement();
	data.MaxParticles = node->IntText();

	node = node->NextSiblingElement();
	data.ReadyTime = node->FloatText();

	node = node->NextSiblingElement();
	data.ReadyRandomTime = node->FloatText();

	node = node->NextSiblingElement();
	data.StartVelocity = node->FloatText();

	node = node->NextSiblingElement();
	data.EndVelocity = node->FloatText();

	node = node->NextSiblingElement();
	data.MinHorizontalVelocity = node->FloatText();

	node = node->NextSiblingElement();
	data.MaxHorizontalVelocity = node->FloatText();

	node = node->NextSiblingElement();
	data.MinVerticalVelocity = node->FloatText();

	node = node->NextSiblingElement();
	data.MaxVerticalVelocity = node->FloatText();

	node = node->NextSiblingElement();
	data.Gravity.x = node->FloatAttribute("X");
	data.Gravity.y = node->FloatAttribute("Y");
	data.Gravity.z = node->FloatAttribute("Z");

	node = node->NextSiblingElement();
	data.ColorAmount = node->FloatText();

	node = node->NextSiblingElement();
	data.MinColor.r = node->FloatAttribute("R");
	data.MinColor.g = node->FloatAttribute("G");
	data.MinColor.b = node->FloatAttribute("B");
	data.MinColor.a = node->FloatAttribute("A");

	node = node->NextSiblingElement();
	data.MaxColor.r = node->FloatAttribute("R");
	data.MaxColor.g = node->FloatAttribute("G");
	data.MaxColor.b = node->FloatAttribute("B");
	data.MaxColor.a = node->FloatAttribute("A");

	node = node->NextSiblingElement();
	data.MinRotateSpeed = node->FloatText();

	node = node->NextSiblingElement();
	data.MaxRotateSpeed = node->FloatText();

	node = node->NextSiblingElement();
	data.MinStartSize = node->FloatText();

	node = node->NextSiblingElement();
	data.MaxStartSize = node->FloatText();

	node = node->NextSiblingElement();
	data.MinEndSize = node->FloatText();

	node = node->NextSiblingElement();
	data.MaxEndSize = node->FloatText();

	SafeDelete(document);
}
