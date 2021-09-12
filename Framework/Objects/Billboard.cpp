#include "Framework.h"
#include "Billboard.h"

Billboard::Billboard(Shader* shader)
	:Renderer(shader)
{
	// geometry에서 면으로 만들거라서 point
	Topology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	sDiffuseMap = shader->AsSRV("BillboardMap");
}

Billboard::~Billboard()
{
	SafeDelete(textureArray);
}

void Billboard::Update()
{
	Super::Update();
}

void Billboard::Render()
{
	// 개수가 바뀌면 계속 추가해야되는 지우고 다시 생성해야된다.
	if (textureNames.size() > 0 && textureArray == NULL)
	{
		SafeDelete(textureArray);

		textureArray = new TextureArray(textureNames);
	}

	// DrawCount 가 변경됬을때 바인드 데이터를 갱신해준다.
	if (vertexCount != vertices.size())
	{
		vertexCount = vertices.size();

		SafeDelete(vertexBuffer);
		vertexBuffer = new VertexBuffer(&vertices[0], vertices.size(), sizeof(VertexBillboard));
		
	}

	Super::Render();

	sDiffuseMap->SetResource(textureArray->SRV());
	shader->Draw(0, Pass(), vertexCount);
}

void Billboard::Add(Vector3 & position, Vector2 & scale, UINT mapIndex)
{
	VertexBillboard vertex =
	{
		position, scale, mapIndex
	};
	// emplace_back 이동생성자를 콜한다. 더 효율적이다.
	vertices.push_back(vertex); // 복사 생성사


	
}

void Billboard::AddTexture(wstring file)
{
	textureNames.push_back(file);
}
