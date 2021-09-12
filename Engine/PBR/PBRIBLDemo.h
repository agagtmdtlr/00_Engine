#pragma once
#include "Systems/IExecute.h"

class PBRIBLDemo : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual void ResizeScreen() override {}	

	void Pass(UINT mesh, UINT model, UINT anim);

private:
	void Mesh();
	void Material();
	void Model();
	void Animator();
private:
	Shader* shader;

	Shadow* shadow;

	Texture* texture;
	Render2D* render2D;
	Render2D* cube2D[6];

	class PBRMeshRender* sphere[10]; // 10 types materials
	class PBRMeshRender* cylinder;
	class PBRMeshRender* cube;
	class PBRMeshRender* grid;


	vector<class MaterialPBR*> sphereMaterials;
	vector<class MaterialPBR*> cylinderMaterials;
	vector<class MaterialPBR*> cubeMaterials;
	vector<class MaterialPBR*> gridMaterials;

	class ModelRenderPBR* model = NULL;
	class ModelAnimatorPBR* animator = NULL;

	CubeSky* sky;

	Timer* timer;

	vector<PBRMeshRender *> meshes;
	vector<ModelRenderPBR *> models;
	vector<ModelAnimatorPBR *> animators;
};