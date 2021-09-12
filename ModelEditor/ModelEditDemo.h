#pragma once
#include "Systems/IExecute.h"

class ModelEditDemo : public IExecute
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

	void Kachujin();
	void KachujinCollider();
	void BoneSphere();

	// obb raycast check
	void CheckIntersection();

	void Pass(UINT mesh, UINT model, UINT anim);

private:
	Shader* shader;


	

	CubeSky* sky;

	
	ModelAnimator* kachujin = NULL;
	Transform* colliderInitTransforms; //무기에 대한 콜라이더
	ColliderObject** colliders;

	ModelRender* weapon = NULL; // 무기
	Transform * weaponInitTransform; // 무기를 보관할 트랜스폼

	Material* white;
	MeshRender* boneSphere;

	int collisionIndex = -1;

	vector<MeshRender *> meshes;
	vector<ModelRender *> models;
	vector<ModelAnimator *> animators;
};