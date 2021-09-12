#pragma once

class CubeSky
{
public:
	CubeSky(wstring file, Shader* shader = NULL);
	CubeSky(ID3D11ShaderResourceView* srv);
	~CubeSky();

	void Update();
	void Render();

	void Pass(UINT val) { pass = val; }

	void SetSRV(ID3D11ShaderResourceView * val);

private:
	UINT pass = 0;

	bool bCreateShader = false;
	bool bCreateSRV = false;
	Shader* shader;
	MeshRender* sphereRender;

	ID3D11ShaderResourceView* srv;
	ID3DX11EffectShaderResourceVariable* sSrv;

	struct Desc
	{
		float lod = 0.0f;
		float padding[3];
	} desc;
	ConstantBuffer * clod = NULL;
	ID3DX11EffectConstantBuffer* sLod;

	ID3D11SamplerState * state;
	ID3D11DepthStencilState* dss;
};