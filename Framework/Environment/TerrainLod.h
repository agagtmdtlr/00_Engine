#pragma once

#define PICKING_INSTANCE_MAX 500;

class TerrainLod : public Renderer
{
public :
	struct InitializeDesc;

public:
	TerrainLod(InitializeDesc& desc);
	~TerrainLod();

	void Update();
	void PreRender();
	void Render();
	void PostRender();

	void BaseMap(wstring file);
	void LayerMap(wstring layer, wstring alpha);
	void NormalMap(wstring file);

private:
	bool InBounds(UINT x, UINT z);
	void CalcPatchBounds(UINT x, UINT z);
	void CalcBoundY();

	void CreateVertexData();
	void CreateIndexData();

	void CreateTerrainTexture();

public:
	struct InitializeDesc
	{
		Shader* shader;

		wstring heightMap;
		float CellSpacing;
		UINT CellsPerPatch;
		float HeightRatio;
	};

	struct BrushDesc
	{
		Vector2 Uv;
		Vector2 padding;
	} brushDesc;

	struct ComputeBrushDesc
	{
		Vector2 Uv;
		Vector2 TextureSize;
		float Range; // uv range (0 ~ 1)
		float Time;
		
		int Up; // -1 0 1;
		int BlurCount;
	} computeBrushDesc;

private:
	// cbuffer CB_TerrainLod
	struct BufferDesc
	{
		float MinDistance = 1.0f;
		float MaxDistance = 500.0f;
		float MinTessellation = 1.0f;
		float MaxTessellation = 64.0f;

		float TexelCellSpaceU;
		float TexelCellSpaceV;
		float WorldCellSpace = 1.0f;
		float HeightRatio = 1.0f;

		Plane WorldFrustumPlanes[6];
	} bufferDesc;

private:
	struct VertexTerrain
	{
		Vector3 Position;
		Vector2 Uv;
		Vector2 Uv1;
		Vector2 BoundsY;
	};

private:
	// 그릴 면의 개수
	UINT faceCount;

	// 가로세로 패치 내 정점의 개수
	UINT vertexPerPatchX;
	UINT vertexPerPatchZ;

private:


private:
	InitializeDesc initDesc;

	// CB_TerrainLod
	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	UINT width, height;

	VertexTerrain* vertices;
	UINT* indices;

	// 기본시야 보다 더 넢게 자를수도 있다
	Frustum* frustum;
	Camera* camera;
	Perspective* perspective;

	// 지면 변화 pixel shader에서 실시간으로 계산한다.
	Texture* heightMap;
	vector<D3DXCOLOR> heightMapPixel;
	ID3DX11EffectShaderResourceVariable* sHeightMap;

	// culling
	vector<Vector2> bounds;

	// texture blending/splatting
	Texture* baseMap = NULL;
	ID3DX11EffectShaderResourceVariable* sBaseMap;

	Texture* layerMap = NULL;
	ID3DX11EffectShaderResourceVariable* sLayerMap;

	Texture* alphaMap = NULL;
	ID3DX11EffectShaderResourceVariable* sAlphaMap;

	Texture* normalMap = NULL;
	ID3DX11EffectShaderResourceVariable* sNormalMap;

	ID3DX11EffectShaderResourceVariable* sPositionMap;

private:
	ID3D11Texture2D * terrainTexture[2];
	ID3D11RenderTargetView* terrainRTV[2];
	ID3D11ShaderResourceView* terrainSRV[2];
	DepthStencil* dsv;

	ConstantBuffer* brushBuffer;
	ID3DX11EffectConstantBuffer* sBrushBuffer;

	float frameTime = 0.0f;
	float frameRate = 2.0f;

private:
	Shader* computeShader;

	TextureBuffer* textureBuffer; // compute hegihtMap
	ID3DX11EffectShaderResourceVariable* computeInput;
	ID3DX11EffectUnorderedAccessViewVariable* computeOutput;

	Render2D* render2D[2];

	Vector4 mapSize;

	struct Compute_Height
	{
		Vector2 MousePosition;
		float Range; // brush Range
	};

	// bilboard Position;
	struct Compute_Picking
	{
		Vector2 Uv;
		Vector2 Range; // random x, y;
	};

	// use picking height
	StructuredBuffer* structureBuffer;
};