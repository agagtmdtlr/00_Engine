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

	// NULL 전체 본에 대한 복사
	// 그 본 이래로 matrix를 적용하는것
	void UpdateTransform(UINT instanceid, UINT boneIndex, Transform& transform);  // 본정보를 계산하기 위함
	void UpdateTransformChild(UINT instanceid, UINT boneIndex, Transform& transform);  // 본정보를 계산하기 위함

	Matrix GetInstanceBoneMatrix(UINT instanceid, UINT boneIndex) { return boneTransforms[instanceid][boneIndex]; }

	Transform* AddTransform();
	Transform* GetTransform(UINT index) { return transforms[index]; }	
	void UpdateTransforms(); // 전체 위치를 gpu로 보내주는것

private:
	void CreateTexture();

private:
	Shader* shader;
	ModelPBR* model;

	vector<Transform *> transforms; // instance transform
	Matrix worlds[MAX_MODEL_INSTANCE];

	VertexBuffer* instanceBuffer;

	// 행 Instance
	// 열 bone
	Matrix boneTransforms[MAX_MODEL_INSTANCE][MAX_MODEL_TRANSFORMS];
	ID3D11Texture2D* texture = NULL; // boneTransforms data
	ID3D11ShaderResourceView* srv;
	ID3DX11EffectShaderResourceVariable* sSRV;
};