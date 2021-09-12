#pragma once

class EnvironmentMap
{

public:
	EnvironmentMap();	
	~EnvironmentMap();

	void LoadEquirectangularMap(wstring file);
	void BakingCubeMap();	
	void BakingEnvironeMentMap();
	void BakingIrradianceMap();
	void BakingReflectionMap();

	void SaveEnvironmentMap();
	void SaveIrradianceMap();
	void SaveReflectionMap();

	ID3D11ShaderResourceView* GetSRVEnvironment() { return srvEnvironment; }
	ID3D11ShaderResourceView* GetSRVIrradiance() { return srvIrradiance; }
	ID3D11ShaderResourceView* GetSRVReflection() { return srvReflection; }
	ID3D11ShaderResourceView* GetSRVTest() { return srvTest; }

private:
	void RenderEnvironmentMap();
	void RenderIrradianceMap();
	void RenderReflectionMap();
	void RenderTestMap();
	void RenderBRDFMap();

	void Initialize();

private:
	struct Desc
	{
		Matrix Views[6];
		Matrix Projection;
		float roughness;
		float mip;
		float padding[2];
	} cubeDesc;

private:
	wstring file;

	float EnvSize = 1024;
	float IrrSize = 1024;
	float RefSize = 1024;
	float TestSize = 1024;
	UINT MaxMipLevel = 5;

	Shader* shader;
	UINT pass = 0;

	ID3D11Texture2D* cubeEnvironment = NULL;
	ID3D11Texture2D* cubeIrradiance = NULL;
	ID3D11Texture2D* cubeReflection = NULL;
	ID3D11Texture2D* cubeCopyToReflection[5];
	ID3D11Texture2D* cubeTest = NULL;

	ID3D11RenderTargetView* rtvEnvironment = NULL;
	ID3D11RenderTargetView* rtvIrradiance = NULL;
	ID3D11RenderTargetView* rtvReflection[5];
	ID3D11RenderTargetView* rtvCopyToReflection[5];
	ID3D11RenderTargetView* rtvTest = NULL;

	ID3D11Texture2D* mapDSVEnvironment;
	ID3D11Texture2D* mapDSVIrradiance;
	ID3D11Texture2D* mapDSVReflection;
	ID3D11Texture2D* mapDSVCopyToReflection[5];
	ID3D11Texture2D* mapDSVTest;

	ID3D11DepthStencilView* dsvEnvironment = NULL;
	ID3D11DepthStencilView* dsvIrradiance = NULL;
	ID3D11DepthStencilView* dsvReflection[5];
	ID3D11DepthStencilView* dsvCopyToReflection[5];
	ID3D11DepthStencilView* dsvTest = NULL;

	ID3D11ShaderResourceView* srvEnvironment = NULL;
	ID3D11ShaderResourceView* srvIrradiance = NULL;
	ID3D11ShaderResourceView* srvReflection = NULL;
	ID3D11ShaderResourceView* srvCopyToReflection[5];
	ID3D11ShaderResourceView* srvTest = NULL;


private:
	ID3DX11EffectShaderResourceVariable* sSRVEnvironment;

private: // constant buffer cube desc
	Perspective* perspective;	

	Viewport* vpEnvironment = NULL;
	Viewport* vpIrradiance = NULL;
	Viewport* vpReflection[5];
	Viewport* vpTest = NULL;

	ConstantBuffer* cCubeDesc;

	Texture* equirectangularMap = NULL;

	MeshRender* cube;
	MeshRender* sphere;

public:
	void ConvertEquirectangularMapToCubeMap(wstring file, float size, bool correct, wstring semi = wstring());
	ID3D11ShaderResourceView * GetConvertSRV() { return srvConvert; }
	ID3D11Texture2D* GetConvertTexture() { return convertMap->GetTexture(); }

private:
	Texture* convertMap = NULL;

	wstring equirFile;

	ID3D11Texture2D* convertCube = NULL;
	ID3D11Texture2D* dsvConvertTXT = NULL;

	ID3D11RenderTargetView* rtvConvert = NULL;
	ID3D11DepthStencilView* dsvConvert= NULL;
	ID3D11ShaderResourceView* srvConvert = NULL;

	Viewport* vpConvert = NULL;


};