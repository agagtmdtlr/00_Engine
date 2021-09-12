#pragma once
#include "Systems/IExecute.h"

class TestDemo : public IExecute
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
private:
	Shader* shader;
	
	Texture* texture;

	ID3D11Texture2D* txt;
	ID3D11RenderTargetView* rtv;
	ID3D11Texture2D* dtxt;
	ID3D11DepthStencilView* dsv;
	ID3D11ShaderResourceView* srv;
	Viewport* vp;

	Render2D * render2D;

	MeshRender* sphere2;

};