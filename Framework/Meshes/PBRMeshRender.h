#pragma once
#define MAX_MESH_INSTANCE 500

class PBRMeshRender : public RendererPBR
{
public:
public:
	// 그릴 매쉬가 넘어옴
	PBRMeshRender(Shader* shader, Mesh* mesh);
	~PBRMeshRender();

	Mesh* GetMesh() { return mesh; }

	void Update();
	void Render();

	void Pass(UINT val) { mesh->Pass(val); }
	void Topology(D3D11_PRIMITIVE_TOPOLOGY topology) { mesh->Topology(topology); }

	void UpdateTransform(UINT instanceId, Transform& transform);

	Transform* AddTransform();
	Transform* GetTransform(UINT index) { return transforms[index]; }
	UINT GetTransformCount() { return transforms.size(); }
	void UpdateTransforms();

private:
	Mesh* mesh;

	vector<Transform *> transforms;
	// gpu 복사
	Matrix worlds[MAX_MESH_INSTANCE];

	VertexBuffer* instanceBuffer;
};