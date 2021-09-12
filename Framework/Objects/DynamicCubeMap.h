#pragma once

// 6면을 하나씩 렌더링 하는 것이 아니라
// 6면을 한번에 렌더링 수행할 것입니다.
// view[6] -> GS
// Geometry Shader 에서는 삼각형을 6방향(maxvertexcount(18))으로 렌더링이 가능하기
// 때문에 gS에서 이루어집니다.
// projection[]

class DynamicCubeMap
{
public:
	DynamicCubeMap(Shader* shader, UINT width, UINT height,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM ,
		DXGI_FORMAT dsvformat = DXGI_FORMAT_D32_FLOAT);
	~DynamicCubeMap();

	void PreRender(Vector3& position, Vector3& scale, float zNear = 0.1f, float zFar = 500.0f, float fov = 0.5f);
	UINT& Type() { return desc.Type; }

	ID3D11ShaderResourceView* SRV() { return srv; }
	ID3D11ShaderResourceView* DepthSRV() { return depthSRV; }
	Perspective* GetPerspective() { return perspective; }
private:
	struct Desc
	{
		UINT Type = 0;
		float Padding[3];

		Matrix Views[6];
		Matrix Projection;

	} desc;

private:
	Shader* shader;
	Vector3 position; // 위치

	UINT width, height; // 큐브맵의 크기

	// 큐브맵 전용
	ID3D11Texture2D* rtvTexture;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;

	ID3D11Texture2D* dsvTexture;
	ID3D11DepthStencilView* dsv;
	ID3D11ShaderResourceView* depthSRV;

	Perspective* perspective; 
	Viewport* viewport;

	class ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer = NULL;
};