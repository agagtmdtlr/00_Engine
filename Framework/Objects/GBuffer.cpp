#include "Framework.h"
#include "GBuffer.h"

GBuffer::GBuffer(Shader * shader, UINT width, UINT height)
	: shader(shader)
{
	this->width = width < 1 ? (UINT)D3D::Width() : width;
	this->height = height < 1 ? (UINT)D3D::Height() : height;

	// 
	diffuseRTV = new RenderTarget(this->width, this->height, DXGI_FORMAT_R8G8B8A8_UNORM);
	specularRTV = new RenderTarget(this->width, this->height, DXGI_FORMAT_R8G8B8A8_UNORM);
	emissiveRTV = new RenderTarget(this->width, this->height, DXGI_FORMAT_R8G8B8A8_UNORM);
	normalRTV = new RenderTarget(this->width, this->height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	tangentRTV = new RenderTarget(this->width, this->height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	depthStencil = new DepthStencil(this->width, this->height, true);// stencil도 사용한다.
	viewport = new Viewport((float)this->width, (float)this->height);

	for (UINT i = 0; i < 5; i++)
	{
		render2D[i] = new Render2D();
		render2D[i]->GetTransform()->Position(100 + (float)i * 200, 100, 0);
		render2D[i]->GetTransform()->Scale(200, 200, 1);
	}

	render2D[0]->SRV(diffuseRTV->SRV());
	render2D[1]->SRV(specularRTV->SRV());
	render2D[2]->SRV(emissiveRTV->SRV());
	render2D[3]->SRV(normalRTV->SRV());
	render2D[4]->SRV(tangentRTV->SRV());

	sSrvs = shader->AsSRV("GBufferMaps");

	pointLightBuffer = new ConstantBuffer(&pointLightDesc, sizeof(PointLightDesc));
	sPointLightBuffer = shader->AsConstantBuffer("CB_Deffered_PointLight");

	spotLightBuffer = new ConstantBuffer(&spotLightDesc, sizeof(SpotLightDesc));
	sSpotLightBuffer = shader->AsConstantBuffer("CB_Deffered_SpotLight");

	CreateRasterizerState();
	CreateDepthStencilView();
	CreateDepthStencilState();

	sRSS = shader->AsRasterizer("Deffered_Rasterizer_State");
	sDSS = shader->AsDepthStencil("Deffered_DepthStencil_State");

}

GBuffer::~GBuffer()
{
	SafeDelete(diffuseRTV);
	SafeDelete(specularRTV);
	SafeDelete(emissiveRTV);
	SafeDelete(normalRTV);
	SafeDelete(tangentRTV);
	SafeDelete(depthStencil);
	SafeDelete(viewport);

	SafeDelete(pointLightBuffer);
	SafeDelete(spotLightBuffer);

	SafeRelease(depthStencilReadOnly);

	SafeRelease(debugRSS);
	SafeRelease(lightRSS);

	SafeRelease(packDSS);
	SafeRelease(directionalLightDSS);
	SafeRelease(lightDSS);



	for (UINT i = 0; i < 5; i++)
		SafeDelete(render2D[i]);
}

void GBuffer::PreRender()
{
	//Pack 원본데이터를 저장하는 단계 : 순서 중요하다.
	RenderTarget* rtvs[5] =
	{
		diffuseRTV,
		specularRTV,
		emissiveRTV,
		normalRTV,
		tangentRTV
	};

	RenderTarget::PreRender(rtvs, 5, depthStencil);
	viewport->RSSetViewport();

	sDSS->SetDepthStencilState(0, packDSS);

}

void GBuffer::Render()
{
	// 스텐실을 사용해서 조명을 그리겠다.
	D3D::Get()->SetRenderTarget(NULL, depthStencilReadOnly);
	D3D::GetDC()->ClearDepthStencilView(depthStencilReadOnly, D3D11_CLEAR_DEPTH, 1, 0);
	
	// unpack GBuffer
	ID3D11ShaderResourceView* srvs[6] =
	{
		depthStencil->SRV(),
		diffuseRTV->SRV(),
		specularRTV->SRV(),
		emissiveRTV->SRV(),
		normalRTV->SRV(),
		tangentRTV->SRV(),
	};
	sSrvs->SetResourceArray(srvs, 0, 6);

	D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);

	DirectionalLight();

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
	PointLights();
	SpotLights();
}

void GBuffer::PostRender()
{
	for (int i = 0; i < 5; i++)
	{
		render2D[i]->Update();
		render2D[i]->Render();
	}
}

void GBuffer::DirectionalLight()
{
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	sDSS->SetDepthStencilState(0, directionalLightDSS);

	shader->Draw(0, 6, 4);
}

void GBuffer::CalcPointLights(UINT count)
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
	}
}

