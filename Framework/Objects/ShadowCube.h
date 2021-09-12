#pragma once

class ShadowCube
{
public:
	ShadowCube(Shader* shader, float width, float height,
		DXGI_FORMAT rtvFormat, bool useStencil, UINT shadowCounts);
	~ShadowCube();

public:
	void Update();
	void PreRender(UINT lightIndex);
	void Render();

	ID3D11ShaderResourceView* GetRenderTargetSRV() { return srv; }
	ID3D11ShaderResourceView* GetDepthStencilSRV() { return depthSRV; }
private:
	void CalcProjection(const Vector3& position,const float range);
private:
	struct ShadowLightDesc
	{
		Vector3 position;
		float farPlane;
	} desc;

	struct DynamicCubeDesc
	{
		UINT Type = 0;
		float Padding[3];

		Matrix Views[6];
		Matrix Projection;

	} cubeDesc;
private:
	Shader * shader;
	float width;
	float height;
	bool useStencil;
	UINT shadowCounts;

	PerFrame* perframe;

	ConstantBuffer* cShadowCubeDesc;
	ID3DX11EffectConstantBuffer* sCShadowCubeDesc;
	
	// Å¥ºê¸Ê Àü¿ë
	ID3D11Texture2D* rtvTexture;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;

	ID3D11Texture2D* dsvTexture;
	ID3D11DepthStencilView** dsv;
	ID3D11ShaderResourceView* depthSRV;

	Perspective* perspective;
	Viewport* viewport;

	class ConstantBuffer* cDynamicCubeDesc;
	ID3DX11EffectConstantBuffer* sCDynamicCubeDesc = NULL;

};