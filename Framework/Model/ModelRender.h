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

	// NULL 전체 본에 대한 복사
	// 그 본 이래로 matrix를 적용하는것
	void UpdateTransform(UINT instanceid, UINT boneIndex, Transform& transform);  // 본정보를 계산하기 위함

	Transform* AddTransform();
	Transform* GetTransform(UINT index) { return transforms[index]; }	
	void UpdateTransforms(); // 전체 위치를 gpu로 보내주는것

private:
	void CreateTexture();

private:


	Shader* shader;

	Model* model;
	// instance data 로 넘어간다
	// 각 모델이 의 트랜스폼을 담는게 아닌
	// texture2d 의 intance 테이블에 각 인스턴스의 본정보를 담을 것이다.
	/*
	bool bRead = false;
	Transform* transform; // 모델을 감사는 월드
	Matrix transforms[MAX_MODEL_TRANSFORMS]; // Bone 전체의 World Transform
	*/

	vector<Transform *> transforms;
	Matrix worlds[MAX_MODEL_INSTANCE];

	VertexBuffer* instanceBuffer;

	// 행 Instance
	// 열 bone
	Matrix boneTransforms[MAX_MODEL_INSTANCE][MAX_MODEL_TRANSFORMS];

	ID3D11Texture2D* texture = NULL;
	ID3D11ShaderResourceView* srv;
};