#pragma once

using Microsoft::WRL::ComPtr;

class Factory
{
public:
	static ComPtr<ID3D11Texture2D>  MakeTexture2D(const D3D11_TEXTURE2D_DESC & desc);
	static ComPtr<ID3D11Texture2D>  CopyTexture2D(const ComPtr<ID3D11Texture2D> & texture);

	static ComPtr<ID3D11ShaderResourceView>  MakeSRV(const D3D11_TEXTURE2D_DESC & desc);
	static ComPtr<ID3D11UnorderedAccessView>  MakeUAV(const D3D11_TEXTURE2D_DESC & desc);
	static ComPtr<ID3D11RenderTargetView>  MakeRTV(const D3D11_TEXTURE2D_DESC & desc);
	static ComPtr<ID3D11DepthStencilView>  MakeDSV(const D3D11_TEXTURE2D_DESC & desc);


};