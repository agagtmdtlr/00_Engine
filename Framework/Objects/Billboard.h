#pragma once

#define MAX_BILLBOARD_COUNT 10000

class Billboard : public Renderer
{
public:
	Billboard(Shader* shader);
	~Billboard();

	void Update();
	void Render();

	void Add(Vector3& position, Vector2& scale, UINT mapIndex);
	void AddTexture(wstring file);

private:
	struct VertexBillboard
	{
		Vector3 Position;
		//Vector2 Uv; geometry���� ���� �����Ѵ�.
		Vector2 Scale;
		UINT MapIndex;
		//float Wind;
	};

private:
	vector<VertexBillboard> vertices;
	//VertexBillboard* vertices; // vertex shader�� ������
	//UINT* indices; // geometry������ �ʿ䰡 ����.

	UINT drawCount = 0;
	UINT prevCount = 0;

	vector<wstring> textureNames;
	TextureArray* textureArray = NULL;
	ID3DX11EffectShaderResourceVariable* sDiffuseMap;
};