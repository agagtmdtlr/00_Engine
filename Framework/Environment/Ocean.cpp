#include "Framework.h"
#include "Ocean.h"

Ocean::Ocean(InitializeDesc & desc)
	:Renderer(desc.shader), initDesc(desc)
{
	//Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	oceanDesc.WaveCount = 8;

	for (UINT i = 0; i < 8; i++)
	{
		float degree = 13.0f * (i);
		float c = cosf(Math::ToRadian(degree));
		float s = sinf(Math::ToRadian(degree));
		float len = 13.0f + i;
		waveDesc[i] = { 1 / 11.0f, len,Vector2(c,s) };
	}


	float sum = 0;
	for (UINT i = 0; i < oceanDesc.WaveCount; i++)
	{
		float k = (2 * Math::PI) / waveDesc[i].WaveLength;
		float a = waveDesc[i].Steepness / k;
		sum += a;
	}

	oceanDesc.AmplitudeMax = sum /2 ;
	


	waveDescCBuffer = new ConstantBuffer(&waveDesc, sizeof(WaveDesc) * WAVE_MAX);
	sWaveDescCBuffer = shader->AsConstantBuffer("CB_Wave");
	oceanDescCBuffer = new ConstantBuffer(&oceanDesc, sizeof(OceanDesc));
	sOceanDescCBuffer = shader->AsConstantBuffer("CB_Ocean");
	

	vertexPerPatchX = (initDesc.Width / initDesc.Patch) + 1;
	vertexPerPatchZ = (initDesc.Height / initDesc.Patch) + 1;

	vertexCount = vertexPerPatchX * vertexPerPatchZ;
	faceCount = (vertexPerPatchX - 1) * (vertexPerPatchZ - 1);
	indexCount = faceCount * 6;

	CreateVertexData();
	CreateIndexData();

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexOcean));
	indexBuffer = new IndexBuffer(indices, indexCount);

	normalMap = new Texture(L"SeaNormal.jpg");
	sNormalMap = shader->AsSRV("NormalMap");
	depthMap = new Texture(L"Terrain/Thumbnails/Terrain_Alpha (7).png");
	sDepthMap = shader->AsSRV("DepthMap");
	displacementMap = new Texture(L"water_displacement.jpg");
	sDisplacementMap = shader->AsSRV("DisplacementMap");
	diffuseMap = new Texture(L"Sea2.jpg");
	sDiffuseMap = shader->AsSRV("DiffuseMap");
}

Ocean::~Ocean()
{
	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeDelete(waveDescCBuffer);
	SafeDelete(oceanDescCBuffer);
}

void Ocean::Update()
{
	Super::Update();

	oceanDesc.Time += Time::Delta();

	static UINT pass = 0;
	if (ImGui::Button("Mode Change"))
		pass = (pass + 1) % 2;
	Pass(pass);
}

void Ocean::Render()
{
	Super::Render();

	// set srv data
	sNormalMap->SetResource(normalMap->SRV());
	sDepthMap->SetResource(depthMap->SRV());
	sDisplacementMap->SetResource(displacementMap->SRV());
	sDiffuseMap->SetResource(diffuseMap->SRV());

	//set constantbuffer data
	oceanDescCBuffer->Render();
	sOceanDescCBuffer->SetConstantBuffer(oceanDescCBuffer->Buffer());
	waveDescCBuffer->Render();
	sWaveDescCBuffer->SetConstantBuffer(waveDescCBuffer->Buffer());

	shader->DrawIndexed(0, Pass(), indexCount);

}

void Ocean::CreateVertexData()
{
	vertices = new VertexOcean[vertexCount];

	float halfWidth = (float)initDesc.Width * 0.5f;
	float halfHeight = (float)initDesc.Height * 0.5f;

	float patchWidth = (float)initDesc.Width / (float)(vertexPerPatchX - 1);
	float patchHeight = (float)initDesc.Height / (float)(vertexPerPatchZ - 1);



	// ÇÑÄ­ÀÇ Å©±â
	float u = 1.0f / (float)(vertexPerPatchX - 1);
	float v = 1.0f / (float)(vertexPerPatchZ - 1);

	for (UINT z = 0; z < vertexPerPatchZ; z++)
	{
		float z1 = halfHeight - (float)z * patchHeight;

		for (UINT x = 0; x < vertexPerPatchX; x++)
		{
			float x1 = -halfWidth + (float)x * patchWidth;
			UINT patchId = vertexPerPatchX * z + x;
			vertices[patchId].Position = Vector3(x1, 0, z1);

			//vertices[patchId].Uv.x = (float)x * u * (float)initDesc.Width * 0.2 ;
			//vertices[patchId].Uv.y = (float)z * v * (float)initDesc.Height * 0.2;
			vertices[patchId].Uv.x = (float)x * u ;
			vertices[patchId].Uv.y = (float)z * v ;

		}
	}

	
}

void Ocean::CreateIndexData()
{
	UINT index = 0;
	this->indices = new UINT[indexCount];
	for (UINT z = 0; z < vertexPerPatchZ - 1; z++)
	{
		for (UINT x = 0; x < vertexPerPatchX - 1; x++)
		{

			//this->indices[index++] = vertexPerPatchX * z + x;
			//this->indices[index++] = vertexPerPatchX * z + x + 1;
			//this->indices[index++] = vertexPerPatchX * (z + 1) + x;
			//this->indices[index++] = vertexPerPatchX * (z + 1) + x + 1;
			this->indices[index + 0] = vertexPerPatchX * z + x;
			this->indices[index + 1] = vertexPerPatchX * (z + 1) + x;
			this->indices[index + 2] = vertexPerPatchX * z + x + 1;
			this->indices[index + 3] = vertexPerPatchX * z + x + 1;
			this->indices[index + 4] = vertexPerPatchX * (z + 1) + x;
			this->indices[index + 5] = vertexPerPatchX * (z + 1) + x + 1;

			index += 6;
		}
	}
}
