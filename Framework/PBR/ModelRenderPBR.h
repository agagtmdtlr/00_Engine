#pragma once

class ModelRenderPBR : public RendererPBR
{
public:
	ModelRenderPBR(Shader* shader);
	~ModelRenderPBR();

	void Update();
	void Render();

public:
	void ReadMesh(wstring file);
	void ReadMaterial(wstring file);
	
	ModelPBR* GetModel() { return model; }

	void Pass(UINT pass);
	void Topology(D3D11_PRIMITIVE_TOPOLOGY t);

	// NULL ��ü ���� ���� ����
	// �� �� �̷��� matrix�� �����ϴ°�
	void UpdateTransform(UINT instanceid, UINT boneIndex, Transform& transform);  // �������� ����ϱ� ����
	void UpdateTransformChild(UINT instanceid, UINT boneIndex, Transform& transform);  // �������� ����ϱ� ����

	Matrix GetInstanceBoneMatrix(UINT instanceid, UINT boneIndex) { return boneTransforms[instanceid][boneIndex]; }

	Transform* AddTransform();
	Transform* GetTransform(UINT index) { return transforms[index]; }	
	void UpdateTransforms(); // ��ü ��ġ�� gpu�� �����ִ°�

private:
	void CreateTexture();

private:
	Shader* shader;
	ModelPBR* model;

	vector<Transform *> transforms; // instance transform
	Matrix worlds[MAX_MODEL_INSTANCE];

	VertexBuffer* instanceBuffer;

	// �� Instance
	// �� bone
	Matrix boneTransforms[MAX_MODEL_INSTANCE][MAX_MODEL_TRANSFORMS];
	ID3D11Texture2D* texture = NULL; // boneTransforms data
	ID3D11ShaderResourceView* srv;
	ID3DX11EffectShaderResourceVariable* sSRV;
};