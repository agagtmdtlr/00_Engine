#include "Framework.h"
#include "PostFX.h"

PostFX::PostFX(wstring shaderFile)
	:Renderer(shaderFile)
{
	Initialize();
}

PostFX::~PostFX()
{
	SafeRelease(tonemapDSS);

	SafeRelease(downScaledUAV);
	SafeRelease(downScaledSRV);
	SafeRelease(downScaledRT);
	for (UINT i = 0; i < 2; i++)
	{
		SafeRelease(tempUAV[i]);
		SafeRelease(tempSRV[i]);
		SafeRelease(tempRT[i]);
	}
	SafeRelease(DOFUAV);
	SafeRelease(DOFSRV);
	SafeRelease(DOFRT);
	SafeRelease(bloomUAV);
	SafeRelease(bloomSRV);
	SafeRelease(bloomRT);
	SafeRelease(toneMapDownScaleSRV);
	SafeRelease(toneMapDownScaleUAV);
	SafeRelease(toneMapDownScaleBuffer);
	SafeRelease(toneMapAvgLumSRV);
	SafeRelease(toneMapAvgLumUAV);
	SafeRelease(toneMapAvgLumBuffer);
	SafeRelease(prevToneMapAvgLumSRV);
	SafeRelease(prevToneMapAvgLumUAV);
	SafeRelease(prevToneMapAvgLumBuffer);

	//SafeDelete(blurDescBuffer);
	SafeDelete(finalPassDescBuffer);
	SafeDelete(downScaleDescBuffer);

	SafeDelete(Debug2D);
	
	SafeDelete(ComputeShader);
}

void PostFX::Initialize()
{
	// NDC 좌표계만큼
	Vertex vertices[6];
	vertices[0].Position = Vector3(-1.0f, -1.0f, 0.0f);
	vertices[1].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[2].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[3].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[4].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[5].Position = Vector3(+1.0f, +1.0f, 0.0f);

	vertexBuffer = new VertexBuffer(vertices, 6, sizeof(Vertex));
	sSRVHDR = shader->AsSRV("HDRMap");
	sSRVDepth = shader->AsSRV("DepthTex");

	transform->Scale(D3D::Width(), D3D::Height(), 1);
	transform->Position(D3D::Width() * 0.5f, D3D::Height() * 0.5f, 0);
	
	CreateDepthStencilState();
	sDSS = shader->AsDepthStencil("postEffect_DepthStencilState");

	CreateResource();
	// debug render texture
	Debug2D = new Render2D();
	Debug2D->SRV(bloomSRV);
	//Debug2D->SRV(tempSRV[1]);
	Debug2D->GetTransform()->Position(75, 225, 0);
	Debug2D->GetTransform()->Scale(150,150,1);
}

void PostFX::Destroy()
{
}


void PostFX::Update()
{
	Super::Update();

	static UINT tonemapType = finalPassDesc.ToneMapType;

	if (ImGui::CollapsingHeader("Post Process Manage"))
	{
		ImGui::Separator();
		ImGui::Text("Post Processing Option");
		ImGui::SliderFloat("MiddleGrey", &MiddleGrey, 0.0, 10.0f);
		ImGui::SliderFloat("white", &white, 1.0f, 1000.0f);
		ImGui::SliderFloat("BloomThreshold", &bloomThreashold, 0.01f, 4.0f);
		ImGui::SliderFloat("BloomScale", &bloomScale, 0.000f, 1.0f);
		ImGui::SliderFloat("DOFFarStart", &DOFFarStart, 0.0f, 999.0f);
		ImGui::SliderFloat("DOFFarRange", &DOFFarRange, 0.001f, 999.0f);
		ImGui::Separator();

		string str = "ToneMap Type : " + to_string(tonemapType);
		if (ImGui::Button(str.c_str()))
		{
			tonemapType++;
			tonemapType %= 5;
		}
	}	
	finalPassDesc.ToneMapType = tonemapType;

	
	Debug2D->Update();
}

void PostFX::PostProcessing()
{
	Super::Render();

	DownScale();
	Bloom();
	Blur(tempSRV[0],bloomUAV);
	Blur(downScaledSRV, DOFUAV);
	FinalPass();

	//Debug2D->Render();
	
	// swap buffer
	ID3D11Buffer* tempBuffer = toneMapAvgLumBuffer;
	ID3D11UnorderedAccessView* tempUAV = toneMapAvgLumUAV;
	ID3D11ShaderResourceView* tempSRV = toneMapAvgLumSRV;

	toneMapAvgLumBuffer = prevToneMapAvgLumBuffer;
	toneMapAvgLumUAV = prevToneMapAvgLumUAV;
	toneMapAvgLumSRV = prevToneMapAvgLumSRV;

	prevToneMapAvgLumBuffer = tempBuffer;
	prevToneMapAvgLumUAV = tempUAV;
	prevToneMapAvgLumSRV = tempSRV;
}

