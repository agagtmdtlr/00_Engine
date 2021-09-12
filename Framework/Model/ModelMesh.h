#pragma once

class ModelBone
{
public:
	friend class Model; // �𵨿��� �������ִ� ����
	friend class ModelPBR;

private: // Model������ ���� �����ϰ�
	ModelBone();
	~ModelBone();

public:
	int Index() { return index; }

	int ParentIndex() { return parentIndex; }
	ModelBone* Parent() { return parent; }

	wstring Name() { return name; }

	Matrix& Transform() { return transform; }
	void Transform(Matrix& matrix) { transform = matrix; }

	vector<ModelBone *>& Childs() { return childs; }

private:
	int index;
	wstring name;

	// Model class���� �������ִ� �Լ� ó���� ���ٰŴ�.
	int parentIndex;
	ModelBone* parent; // �θ��� ��

	Matrix transform;
	vector<ModelBone*> childs;
};

/////////////////////////////////////////////////////////////////////////////////

class ModelMesh // ���� ������ �־ Renderer Ŭ������ ��ӹ��� �ʾҴ�.
{
public:
	friend class Model;
	friend class ModelPBR;

private:
	ModelMesh();
	~ModelMesh();

	void Binding(Model* model);
	void Binding(ModelPBR* model);

public:
	void Pass(UINT val) { pass = val; }
	void Topology(D3D11_PRIMITIVE_TOPOLOGY t) { topology = t; }
	void SetShader(Shader* shader);
	void SetShaderPBR(Shader* shader);

	void Update();
	//void Render();
	void Render(UINT drawCount);

	wstring Name() { return name; }	

	int BoneIndex() { return boneIndex; }	
	class ModelBone* Bone() { return bone; }

	// �Ʒ� �� �Լ��� �򰥸��� �ʵ��� �Ѵ�.

	// void Transforms(Matrix* transforms);
	// BoneDesc::Transfroms�� �������ֱ� ���� �Լ��̴�.

	// void SetTransform(Transform* transform);
	// Model�� ��ġ�� ������ �����͸� �����ϴ� �Լ��̴�.

	void TransformsSRV(ID3D11ShaderResourceView* srv) { transformsSRV = srv; }

private:
	struct BoneDesc
	{
		// �迭�� ���� �� Cbuffer�� ���̴��� ����
		// �� ��ü�� ��� �迭�� �� �迭���� ���� �׷��� �Ž��� ������ ���� ��ȣ�� ���� �Ѿ�ϴ�.
		//Matrix Transforms[MAX_MODEL_TRANSFORMS]; // instancing������ �ʿ䰡 ����
		
		// �ִϸ��̼� ȣȯ���� ���� �ʿ��� �������̴�.
		UINT Index;
		float Padding[3];
	} boneDesc;

private: // ������ ��ü
	wstring name;

	Shader* shader;
	UINT pass = 0;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//Transform* transform = NULL;
	PerFrame* perFrame = NULL;

	wstring materialName = L"";
	Material* material = NULL;
	wstring materialNamePBR = L"";
	class MaterialPBR* meterialPBR = NULL;

	int boneIndex;
	class ModelBone* bone;

	VertexBuffer* vertexBuffer;
	UINT vertexCount;
	Model::ModelVertex* vertices;

	IndexBuffer* indexBuffer;
	UINT indexCount;
	UINT* indices;

	ConstantBuffer* boneBuffer;
	ID3DX11EffectConstantBuffer* sBoneBuffer;

	ID3D11ShaderResourceView* transformsSRV = NULL;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV = NULL;
};