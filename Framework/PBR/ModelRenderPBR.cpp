#include "Framework.h"
#include "ModelRenderPBR.h"

ModelRenderPBR::ModelRenderPBR(Shader * shader)
	: shader(shader)
{
	model = new ModelPBR();

	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]);

	instanceBuffer = new VertexBuffer(worlds, MAX_MODEL_INSTANCE, sizeof(Matrix), 1, true);
	sSRV = shader->AsSRV("TransformsMap");
}

ModelRenderPBR::~ModelRenderPBR()
{
	SafeDelete(model);

	for (Transform* transform : transforms) // instance transform
		SafeDelete(transform);

	SafeDelete(instanceBuffer);

	SafeRelease(texture);
	SafeRelease(srv);
}

void ModelRenderPBR::Update()
{
	if (texture == NULL) // ���� �ʱ�ȭ�� ���� ���� ����
	{
		// �����ÿ� ����� texture�� ���̴��� �� �޽����� �������ش�.
		for (SkinMesh* mesh : model->Meshes())
			mesh->SetShader(shader);

		CreateTexture();
	}

	for (SkinMesh* mesh : model->Meshes())
	{
		mesh->Update();
	}
}

void ModelRenderPBR::Render()
{
	sSRV->SetResource(srv);
	instanceBuffer->Render();
	// �������� ������ Model Mesh �̸� ���� �׷��� transform�� ������ �ش�.
	for (SkinMesh* mesh : model->Meshes())
	{
		UINT instanceCount = transforms.size();
		mesh->Render(instanceCount);
	}
}

void ModelRenderPBR::ReadMesh(wstring file)
{
	model->ReadMesh(file);
}

void ModelRenderPBR::ReadMaterial(wstring file)
{
	model->ReadMaterial(file);
}

void ModelRenderPBR::Pass(UINT pass)
{
	for (SkinMesh* mesh : model->Meshes())
	{
		mesh->Pass(pass);
	}
}

void ModelRenderPBR::Topology(D3D11_PRIMITIVE_TOPOLOGY t)
{
	for (SkinMesh* mesh : model->Meshes())
	{
		mesh->Topology(t);
	}
}

// transform�� �����Ѵ�.
void ModelRenderPBR::UpdateTransform(UINT instanceId, UINT boneIndex, Transform& transform)
{
	Matrix destMatrix = transform.World();

	ModelBone* bone = model->BoneByIndex(boneIndex);
	// destMatrix * MyMatrix * ParentMatrix
	boneTransforms[instanceId][boneIndex] = destMatrix * boneTransforms[instanceId][boneIndex];

	int childIndex;
	// �Ļ��� �ڽ� ����� ���� Ʈ�������� ��������ش�.
	for (ModelBone* child : bone->Childs())
	{
		childIndex = child->Index();
		Matrix parent = boneTransforms[instanceId][boneIndex];
		Matrix invParent; 
		D3DXMatrixInverse(&invParent, NULL, &parent);
		Matrix temp = boneTransforms[instanceId][childIndex] * invParent;
		boneTransforms[instanceId][childIndex] = temp * destMatrix * parent;
		UpdateTransformChild(instanceId, childIndex, transform);
	}

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, boneTransforms, MAX_MODEL_INSTANCE * MAX_MODEL_TRANSFORMS * sizeof(Matrix));
	}
	D3D::GetDC()->Unmap(texture, 0);
}

void ModelRenderPBR::UpdateTransformChild(UINT instanceid, UINT boneIndex, Transform & transform)
{
	Matrix destMatrix = transform.World();
	ModelBone* bone = model->BoneByIndex(boneIndex);

	int childIndex;
	// �Ļ��� �ڽ� ����� ���� Ʈ�������� ��������ش�.
	for (ModelBone* child : bone->Childs())
	{
		Matrix parent = boneTransforms[instanceid][boneIndex];
		Matrix invParent;
		D3DXMatrixInverse(&invParent, NULL, &parent);
		childIndex = child->Index();
		Matrix temp = boneTransforms[instanceid][childIndex] * invParent;
		boneTransforms[instanceid][childIndex] = temp * destMatrix * parent;
		UpdateTransformChild(instanceid, childIndex, transform);
	}
}

Transform * ModelRenderPBR::AddTransform()
{
	Transform* transform = new Transform();
	transforms.push_back(transform); // instance data �� �־��ش� 

	return transform;
}

void ModelRenderPBR::UpdateTransforms()
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
void ModelRenderPBR::CreateTexture()
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
	Matrix bones[MAX_MODEL_TRANSFORMS];
	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
	{
		for (UINT b = 0; b < model->BoneCount(); b++) // �� �ν��Ͻ��� ���� ������ ����.
		{
			ModelBone* bone = model->BoneByIndex(b); // �� ���� �о�´�			
			Matrix matrix = bone->Transform();
			Matrix offset;
			D3DXMatrixInverse(&offset, NULL, &matrix);
			Matrix orig;
			
			Matrix pp;
			Matrix parent;
			int parentIndex = bone->ParentIndex();
			if (parentIndex < 0)
			{
				D3DXMatrixIdentity(&parent);
				D3DXMatrixIdentity(&pp);
			}
			else // ��Ʈ ��尡 �ƴϸ� �θ� Ʈ������
			{
				parent = bones[parentIndex];
				pp = bone->Parent()->Transform();
			}

			Matrix parentInv;
			D3DXMatrixInverse(&parentInv, NULL, &pp);
			orig = matrix * parentInv;

			Transform temp;
			temp.RotationDegree(90, 0, 0);
			//temp.Position(0, 20, 0);


			bones[b] = orig * parent;

			if (b == 1)
			{
				Vector3 s, r,rd, t;
				Matrix S, R, T;
				Math::MatrixDecompose(orig, s, r, t);
				rd = Math::ToDegree(r);
				D3DXMatrixScaling(&S, s.x, s.y, s.z);
				D3DXMatrixRotationYawPitchRoll(&R, r.y, r.x, r.z);
				D3DXMatrixTranslation(&T, t.x, t.z, t.y);
				orig = S * R * temp.World() * T;

				bones[b] = orig * parent * temp.World();
			}


			boneTransforms[i][b] = offset * bones[b];
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
}

