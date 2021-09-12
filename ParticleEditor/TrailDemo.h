#pragma once
#include "Systems/IExecute.h"

class TrailDemo : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override {}
	virtual void Render() override;
	virtual void PostRender() override {}
	virtual void ResizeScreen() override {}

private:
	void Mesh();
	void Kachujin();
	void KachujinCollider();

	void Pass(UINT mesh, UINT model, UINT anim);

private:


private:
	Shader* shader;

	CubeSky* sky;

	Material* floor;

	MeshRender* grid;

	ModelAnimator* kachujin = NULL;
	Transform* colliderInitTransforms; //무기에 대한 콜라이더
	ColliderObject* colliders[2];

	Transform* trailTransform;

	TrailEffect* trail;


	vector<MeshRender *> meshes;
	vector<ModelRender *> models;
	vector<ModelAnimator *> animators;
};