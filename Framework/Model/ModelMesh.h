#pragma once

class ModelBone
{
public:
	friend class Model; // 모델에게 개방해주는 입장
	friend class ModelPBR;

private: // Model에서만 생성 가능하게
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

	// Model class에서 연결해주는 함수 처리를 해줄거다.
	int parentIndex;
	ModelBone* parent; // 부모의 본

	Matrix transform;
	vector<ModelBone*> childs;
};

/////////////////////////////////////////////////////////////////////////////////

class ModelMesh // 꼬일 염려가 있어서 Renderer 클래스를 상속받지 않았다.
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

	// 아래 두 함수가 헷갈리지 않도록 한다.

	// void Transforms(Matrix* transforms);
	// BoneDesc::Transfroms를 복사해주기 위한 함수이다.

	// void SetTransform(Transform* transform);
	// Model의 위치를 움직일 데이터를 세팅하는 함수이다.

	void TransformsSRV(ID3D11ShaderResourceView* srv) { transformsSRV = srv; }

private:
	struct BoneDesc
	{
		// 배열로 만든 후 Cbuffer로 세이더에 전송
		// 본 전체의 행렬 배열과 그 배열에서 현재 그려질 매쉬가 참조할 본의 번호가 같이 넘어갑니다.
		//Matrix Transforms[MAX_MODEL_TRANSFORMS]; // instancing에서는 필요가 없음
		
		// 애니메이션 호환성을 위해 필요한 데이터이다.
		UINT Index;
		float Padding[3];
	} boneDesc;

private: // 렌더링 주체
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