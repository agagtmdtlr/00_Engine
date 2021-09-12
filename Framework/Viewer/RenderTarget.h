#pragma once

class RenderTarget
{
public:
	// 정밀도에 따라 format을 변경 // upscailing  방식으로 해볼수 도 잇다.
	RenderTarget(UINT width = 0, UINT height = 0, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	~RenderTarget();

	ID3D11RenderTargetView* RTV() { return rtv; }
	ID3D11ShaderResourceView* SRV() { return srv; }

	void SaveTexture(wstring file);

	void PreRender(class DepthStencil* depthStencil);

	// 여러개의 렌더타겟 찌ㅓㅇ하기
	static void PreRender(RenderTarget** targets, UINT count, class DepthStencil* depthStencil);


private:
	UINT width, height;
	DXGI_FORMAT format;

	ID3D11Texture2D* texture; // rtv - texture
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv; // 받아온 texture를 최종 세이더 넘겨줄 srv
};