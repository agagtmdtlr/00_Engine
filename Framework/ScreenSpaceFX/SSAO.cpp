#include "Framework.h"
#include "SSAO.h"

SSAO::SSAO()
{
	Initialize();
}

SSAO::~SSAO()
{
	SafeRelease(bufferUAV);
	SafeRelease(bufferSRV);
	SafeRelease(buffer);
	SafeRelease(SSAOUAV);
	SafeRelease(SSAOSRV);
	SafeRelease(SSAOtexture);
	for (int i = 0; i < 2; i++)
	{
		SafeRelease(textureSRV[i]);
		SafeRelease(textureUAV[i]);
		SafeRelease(texture[i])
	}

	SafeDelete(descBuffer);
	SafeDelete(blurBuffer);

	SafeDelete(ssaoRT);
	SafeDelete(ssaoDSV);
	SafeDelete(ssaoVP);
	SafeDelete(blurRT);

	SafeDelete(shader);
}

void SSAO::ResizeScreen()
{
	SafeDelete(ssaoRT);
	SafeDelete(ssaoDSV);
	SafeDelete(ssaoVP);
	SafeDelete(blurRT);

	ssaoRT = new RenderTarget((UINT)D3D::Width(), (UINT)D3D::Height(),
		DXGI_FORMAT_R16_FLOAT);
	ssaoDSV = new DepthStencil((UINT)D3D::Width(), (UINT)D3D::Height());
	ssaoVP = new Viewport(D3D::Width(), D3D::Height());

	blurRT = new RenderTarget((UINT)D3D::Width(), (UINT)D3D::Height(),
		DXGI_FORMAT_R16_FLOAT);
}

void SSAO::Update()
{
	if (ImGui::CollapsingHeader("SSAO Manage"))
	{
		ImGui::Separator();
		ImGui::Checkbox("SSAO On/Off : ", &doSSAO);
		ImGui::SliderFloat("SSAO offsetRadius", &offsetRadius, 0.1f, 5.0f);
		ImGui::SliderFloat("SSAO radius", &radius, 1.0f, 20.0f);
		ImGui::Separator();
	}
	
}

ID3D11ShaderResourceView* SSAO::ComputeSSAO()
{
	DownScale();
	FinalPass();
	//Blur(SSAOSRV,SSAOUAV);
	return SSAOSRV;
}

ID3D11ShaderResourceView * SSAO::ComputeSSAORenderTarget()
{

	if (doSSAO == true)
	{
		ConstantUpdate();
		SSAORenderTarget();
		BlurRenderTarget();
	}
	else
	{
		D3D::GetDC()->ClearRenderTargetView(ssaoRT->RTV(), Color(1, 1, 1, 1));
		D3D::GetDC()->ClearRenderTargetView(blurRT->RTV(), Color(1, 1, 1, 1));

	}

	return blurRT->SRV();
}

void SSAO::DownScale()
{
	ConstantUpdate();

	// Output
	shader->AsUAV("MiniDepthRW")->SetUnorderedAccessView(bufferUAV);

	// Input
	shader->AsSRV("DepthTex")->SetResource(packDepth);
	shader->AsSRV("NormalsTex")->SetResource(packNormal);

	// Execute the downscales first pass with enough groups to cover the entire full res HDR buffer
	UINT dispatchCounts = (UINT)ceil((float)(Width * Height) / 1024.0f);
	shader->Dispatch(0, 0, dispatchCounts, 1, 1);

}

void SSAO::FinalPass()
{
	//Output
	shader->AsUAV("AO")->SetUnorderedAccessView(SSAOUAV);
	//Input
	shader->AsSRV("MiniDepth")->SetResource(bufferSRV);
	//Constant
	sDescBuffer->SetConstantBuffer(descBuffer->Buffer());
	
	// Execute the downscales first pass with enough groups to cover the entire full res HDR buffer
	shader->Dispatch(0,1,(UINT)ceil((float)(Width * Height) / 1024.0f), 1, 1);
}

void SSAO::Blur(ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output)
{
	/////////////////////////////////////////////////////////////////////
	// Second pass - Horizontal gaussian filter
	// Output
	shader->AsUAV("Output")->SetUnorderedAccessView(textureUAV[1]);
	// Input 
	shader->AsSRV("Input")->SetResource(input);
	// Execute the horizontal filter
	shader->Dispatch(0, 2,
		(UINT)ceil((Width) / (128.0f - 12.0f)),  // thread x
		(UINT)ceil(Height), // thread y
		1);

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// First pass - vertical gaussian filter
	// Output
	shader->AsUAV("Output")->SetUnorderedAccessView(output);
	// Input
	shader->AsSRV("Input")->SetResource(textureSRV[1]);
	// Execute the vertical filter
	shader->Dispatch(0, 3,
		(UINT)ceil(Width),
		(UINT)ceil((Height) / (128.0f - 12.0f)) + 1,
		1);
}

