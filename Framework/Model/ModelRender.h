#pragma once

class ModelRender
{
public:
	ModelRender(Shader* shader);
	~ModelRender();

	void Update();
	void Render();

public:
	void ReadMesh(wstring file);
	void ReadMaterial(wstring file);
	
	Model* GetModel() { return model; }

	void Pass(UINT pass);

	// NULL ��ü ���� ���� ����
	// �� �� �̷��� matrix�� �����ϴ°�
	void UpdateTransform(UINT instanceid, UINT boneIndex, Transform& transform);  // �������� ����ϱ� ����

	Transform* AddTransform();
	Transform* GetTransform(UINT index) { return transforms[index]; }	
	void UpdateTransforms(); // ��ü ��ġ�� gpu�� �����ִ°�

private:
	void CreateTexture();

private:


	Shader* shader;

	Model* model;
	// instance data �� �Ѿ��
	// �� ���� �� Ʈ�������� ��°� �ƴ�
	// texture2d �� intance ���̺� �� �ν��Ͻ��� �������� ���� ���̴�.
	/*
	bool bRead = false;
	Transform* transform; // ���� ����� ����
	Matrix transforms[MAX_MODEL_TRANSFORMS]; // Bone ��ü�� World Transform
	*/

	vector<Transform *> transforms;
	Matrix worlds[MAX_MODEL_INSTANCE];

	VertexBuffer* instanceBuffer;

	// �� Instance
	// �� bone
	Matrix boneTransforms[MAX_MODEL_INSTANCE][MAX_MODEL_TRANSFORMS];

	ID3D11Texture2D* texture = NULL;
	ID3D11ShaderResourceView* srv;
};