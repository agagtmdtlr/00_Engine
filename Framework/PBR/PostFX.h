#pragma once

class PostFX : public Renderer
{
public:
	PostFX(wstring shaderFile);
	~PostFX();

	void Initialize();
	void Destroy();

	void ResizeScreen();

	void Update();

	void PostProcessing();
	

	void HDRMap(ID3D11ShaderResourceView * HDRSRV);
	void GBufferDepthMap(ID3D11ShaderResourceView * DepthSRV);

	void SetTonemapParameter(float Adapt) { Adaption = Adapt; }
private:
	void Bloom();
	void Blur(ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output);
	void DownScale();
	void FinalPass();

	void CreateDepthStencilState();
	void CreateResource();

	void SaveTexture(ID3D11Texture2D* txt, wstring file);

	void ResizeDestroy();
private:// tone map resource get from gbuffer
	ID3D11ShaderResourceView* HDRSRV = NULL;
	ID3DX11EffectShaderResourceVariable* sSRVHDR;

	ID3D11ShaderResourceView* DepthSRV = NULL;
	ID3DX11EffectShaderResourceVariable* sSRVDepth;

	ID3D11DepthStencilState * tonemapDSS = NULL;
	ID3DX11EffectDepthStencilVariable* sDSS;
private:
	Shader* ComputeShader;
private: // compute shader resource
	// compute original HDR texture sSRV
	ID3DX11EffectShaderResourceVariable * sHDRTexSRV = NULL;
	// Downscaled scene texture
	ID3D11Texture2D* downScaledRT = NULL;
	ID3D11ShaderResourceView* downScaledSRV = NULL;
	ID3D11UnorderedAccessView * downScaledUAV = NULL;
	// DOF texture
	ID3D11Texture2D* DOFRT = NULL;
	ID3D11ShaderResourceView* DOFSRV = NULL;
	ID3D11UnorderedAccessView * DOFUAV = NULL;
	// Temporary texture
	ID3D11Texture2D* tempRT[2] = { NULL, NULL };
	ID3D11ShaderResourceView* tempSRV[2] = { NULL,NULL };
	ID3D11UnorderedAccessView * tempUAV[2] = { NULL, NULL };
	// Bloom texture
	ID3D11Texture2D* bloomRT = NULL;
	ID3D11ShaderResourceView* bloomSRV = NULL;
	ID3D11UnorderedAccessView * bloomUAV = NULL;
	// down scale operation for avg lum 1D storage
	ID3D11Buffer* toneMapDownScaleBuffer = NULL;
	ID3D11ShaderResourceView* toneMapDownScaleSRV = NULL;
	ID3D11UnorderedAccessView * toneMapDownScaleUAV = NULL;
	ID3DX11EffectShaderResourceVariable* sToneMapDownSacleSRV;
	ID3DX11EffectUnorderedAccessViewVariable * sToneMapDownScaleUAV;
	// average luminance
	ID3D11Buffer* toneMapAvgLumBuffer = NULL;
	ID3D11ShaderResourceView* toneMapAvgLumSRV = NULL;
	ID3D11UnorderedAccessView * toneMapAvgLumUAV = NULL;
	ID3DX11EffectShaderResourceVariable* sToneMapAvgLumSRV;
	ID3DX11EffectUnorderedAccessViewVariable * sToneMapAvgLumUAV;

	ID3D11Buffer* prevToneMapAvgLumBuffer = NULL;
	ID3D11ShaderResourceView* prevToneMapAvgLumSRV = NULL;
	ID3D11UnorderedAccessView * prevToneMapAvgLumUAV = NULL;
	ID3DX11EffectShaderResourceVariable* sPrevToneMapAvgLumSRV;

	UINT Width;
	UINT Height;
	UINT DownScaleGroups;
	float MiddleGrey = 0.863f; // 6
	float white = 100.53f; // 6
	float Adaption;
	float bloomThreashold = 2.0f;
	float bloomScale = 0.1f;

	float DOFFarStart = 999.0f;
	float DOFFarRange = 999.0f;

	struct DownScaleDesc
	{
		UINT Width;
		UINT Height;
		UINT TotalPixels;
		UINT GroupSize;
		float Adaption;
		float BloomThreshold;
		float padding[2];
	} downScaleDesc;

	struct FinalPassCB
	{
		UINT ToneMapType = 3; // ACES tone map default
		float fMiddleGrey;
		float fLumWhite; 
		float BloomScale;
		Vector2 ProjectionValues;
		Vector2 DOFFarValues;
		//UINT bloom;
		//UINT dof;
		//UINT d
	} finalPassDesc;

	struct BlurCB
	{
		UINT numApproxPasses;
		float HalfBoxFilterWidth;			// w/2
		float FracHalfBoxFilterWidth;		// frac(w/2 + 0.5)
		float InvFracHalfBoxFilterWidth;	// 1 - frac(w/2 + 0.5)
		float RcpBoxFilterWidth;			// 1/ w
		UINT pad[3];
	} blurDesc;

	ConstantBuffer* downScaleDescBuffer;
	ID3DX11EffectConstantBuffer* sDownScaleDescBuffer;
	ConstantBuffer* finalPassDescBuffer;
	ID3DX11EffectConstantBuffer* sFinalPassDescBuffer;
	ConstantBuffer* blurDescBuffer;
	ID3DX11EffectConstantBuffer* sBlurDescBuffer;;


private:
	float frameTime = 0.0f;

	Render2D* Debug2D;
};