void SSAO::ConstantUpdate()
{
	//Constant
	downScaleDesc.Width = (UINT)D3D::Width() / 2;
	downScaleDesc.Height = (UINT)D3D::Height() / 2;
	downScaleDesc.ResRcp.x = (float)1.0f / downScaleDesc.Width;
	downScaleDesc.ResRcp.x = (float)1.0f / downScaleDesc.Height;


	// Project data setting
	{
		Matrix projm;
		Context::Get()->GetPerspective()->GetMatrix(&projm);
		downScaleDesc.ProjParams.x = 1.0f / projm.m[0][0];
		downScaleDesc.ProjParams.y = 1.0f / projm.m[1][1];
		Projection* proj = Context::Get()->GetPerspective();
		float fQ = proj->GetFarClip() / (proj->GetFarClip() - proj->GetNearClip());
		downScaleDesc.ProjParams.z = -proj->GetNearClip() * fQ;
		downScaleDesc.ProjParams.w = -fQ;
	}

	Context::Get()->GetCamera()->GetMatrix(&downScaleDesc.ViewMatrix);
	Context::Get()->GetPerspective()->GetMatrix(&downScaleDesc.ProjMatrix);
	downScaleDesc.OffsetRadius = offsetRadius;
	downScaleDesc.Radius = radius;
	descBuffer->Render();
	sDescBuffer->SetConstantBuffer(descBuffer->Buffer());
}

void SSAO::SSAORenderTarget()
{
	//Output
	ssaoRT->PreRender(ssaoDSV);
	D3D::GetDC()->ClearRenderTargetView(ssaoRT->RTV(), Color(1, 1, 1, 1));
	ssaoVP->RSSetViewport();
	// Input
	shader->AsSRV("DepthTex")->SetResource(packDepth);
	shader->AsSRV("NormalsTex")->SetResource(packNormal);
	shader->AsSRV("NoiseTexture")->SetResource(noiseTextureSRV);

	D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	shader->Draw(0, 4, 4); // ssao
}

void SSAO::BlurRenderTarget()
{
	// Input;
	shader->AsSRV("SSAOInput")->SetResource(ssaoRT->SRV());
	ssaoVP->RSSetViewport();
	blurDesc.texelX = 1.0f / D3D::Width();
	blurDesc.texelY = 1.0f / D3D::Height();
	blurBuffer->Render();
	sBlurBuffer->SetConstantBuffer(blurBuffer->Buffer());
	// Output
	D3D::GetDC()->ClearRenderTargetView(blurRT->RTV(), Color(1, 1, 1, 1));
	blurRT->PreRender(ssaoDSV);

	D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
	shader->Draw(0, 5, 4); // blur
}

void SSAO::Initialize()
{
	shader = new Shader(L"00_SSAOCompute.fx");
	CreateResource();
}

