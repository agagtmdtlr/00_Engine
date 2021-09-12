#include "stdafx.h"
#include "Converter/BRDFMap.h"

BRDFMap::BRDFMap()
{
	Initialize();
}

BRDFMap::~BRDFMap()
{
	SafeDelete(shader);
	SafeRelease(dsvBRDF);
	SafeRelease(srvBRDF);
	SafeRelease(rtvBRDF);
	SafeRelease(mapDSVBRDF);
	SafeRelease(txtBRDF);
	SafeDelete(vpBRDF);
}

void BRDFMap::BakingBRDF()
{
	RenderBRDF();
	SaveBRDF();
}

void BRDFMap::SaveBRDF()
{
	wstring path = L"../../_Textures/BRDF/BRDF2.dds";
	HRESULT	hr = D3DX11SaveTextureToFile(D3D::GetDC(), txtBRDF, D3DX11_IFF_DDS, path.c_str());

}

void BRDFMap::RenderBRDF()
{
	D3D::Get()->SetRenderTarget(rtvBRDF, dsvBRDF);
	D3D::Get()->Clear(Color(0, 0, 0, 1), rtvBRDF, dsvBRDF);

	vertexBuffer->Render();
	indexBuffer->Render();	
	vpBRDF->RSSetViewport();

	shader->DrawIndexed(0, 0, 6);
}

void BRDFMap::Initialize()
{
	shader = new Shader(L"127_BakingBRDF.fx");

	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = (UINT)BRDFSize;
		desc.Height = (UINT)BRDFSize;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.MipLevels = 1;
		desc.MiscFlags = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.SampleDesc.Count = 1;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &txtBRDF));

		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.MiscFlags = 0;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &mapDSVBRDF));
	}

	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		Check(D3D::GetDevice()->CreateRenderTargetView(txtBRDF, &desc, &rtvBRDF));
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;

		Check(D3D::GetDevice()->CreateShaderResourceView(txtBRDF, &desc, &srvBRDF));
	}

	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		Check(D3D::GetDevice()->CreateDepthStencilView(mapDSVBRDF, &desc, &dsvBRDF));
	}

	vpBRDF = new Viewport(BRDFSize, BRDFSize);

	{
		vertices[0].Position = Vector3(-1, -1, 0);
		vertices[1].Position = Vector3(-1, 1, 0);
		vertices[2].Position = Vector3(1, -1, 0);
		vertices[3].Position = Vector3(1, 1, 0);

		//vertices[0].Uv = Vector2(0,0);
		//vertices[1].Uv = Vector2(0,1);
		//vertices[2].Uv = Vector2(1,0);
		//vertices[3].Uv = Vector2(1,1);
		vertices[0].Uv = Vector2(0, 1);
		vertices[1].Uv = Vector2(0, 0);
		vertices[2].Uv = Vector2(1, 0);
		vertices[3].Uv = Vector2(1, 1);

		vertexBuffer = new VertexBuffer(vertices, 4, sizeof(VertexBRDF));
		indexBuffer = new IndexBuffer(indices, 6);
	}
}

