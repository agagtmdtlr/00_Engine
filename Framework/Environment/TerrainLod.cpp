#include "Framework.h"
#include "TerrainLod.h"

TerrainLod::TerrainLod(InitializeDesc & desc)
	: Renderer(desc.shader), initDesc(desc)
{
	Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	sBaseMap = shader->AsSRV("BaseMap");
	sLayerMap = shader->AsSRV("LayerMap");
	sAlphaMap = shader->AsSRV("AlphaMap");
	sHeightMap = shader->AsSRV("HeightMap");
	sNormalMap = shader->AsSRV("NormalMap");
	sPositionMap = shader->AsSRV("PositionMap");

	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_TerrainLod");

	heightMap = new Texture(initDesc.heightMap);
	sHeightMap->SetResource(heightMap->SRV());
	heightMap->ReadPixel(DXGI_FORMAT_R8G8B8A8_UINT, &heightMapPixel);

	// heightmap의 크기에 맞춰서 맵을 만든다.
	width = this->heightMap->GetWidth() - 1; // 가로의 너비
	height = this->heightMap->GetHeight() - 1; // 정점의 너비

	// CellsPerPatch : 16 // 소수점 나머지를 고려해서 +1을 해준다.
	vertexPerPatchX = (width / initDesc.CellsPerPatch) + 1; // 가로줄의 패치 개수
	vertexPerPatchZ = (height / initDesc.CellsPerPatch) + 1; // 세로줄의 패치개수

	vertexCount = vertexPerPatchX * vertexPerPatchZ; // 한 패치의 정점 개수
	// 칸의 개수는 정점의 개수 -1 이니깐
	faceCount = (vertexPerPatchX - 1) * (vertexPerPatchZ - 1); // 면의 개수
	indexCount = faceCount * 4; // 

	CalcBoundY();
	CreateVertexData();
	CreateIndexData();

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTerrain));
	indexBuffer = new IndexBuffer(indices, indexCount);

	// 가로 세로 한 픽셀의 크기
	bufferDesc.TexelCellSpaceU = 1.0f / (float)heightMap->GetWidth() - 1;
	bufferDesc.TexelCellSpaceV = 1.0f / (float)heightMap->GetHeight() - 1;
	bufferDesc.HeightRatio = initDesc.HeightRatio / initDesc.CellSpacing;
	// TerrainLod의 절두체
	camera = new Fixity();
	perspective = new Perspective(D3D::Width(), D3D::Height(), 0.1f, 10000.0f, Math::PI * 0.35f);

	frustum = new Frustum(NULL, perspective);
	//frustum = new Frustum(camera, perspective);
	
	CreateTerrainTexture();


	
	
	dsv = new DepthStencil();

	brushBuffer = new ConstantBuffer(&brushDesc, sizeof(BrushDesc));
	sBrushBuffer = shader->AsConstantBuffer("CB_BrushLod");

	{
		computeShader = new Shader(L"00_TerrainCompute.fxo");

		textureBuffer = new TextureBuffer(heightMap->GetTexture());
		//textureBuffer->CopyToInput(heightMap->GetTexture());

		computeInput = computeShader->AsSRV("Input");
		computeOutput = computeShader->AsUAV("Output");
	}
	
}

TerrainLod::~TerrainLod()
{
	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeDelete(buffer);
	SafeDelete(frustum);
	SafeDelete(camera);
	SafeDelete(perspective);

	SafeDelete(heightMap);

	SafeDelete(baseMap);
	SafeDelete(layerMap);
	SafeDelete(alphaMap);

	SafeDelete(brushBuffer);
	SafeDelete(textureBuffer);


	for (int i = 0; i < 2; i++)
	{
		SafeDelete(render2D[i]);
		SafeRelease(terrainTexture[i]);
	}
}

void TerrainLod::Update()
{
	Super::Update();

	Vector3 mousepos = Mouse::Get()->GetPosition();
	brushDesc.Uv.x = mousepos.x / D3D::Width();
	brushDesc.Uv.y = mousepos.y / D3D::Height();	

	ImGui::Separator();

	ImGui::SliderFloat("MinDistance", &bufferDesc.MinDistance, 1, 100);
	ImGui::SliderFloat("MaxDistance", &bufferDesc.MaxDistance, 1, 10000);

	perspective->Set(D3D::Width(), D3D::Height(), bufferDesc.MinDistance, bufferDesc.MaxDistance, Math::PI * 0.35f);
	
	camera->Update();

	
	frustum->Update();
	frustum->Planes(bufferDesc.WorldFrustumPlanes);

	for(int i = 0;i < 2;i++)
		render2D[i]->Update();
}