void SSAO::CreateResource()
{
	Width = (UINT)D3D::Width() / 2;
	Height = (UINT)D3D::Height() / 2;
	UINT totalPixels = Width * Height;
	///////////////////////////////////////////////////////////////
	// Allocate down scale depth buffer
	///////////////////////////////////////////////////////////////
	D3D11_BUFFER_DESC bfDesc;
	ZeroMemory(&bfDesc, sizeof(D3D11_BUFFER_DESC));
	bfDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bfDesc.StructureByteStride = 4 * sizeof(float);
	bfDesc.ByteWidth = bfDesc.StructureByteStride * totalPixels;
	bfDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	Check(D3D::GetDevice()->CreateBuffer(&bfDesc, NULL, &buffer));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uDesc;
	ZeroMemory(&uDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uDesc.Format = DXGI_FORMAT_UNKNOWN;
	uDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uDesc.Buffer.NumElements = totalPixels;
	uDesc.Buffer.FirstElement = 0;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(buffer, &uDesc, &bufferUAV));

	D3D11_SHADER_RESOURCE_VIEW_DESC sDesc;
	ZeroMemory(&sDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	sDesc.Format = DXGI_FORMAT_UNKNOWN;
	sDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	sDesc.Buffer.NumElements = totalPixels;
	sDesc.Buffer.FirstElement = 0;
	Check(D3D::GetDevice()->CreateShaderResourceView(buffer, &sDesc, &bufferSRV));
	///////////////////////////////////////////////////////////////
	// Allocate SSAO
	///////////////////////////////////////////////////////////////
	D3D11_TEXTURE2D_DESC tDesc;
	ZeroMemory(&tDesc, sizeof(D3D11_TEXTURE2D_DESC));	
	tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	tDesc.Width = Width;
	tDesc.Height = Height;
	tDesc.MipLevels = 1;
	tDesc.ArraySize = 1;
	tDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	tDesc.SampleDesc.Count = 1;
	Check(D3D::GetDevice()->CreateTexture2D(&tDesc, NULL, &texture[0]));
	Check(D3D::GetDevice()->CreateTexture2D(&tDesc, NULL, &texture[1]));
	
	Check(D3D::GetDevice()->CreateTexture2D(&tDesc, NULL, &SSAOtexture));

	// random sample noise texture
	tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tDesc.Width = 4;
	tDesc.Height = 4;
	tDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	
	Vector4 noiseDatas[16];
	for (int i = 0; i < 16; i++)
	{
		Vector2 noise = Math::RandomVec2(0.0f, 1.0f) * 2.0f - Vector2(1.0f, 1.0f);
		D3DXVec2Normalize(&noise, &noise);
		noiseDatas[i] = Vector4(noise.x, noise.y,0,1);
	}
	D3D11_SUBRESOURCE_DATA sub = { 0 };
	sub.pSysMem = noiseDatas;
	//sub.SysMemPitch = sizeof(Vector4) * 4;
	//sub.SysMemSlicePitch = sizeof(Vector4) * 4 * 4;

	sub.SysMemPitch =  4;
	sub.SysMemSlicePitch = 4 * 4;
	
	Check(D3D::GetDevice()->CreateTexture2D(&tDesc, &sub, &noiseTexture));

	//////////////////////////////////////////////////////////////////////
	// Allocate UAV
	////////////////////////////////////////////////////////////////////////
	uDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	Check(D3D::GetDevice()->CreateUnorderedAccessView(texture[0], &uDesc, &textureUAV[0]));
	Check(D3D::GetDevice()->CreateUnorderedAccessView(texture[1], &uDesc, &textureUAV[1]));
	
	Check(D3D::GetDevice()->CreateUnorderedAccessView(SSAOtexture, &uDesc, &SSAOUAV));

	///////////////////////////////////////////////////////////////////////////
	// Allocate SRV
	///////////////////////////////////////////////////////////////////////////
	sDesc.Format = DXGI_FORMAT_R32_FLOAT;
	sDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sDesc.Texture2D.MipLevels = 1;
	Check(D3D::GetDevice()->CreateShaderResourceView(texture[0], &sDesc, (ID3D11ShaderResourceView**)&textureSRV[0]));
	Check(D3D::GetDevice()->CreateShaderResourceView(texture[1], &sDesc, (ID3D11ShaderResourceView**)&textureSRV[1]));

	Check(D3D::GetDevice()->CreateShaderResourceView(SSAOtexture, &sDesc, &SSAOSRV));

	// noise texute 4x4
	sDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	Check(D3D::GetDevice()->CreateShaderResourceView(noiseTexture, &sDesc, &noiseTextureSRV));

	/////////////////////////////////////////////////////////////////////////////
	// Constant Samples valuse initialize
	///////////////////////////////////////////////////////////////////
	{
		for (int i = 0; i < 64; i++)
		{
			Vector3 vec = Math::RandomVec3(0.0f, 1.0f);
			vec.x = vec.x * 2.0f - 1.0f;
			vec.y = vec.x * 2.0f - 1.0f;
			D3DXVec3Normalize(&vec, &vec);
			float scale = float(i) / 64.0f;
			scale = Math::Lerp(0.1f, 1.0f, scale * scale);//distribution weight value
			vec *= scale;
			downScaleDesc.Samples[i] = Vector4(vec, 1);
		}
	}


	descBuffer = new ConstantBuffer(&downScaleDesc, sizeof(DownScaleDesc));
	sDescBuffer = shader->AsConstantBuffer("DownScaleConstants");

	blurBuffer = new ConstantBuffer(&blurDesc, sizeof(BlurDesc));
	sBlurBuffer = shader->AsConstantBuffer("CB_Blur");

	ssaoRT = new RenderTarget((UINT)D3D::Width(),(UINT)D3D::Height(),
		DXGI_FORMAT_R32_FLOAT);
	ssaoDSV = new DepthStencil((UINT)D3D::Width(),(UINT) D3D::Height());
	ssaoVP = new Viewport(D3D::Width(), D3D::Height());

	blurRT = new RenderTarget((UINT)D3D::Width(), (UINT)D3D::Height(),
		DXGI_FORMAT_R32_FLOAT);
}
