#include "Framework.h"
#include "SSRManager.h"

SSRManager::SSRManager(Shader* shader)
	:shader(shader)
{
	Initialize();
}

SSRManager::~SSRManager()
{
	Destroy();
}

void SSRManager::Initialize()
{
	ssrPSConstant = new ConstantBuffer(&ssrPSDesc, sizeof(SSRPSDesc));
	

	ResizeScreen();
	CreateBlendState();
	CreateDepthStencilState();

	render2D = new Render2D();
	render2D->SRV(reflectRT->SRV());
	render2D->GetTransform()->Position(D3D::Width() / 2, D3D::Height() / 2, 0);
	render2D->GetTransform()->Scale(D3D::Width(), D3D::Height() , 0);

}

void SSRManager::Destroy()
{
	SafeDelete(reflectRT);
	SafeDelete(reflectDSV);
	SafeDelete(reflectVP);

	SafeRelease(bss);
	SafeRelease(dss);

	SafeDelete(ssrPSConstant);
}

void SSRManager::ResizeScreen()
{
	SafeDelete(reflectRT);
	SafeDelete(reflectDSV);
	SafeDelete(reflectVP);

	width = (UINT)D3D::Width();
	height = (UINT)D3D::Height();

	reflectRT = new RenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	reflectVP = new Viewport((float)width, (float)height);
}

void SSRManager::Update()
{
	if (ImGui::CollapsingHeader("SSR Manage"))
	{
		ImGui::Separator();
		ImGui::Checkbox("SSR On/Off", &showSSR);
		ImGui::SliderFloat("SSR SampleRange", &ssrPSDesc.maxDistance, 0.0f, 150.0f);
		ImGui::SliderFloat("SSR Thickness", &ssrPSDesc.zThickness, 0.0f, 10.0f);
		//ImGui::SliderFloat("SSR maxSteps", &ssrPSDesc.maxSteps, 2.0f, 10.0f);
		//ImGui::SliderFloat("SSR stride", &ssrPSDesc.stride, 1.0f, 10.0f);
		//ImGui::SliderFloat("SSR strideZCutoff", &ssrPSDesc.strideZCutoff, 1.0f, 10.0f);
		//ImGui::SliderFloat("SSR fadeStart", &ssrPSDesc.fadeStart, 0.0f, 1024.0f);
		//ImGui::SliderFloat("SSR fadeEnd", &ssrPSDesc.fadeEnd, 0.0f, 1024.0f);
	}



	render2D->Update();
}

void SSRManager::PostRender()
{
	render2D->Render();

}

void SSRManager::PreRenderReflection()
{
	if (showSSR == false) return;
	ReflectionSetting();
	
	shader->AsDepthStencil("SSR_DSS")->SetDepthStencilState(0,dss);
	shader->AsBlend("SSR_BS")->SetBlendState(0, bss);

	shader->AsSRV("SSR_HDRTex")->SetResource(hdrSRV);
	shader->AsSRV("SSR_DepthTex")->SetResource(depthSRV);
}

void SSRManager::FullScreenReflection()
{
	if (showSSR == false) return;
	ReflectionSetting();

	shader->AsDepthStencil("SSR_DSS")->SetDepthStencilState(0, fullscreenDSS);
	shader->AsBlend("SSR_BS")->SetBlendState(0, bss);

	shader->AsSRV("SSR_HDRTex")->SetResource(hdrSRV);
	shader->AsSRV("SSR_DepthTex")->SetResource(depthSRV);
}

void SSRManager::DoReflectionBlend()
{
	if (showSSR == false) return;

	shader->AsSRV("SSR_HDRTex")->SetResource(reflectRT->SRV());
	D3D::GetDC()->OMSetRenderTargets(1, &hdrRTV, depthReadOnlyDSV);

	D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	shader->Draw(0, 27, 4);
	D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


}

void SSRManager::CreateBlendState()
{
	D3D11_BLEND_DESC descBlend;
	descBlend.AlphaToCoverageEnable = FALSE;
	descBlend.IndependentBlendEnable = FALSE;
	const D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		TRUE,
		D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
		D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
		D3D11_COLOR_WRITE_ENABLE_ALL,
	};
	descBlend.RenderTarget[0] = defaultRenderTargetBlendDesc;

	//D3D11_BLEND_DESC desc;
	//ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));
	//desc.RenderTarget->BlendEnable = true;
	//desc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
	//desc.RenderTarget->DestBlend = D3D11_BLEND_ONE;
	//desc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
	//desc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	//desc.RenderTarget->DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	//desc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
	//desc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	Check(D3D::GetDevice()->CreateBlendState(&descBlend, &bss));
}

void SSRManager::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	desc.StencilEnable = FALSE;
	
	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &dss));

	desc.DepthEnable = FALSE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	desc.StencilEnable = FALSE;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	D3D11_DEPTH_STENCILOP_DESC stencilDesc;
	stencilDesc.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.BackFace = stencilDesc;
	desc.FrontFace = stencilDesc;

	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &fullscreenDSS));


}

void SSRManager::ReflectionSetting()
{
	// output
	D3D::GetDC()->ClearRenderTargetView(reflectRT->RTV(), Color(0, 0, 0, 0));
	ID3D11RenderTargetView* rt = reflectRT->RTV();
	D3D::GetDC()->OMSetRenderTargets(1, &rt, depthReadOnlyDSV);

	// input
	// Project data setting
	{
		Matrix projm;
		Context::Get()->GetPerspective()->GetMatrix(&projm);

		ssrPSDesc.ProjMatrix = projm;
		ssrPSDesc.ProjParams.x = 1.0f / projm.m[0][0];
		ssrPSDesc.ProjParams.y = 1.0f / projm.m[1][1];
		Projection* proj = Context::Get()->GetPerspective();
		float fQ = proj->GetFarClip() / (proj->GetFarClip() - proj->GetNearClip());
		ssrPSDesc.ProjParams.z = -proj->GetNearClip() * fQ;
		ssrPSDesc.ProjParams.w = -fQ;

		//ssrPSDesc.ProjParams.z = projm.m[3][2];
		//ssrPSDesc.ProjParams.w = -projm.m[2][2];
	}
	ssrPSDesc.numMips = Context::Get()->GetPerspective()->GetNearClip();
	ssrPSDesc.TexSize = { D3D::Width(), D3D::Height() };

	ssrPSConstant->Render();
	shader->AsConstantBuffer("SSReflectionPSConstants")->SetConstantBuffer(ssrPSConstant->Buffer());

}
