#pragma once
#include "Systems/IExecute.h"

class PBRDefferedDemo : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual void ResizeScreen() override;

	void Pass(UINT mesh, UINT model, UINT anim);

private:
	void Mesh();
	void Material();
	void Model();
	void Animator();

	void DirectionalLighting();
	void PointLighting();
	void SpotLighting();

	void LightMesh();
	void CameraSetting();
	

	void LightManage();
	void ModelManage();
	void CameraManage();
	void IBL();
	void ChangeIBL();

private:
	Shader* shader;
	Shader* testShader;

	Shadow* shadow;
	class ShadowCube * shadowCube;
	class ShadowSpot* shadowSpot;

	class SSRManager* ssrManager;

	Render2D* cube2D[6];

private:
	class PostFX* postfx;
	float gAdaption = 0.01f;
	
	float AdaptionNorm;
private:

	class GBufferPBR* gbuffer;

	class PBRMeshRender* sphere[10]; // 10 types materials
	class PBRMeshRender* cylinder;
	class PBRMeshRender* cube;

	class PBRMeshRender* BigSphere;

	class PBRMeshRender* grid[2];

	class PBRMeshRender* ssaoCube; // mesh for ssao effect showing

	MeshRender* pointLightMesh;
	MeshRender* spotLightMesh;

private: // LightMesh Data
	class LightMeshRender* lightMesh;

	bool bDrawLight = false;

private:
	vector<class MaterialPBR*> sphereMaterials;
	vector<class MaterialPBR*> cylinderMaterials;
	vector<class MaterialPBR*> cubeMaterials;
	vector<class MaterialPBR*> gridMaterials;

	MaterialPBR* BigSphereMaterials;

	class ModelRenderPBR* model = NULL;
	class ModelAnimatorPBR* animator = NULL;

	CubeSky* sky;

	Timer* timer;

	vector<class PBRMeshRender *> meshes;
	vector<class ModelRenderPBR *> models;
	vector<class ModelAnimatorPBR *> animators;

	float velocity = 1;
	bool bRenderAnim = true;

	vector<struct IBL_Texture> IBLs;
	UINT IBL_index = 0;

	UINT grid_mat_index = 0;
private:
	UINT camera_mode = 0;
	UINT camera_index = 0;
	Vector3 cameraPositons[14];

	class Camera* cameras[2];
};