#pragma once
#include "Systems/IExecute.h"

class PBRInteriorDemo : public IExecute
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
	void StaticMesh();
	void Material();
	void Model();
	void Animator();

	void DirectionalLighting();
	void PointLighting();
	void SpotLighting();

	void LightMesh();
	void LightManage();

	void IBL();
	void ChangeIBL();

private:
	Shader* shader;
	Shader* testShader;

	Shadow* shadow;
	class ShadowCube * shadowCube;
	class ShadowSpot* shadowSpot;

	class SSRManager* ssrManager;

	Texture* texture;
	Render2D* render2D;
	Render2D* cube2D[6];

private:
	class PostFX* postfx;
	float gAdaption = 0.01f;
	
	float AdaptionNorm;
private:

	class GBufferPBR* gbuffer;

	MeshRender* pointLightMesh;
	MeshRender* spotLightMesh;

private: // LightMesh Data
	class LightMeshRender* lightMesh;


private:

	class ModelRenderPBR* model = NULL;
	class StaticModelRender* staticModel = NULL;
	class ModelAnimatorPBR * animator = NULL;
	CubeSky* sky;
	Timer* timer;

	vector<class PBRMeshRender *> meshes;
	vector<class ModelRenderPBR *> models;
	vector<class StaticModelRender*> staticModels;
	vector<class ModelAnimatorPBR *> animators;

	vector<struct IBL_Texture> IBLs;

	float velocity = 1;
	bool bRenderAnim = true;
	UINT IBL_index = 0;

private:
	float lightTime = 0.0f;
	class Camera* camera;
};