#pragma once

class GBuffer
{

public:
	GBuffer(Shader* shader, UINT width = 0, UINT height = 0);
	~GBuffer();

	void PreRender();
	void Render();
	void PostRender();

	void DrawDebug(bool val) { bDrawDebug = val; }

private:
	// computeLight // diffuse ambient specular emissive
	void DirectionalLight();

	void CalcPointLights(UINT count);
	void PointLights();

	void CalcSpotLights(UINT count);
	void SpotLights();

	void CreateRasterizerState();
	void CreateDepthStencilView();
	void CreateDepthStencilState();
private:
	struct PointLightDesc
	{
		float TessFactor = 16.0f;
		float Padding[3];

		Matrix Projection[MAX_POINT_LIGHTS];
		PointLight PointLight[MAX_POINT_LIGHTS];
	} pointLightDesc;

	struct SpotLightDesc
	{
		float TessFactor = 16.0f;
		float Padding[3];

		Vector4 Angle[MAX_SPOT_LIGHTS];
		Matrix Projection[MAX_SPOT_LIGHTS];
		SpotLight SpotLight[MAX_SPOT_LIGHTS];
	} spotLightDesc;



private:
	bool bDrawDebug = false;

	Shader* shader;
	UINT width, height;

	RenderTarget* diffuseRTV; // Color(24), Alpha(8) - R8G8B8A8
	RenderTarget* specularRTV; // Color(24), Power(8) - R8G8B8A8
	RenderTarget* emissiveRTV; // Color(24), Power(8) - R8G8B8A8	
	RenderTarget* normalRTV; // x(32), y(32), z(32) - R32G32B32A32 // float4 ∞° « ø‰«‘
	RenderTarget* tangentRTV;  // x(32), y(32), z(32) - R32G32B32A32	
	DepthStencil* depthStencil; // ±Ì¿ÃΩ∫≈ƒΩ«¿∫ 1∞≥π€ø° æ»µ 
	Viewport* viewport;

	// rtv5 + dsv1
	Render2D* render2D[5];

	ConstantBuffer* pointLightBuffer;
	ID3DX11EffectConstantBuffer* sPointLightBuffer;

	ConstantBuffer* spotLightBuffer;
	ID3DX11EffectConstantBuffer* sSpotLightBuffer;

	ID3DX11EffectShaderResourceVariable * sSrvs;

	ID3D11RasterizerState* debugRSS;
	ID3D11RasterizerState* lightRSS;
	ID3DX11EffectRasterizerVariable* sRSS;

	ID3D11DepthStencilView* depthStencilReadOnly;

	ID3D11DepthStencilState* packDSS;
	ID3D11DepthStencilState* directionalLightDSS;
	ID3D11DepthStencilState* lightDSS;
	ID3D11DepthStencilState* testDSS;

	ID3DX11EffectDepthStencilVariable* sDSS;

};