void GBuffer::PointLights()
{
	ImGui::InputFloat("PointLight Factor", &pointLightDesc.TessFactor, 1.0f);
	sPointLightBuffer->SetConstantBuffer(pointLightBuffer->Buffer());

	
	UINT count = Lighting::Get()->PointLights(pointLightDesc.PointLight);
	CalcPointLights(count);
	pointLightBuffer->Render();

	//Debug
	if(bDrawDebug == true)
	{
		// 배열의 인덱스 , 레스터라이즈 스테이트
		sRSS->SetRasterizerState(0, debugRSS);
		//D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST
		shader->Draw(0, 7, count * 2);
	}
	
	// Draw Light
	{
		sRSS->SetRasterizerState(0, lightRSS);
		sDSS->SetDepthStencilState(0, lightDSS);
	}
	//shader->Draw(0, 8, count * 2);

}

void GBuffer::CalcSpotLights(UINT count)
{
	for (UINT i = 0; i < count; i++)
	{
		float angle = Math::ToRadian(spotLightDesc.SpotLight[i].Angle);

		spotLightDesc.Angle[i].x = cosf(angle);
		spotLightDesc.Angle[i].y = sinf(angle);


		Matrix S, R, T;
		float s = spotLightDesc.SpotLight[i].Range;
		Vector3 t = spotLightDesc.SpotLight[i].Position;

		D3DXMatrixScaling(&S, s, s, s);
		D3DXMatrixTranslation(&T, t.x, t.y, t.z);

		Vector3 direction = spotLightDesc.SpotLight[i].Direction;
		// 짐벌랑 방지를 위한 보정값
		bool bUp = (direction.y > 1 - 1e-6f || direction.y < -1 + 1e-6f);
		Vector3 up = bUp ? Vector3(0, 0, direction.y) : Vector3(0, 1, 0);
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
	}
}

void GBuffer::SpotLights()
{
	ImGui::InputFloat("SpotLight Factor", &spotLightDesc.TessFactor, 1.0f);
	sSpotLightBuffer->SetConstantBuffer(spotLightBuffer->Buffer());


	UINT count = Lighting::Get()->SpotLights(spotLightDesc.SpotLight);
	CalcSpotLights(count);
	spotLightBuffer->Render();

	//Debug
	if (bDrawDebug == true)
	{
		// 배열의 인덱스 , 레스터라이즈 스테이트
		sRSS->SetRasterizerState(0, debugRSS);
		//D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST
		sDSS->SetDepthStencilState(0, testDSS);
		shader->Draw(0, 11, count);
	}

	// Draw Light
	{
		sRSS->SetRasterizerState(0, lightRSS);
		sDSS->SetDepthStencilState(0, lightDSS);
	}
	shader->Draw(0, 10, count);
}

void GBuffer::CreateRasterizerState()
{
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));

	desc.FillMode = D3D11_FILL_WIREFRAME;
	desc.CullMode = D3D11_CULL_NONE; // 앞뒤 다 그릴려고
	Check(D3D::GetDevice()->CreateRasterizerState(&desc, &debugRSS));

	// 앞에 부분만 그린다.
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_FRONT;
	Check(D3D::GetDevice()->CreateRasterizerState(&desc, &lightRSS));
}

void GBuffer::CreateDepthStencilView()
{
	// deffered 때 사용할 DSV [읽기 전용으로 쓸거다]
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;
	desc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;

	Check(D3D::GetDevice()->CreateDepthStencilView(depthStencil->Resource(), &desc, &depthStencilReadOnly));
}

void GBuffer::CreateDepthStencilState()
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

	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp3 =
	{ // 유지하라 D3D11_COMPARISON_EQUAL 갚은 깊이만
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
	};
	desc.FrontFace = stencilMarkOp3;
	desc.BackFace = stencilMarkOp3;
	Check(D3D::GetDevice()->CreateDepthStencilState(&desc, &testDSS));


}

