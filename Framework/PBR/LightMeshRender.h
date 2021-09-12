#pragma once

class LightMeshRender : public RendererPBR
{
public:
	LightMeshRender();
	~LightMeshRender();

	void Update();
	void Render();

public:	
	
	void Pass(UINT pass);
	void Topology(D3D11_PRIMITIVE_TOPOLOGY t);

	void UpdatePointTransform(UINT instanceid, Transform& transform);  
	void UpdateSpotTransform(UINT instanceid, Transform& transform);

	Transform* AddPointTransform();
	Transform* GetPointTransform(UINT index) { return pointLightMesh->GetTransform(index); }
	Transform* AddSpotTransform();
	Transform* GetSpotTransform(UINT index) { return spotLightMesh->GetTransform(index); }

	void UpdateTransforms(); // 전체 위치를 gpu로 보내주는것

private:
	void CreatDepthStencilState();

private:
	Shader* shader;
	class PBRMeshRender* pointLightMesh;
	class PBRMeshRender* spotLightMesh;

	bool bInitialized = false;

	ID3D11DepthStencilState* dss;

};