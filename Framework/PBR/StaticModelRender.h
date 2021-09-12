#pragma once

class StaticModelRender : public RendererPBR
{
public:
	StaticModelRender(Shader* shader);
	~StaticModelRender();

	void Update();
	void Render();
private:
	void MaterialRender(wstring name);

public:
	void ReadMesh(wstring file);	
	void ReadMaterial(wstring file);
	
	void Pass(UINT pass);
	void Topology(D3D11_PRIMITIVE_TOPOLOGY t);

	void UpdateTransform(UINT instanceid, Transform& transform);  

	Transform* AddTransform();
	Transform* GetTransform(UINT index) { return transforms[index]; }	
	void UpdateTransforms(); // 전체 위치를 gpu로 보내주는것

private:
	void ReadMode(wstring file);
	void ReadMode2(wstring file);

private:
	Shader* shader;

	vector<Transform *> transforms; // instance transform
	Matrix worlds[MAX_MODEL_INSTANCE];

	VertexBuffer* instanceBuffer;

	vector<StaticMesh*> meshes;
	vector<MaterialPBR* > materials;

	bool bInitialized = false;

};