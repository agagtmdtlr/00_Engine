#include "Framework.h"
#include "ModelMesh.h"

ModelBone::ModelBone()
{
}

ModelBone::~ModelBone()
{

}

/////////////////////////////////////////////////////////////////////////////////

ModelMesh::ModelMesh()
{
	boneBuffer = new ConstantBuffer(&boneDesc, sizeof(BoneDesc));
}

ModelMesh::~ModelMesh()
{
	SafeDelete(perFrame);

	SafeDelete(material);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeDelete(vertexBuffer);
	SafeDelete(indexBuffer);

	SafeDelete(boneBuffer);
}

// Non PBR Model Binding
void ModelMesh::Binding(Model * model) // ���͸����� ������ ��
{
	// �ش� �޽��� ����,�ε��� ���۸� �����Ѵ�.
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(Model::ModelVertex));
	indexBuffer = new IndexBuffer(indices, indexCount);
	
	// �ش� �޽��� ���͸����̸��� ������ ���͸����� �����´�.
	Material* srcMaterial = model->MaterialByName(materialName);

	// �ش� ���͸��� ������ ���� ���͸��� �����Ѵ�.
	material = new Material();
	material->CopyFrom(srcMaterial);
}

// PBR Model Binding
void ModelMesh::Binding(ModelPBR * model)
{
	// �ش� �޽��� ����,�ε��� ���۸� �����Ѵ�.
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(Model::ModelVertex));
	indexBuffer = new IndexBuffer(indices, indexCount);

	// �ش� �޽��� ���͸����̸��� ������ ���͸����� �����´�.
	MaterialPBR* srcMaterial = model->MaterialByName(materialName);
	meterialPBR = new MaterialPBR();
	meterialPBR->CopyFrom(srcMaterial);
}

// new shader�� ���� ������ �ٲ��� �ϹǷ� ���� transform perframe�� ����� ���� �����.
void ModelMesh::SetShader(Shader * shader)
{
	this->shader = shader;

	// transform�� instance buffer���� ������ ó���Ѵ�.

	SafeDelete(perFrame);
	perFrame = new PerFrame(shader);

	sBoneBuffer = shader->AsConstantBuffer("CB_Bone");

	material->SetShader(shader);

	sTransformsSRV = shader->AsSRV("TransformsMap");
}

void ModelMesh::SetShaderPBR(Shader * shader)
{
	this->shader = shader;

	// transform�� instance buffer���� ������ ó���Ѵ�.

	SafeDelete(perFrame);
	perFrame = new PerFrame(shader);

	sBoneBuffer = shader->AsConstantBuffer("CB_Bone");

	if(meterialPBR != NULL) meterialPBR->SetShader(shader);

	sTransformsSRV = shader->AsSRV("TransformsMap");
}

void ModelMesh::Update()
{
	boneDesc.Index = boneIndex;
	perFrame->Update();
	// transform�� instance buffer���� ������ ó���Ѵ�.
}



// instanceRender
void ModelMesh::Render(UINT drawCount)
{
	boneBuffer->Render();
	sBoneBuffer->SetConstantBuffer(boneBuffer->Buffer());

	perFrame->Render();
	if (material != NULL) material->Render();
	if (meterialPBR != NULL) meterialPBR->Render();


	vertexBuffer->Render();
	indexBuffer->Render(); // instance�� world transform�� ������ ������ �ִ�.

	if (transformsSRV != NULL)
	{
		sTransformsSRV->SetResource(transformsSRV);
	}

	D3D::GetDC()->IASetPrimitiveTopology(topology);

	shader->DrawIndexedInstanced(0, pass, indexCount, drawCount);
}

