#include "Framework.h"
#include "ShadowCube.h"

// TODO:: shaowcube를 실행해보고 정상적으로 작동하는지 테스트하기
ShadowCube::ShadowCube(Shader* shader, float width, float height, DXGI_FORMAT rtvFormat, bool useStencil, UINT shadowCounts)
	:shader(shader), width(width), height(height), useStencil(useStencil), shadowCounts(shadowCounts)
{
	//cube = new DynamicCubeMap(shader, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
	perframe = new PerFrame(shader);

	cShadowCubeDesc = new ConstantBuffer(&desc, sizeof(ShadowLightDesc));
	sCShadowCubeDesc = shader->AsConstantBuffer("CB_ShadowDynamicCube");
	cDynamicCubeDesc = new ConstantBuffer(&cubeDesc, sizeof(DynamicCubeDesc));
	sCDynamicCubeDesc = shader->AsConstantBuffer("CB_DynamicCube");


	//Create Texture2D - RTV
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = (UINT)width;
		desc.Height = (UINT)height;
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
		desc.Width = (UINT)width;
		desc.Height = (UINT)height;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ArraySize = 6 * shadowCounts;
		desc.Format = useStencil ? DXGI_FORMAT_R24G8_TYPELESS  : DXGI_FORMAT_R32_TYPELESS;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL; // dsv 연결
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // 큐브
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		HRESULT hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &dsvTexture);
		assert(SUCCEEDED(hr));
	}
	dsv = new ID3D11DepthStencilView*[shadowCounts];
	//CreateDSV // OM system setting
	for (UINT i = 0; i<shadowCounts;i++)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = useStencil ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = 6;
		desc.Texture2DArray.FirstArraySlice = 6 * i;

		Check(D3D::GetDevice()->CreateDepthStencilView(dsvTexture, &desc, &dsv[i]));
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = useStencil ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
		desc.TextureCubeArray.First2DArrayFace = 0;
		desc.TextureCubeArray.MipLevels = 1;
		desc.TextureCubeArray.MostDetailedMip = 0;
		desc.TextureCubeArray.NumCubes = shadowCounts;
		D3D::GetDevice()->CreateShaderResourceView(dsvTexture, &desc, &depthSRV);
	}

	viewport = new Viewport(width, height);
}

ShadowCube::~ShadowCube()
{
	SafeDelete(cShadowCubeDesc);
	SafeDelete(perframe);
	SafeRelease(srv);
	SafeRelease(rtv);
	SafeRelease(rtvTexture);
	SafeRelease(depthSRV);
	for (UINT i = 0; i < shadowCounts; i++)
	{
		SafeRelease(dsv[i]);
	}
	SafeRelease(dsvTexture);
}

void ShadowCube::Update()
{	
	
	perframe->Update();
}

void ShadowCube::PreRender(UINT lightIndex)
{
	PointLight& light = Lighting::Get()->GetPointLight(lightIndex);
	desc.position = light.Position;
	desc.farPlane = light.Range;

	Vector3 s = Vector3(desc.farPlane, desc.farPlane, desc.farPlane);
	cShadowCubeDesc->Render();
	sCShadowCubeDesc->SetConstantBuffer(cShadowCubeDesc->Buffer());
	CalcProjection(desc.position,desc.farPlane);
	D3D::Get()->SetRenderTarget(rtv, dsv[lightIndex]);
	D3D::Get()->Clear(Color(0, 0, 0, 1), rtv, dsv[lightIndex]);
}

void ShadowCube::Render()
{
	shader->AsSRV("ShadowCube")->SetResource(depthSRV);
}

void ShadowCube::CalcProjection(const Vector3& position, const float range)
{
	position;
	Vector3 scale = Vector3( range,range,range );

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
			D3DXMatrixLookAtLH(&cubeDesc.Views[i], &position, &lookAt[i].LookAt, &lookAt[i].Up);
		}

		// 정투영에 가깝게 되게하기 위해서 (왜곡을 방지하기 위해서이다)
		perspective = new Perspective(1, 1, 0.1f, range, Math::PI * 0.5f);
		perspective->GetMatrix(&cubeDesc.Projection);

		cDynamicCubeDesc->Render();
		sCDynamicCubeDesc->SetConstantBuffer(cDynamicCubeDesc->Buffer());

		viewport->RSSetViewport();
	}
}
