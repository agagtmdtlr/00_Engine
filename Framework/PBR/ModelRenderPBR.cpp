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
	if (texture == NULL) // 아직 초기화가 되지 않은 상태
	{
		// 렌더시에 사용할 texture와 쉐이더를 각 메쉬별로 연결해준다.
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
	// 렌더링의 기준은 Model Mesh 이며 모델이 그려질 transform을 세팅해 준다.
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

// transform을 갱신한다.
void ModelRenderPBR::UpdateTransform(UINT instanceId, UINT boneIndex, Transform& transform)
{
	Matrix destMatrix = transform.World();

	ModelBone* bone = model->BoneByIndex(boneIndex);
	// destMatrix * MyMatrix * ParentMatrix
	boneTransforms[instanceId][boneIndex] = destMatrix * boneTransforms[instanceId][boneIndex];

	int childIndex;
	// 파생된 자식 노드의 본의 트랜스폼도 변경시켜준다.
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
	// 파생된 자식 노드의 본의 트랜스폼도 변경시켜준다.
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
	transforms.push_back(transform); // instance data 에 넣어준다 

	return transform;
}

void ModelRenderPBR::UpdateTransforms()
{
	// 버퍼데이터에 복사해준다.
	for (UINT i = 0; i < transforms.size(); i++)
		memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix));

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, worlds, sizeof(Matrix) * MAX_MODEL_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
}

// 각 인스턴스별 각 본의 transform을 담아서 사용할 텍스처를 만든다.
// 가로 : 각 본의 transform matrix , 세로 : 각 인스터스
void ModelRenderPBR::CreateTexture()
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = MAX_MODEL_TRANSFORMS * 4; // 
	desc.Height = MAX_MODEL_INSTANCE; 
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // 4byte * 4 ->  Vector4
	desc.Usage = D3D11_USAGE_DYNAMIC; // 수정할거니끄아
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;

	// 초기화 작업
	Matrix bones[MAX_MODEL_TRANSFORMS];
	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
	{
		for (UINT b = 0; b < model->BoneCount(); b++) // 각 인스턴스별 본의 정보를 쓴다.
		{
			ModelBone* bone = model->BoneByIndex(b); // 각 본을 읽어온다			
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
			else // 루트 노드가 아니면 부모 트랜스폼
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
	subResource.pSysMem = boneTransforms; // 인스턴스별 본의 정보를 담은 이차원 배열
	subResource.SysMemPitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
	subResource.SysMemSlicePitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix) * MAX_MODEL_INSTANCE;

	Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &texture));

	// rendering에 바인드할 srv 생성
	// srv를 바탕으로 instance 별로 참조를 한다.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Format = desc.Format;

	Check(D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv));
}

