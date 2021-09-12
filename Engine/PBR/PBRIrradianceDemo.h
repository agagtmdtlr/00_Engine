#pragma once
#include "Systems/IExecute.h"

class PBRIrradianceDemo : public IExecute
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


private:
	void Mesh();
	void Material();

private:
	Shader* shader;

	Texture* texture;
	Render2D* render2D;
	Render2D* cube2D[6];

	class PBRMeshRender* sphere;
	vector<class MaterialPBR*> materials;

	CubeSky* sky;
};