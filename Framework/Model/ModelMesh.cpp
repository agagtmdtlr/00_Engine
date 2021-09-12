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
void ModelMesh::Binding(Model * model) // 메터리얼을 참조할 모델
{
	// 해당 메쉬의 정점,인덱스 버퍼를 생성한다.
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(Model::ModelVertex));
	indexBuffer = new IndexBuffer(indices, indexCount);
	
	// 해당 메쉬의 메터리얼이름을 가지고 메터리얼을 자져온다.
	Material* srcMaterial = model->MaterialByName(materialName);

	// 해당 메터리얼 정보를 나의 메터리얼에 본사한다.
	material = new Material();
	material->CopyFrom(srcMaterial);
}

// PBR Model Binding
void ModelMesh::Binding(ModelPBR * model)
{
	// 해당 메쉬의 정점,인덱스 버퍼를 생성한다.
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(Model::ModelVertex));
	indexBuffer = new IndexBuffer(indices, indexCount);

	// 해당 메쉬의 메터리얼이름을 가지고 메터리얼을 자져온다.
	MaterialPBR* srcMaterial = model->MaterialByName(materialName);
	meterialPBR = new MaterialPBR();
	meterialPBR->CopyFrom(srcMaterial);
}

// new shader에 대한 정보를 바뀌어야 하므로 기존 transform perframe을 지우고 새로 만든다.
void ModelMesh::SetShader(Shader * shader)
{
	this->shader = shader;

	// transform은 instance buffer에서 가져와 처리한다.

	SafeDelete(perFrame);
	perFrame = new PerFrame(shader);

	sBoneBuffer = shader->AsConstantBuffer("CB_Bone");

	material->SetShader(shader);

	sTransformsSRV = shader->AsSRV("TransformsMap");
}

void ModelMesh::SetShaderPBR(Shader * shader)
{
	this->shader = shader;

	// transform은 instance buffer에서 가져와 처리한다.

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
	// transform은 instance buffer에서 가져와 처리한다.
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
	indexBuffer->Render(); // instance별 world transform의 정보를 가지고 있다.

	if (transformsSRV != NULL)
	{
		sTransformsSRV->SetResource(transformsSRV);
	}

	D3D::GetDC()->IASetPrimitiveTopology(topology);

	shader->DrawIndexedInstanced(0, pass, indexCount, drawCount);
}