void PostFX::Bloom()
{
	sDownScaleDescBuffer->SetConstantBuffer(downScaleDescBuffer->Buffer());
	// Input
	ComputeShader->AsSRV("AvgLum")->SetResource(toneMapAvgLumSRV);
	ComputeShader->AsSRV("HDRDownScaleTex")->SetResource(downScaledSRV);
	// Output
	ComputeShader->AsUAV("Bloom")->SetUnorderedAccessView(tempUAV[0]);
	// Execute the downscales first pass 
	//with enough groups to cover the entire full res HDR buffer
	ComputeShader->Dispatch(0, 2, DownScaleGroups, 1, 1);
}

void PostFX::Blur(ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output)
{
	/////////////////////////////////////////////////////////////////////
	// Second pass - Horizontal gaussian filter
	// Output
	ComputeShader->AsUAV("Output")->SetUnorderedAccessView(tempUAV[1]);
	// Input 
	ComputeShader->AsSRV("Input")->SetResource(input);
	// Execute the horizontal filter
	ComputeShader->Dispatch(0, 3,
		(UINT)ceil((Width / 4.0f) / (128.0f - 12.0f)) + 1,  // thread x
		(UINT)ceil(Height / 4.0f), // thread y
		1);

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// First pass - vertical gaussian filter
	// Output
	ComputeShader->AsUAV("Output")->SetUnorderedAccessView(output);
	// Input
	ComputeShader->AsSRV("Input")->SetResource(tempSRV[1]); 
	// Execute the vertical filter
	ComputeShader->Dispatch(0, 4,
		(UINT)ceil(Width / 4.0f),
		(UINT)ceil((Height / 4.0f) / (128.0f - 12.0f)) + 1,
		1);
}

void PostFX::HDRMap(ID3D11ShaderResourceView * HDRSRV)
{
	this->HDRSRV = HDRSRV;
}

void PostFX::GBufferDepthMap(ID3D11ShaderResourceView * DepthSRV)
{
	this->DepthSRV = DepthSRV;
}

void PostFX::DownScale()
{
	// Output	
	sToneMapAvgLumUAV->SetUnorderedAccessView(toneMapDownScaleUAV); // 1D luminance
	ComputeShader->AsUAV("HDRDownScale")->SetUnorderedAccessView(downScaledUAV); // down scale hdr
	// Input
	sHDRTexSRV->SetResource(HDRSRV);
	// Constant
	downScaleDesc.Width = Width / 4;
	downScaleDesc.Height = Height / 4;
	downScaleDesc.TotalPixels = downScaleDesc.Width * downScaleDesc.Height;
	downScaleDesc.GroupSize = DownScaleGroups;
	downScaleDesc.Adaption = Adaption;
	downScaleDesc.BloomThreshold = bloomThreashold;
	downScaleDescBuffer->Render();
	sDownScaleDescBuffer->SetConstantBuffer(downScaleDescBuffer->Buffer());
	// Execute the downscales first pass with enough groups to cover the entire full res HDR buffer
	ComputeShader->Dispatch(0, 0, DownScaleGroups, 1, 1);

	////////////////////////////////////////////////////////////////////////
	// second pass
	////////////////////////////////////////////////////////////////////////
	//Output
	sToneMapAvgLumUAV->SetUnorderedAccessView(toneMapAvgLumUAV);
	//Input
	sToneMapDownSacleSRV->SetResource(toneMapDownScaleSRV);
	//Constant
	sDownScaleDescBuffer->SetConstantBuffer(downScaleDescBuffer->Buffer());
	// Excute with a single group - this group has enough threads to process all the pixels
	UINT pass = (DownScaleGroups > 64 ? 5 : 1); // select pass by resolution;
	ComputeShader->Dispatch(0, pass, 1, 1, 1);
}

