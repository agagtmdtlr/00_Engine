#include "Framework.h"
#include "Factory.h"

ComPtr<ID3D11Texture2D> Factory::MakeTexture2D(const D3D11_TEXTURE2D_DESC & desc)
{
	return ComPtr<ID3D11Texture2D>();
}

ComPtr<ID3D11Texture2D> Factory::CopyTexture2D(const ComPtr<ID3D11Texture2D>& texture)
{
	return ComPtr<ID3D11Texture2D>();
}

ComPtr<ID3D11ShaderResourceView> Factory::MakeSRV(const D3D11_TEXTURE2D_DESC & desc)
{
	return ComPtr<ID3D11ShaderResourceView>();
}

ComPtr<ID3D11UnorderedAccessView> Factory::MakeUAV(const D3D11_TEXTURE2D_DESC & desc)
{
	return ComPtr<ID3D11UnorderedAccessView>();
}

ComPtr<ID3D11RenderTargetView> Factory::MakeRTV(const D3D11_TEXTURE2D_DESC & desc)
{
	return ComPtr<ID3D11RenderTargetView>();
}

ComPtr<ID3D11DepthStencilView> Factory::MakeDSV(const D3D11_TEXTURE2D_DESC & desc)
{
	return ComPtr<ID3D11DepthStencilView>();
}
