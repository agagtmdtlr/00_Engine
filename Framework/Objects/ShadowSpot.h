#pragma once

class ShadowSpot
{
public:
	ShadowSpot(Shader* shader, float width, float height,
		DXGI_FORMAT rtvFormat, bool useStencil, UINT shadowCounts);
	~ShadowSpot();



public:
	
	void ResizeScreen();

	void Update();
	void PreRender(UINT lightIndex);
	void Render();

	ID3D11ShaderResourceView* GetRenderTargetSRV() { return rtv->SRV(); }
	ID3D11ShaderResourceView* GetDepthStencilSRV() { return depthSRV; }

private:
	void ResizeDestroy();

	Matrix CalcProjection(const SpotLightPBR ligh);
	void CreateRasterizerState();
private:
struct ShadowLightDesc
{
	Matrix ShadowGenMatrix;
	Vector3 LightPosition;
	float Range;
} desc;

struct ForwardSpotLightShadowDesc
{
	Matrix ForwardSpotShadwProjection[2];
};

private:
	Shader * shader;
	float width;
	float height;
	DXGI_FORMAT rtvFormat;
	bool useStencil;
	UINT shadowCounts;

	PerFrame* perframe;

	ConstantBuffer* cShadowDesc;
	ID3DX11EffectConstantBuffer* sCShadowDesc;

	RenderTarget* rtv = NULL;
	ID3D11Texture2D* dsvTexture = NULL;
	ID3D11DepthStencilView** dsv = NULL;
	ID3D11ShaderResourceView* depthSRV = NULL;

	ID3D11RasterizerState* rss;
	ID3DX11EffectRasterizerVariable* sRSS;
	
	Viewport* viewport = NULL;
};