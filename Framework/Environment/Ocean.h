#pragma once

#define WAVE_MAX 64

class Ocean : public Renderer
{
public :
	struct InitializeDesc
	{
		Shader * shader;
		UINT Width;
		UINT Height;
		UINT Patch;
	};

	typedef VertexTexture VertexOcean;

public:

	Ocean(InitializeDesc& desc);
	~Ocean();

	void Update();
	void Render();

private:
	void CreateVertexData();
	void CreateIndexData();

private:
	struct OceanDesc
	{
		UINT WaveCount;
		float AmplitudeMax;
		float Time = 0;
		float Padding;
	} oceanDesc;
	

	struct WaveDesc
	{
		float Steepness; // reality amplitude
		float WaveLength; // lamda ÆÄÀå
		Vector2 Direction;
	} waveDesc[WAVE_MAX];

	InitializeDesc initDesc;

	VertexOcean * vertices;
	UINT * indices;

	UINT vertexCount;
	UINT indexCount;
	UINT faceCount;

	UINT vertexPerPatchX;
	UINT vertexPerPatchZ;

	ConstantBuffer* waveDescCBuffer;	
	ID3DX11EffectConstantBuffer* sWaveDescCBuffer;
	ConstantBuffer* oceanDescCBuffer;
	ID3DX11EffectConstantBuffer* sOceanDescCBuffer;


private:
	Texture* normalMap;
	ID3DX11EffectShaderResourceVariable* sNormalMap;
	Texture* depthMap;
	ID3DX11EffectShaderResourceVariable* sDepthMap;
	Texture* displacementMap;
	ID3DX11EffectShaderResourceVariable* sDisplacementMap;
	Texture* diffuseMap;
	ID3DX11EffectShaderResourceVariable* sDiffuseMap;
};