void TerrainLod::PreRender()
{
	Super::Render();


	for (UINT i = 0; i < 2; i++)
	{
		D3D::GetDC()->ClearRenderTargetView(terrainRTV[i], Color(0, 0, 0, 0));
	}
	D3D::GetDC()->ClearDepthStencilView(dsv->DSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	D3D::GetDC()->OMSetRenderTargets(2, &terrainRTV[0], dsv->DSV());

	sHeightMap->SetResource(textureBuffer->OutputSRV());


	if (baseMap != NULL)
		sBaseMap->SetResource(baseMap->SRV());

	if (layerMap != NULL)
		sLayerMap->SetResource(layerMap->SRV());

	if (alphaMap != NULL)
		sAlphaMap->SetResource(alphaMap->SRV());

	if (normalMap != NULL)
		sNormalMap->SetResource(normalMap->SRV());


	buffer->Render();
	sBuffer->SetConstantBuffer(buffer->Buffer());
	shader->DrawIndexed(0, Pass(), indexCount);

	Vector3 p = Mouse::Get()->GetPosition();
	ImGui::SliderFloat3("mp", p,-1000,1000);

	
	UINT width = textureBuffer->Width();
	UINT height = textureBuffer->Height();
	UINT arraySize = textureBuffer->ArraySize();

	// 한 스레드 그루벵서는 스레드를 1024만 사용할수 있다. 쪼개서 해야된다.
	float x = ((float)width / 32) < 1.0f ? 1.0f : ((float)width / 32);
	float y = ((float)height / 32) < 1.0f ? 1.0f : ((float)height / 32);

	computeInput->SetResource(textureBuffer->SRV());
	computeOutput->SetUnorderedAccessView(textureBuffer->UAV());

	// 반올림시켜준다. 연산을 수행한다.
	// thread group을 보낸다.
	computeShader->Dispatch(0, 0, (UINT)ceilf(x), (UINT)ceilf(y), arraySize);


	textureBuffer->CopyToInput(textureBuffer->CopyFromOutput());

	render2D[0]->SRV(terrainSRV[0]);
	render2D[1]->SRV(terrainSRV[1]);
}

void TerrainLod::Render()
{
	Super::Render();

	
	sHeightMap->SetResource(textureBuffer->OutputSRV());


	if (baseMap != NULL)
		sBaseMap->SetResource(baseMap->SRV());

	if (layerMap != NULL)
		sLayerMap->SetResource(layerMap->SRV());

	if (alphaMap != NULL)
		sAlphaMap->SetResource(alphaMap->SRV());

	if (normalMap != NULL)
		sNormalMap->SetResource(normalMap->SRV());

	sPositionMap->SetResource(terrainSRV[0]);

	brushBuffer->Render();
	sBrushBuffer->SetConstantBuffer(brushBuffer->Buffer());

	buffer->Render();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	shader->DrawIndexed(0, Pass(), indexCount);

	ImGui::Image(baseMap->SRV(), ImVec2(64, 64));
	ImGui::Image(layerMap->SRV(), ImVec2(64, 64));
	ImGui::Image(textureBuffer->OutputSRV(), ImVec2(64, 64));
}

void TerrainLod::PostRender()
{
	for (int i = 0; i < 2; i++)
		render2D[i]->Render();
}

void TerrainLod::BaseMap(wstring file)
{
	SafeDelete(baseMap);

	baseMap = new Texture(file);
}

void TerrainLod::LayerMap(wstring layer, wstring alpha)
{
	SafeDelete(alphaMap);
	SafeDelete(layerMap);

	alphaMap = new Texture(alpha);
	layerMap = new Texture(layer);
}

void TerrainLod::NormalMap(wstring file)
{
	SafeDelete(normalMap);

	normalMap = new Texture(file);
}

bool TerrainLod::InBounds(UINT x, UINT z)
{
	return x >= 0 && x < width && z >= 0 && z < height;
}

void TerrainLod::CalcPatchBounds(UINT x, UINT z)
{
	UINT x0 = x * initDesc.CellsPerPatch;
	UINT x1 = (x + 1) * initDesc.CellsPerPatch;

	UINT z0 = z * initDesc.CellsPerPatch;
	UINT z1 = (z + 1) * initDesc.CellsPerPatch;


	float minY = FLT_MAX;
	float maxY = FLT_MIN;

	for (UINT z = z0; z <= z1; z++)
	{
		for (UINT x = x0; x <= x1; x++)
		{
			float y = 0.0f;
			UINT pixel = width * (height - 1 - z) + x;

			// 구역의 넓이 부피의 y도 계산한다.
			if (InBounds(x, z))
				y = heightMapPixel[pixel].b * 255 / initDesc.HeightRatio;

			minY = min(minY, y);
			maxY = max(maxY, y);
		}
	}

	UINT patchID = (vertexPerPatchX - 1) * z + x;
	bounds[patchID] = Vector2(minY, maxY);
}

void TerrainLod::CalcBoundY()
{
	bounds.assign(faceCount, Vector2());

	for (UINT z = 0; z < vertexPerPatchZ - 1; z++)
	{
		for (UINT x = 0; x < vertexPerPatchX - 1; x++)
		{
			CalcPatchBounds(x, z);
		}
	}
}

void TerrainLod::CreateVertexData()
{
	vertices = new VertexTerrain[vertexCount];

	float halfWidth = (float)width * 0.5f * initDesc.CellSpacing;
	float halfHeight = (float)height * 0.5f * initDesc.CellSpacing;

	float patchWidth = (float)width / (float)(vertexPerPatchX - 1);
	float patchHeight = (float)height / (float)(vertexPerPatchZ - 1);

	// use to check vertical picking
	mapSize.x = -halfWidth;
	mapSize.y = +halfWidth;
	mapSize.z = -halfHeight;
	mapSize.w = +halfHeight;


	// 한칸의 크기
	float u = 1.0f / (float)(vertexPerPatchX - 1);
	float v = 1.0f / (float)(vertexPerPatchZ - 1);

	for (UINT z = 0; z < vertexPerPatchZ; z++)
	{
		// 세로축은 뒤집힌다.
		// 범위 +halfHeight ~ -halfHeight // textuer 좌상단부터 시작이니깐
		// +에서 -방향으로 가는게 맞다.
		float z1 = halfHeight - (float)z * patchHeight * initDesc.CellSpacing;

		for (UINT x = 0; x < vertexPerPatchX; x++)
		{
			// 범위 -halfWidth ~ + halfWidth
			float x1 = -halfWidth + (float)x * patchWidth * initDesc.CellSpacing;

			UINT patchId = vertexPerPatchX * z + x;
			
			vertices[patchId].Position = Vector3(x1, 0, z1);
			// 0 ~ 1 의 값이 나온다.
			vertices[patchId].Uv = Vector2((float)x * u, (float)z * v);
			vertices[patchId].Uv1 = Vector2((float)x , (float)z );

		}
	}

	for (UINT z = 0; z < vertexPerPatchZ - 1; z++)
	{
		for (UINT x = 0; x < vertexPerPatchX - 1; x++)
		{
			UINT patchID = (vertexPerPatchX - 1) * z + x;
			UINT vertID = vertexPerPatchX * z + x;

			vertices[vertID].BoundsY = bounds[patchID];
		}
	}

}

void TerrainLod::CreateIndexData()
{
	UINT index = 0;
	this->indices = new UINT[indexCount];
	for (UINT z = 0; z < vertexPerPatchZ - 1; z++)
	{
		for (UINT x = 0; x < vertexPerPatchX - 1; x++)
		{
			this->indices[index++] = vertexPerPatchX * z + x;
			this->indices[index++] = vertexPerPatchX * z + x + 1;
			this->indices[index++] = vertexPerPatchX * (z + 1) + x;
			this->indices[index++] = vertexPerPatchX * (z + 1) + x + 1;
		}
	}
}

void TerrainLod::CreateTerrainTexture()
{
	D3D11_TEXTURE2D_DESC desc;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	// terrain position map texture
	{
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = (UINT)D3D::Get()->Width();
		desc.Height = (UINT)D3D::Get()->Height();
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &terrainTexture[0]));
	}
	// terrain position map rtv
	{
		ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		Check(D3D::GetDevice()->CreateRenderTargetView(terrainTexture[0], &rtvDesc, &terrainRTV[0]));
	}
	// terrain position map srv
	{
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		Check(D3D::GetDevice()->CreateShaderResourceView(terrainTexture[0], &srvDesc, &terrainSRV[0]));
	}
	// terrain uv map texture
	{
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &terrainTexture[1]));
	}
	// terrain uv map rtv
	{
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Check(D3D::GetDevice()->CreateRenderTargetView(terrainTexture[1], &rtvDesc, &terrainRTV[1]));
	}
	{
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Check(D3D::GetDevice()->CreateShaderResourceView(terrainTexture[1], &srvDesc, &terrainSRV[1]));
	}

	for (int i = 0; i < 2; i++)
	{
		render2D[i] = new Render2D();
		render2D[i]->GetTransform()->Scale(200.0f, 200.0f, 1.0f);
		render2D[i]->GetTransform()->Position(100.0f + ((float)i * 200.0f), 100.0f, 0.0f);
	}

	
}
