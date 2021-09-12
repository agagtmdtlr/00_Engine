#include "Framework.h"
#include "MeshType.h"

template<typename T>
MeshPBR<T>::MeshPBR()
{
}

template<typename T>
MeshPBR<T>::~MeshPBR()
{
	SafeDelete(perFrame);

	SafeDelete(vertexBuffer);
	SafeDelete(indexBuffer);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
}

template<typename T>
void MeshPBR<T>::SetShader(Shader * shader)
{
	this->shader = shader;

	SafeDelete(perFrame);
	perFrame = new PerFrame(shader);
}

template<typename T>
void MeshPBR<T>::Update()
{
	perFrame->Update();
}

template<typename T>
void MeshPBR<T>::Render(UINT drawCount)
{
	if (vertexBuffer == NULL || indexBuffer == NULL)
	{
		Create();

		vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(MeshVertex));
		indexBuffer = new IndexBuffer(indices, indexCount);

		SafeDeleteArray(vertices);
		SafeDeleteArray(indices);
	}

	perFrame->Render();
	vertexBuffer->Render();
	indexBuffer->Render();

	D3D::GetDC()->IASetPrimitiveTopology(topology);
	// 인스턴싱을 이용하여 드로우 한다.
	shader->DrawIndexedInstanced(0, pass, indexCount, drawCount);
}

StaticMesh::StaticMesh()
{
}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::Create()
{
}

SkinMesh::SkinMesh()
{
	boneCBuffer = new ConstantBuffer(&boneDesc, sizeof(BoneDesc));
	
}

SkinMesh::~SkinMesh()
{
	SafeDelete(material);
	SafeDelete(boneCBuffer);
}

void SkinMesh::Create()
{
}

void SkinMesh::Update()
{
	__super::Update();
	boneDesc.Index = boneIndex;
}

void SkinMesh::Render(UINT drawCount)
{
	boneCBuffer->Render();
	sBoneCBuffer->SetConstantBuffer(boneCBuffer->Buffer());

	if (material != NULL)
	{
		material->Render();
	}

	__super::Render(drawCount);
}

void SkinMesh::SetShader(Shader * shader)
{
	__super::SetShader(shader);

	sBoneCBuffer = shader->AsConstantBuffer("CB_Bone");

	if (material != NULL) material->SetShader(shader);
}

void SkinMesh::Binding(ModelPBR * model)
{
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(Model::ModelVertex));
	indexBuffer = new IndexBuffer(indices, indexCount);

	MaterialPBR* modelMaterial = model->MaterialByName(materialName);
	material = new MaterialPBR();
	material->CopyFrom(modelMaterial);
}
