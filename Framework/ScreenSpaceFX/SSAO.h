#pragma once

class SSAO
{
public:
	SSAO();
	~SSAO();
	void ResizeScreen();
	void Update();

	void SetPackAO(ID3D11ShaderResourceView* pack) { this->packAO = pack; }
	void SetPackNormal(ID3D11ShaderResourceView* pack) { this->packNormal = pack; }
	void SetPackDepth(ID3D11ShaderResourceView* pack) { this->packDepth = pack; }

	ID3D11ShaderResourceView* ComputeSSAO();
	ID3D11ShaderResourceView* ComputeSSAORenderTarget();

	bool IsSSAOOn() { return doSSAO; }
private:
	void DownScale();
	void FinalPass();
	void Blur(ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output);

	void ConstantUpdate();
private:
	void SSAORenderTarget();
	void BlurRenderTarget();
private:
	void Initialize();	
	void CreateResource();

private:
	struct DownScaleDesc
	{
		UINT Width;
		UINT Height;
		Vector2 ResRcp;
		Vector4 ProjParams;
		Matrix ViewMatrix;
		Matrix ProjMatrix;
		float OffsetRadius;
		float Radius;
		float pad[2];
		Vector4 Samples[64];
	} downScaleDesc;

	struct BlurDesc
	{
		float texelX;
		float texelY;
		float padding[2];
	} blurDesc;


	float offsetRadius = 10.0f;
	float radius = 13.0f;
	bool doSSAO = true;
private:
	Shader* shader;

	ID3D11ShaderResourceView* packAO;
	ID3D11ShaderResourceView* packDepth;
	ID3D11ShaderResourceView* packNormal;


	ID3D11Buffer* buffer;
	ID3D11ShaderResourceView* bufferSRV;
	ID3D11UnorderedAccessView* bufferUAV;

	ID3D11Texture2D* SSAOtexture;
	ID3D11ShaderResourceView* SSAOSRV;
	ID3D11UnorderedAccessView* SSAOUAV; 

	ID3D11Texture2D* texture[2];
	ID3D11ShaderResourceView* textureSRV[2];
	ID3D11UnorderedAccessView* textureUAV[2];

	ConstantBuffer* descBuffer;
	ID3DX11EffectConstantBuffer* sDescBuffer;
	ConstantBuffer* blurBuffer;;
	ID3DX11EffectConstantBuffer* sBlurBuffer;

	ID3D11Texture2D* noiseTexture;
	ID3D11ShaderResourceView* noiseTextureSRV;

	RenderTarget* ssaoRT;
	DepthStencil* ssaoDSV;
	Viewport* ssaoVP;

	RenderTarget* blurRT;

	UINT Width;
	UINT Height;
};