void PostFX::FinalPass()
{
	// Shader Views
	sToneMapAvgLumSRV->SetResource(toneMapAvgLumSRV); // 휘도 평균값
	sSRVHDR->SetResource(HDRSRV);
	sSRVDepth->SetResource(DepthSRV);
	shader->AsSRV("BloomTex")->SetResource(bloomSRV);
	shader->AsSRV("DOFBlurTex")->SetResource(DOFSRV);
	// Constants
	finalPassDesc.fMiddleGrey = MiddleGrey;
	finalPassDesc.fLumWhite = white;
	finalPassDesc.BloomScale = bloomScale;
	// Get Projection Value for linear depth
	//Matrix Proj;
	//Context::Get()->GetPerspective()->GetMatrix(&Proj);
	{
		Projection* Proj = Context::Get()->GetPerspective();
		float fQ = Proj->GetFarClip() / (Proj->GetFarClip() - Proj->GetNearClip());
		finalPassDesc.ProjectionValues.x = -Proj->GetNearClip() * fQ;
		finalPassDesc.ProjectionValues.y = -fQ;
	}
	
	// set dof far value for blurred intervals
	finalPassDesc.DOFFarValues.x = DOFFarStart;
	finalPassDesc.DOFFarValues.y = 1 / DOFFarRange;

	finalPassDescBuffer->Render();
	sFinalPassDescBuffer->SetConstantBuffer(finalPassDescBuffer->Buffer());
	// Depth stensil state
	sDSS->SetDepthStencilState(0, tonemapDSS);
	shader->Draw(0, Pass(), 6);
}

void PostFX::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	desc.DepthEnable = true;
	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.StencilEnable = TRUE; // 가려주는 역할
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK; // 기본 stencil 세팅
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	D3D11_DEPTH_STENCILOP_DESC opdesc;
	ZeroMemory(&opdesc, sizeof(D3D11_DEPTH_STENCILOP_DESC));
	opdesc.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	opdesc.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	opdesc.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	opdesc.StencilFunc = D3D11_COMPARISON_ALWAYS;

	desc.BackFace = opdesc;
	desc.FrontFace = opdesc;	

	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &tonemapDSS));
}

