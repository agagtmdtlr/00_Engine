#include "Framework.h"
#include "CubeSky.h"

CubeSky::CubeSky(wstring file, Shader * shader)
	: shader(shader)
{
	if (shader == NULL)
	{
		this->shader = new Shader(L"29_CubeSky.fxo");
		bCreateShader = true;
	}


	sphereRender = new MeshRender(this->shader, new MeshSphere(0.5f));
	// far 거리가 500인 이유
	sphereRender->AddTransform()->Scale(500, 500, 500);

	wstring temp = L"../../_Textures/" + file;
	Check(D3DX11CreateShaderResourceViewFromFile
	(
		D3D::GetDevice(), temp.c_str(), NULL, NULL, &srv, NULL
	));

	sSrv = this->shader->AsSRV("SkyCubeMap");
	bCreateSRV = true;
	
	{
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));
		desc.MinLOD = 0;
		desc.MaxLOD = 4;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	}

	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		desc.DepthEnable = FALSE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = TRUE; 
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK; // 기본 stencil 세팅
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp =
		{
			D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_NOT_EQUAL
		};
		desc.FrontFace = stencilMarkOp;
		desc.BackFace = stencilMarkOp;
		Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &dss));
	}
}

CubeSky::CubeSky(ID3D11ShaderResourceView* srv)
	:shader(NULL),srv(srv)
{
	if (shader == NULL)
	{
		this->shader = new Shader(L"29_CubeSky.fxo");
		bCreateShader = true;
	}
	sphereRender = new MeshRender(this->shader, new MeshSphere(0.5f));
	// far 거리가 500인 이유
	sphereRender->AddTransform()->Scale(500, 500, 500);
	//sphereRender->AddTransform();

	sSrv = this->shader->AsSRV("SkyCubeMap");

	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		desc.DepthEnable = FALSE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = TRUE;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK; // 기본 stencil 세팅
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp =
		{
			D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_NOT_EQUAL
		};
		desc.FrontFace = stencilMarkOp;
		desc.BackFace = stencilMarkOp;
		Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &dss));
	}
}

CubeSky::~CubeSky()
{
	if (bCreateShader == true)
		SafeDelete(shader);
	if (bCreateSRV == true)
		SafeRelease(srv);
	SafeDelete(sphereRender);
	SafeDelete(clod);
}

void CubeSky::Update()
{
	if (clod == NULL)
	{
		clod = new ConstantBuffer(&desc, sizeof(Desc));
		sLod = shader->AsConstantBuffer("MipMapLevel");
	}

	Vector3 position;
	Context::Get()->GetCamera()->Position(&position);
	sphereRender->GetTransform(0)->Position(position);
	sphereRender->UpdateTransforms();

	ImGui::Separator();
	ImGui::SliderFloat("CubeSky MipMapLevel : ", &desc.lod, 0.0, 4.0);
	ImGui::Separator();
}

void CubeSky::Render()
{
	clod->Render();
	sLod->SetConstantBuffer(clod->Buffer());
	
	sSrv->SetResource(srv);

	shader->AsDepthStencil("Sky_DSS")->SetDepthStencilState(0,dss);

	sphereRender->Pass(pass);
	sphereRender->Render();
}

void CubeSky::SetSRV(ID3D11ShaderResourceView * val)
{
	if (bCreateSRV == true)
	{
		SafeRelease(srv);
		bCreateSRV = false;
	}
	srv = val;
}
