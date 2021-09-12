#pragma once

class SSRManager
{
public:
	SSRManager(Shader* shader);
	~SSRManager();

	void Parameter(ID3D11RenderTargetView* hdrRTV, ID3D11ShaderResourceView* hdrSRV,
		ID3D11ShaderResourceView* depthSRV, ID3D11DepthStencilView* depthReadOnlyDSV)
	{
		this->hdrRTV = hdrRTV;
		this->hdrSRV = hdrSRV;
		this->depthSRV = depthSRV;
		this->depthReadOnlyDSV = depthReadOnlyDSV;
	}
	void ResizeScreen();

	void Update();
	void PostRender();

	void PreRenderReflection();
	void FullScreenReflection();
	void DoReflectionBlend();

	bool IsSSROn() { return showSSR; }

private:
	void Destroy();

	void Initialize();
	void CreateBlendState();
	void CreateDepthStencilState();
	void ReflectionSetting();

private:
	Shader* shader;

private:
	struct SSRPSDesc
	{
		Matrix ProjMatrix;
		float zThickness = 1.0f;
		float nearPlaneZ;
		float stride = 8.0f;
		float maxSteps = 10.0f;

		float maxDistance = 8.0f;
		float strideZCutoff = 2.0f;		
		float numMips = 0.0f;
		float fadeStart = 0.5f;

		float fadeEnd = 1.0f;
		float padding;
		Vector2 TexSize;
		Vector4 ProjParams;
	}ssrPSDesc;
	
	bool showSSR = false;
////////////////////////////////////////////////////
// Outside Resoure
////////////////////////////////////////////////////
private:
	ID3D11ShaderResourceView* hdrSRV;
	ID3D11RenderTargetView* hdrRTV;
	ID3D11ShaderResourceView * depthSRV;
	ID3D11DepthStencilView* depthReadOnlyDSV;

////////////////////////////////////////////////////
// Inside Resoure
////////////////////////////////////////////////////
private:
	RenderTarget* reflectRT = NULL;
	DepthStencil* reflectDSV = NULL;
	Viewport* reflectVP = NULL;
private: // create resource
	ID3D11BlendState* bss;
	ID3D11DepthStencilState* dss;
	ID3D11DepthStencilState* fullscreenDSS;


	ConstantBuffer* ssrPSConstant;

	UINT width;
	UINT height;

	Render2D* render2D;

};