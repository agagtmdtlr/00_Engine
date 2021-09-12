#pragma once

class GBufferPBR
{
public:
	GBufferPBR(Shader* shader);
	~GBufferPBR();

	void Initialize();	

	void Pass(UINT p1, UINT p2, UINT p3);
	void DebugPass(UINT p1, UINT p2);
	void SetReadOnlyDSV();

	void SaveTexture(wstring file);

	void ResizeScreen();

	void Update();
	
	void PreRender();
	void ReadySkyRender();
	void ReadyRender();
	void Render();
	void PostRender();

	void DrawDebug(bool val) { bDrawDebug = val; }
	void DrawLight(bool val) { bDrawLight = val; }

	ID3D11ShaderResourceView* GetResult() { return result->SRV(); }
	ID3D11RenderTargetView* GetResultRTV() { return result->RTV(); }
	ID3D11ShaderResourceView* GetDepthSRV() { return dsv->SRV(); }
	ID3D11DepthStencilView* GetReadOnlyDSV() { return depthStencilReadOnly; }

private: // compute Light method
	void Destroy();

	void DirectionalLight();
	void CalcPointLights(UINT count);
	void PointLights();
	void CalcSpotLights(UINT count);
	void SpotLights();
	void AmbientLights();

	void CreateBlendState();
	void CreateRasterizerState();
	void CreateDepthStencilView();
	void CreateDepthStencilState();

private:
	class SSAO* ssao = NULL;
	ID3D11ShaderResourceView* ssaoSRV = NULL;
	ID3DX11EffectShaderResourceVariable* sSSAOSRV = NULL;


private: // Deffered Light Desc
	struct DefferedDesc
	{
		Vector4 ProjectValues;
	}defferedDesc;

	struct PointLightDesc
	{
		float TessFactor = 32.0f;
		float Padding[3];

		Matrix ShadowCubeProjection[MAX_POINT_LIGHTS];
		Matrix Projection[MAX_POINT_LIGHTS];
		PointLight PointLight[MAX_POINT_LIGHTS];
	} pointLightDesc;

	struct SpotLightDesc
	{
		float TessFactor = 64.0f;
		float Bias = 0.00f;
		float Padding[2];

		Vector4 Angle[MAX_SPOT_LIGHTS]; // sin / cos -> domain shader
		Matrix Projection[MAX_SPOT_LIGHTS]; //
		SpotLightPBR SpotLight[MAX_SPOT_LIGHTS];
	} spotLightDesc;

	struct DescPS
	{
		Matrix ShadowMap[MAX_SPOT_LIGHTS]; // light mesh world to shadow space
	}spotLightDescPS;

private:
	Shader* shader;

	UINT width, height;
	UINT directPass;
	UINT pointPass;
	UINT spotPass;
	UINT debugPass[2];
	bool bDrawDebug = true;

private: // Debug 2D Imgui Optiion
	bool bSSAO = true;
	bool bRender2D = false;
	bool bCloseUp = false;
	UINT selectedGBuffer2D = 0;
private: // deffered light render Option
	bool bDrawLight = true;

private: // GBuffer Resource;
	RenderTarget* albedoRTV = NULL;
	RenderTarget* metallicRTV = NULL;
	RenderTarget* roughnessRTV = NULL;
	RenderTarget* aoRTV = NULL;
	RenderTarget* normalRTV = NULL;
	RenderTarget* tangentRTV = NULL;
	DepthStencil* dsv = NULL;
	Viewport* viewPort = NULL;

	ID3DX11EffectShaderResourceVariable* sGBufferMapsSRV;

	Render2D* render2D[6];
	Render2D* resultRender2D;

private:// save result for light and any 2pass rendering
	RenderTarget* result = NULL;	
private:
	ConstantBuffer* defferedBuffer = NULL;
	ID3DX11EffectConstantBuffer* sDefferedBuffer = NULL;


	ConstantBuffer* pointLightBuffer = NULL;
	ID3DX11EffectConstantBuffer* sPointLightBuffer = NULL;
	ConstantBuffer* spotLightBuffer = NULL;
	ID3DX11EffectConstantBuffer* sSpotLightBuffer = NULL;
	ConstantBuffer* spotLightBuffer2 = NULL;
	ID3DX11EffectConstantBuffer* sSpotLightBuffer2 = NULL;
	

	ID3D11RasterizerState* debugRSS;
	ID3D11RasterizerState* lightRSS;
	ID3D11RasterizerState* spotRSS;
	ID3DX11EffectRasterizerVariable* sRSS;

	ID3D11DepthStencilView* depthStencilReadOnly = NULL;

	ID3D11DepthStencilState* packDSS;
	ID3D11DepthStencilState* directionalLightDSS;
	ID3D11DepthStencilState* lightDSS;
	ID3D11DepthStencilState* spotLightDSS;
	ID3D11DepthStencilState* testDSS;
	ID3DX11EffectDepthStencilVariable* sDSS;

	ID3D11BlendState* lightBS;
	ID3DX11EffectBlendVariable* sBS;

};