void PostFX::CreateResource()
{
	ComputeShader = new Shader(L"129_PostCompute.fxo");

	Width = (UINT)D3D::Width();
	Height = (UINT)D3D::Height();
	// 720p : 57 thread group (반올림)
	DownScaleGroups = (UINT)ceil((float)(Width * Height / 16.0f) / 1024.0f);
	
	


	{
		////////////////////////////////////////////////////////////////////////'
		// Allocate the downscaled target
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = Width / 4; // ex 1280 / 4 = 320
		desc.Height = Height / 4; // ex 720 / 4 = 180
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R16G16B16A16_TYPELESS;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &downScaledRT));
		////////////////////////////////////////////////////////////////////////'
		// Allocate temporary target
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &tempRT[0]));
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &tempRT[1]));
		////////////////////////////////////////////////////////////////////////'
		// Allocate Bloom Target
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &bloomRT));
		////////////////////////////////////////////////////////////////////////'
		// DOF blurred Target
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &DOFRT));
	}
	{
		// 1 strid : 16pixels avg
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.StructureByteStride = sizeof(float);
		desc.ByteWidth = desc.StructureByteStride * DownScaleGroups;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		
		Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &toneMapDownScaleBuffer));
		// 최종 avg를 담는다.
		desc.ByteWidth = 4;
		Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &toneMapAvgLumBuffer));
		Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &prevToneMapAvgLumBuffer));
	}
	{
		////////////////////////////////////////////////////////////////////////'
		// Allocate the downscaled uav
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = Width * Height / 16;	// down scales total pixels counts	
		Check(D3D::GetDevice()->CreateUnorderedAccessView(downScaledRT, &desc, &downScaledUAV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate temporary uav
		Check(D3D::GetDevice()->CreateUnorderedAccessView(tempRT[0], &desc, &tempUAV[0]));
		Check(D3D::GetDevice()->CreateUnorderedAccessView(tempRT[1], &desc, &tempUAV[1]));
		////////////////////////////////////////////////////////////////////////'
		// Allocate Bloom uav
		Check(D3D::GetDevice()->CreateUnorderedAccessView(bloomRT, &desc, &bloomUAV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate DOF blurred uav
		Check(D3D::GetDevice()->CreateUnorderedAccessView(DOFRT, &desc, &DOFUAV));
		
		////////////////////////////////////////////////////////////////////////'
		// Allocate down scaled luminance uav
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.NumElements = DownScaleGroups;
		Check(D3D::GetDevice()->CreateUnorderedAccessView(toneMapDownScaleBuffer, &desc, &toneMapDownScaleUAV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate average luminance uav
		desc.Buffer.NumElements = 1;
		Check(D3D::GetDevice()->CreateUnorderedAccessView(toneMapAvgLumBuffer, &desc, &toneMapAvgLumUAV));
		Check(D3D::GetDevice()->CreateUnorderedAccessView(prevToneMapAvgLumBuffer, &desc, &prevToneMapAvgLumUAV));
	}
	{
		////////////////////////////////////////////////////////////////////////'
		// Allocate the downscaled srv
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
		Check(D3D::GetDevice()->CreateShaderResourceView(downScaledRT, &desc, &downScaledSRV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate temporary srv
		Check(D3D::GetDevice()->CreateShaderResourceView(tempRT[0], &desc, &tempSRV[0]));
		Check(D3D::GetDevice()->CreateShaderResourceView(tempRT[1], &desc, &tempSRV[1]));
		////////////////////////////////////////////////////////////////////////'
		// Allocate Bloom srv
		Check(D3D::GetDevice()->CreateShaderResourceView(bloomRT, &desc, &bloomSRV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate Bloom srv
		Check(D3D::GetDevice()->CreateShaderResourceView(DOFRT, &desc, &DOFSRV));
		
		////////////////////////////////////////////////////////////////////////'
		// Allocate down sclaed luminance srv
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.NumElements = DownScaleGroups;
		Check(D3D::GetDevice()->CreateShaderResourceView(toneMapDownScaleBuffer, &desc, &toneMapDownScaleSRV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate average luminance srv
		desc.Buffer.NumElements = 1;
		Check(D3D::GetDevice()->CreateShaderResourceView(toneMapAvgLumBuffer, &desc, &toneMapAvgLumSRV));
		Check(D3D::GetDevice()->CreateShaderResourceView(prevToneMapAvgLumBuffer, &desc, &prevToneMapAvgLumSRV));
	}

	// constant buffer
	downScaleDescBuffer = new ConstantBuffer(&downScaleDesc, sizeof(DownScaleDesc));
	sDownScaleDescBuffer = ComputeShader->AsConstantBuffer("DownScaleConstants");
	finalPassDescBuffer = new ConstantBuffer(&finalPassDesc, sizeof(FinalPassCB));
	sFinalPassDescBuffer = shader->AsConstantBuffer("FinalPassConstants");

	// structuredbuffer to uav and srv;
	sHDRTexSRV = ComputeShader->AsSRV("HDRTex");
	//sToneMapDownScaleUAV = ComputeShader->AsUAV("AverageLum");
	sToneMapDownSacleSRV = ComputeShader->AsSRV("AverageValues1D");
	sToneMapAvgLumUAV = ComputeShader->AsUAV("AverageLum");
	sPrevToneMapAvgLumSRV = ComputeShader->AsSRV("PrevAvgLum");
	sToneMapAvgLumSRV = shader->AsSRV("AvgLum");

}

void PostFX::SaveTexture(ID3D11Texture2D* txt, wstring file)
{
	Check(D3DX11SaveTextureToFile(D3D::GetDC(), txt, D3DX11_IFF_PNG, file.c_str()));
}

void PostFX::ResizeDestroy()
{
	SafeRelease(downScaledUAV);
	SafeRelease(downScaledSRV);
	SafeRelease(downScaledRT);
	for (UINT i = 0; i < 2; i++)
	{
		SafeRelease(tempUAV[i]);
		SafeRelease(tempSRV[i]);
		SafeRelease(tempRT[i]);
	}
	SafeRelease(DOFUAV);
	SafeRelease(DOFSRV);
	SafeRelease(DOFRT);
	SafeRelease(bloomUAV);
	SafeRelease(bloomSRV);
	SafeRelease(bloomRT);
	SafeRelease(toneMapDownScaleSRV);
	SafeRelease(toneMapDownScaleUAV);
	SafeRelease(toneMapDownScaleBuffer);
	SafeRelease(toneMapAvgLumSRV);
	SafeRelease(toneMapAvgLumUAV);
	SafeRelease(toneMapAvgLumBuffer);
	SafeRelease(prevToneMapAvgLumSRV);
	SafeRelease(prevToneMapAvgLumUAV);
	SafeRelease(prevToneMapAvgLumBuffer);
}

void PostFX::ResizeScreen()
{
	ResizeDestroy();

	transform->Scale(D3D::Width(), D3D::Height(), 1);
	transform->Position(D3D::Width() * 0.5f, D3D::Height() * 0.5f, 0);

	Width = (UINT)D3D::Width();
	Height = (UINT)D3D::Height();
	// 720p : 57 thread group (반올림)
	DownScaleGroups = (UINT)ceil((float)(Width * Height / 16.0f) / 1024.0f);

	{
		////////////////////////////////////////////////////////////////////////'
		// Allocate the downscaled target
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = Width / 4; // ex 1280 / 4 = 320
		desc.Height = Height / 4; // ex 720 / 4 = 180
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R16G16B16A16_TYPELESS;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &downScaledRT));
		////////////////////////////////////////////////////////////////////////'
		// Allocate temporary target
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &tempRT[0]));
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &tempRT[1]));
		////////////////////////////////////////////////////////////////////////'
		// Allocate Bloom Target
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &bloomRT));
		////////////////////////////////////////////////////////////////////////'
		// DOF blurred Target
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &DOFRT));
	}
	{
		// 1 strid : 16pixels avg
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.StructureByteStride = sizeof(float);
		desc.ByteWidth = desc.StructureByteStride * DownScaleGroups;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &toneMapDownScaleBuffer));
		// 최종 avg를 담는다.
		desc.ByteWidth = 4;
		Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &toneMapAvgLumBuffer));
		Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &prevToneMapAvgLumBuffer));
	}
	{
		////////////////////////////////////////////////////////////////////////'
		// Allocate the downscaled uav
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = Width * Height / 16;	// down scales total pixels counts	
		Check(D3D::GetDevice()->CreateUnorderedAccessView(downScaledRT, &desc, &downScaledUAV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate temporary uav
		Check(D3D::GetDevice()->CreateUnorderedAccessView(tempRT[0], &desc, &tempUAV[0]));
		Check(D3D::GetDevice()->CreateUnorderedAccessView(tempRT[1], &desc, &tempUAV[1]));
		////////////////////////////////////////////////////////////////////////'
		// Allocate Bloom uav
		Check(D3D::GetDevice()->CreateUnorderedAccessView(bloomRT, &desc, &bloomUAV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate DOF blurred uav
		Check(D3D::GetDevice()->CreateUnorderedAccessView(DOFRT, &desc, &DOFUAV));

		////////////////////////////////////////////////////////////////////////'
		// Allocate down scaled luminance uav
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.NumElements = DownScaleGroups;
		Check(D3D::GetDevice()->CreateUnorderedAccessView(toneMapDownScaleBuffer, &desc, &toneMapDownScaleUAV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate average luminance uav
		desc.Buffer.NumElements = 1;
		Check(D3D::GetDevice()->CreateUnorderedAccessView(toneMapAvgLumBuffer, &desc, &toneMapAvgLumUAV));
		Check(D3D::GetDevice()->CreateUnorderedAccessView(prevToneMapAvgLumBuffer, &desc, &prevToneMapAvgLumUAV));
	}
	{
		////////////////////////////////////////////////////////////////////////'
		// Allocate the downscaled srv
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
		Check(D3D::GetDevice()->CreateShaderResourceView(downScaledRT, &desc, &downScaledSRV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate temporary srv
		Check(D3D::GetDevice()->CreateShaderResourceView(tempRT[0], &desc, &tempSRV[0]));
		Check(D3D::GetDevice()->CreateShaderResourceView(tempRT[1], &desc, &tempSRV[1]));
		////////////////////////////////////////////////////////////////////////'
		// Allocate Bloom srv
		Check(D3D::GetDevice()->CreateShaderResourceView(bloomRT, &desc, &bloomSRV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate Bloom srv
		Check(D3D::GetDevice()->CreateShaderResourceView(DOFRT, &desc, &DOFSRV));

		////////////////////////////////////////////////////////////////////////'
		// Allocate down sclaed luminance srv
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.NumElements = DownScaleGroups;
		Check(D3D::GetDevice()->CreateShaderResourceView(toneMapDownScaleBuffer, &desc, &toneMapDownScaleSRV));
		////////////////////////////////////////////////////////////////////////'
		// Allocate average luminance srv
		desc.Buffer.NumElements = 1;
		Check(D3D::GetDevice()->CreateShaderResourceView(toneMapAvgLumBuffer, &desc, &toneMapAvgLumSRV));
		Check(D3D::GetDevice()->CreateShaderResourceView(prevToneMapAvgLumBuffer, &desc, &prevToneMapAvgLumSRV));
	}

}
