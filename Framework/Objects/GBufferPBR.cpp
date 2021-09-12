#include "Framework.h"
#include "GBufferPBR.h"
#include "ScreenSpaceFX/SSAO.h"

GBufferPBR::GBufferPBR(Shader * shader)
	: shader(shader)
{
	Destroy();
	Initialize();
}

GBufferPBR::~GBufferPBR()
{
	Destroy();
	SafeDelete(defferedBuffer);
	SafeDelete(pointLightBuffer);
	SafeDelete(spotLightBuffer);
	SafeDelete(spotLightBuffer2);

	SafeRelease(depthStencilReadOnly);
	
	SafeRelease(debugRSS);
	SafeRelease(lightRSS);

	SafeRelease(packDSS);
	SafeRelease(directionalLightDSS);
	SafeRelease(lightDSS);

	for (UINT i = 0; i < 6; i++)
		SafeDelete(render2D[i]);

	SafeDelete(resultRender2D);
}

void GBufferPBR::Initialize()
{
	
	// Debug RT
	for (UINT i = 0; i < 6; i++)
	{
		render2D[i] = new Render2D();
		render2D[i]->GetTransform()->Position(75 + (float)i * 150, 75, 0);
		render2D[i]->GetTransform()->Scale(150, 150, 1);
	}
	resultRender2D = new Render2D();
	resultRender2D->GetTransform()->Position(D3D::Width() / 2, D3D::Height() / 2, 0);
	resultRender2D->GetTransform()->Scale(D3D::Width(), D3D::Height(), 0);
	
	ResizeScreen();

	// SSAO Init
	ssao = new SSAO();
	sSSAOSRV = shader->AsSRV("SSAOMap");

	sGBufferMapsSRV = shader->AsSRV("GBufferMaps");

	pointLightBuffer = new ConstantBuffer(&pointLightDesc, sizeof(PointLightDesc));
	sPointLightBuffer = shader->AsConstantBuffer("CB_Deffered_PointLight");

	spotLightBuffer = new ConstantBuffer(&spotLightDesc, sizeof(SpotLightDesc));
	sSpotLightBuffer = shader->AsConstantBuffer("CB_Deffered_SpotLight");
	spotLightBuffer2 = new ConstantBuffer(&spotLightDescPS, sizeof(DescPS));
	sSpotLightBuffer2 = shader->AsConstantBuffer("CB_Deffered_SpotLight_PS");

	defferedBuffer = new ConstantBuffer(&defferedDesc, sizeof(DefferedDesc));
	sDefferedBuffer = shader->AsConstantBuffer("CB_Deffered_Desc");

	CreateBlendState();
	CreateRasterizerState();
	CreateDepthStencilState();

	sRSS = shader->AsRasterizer("Deffered_Rasterizer_State");
	sDSS = shader->AsDepthStencil("Deffered_DepthStencil_State");
	sBS = shader->AsBlend("Deffered_Blend_State");

}

void GBufferPBR::Destroy()
{
	SafeDelete(albedoRTV);
	SafeDelete(metallicRTV);
	SafeDelete(roughnessRTV);
	SafeDelete(aoRTV);
	SafeDelete(normalRTV);
	SafeDelete(tangentRTV);
	SafeDelete(dsv);
	SafeDelete(viewPort);
	SafeDelete(dsv);
	SafeDelete(result);
}

void GBufferPBR::Pass(UINT p1, UINT p2, UINT p3)
{
	directPass = p1; 
	pointPass = p2; 
	spotPass = p3; 
}

void GBufferPBR::DebugPass(UINT p1, UINT p2)
{
	debugPass[0] = p1;
	debugPass[1] = p2;
}

void GBufferPBR::SetReadOnlyDSV()
{
	D3D::Get()->SetRenderTarget(NULL, depthStencilReadOnly);
}

void GBufferPBR::SaveTexture(wstring file)
{
	result->SaveTexture(file);
}

