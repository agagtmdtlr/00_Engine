#include "Framework.h"
#include "ModelRender.h"

ModelRender::ModelRender(Shader * shader)
	: shader(shader)
{
	model = new Model();

	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]);

	instanceBuffer = new VertexBuffer(worlds, MAX_MODEL_INSTANCE, sizeof(Matrix), 1, true);
}

ModelRender::~ModelRender()
{
	SafeDelete(model);

	for (Transform* transform : transforms)
		SafeDelete(transform);

	SafeDelete(instanceBuffer);

	SafeRelease(texture);
	SafeRelease(srv);
}

void ModelRender::Update()
{
	if (texture == NULL) // ���� �ʱ�ȭ�� ���� ���� ����
	{
		// �����ÿ� ����� texture�� ���̴��� �� �޽����� �������ش�.
		for (ModelMesh* mesh : model->Meshes())
			mesh->SetShader(shader);

		CreateTexture();
	}

	for (ModelMesh* mesh : model->Meshes())
	{
		mesh->Update();
	}
}

void ModelRender::Render()
{
	instanceBuffer->Render();
	// �������� ������ Model Mesh �̸� ���� �׷��� transform�� ������ �ش�.
	for (ModelMesh* mesh : model->Meshes())
	{
		mesh->Render(transforms.size());
	}
}

void ModelRender::ReadMesh(wstring file)
{
	model->ReadMesh(file);
}

void ModelRender::ReadMaterial(wstring file)
{
	model->ReadMaterial(file);
}

void ModelRender::Pass(UINT pass)
{
	for (ModelMesh* mesh : model->Meshes())
	{
		mesh->Pass(pass);
	}
}

// transform�� �����Ѵ�.
void ModelRender::UpdateTransform(UINT instanceId, UINT boneIndex, Transform& transform)
{
	Matrix destMatrix = transform.World();

	ModelBone* bone = model->BoneByIndex(boneIndex);
	// destMatrix * MyMatrix * ParentMatrix
	boneTransforms[instanceId][boneIndex] = destMatrix * boneTransforms[instanceId][boneIndex];

	int tempBoneIndex = boneIndex;
	// �Ļ��� �ڽ� ����� ���� Ʈ�������� ��������ش�.
	for (ModelBone* child : bone->Childs())
	{
		// purpose result : childMatrix * destMatrix * MyMatrix * ParentMatrix

		// ����� �θ��� Ʈ������ (  destMatrix * MyMatrix * ParentMatrix )
		Matrix parent = boneTransforms[instanceId][boneIndex];

		Matrix invParent; // (ParentInv * MyInv * desInv )
		D3DXMatrixInverse(&invParent, NULL, &parent);
		tempBoneIndex++;

		// current Child : ( childMatrix * MyMatrix * ParentMatrix )
		// ch2 * ch * p * pi * chi * desti = ch2 * desti

		// convert : ( childMatrix * MyMatrix * ParentMatrix ) *  (ParentInv * MyInv * desInv )
		// -> ���� ��Ģ�� ����
		// convert result : childMatrix * desInvMatrx
		Matrix temp = boneTransforms[instanceId][tempBoneIndex] * invParent;
		// ch2 * desti * dest * dest * ch * p = ch2 * dest * ch * p

		// current child : childMatrix * desInvMatrx
		// childMatrix * destInvMatrix * destMatrix * (  destMatrix * MyMatrix * ParentMatrix )
		// final result : childMatrix * destMatrix * MyMatrix * ParentMatrix
		boneTransforms[instanceId][tempBoneIndex] = temp * destMatrix * parent;
	}

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, boneTransforms, MAX_MODEL_INSTANCE * MAX_MODEL_TRANSFORMS * sizeof(Matrix));
	}
	D3D::GetDC()->Unmap(texture, 0);
}

Transform * ModelRender::AddTransform()
{
	Transform* transform = new Transform();
	transforms.push_back(transform); // instance data �� �־��ش� 

	return transform;
}

void ModelRender::UpdateTransforms()
{
	// ���۵����Ϳ� �������ش�.
	for (UINT i = 0; i < transforms.size(); i++)
		memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix));

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, worlds, sizeof(Matrix) * MAX_MODEL_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
}

// �� �ν��Ͻ��� �� ���� transform�� ��Ƽ� ����� �ؽ�ó�� �����.
// ���� : �� ���� transform matrix , ���� : �� �ν��ͽ�
void ModelRender::CreateTexture()
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = MAX_MODEL_TRANSFORMS * 4; // 
	desc.Height = MAX_MODEL_INSTANCE; 
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // 4byte * 4 ->  Vector4
	desc.Usage = D3D11_USAGE_DYNAMIC; // �����ҰŴϲ���
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;

	// �ʱ�ȭ �۾�
	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
	{
		for (UINT b = 0; b < model->BoneCount(); b++) // �� �ν��Ͻ��� ���� ������ ����.
		{
			ModelBone* bone = model->BoneByIndex(b); // �� ���� �о�´�			

			Matrix matrix = bone->Transform();
			boneTransforms[i][b] = matrix;

		}
	}

	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = boneTransforms; // �ν��Ͻ��� ���� ������ ���� ������ �迭
	subResource.SysMemPitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
	subResource.SysMemSlicePitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix) * MAX_MODEL_INSTANCE;

	Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &texture));

	// rendering�� ���ε��� srv ����
	// srv�� �������� instance ���� ������ �Ѵ�.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Format = desc.Format;

	Check(D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv));

	// ������ srv�� �� �޽��� �������ش�.
	for (ModelMesh* mesh : model->Meshes())
		mesh->TransformsSRV(srv);
}

