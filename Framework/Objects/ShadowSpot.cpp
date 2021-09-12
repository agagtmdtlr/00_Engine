#include "Framework.h"
#include "ShadowSpot.h"

ShadowSpot::ShadowSpot(Shader * shader, float width, float height, DXGI_FORMAT rtvFormat, bool useStencil, UINT shadowCounts)
	:shader(shader), width(width), height(height), shadowCounts(shadowCounts)
	, rtvFormat(rtvFormat), useStencil(useStencil)
{
	perframe = new PerFrame(shader);
	
	cShadowDesc = new ConstantBuffer(&desc, sizeof(ShadowLightDesc));
	sCShadowDesc = shader->AsConstantBuffer("CB_ShadowSpotGen");
	
	ResizeScreen();
	
	
	CreateRasterizerState();
	sRSS = shader->AsRasterizer("ShadowSpotRasterizerState");

}

ShadowSpot::~ShadowSpot()
{ // delete reverse order [ stack frame ]
	SafeRelease(depthSRV);
	for (UINT i = 0; i < shadowCounts; i++)
		SafeRelease(dsv[i]);
	SafeRelease(dsvTexture);
	SafeDelete(rtv);
	SafeDelete(cShadowDesc);
	SafeDelete(perframe);
	SafeDelete(viewport);

	SafeRelease(rss);
}

void ShadowSpot::ResizeScreen()
{
	ResizeDestroy();

	rtv = new RenderTarget((UINT)width, (UINT)height, rtvFormat);
	viewport = new Viewport(width, height);

	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Format = useStencil ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_R32_TYPELESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		desc.ArraySize = shadowCounts;
		desc.Width = (UINT)width;
		desc.Height = (UINT)height;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &dsvTexture));
	}
	dsv = new ID3D11DepthStencilView*[shadowCounts];
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = useStencil ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = 1;
		for (UINT i = 0; i < shadowCounts; i++)
		{
			desc.Texture2DArray.FirstArraySlice = i;
			Check(D3D::GetDevice()->CreateDepthStencilView(dsvTexture, &desc, &dsv[i]));
		}
	}
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = useStencil ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = shadowCounts;
		desc.Texture2DArray.MipLevels = 1;
		desc.Texture2DArray.FirstArraySlice = 0;
		Check(D3D::GetDevice()->CreateShaderResourceView(dsvTexture, &desc, &depthSRV));
	}
}

void ShadowSpot::Update()
{
	perframe->Update();
}

void ShadowSpot::PreRender(UINT lightIndex)
{
	perframe->Render();
	SpotLightPBR& light = Lighting::Get()->GetSpotLightPBR(lightIndex);
	desc.ShadowGenMatrix = CalcProjection(light);
	desc.LightPosition = light.Position;
	desc.Range = light.Range;
	cShadowDesc->Render();
	sCShadowDesc->SetConstantBuffer(cShadowDesc->Buffer());

	sRSS->SetRasterizerState(0, rss);


	D3D::Get()->SetRenderTarget(rtv->RTV(), dsv[lightIndex]);
	D3D::Get()->Clear(Color(0, 0, 0, 1), rtv->RTV(), dsv[lightIndex]);

	viewport->RSSetViewport();
}

void ShadowSpot::Render()
{
	shader->AsSRV("ShadowSpot")->SetResource(depthSRV);
}

void ShadowSpot::ResizeDestroy()
{
	SafeDelete(rtv);
	SafeDelete(viewport);
	SafeRelease(depthSRV);

	if (dsv != NULL)
	{
		for (UINT i = 0; i < shadowCounts; i++)
			SafeRelease(dsv[i]);
	}
	
	SafeRelease(dsvTexture);
}

Matrix ShadowSpot::CalcProjection(const SpotLightPBR light)
{
	Matrix view;
	Vector3 look = light.Position + light.Direction;
	bool bUp = (light.Direction.y > 0.9f || light.Direction.y < -0.9f);
	Vector3 up = bUp ? Vector3(0, 0, light.Direction.y) : Vector3(0, 1, 0);
	D3DXMatrixLookAtLH(&view, &light.Position, &look, &up);
	Matrix proj;
	D3DXMatrixPerspectiveFovLH(&proj, 2.0f *  Math::ToRadian(light.OuterAngle), 1, 0.1f, light.Range);

	return view * proj;
}

void ShadowSpot::CreateRasterizerState()
{
	D3D11_RASTERIZER_DESC descRast = {
		D3D11_FILL_SOLID,
		D3D11_CULL_FRONT,
		FALSE,
		D3D11_DEFAULT_DEPTH_BIAS,
		D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE,
		FALSE,
		FALSE
	};
	descRast.DepthBias = 65;
	descRast.SlopeScaledDepthBias = 15.0f;

	Check(D3D::GetDevice()->CreateRasterizerState(&descRast, &rss));
}
