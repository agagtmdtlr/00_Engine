#include "Framework.h"
#include "DynamicCubeMap.h"	

DynamicCubeMap::DynamicCubeMap(Shader * shader, UINT width, UINT height,DXGI_FORMAT format, DXGI_FORMAT dsvformat)
	:shader(shader), position(0, 0, 0), width(width), height(height)
{
	DXGI_FORMAT rtvFormat = format;

	//Create Texture2D - RTV
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = width;
		desc.Height = height;
		desc.ArraySize = 6; // 큐브맵을 위한 세팅
		desc.Format = rtvFormat;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // 큐브맵을 위한 세팅
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &rtvTexture));
	}

	// 큐브맵 텍스처를 렌더 타겟용으로
	//Create RTV // directx setting // OM setting system
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		desc.Format = rtvFormat;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = 6;

		Check(D3D::GetDevice()->CreateRenderTargetView(rtvTexture, &desc, &rtv));
	}

	// 큐브맵 텍스처를 쉐이더 리소스 뷰 용으로
	//Create RTV // shader setting
	//Create SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = rtvFormat;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MipLevels = 1;

		Check(D3D::GetDevice()->CreateShaderResourceView(rtvTexture, &desc, &srv));
	}


	DXGI_FORMAT dsvFormat = DXGI_FORMAT_D32_FLOAT;
	dsvFormat = DXGI_FORMAT_R32_TYPELESS;
	//Create Texture - DSV
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = width;
		desc.Height = height;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ArraySize = 6;
		desc.Format = dsvFormat;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL; // dsv 연결
		//desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // 큐브
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // 큐브
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		HRESULT hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &dsvTexture);
		assert(SUCCEEDED(hr));
	}

	//CreateDSV // OM system setting
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = 6;

		Check(D3D::GetDevice()->CreateDepthStencilView(dsvTexture, &desc, &dsv));
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MipLevels = 1;

		Check(D3D::GetDevice()->CreateShaderResourceView(dsvTexture, &desc, &depthSRV));
	}

	viewport = new Viewport((float)width, (float)height);

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_DynamicCube");

}

DynamicCubeMap::~DynamicCubeMap()
{
	SafeRelease(rtvTexture);
	SafeRelease(srv);
	SafeRelease(rtv);

	SafeRelease(dsvTexture);
	SafeRelease(dsv);

	SafeDelete(viewport);
	SafeDelete(buffer);
}

void DynamicCubeMap::PreRender(Vector3 & position, Vector3 & scale, float zNear, float zFar, float fov)
{
	this->position = position;

	//Create Views
	{
		float x = position.x;
		float y = position.y;
		float z = position.z;

		// View 행렬을 6방향으로 만들기 위해 6개의 LookAt과
		// 각 방향의 Up이 필요,
		// 위치는 물체의 위치를 그래도 사용한다.
		struct LookAt
		{
			Vector3 LookAt;
			Vector3 Up;
		} lookAt[6];

		lookAt[0] = { Vector3(x + scale.x, y , z), Vector3(0,1,0) }; // right
		lookAt[1] = { Vector3(x - scale.x, y , z), Vector3(0,1,0) }; // left
		lookAt[2] = { Vector3(x , y + scale.y , z), Vector3(0,0, -1) }; // up
		lookAt[3] = { Vector3(x , y - scale.y , z), Vector3(0,0, +1) }; // bottom
		lookAt[4] = { Vector3(x , y, z + scale.z), Vector3(0,1,0) }; // front
		lookAt[5] = { Vector3(x , y, z - scale.z), Vector3(0,1,0) }; // down

		for (UINT i = 0; i < 6; i++)
		{
			D3DXMatrixLookAtLH(&desc.Views[i], &position, &lookAt[i].LookAt, &lookAt[i].Up);
		}

		// 정투영에 가깝게 되게하기 위해서 (왜곡을 방지하기 위해서이다)
		perspective = new Perspective(1, 1, zNear, zFar, Math::PI * fov);
		perspective->GetMatrix(&desc.Projection);

		buffer->Render();
		sBuffer->SetConstantBuffer(buffer->Buffer());


		D3D::Get()->SetRenderTarget(rtv, dsv);
		D3D::Get()->Clear(Color(0, 0, 0, 1), rtv, dsv);

		viewport->RSSetViewport();
	}
}