void GBufferPBR::ResizeScreen()
{
	Destroy(); // delete prev resource

	this->width = (UINT)D3D::Width();
	this->height = (UINT)D3D::Height();

	if (ssao != NULL)
		ssao->ResizeScreen();

	// GBuffer Packed RT Init
	albedoRTV = new RenderTarget(width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	metallicRTV = new RenderTarget(width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	roughnessRTV = new RenderTarget(width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	aoRTV = new RenderTarget(width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	normalRTV = new RenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	tangentRTV = new RenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	dsv = new DepthStencil(width, height, true);
	viewPort = new Viewport((float)width, (float)height);
	// 2Pass RT
	result = new RenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);

	render2D[0]->SRV(albedoRTV->SRV());
	render2D[1]->SRV(metallicRTV->SRV());
	render2D[2]->SRV(roughnessRTV->SRV());
	render2D[3]->SRV(aoRTV->SRV());
	render2D[4]->SRV(normalRTV->SRV());
	render2D[5]->SRV(tangentRTV->SRV());

	SafeRelease(depthStencilReadOnly);
	CreateDepthStencilView();

}

void GBufferPBR::Update()
{
	if (ImGui::CollapsingHeader("Deffered Manage"))
	{
		ImGui::Separator();
		ImGui::Checkbox("Render Debug RT", &bRender2D);
		ImGui::Checkbox("Debug RT CloseUP", &bCloseUp);
		ImGui::InputInt("Selected RT Index", (int*)&selectedGBuffer2D);
		selectedGBuffer2D %= 6;

		ImGui::InputFloat("PointLight Factor", &pointLightDesc.TessFactor, 1.0f);
		ImGui::InputFloat("SpotLight Factor", &spotLightDesc.TessFactor, 1.0f);

	}

	for (UINT i = 0; i < 6; i++)
	{
		render2D[i]->GetTransform()->Position(75 + (float)i * 150, 75, 0);
		render2D[i]->GetTransform()->Scale(150, 150, 1);
	}
	if (bCloseUp == true)
	{
		render2D[selectedGBuffer2D]->GetTransform()->Position(D3D::Width() / 2.0f, D3D::Height() / 2.0f, 0);
		render2D[selectedGBuffer2D]->GetTransform()->Scale(D3D::Width() , D3D::Height(), 0);
	}

	ssao->Update();
}

void GBufferPBR::PreRender()
{
	RenderTarget* rtvs[6] =
	{
		albedoRTV,
		metallicRTV,
		roughnessRTV,
		aoRTV,
		normalRTV,
		tangentRTV
	};

	RenderTarget::PreRender(rtvs, 6, dsv);

	D3D::GetDC()->ClearRenderTargetView(aoRTV->RTV(), Color(1, 1, 1, 1));
	viewPort->RSSetViewport();
	sDSS->SetDepthStencilState(0, packDSS); //GPack에서 dsv 상태 서술

	D3D::GetDC()->ClearRenderTargetView(result->RTV(), Color(0, 0, 0, 1));
	D3D::GetDC()->ClearDepthStencilView(dsv->DSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}

void GBufferPBR::ReadySkyRender()
{
	D3D::Get()->SetRenderTarget(result->RTV(), depthStencilReadOnly);

}

void GBufferPBR::ReadyRender()
{
	D3D::Get()->SetRenderTarget(result->RTV(), depthStencilReadOnly);
	//D3D::GetDC()->ClearDepthStencilView(depthStencilReadOnly, D3D11_CLEAR_DEPTH, 1, 0);

}

void GBufferPBR::Render()
{
	ID3D11ShaderResourceView* srvs[7] =
	{
		dsv->SRV(),
		albedoRTV->SRV(),
		metallicRTV->SRV(),
		roughnessRTV->SRV(),
		aoRTV->SRV(),
		normalRTV->SRV(),
		tangentRTV->SRV(),
	};

	sGBufferMapsSRV->SetResourceArray(srvs, 0, 7);
	ssao->SetPackDepth(dsv->SRV());
	ssao->SetPackNormal(normalRTV->SRV());
	
	ssaoSRV = ssao->ComputeSSAORenderTarget();
	sSSAOSRV->SetResource(ssaoSRV);	
	render2D[3]->SRV2(ssaoSRV);
	// Project data setting
	{
		Matrix projm;
		Context::Get()->GetPerspective()->GetMatrix(&projm);
		defferedDesc.ProjectValues.x = 1.0f / projm.m[0][0];
		defferedDesc.ProjectValues.y = 1.0f / projm.m[1][1];
		Projection* proj = Context::Get()->GetPerspective();
		float fQ = proj->GetFarClip() / (proj->GetFarClip() - proj->GetNearClip());
		defferedDesc.ProjectValues.z = -proj->GetNearClip() * fQ;
		defferedDesc.ProjectValues.w = -fQ;
	}
	defferedBuffer->Render();
	sDefferedBuffer->SetConstantBuffer(defferedBuffer->Buffer());

	ReadyRender();
	sBS->SetBlendState(0,lightBS);

	D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	DirectionalLight();
	AmbientLights();

	if (bDrawLight)
	{
		// light mesh 를 tessellation으로 만들기 위해서 topology를 변경한다.
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		PointLights();
		SpotLights();
	}
}

void GBufferPBR::PostRender()
{
	if (bRender2D == true)
	{
		for (int i = 0; i < 6; i++)
		{
			render2D[i]->Update();
			render2D[i]->Render();
		}
	}
	
}

void GBufferPBR::DirectionalLight()
{
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	sDSS->SetDepthStencilState(0, directionalLightDSS);
	shader->Draw(0, directPass, 4);
}

void GBufferPBR::CalcPointLights(UINT count)
{
	// 조명 위치
	// 조명 범위
	// 카메라 뷰 / 카메라 투영 행렬
	// calc point light projection matrix
	for (UINT i = 0; i < count; i++)
	{
		// 구는 회전이 의미가 없으므로 뺌
		Matrix S, T;
		float s = pointLightDesc.PointLight[i].Range;
		Vector3 t = pointLightDesc.PointLight[i].Position;

		D3DXMatrixScaling(&S, s, s, s);
		D3DXMatrixTranslation(&T, t.x, t.y, t.z);

		pointLightDesc.Projection[i] = S * T * Context::Get()->View() * Context::Get()->Projection();
		//Aspect : 비율...
		// 1대1이 되면 시야각이 90도가 되서 거의 정투영이 된다.
		float aspect = 1;
		D3DXMatrixPerspectiveFovLH(&pointLightDesc.ShadowCubeProjection[i], Math::PI / 2.0f, aspect, 0.1f, pointLightDesc.PointLight[i].Range);
	}
	
}

void GBufferPBR::PointLights()
{
	sPointLightBuffer->SetConstantBuffer(pointLightBuffer->Buffer());

	UINT count = Lighting::Get()->PointLights(pointLightDesc.PointLight);
	CalcPointLights(count);
	pointLightBuffer->Render();

	//Debug
	if (bDrawDebug == true)
	{
		// 배열의 인덱스 , 레스터라이즈 스테이트
		sRSS->SetRasterizerState(0, debugRSS);
		//D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST
		shader->Draw(0, debugPass[0], count * 2);
	}

	// Draw Light
	{
		sRSS->SetRasterizerState(0, lightRSS);
		sDSS->SetDepthStencilState(0, lightDSS);
	}
	shader->Draw(0, pointPass, count * 2);
}

void GBufferPBR::CalcSpotLights(UINT count)
{
	for (UINT i = 0; i < count; i++)
	{
		float outerAngle = Math::ToRadian(spotLightDesc.SpotLight[i].OuterAngle);
		float innerAngle = Math::ToRadian(spotLightDesc.SpotLight[i].InnerAngle);
		float outCosAngle = cosf(outerAngle);
		float innerCosAngle = cosf(innerAngle);

		spotLightDesc.Angle[i].x = cosf(outerAngle);
		spotLightDesc.Angle[i].y = sinf(outerAngle);
		spotLightDesc.Angle[i].z = innerCosAngle - outCosAngle; // attRange;

		Matrix S, R, T;
		float s = spotLightDesc.SpotLight[i].Range;
		Vector3 t = spotLightDesc.SpotLight[i].Position;

		D3DXMatrixScaling(&S, s, s, s);
		D3DXMatrixTranslation(&T, t.x, t.y, t.z);

		Vector3 direction = spotLightDesc.SpotLight[i].Direction;
		// 짐벌랑 방지를 위한 보정값
		bool bUp = (direction.y > 1 - 1e-6f || direction.y < -1 + 1e-6f);
		Vector3 up = bUp ? Vector3(0, 0, -direction.y) : Vector3(0, 1, 0);
		Vector3 right;
		D3DXVec3Cross(&right, &up, &direction);
		D3DXVec3Normalize(&right, &right);

		D3DXVec3Cross(&up, &direction, &right);
		D3DXVec3Normalize(&up, &up);

		D3DXMatrixIdentity(&R);
		// LH rotation의 matrix
		// row 이전 좌표계에서의 이후 좌표
		// col 이후 좌표계에서의 이전 좌표
		for (int k = 0; k < 3; k++)
		{
			R.m[0][k] = right[k];
			R.m[1][k] = up[k];
			R.m[2][k] = direction[k];
		}
		spotLightDesc.Projection[i] = S * R * T * Context::Get()->View() * Context::Get()->Projection();
		{// cal
			SpotLightPBR& l = spotLightDesc.SpotLight[i];
			Matrix cameraview;
			Vector3 look = l.Position + l.Direction;
			bool upcheck = (l.Direction.y > 1 - 1e-6f || l.Direction.y < -1 + 1e-6f);
			Vector3 up = upcheck ? Vector3(0, 0, l.Direction.y) : Vector3(0, 1, 0);
			D3DXMatrixLookAtLH(&cameraview, &l.Position, &look, &up);
			Matrix Projection;
			D3DXMatrixPerspectiveFovLH(&Projection, 2.0f * Math::ToRadian(l.OuterAngle), 1, 0.1f, l.Range);

			spotLightDescPS.ShadowMap[i] = cameraview * Projection;
		}

	}
}

void GBufferPBR::SpotLights()
{
	sSpotLightBuffer->SetConstantBuffer(spotLightBuffer->Buffer());

	UINT count = Lighting::Get()->SpotLightPBRs(spotLightDesc.SpotLight);
	CalcSpotLights(count);

	spotLightBuffer->Render();
	spotLightBuffer2->Render();
	sSpotLightBuffer2->SetConstantBuffer(spotLightBuffer2->Buffer());
	//Debug
	if (bDrawDebug == true)
	{
		// 배열의 인덱스 , 레스터라이즈 스테이트
		sRSS->SetRasterizerState(0, debugRSS);
		//D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST
		sDSS->SetDepthStencilState(0, testDSS);
		shader->Draw(0, debugPass[1], count);
	}

	// Draw Light
	{
		sRSS->SetRasterizerState(0, lightRSS);
		sDSS->SetDepthStencilState(0, lightDSS);
		shader->Draw(0, spotPass, count);
	}
}

void GBufferPBR::AmbientLights()
{
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	sDSS->SetDepthStencilState(0, directionalLightDSS);
	shader->Draw(0, 12, 4);
}

void GBufferPBR::CreateBlendState()
{
	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Check(D3D::GetDevice()->CreateBlendState(&desc,&lightBS))
}

void GBufferPBR::CreateRasterizerState()
{
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));

	desc.FillMode = D3D11_FILL_WIREFRAME;
	desc.CullMode = D3D11_CULL_NONE; // 앞뒤 다 그릴려고
	Check(D3D::GetDevice()->CreateRasterizerState(&desc, &debugRSS));

	// 뒷면만 그린다.
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_FRONT;
	Check(D3D::GetDevice()->CreateRasterizerState(&desc, &lightRSS));
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
	Check(D3D::GetDevice()->CreateRasterizerState(&descRast, &spotRSS));

}

void GBufferPBR::CreateDepthStencilView()
{
	// deffered 때 사용할 DSV [읽기 전용으로 쓸거다]
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;
	desc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;

	Check(D3D::GetDevice()->CreateDepthStencilView(dsv->Resource(), &desc, &depthStencilReadOnly));
}

void GBufferPBR::CreateDepthStencilState()// PackGBuffer & Deffered Directional Light
{
	//RTV 5 DSV 1 [ 초기화 시키고 다시 그리기 ] [ 하늘같은거 지울려고 ]
	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = TRUE; // 깊이 테스트 활성화
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 깊이 스텐실 쓰기를 작동합니다.
	desc.DepthFunc = D3D11_COMPARISON_LESS; // 깊이 중에 작은ㄷ걸 그리겠다. 가까운거
	desc.StencilEnable = TRUE; // 가려주는 역할
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK; // 기본 stencil 세팅
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	// 기존에 있던걸 가려준다는 옵션
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp =
	{
		D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_ALWAYS
	};
	desc.FrontFace = stencilMarkOp;
	desc.BackFace = stencilMarkOp;

	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &packDSS));

	desc.DepthFunc = D3D11_COMPARISON_LESS; // 깊이 중에 작은ㄷ걸 그리겠다. 가까운거

	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 깊이 스텐실 쓰기를 끕니다 읽기전용
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp2 =
	{ // 유지하라 D3D11_COMPARISON_EQUAL 갚은 깊이만
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_EQUAL
	};
	desc.FrontFace = stencilMarkOp2;
	desc.BackFace = stencilMarkOp2;

	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &directionalLightDSS));

	// 앞면을 걸링하고 크거나 같은 깊이 비교를 수행한다.
	// 카메라가 볼륨안에 포함돼 있을때 포인트 라이트 볼륨 면 중 일부나 전부를 컬링하지 않게 방지한다.
	//DepthEnable True
	//DepthWriteMask D3D11_DEPTH_WRITE_MASK_ZERO
	//DepthFunc D3D11_COMPARISON_GREATER_EQUAL
	desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &lightDSS));
	
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 깊이 스텐실 쓰기를 작동합니다.
	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &spotLightDSS));


	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 깊이 스텐실 쓰기를 작동합니다.
	desc.DepthEnable = false;
	desc.StencilEnable = true;
	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp3 =
	{ // 유지하라 D3D11_COMPARISON_EQUAL 갚은 깊이만
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
	};
	desc.FrontFace = stencilMarkOp3;
	desc.BackFace = stencilMarkOp3;
	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &testDSS));
}
