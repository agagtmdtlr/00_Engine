#include "Framework.h"
#include "PBR/ModelAnimatorPBR.h"

ModelAnimatorPBR::ModelAnimatorPBR(Shader * shader)
	:shader(shader)
{
	model = new ModelPBR();

	tweenBuffer = new ConstantBuffer(&tweenDesc, sizeof(TweenDesc) * MAX_MODEL_INSTANCE);
	sTweenBuffer = shader->AsConstantBuffer("CB_TweenFrame");

	blendBuffer = new ConstantBuffer(&blendDesc, sizeof(BlendDesc) * MAX_MODEL_INSTANCE);
	sBlendBuffer = shader->AsConstantBuffer("CB_BlendFrame");

	instanceBuffer = new VertexBuffer(worlds, MAX_MODEL_INSTANCE, sizeof(Matrix), 1, true);

	sSRV = shader->AsSRV("TransformsMap");
	
	//Create Compute Shader
	{
		computeShader = new Shader(L"71_GetMultiBones.fxo");

		// �ν��Ͻ��� ���� ���(Y)
		inputWorldBuffer = new StructuredBuffer(worlds, sizeof(Matrix), MAX_MODEL_INSTANCE);
		sInputWorldSRV = computeShader->AsSRV("InputWorlds");
		// ���� �� ���(x)
		inputBoneBuffer = new StructuredBuffer(NULL, sizeof(Matrix), MAX_MODEL_TRANSFORMS);
		sInputBoneSRV = computeShader->AsSRV("InputBones");

		ID3D11Texture2D* texture;
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = MAX_MODEL_TRANSFORMS * 4; 
		desc.Height = MAX_MODEL_INSTANCE;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture));
		
		// ���� �� * �ִϸ��̼� ��� * ���� ���
		// �� �ν��Ͻ��� ��� ���� ���� transform ����� �ؽ�ó�� ��ȯ�Ѵ�.
		outputBuffer = new TextureBuffer(texture);
		SafeRelease(texture);

		sOutputUAV = computeShader->AsUAV("Output");
		
		// ������ tranfrom ������ (�ִϸ��̼� ��� )
		sTransformsSRV = computeShader->AsSRV("TransformsMap");
		sComputeTweenBuffer = computeShader->AsConstantBuffer("CB_TweenFrame");
		sComputeBlendBuffer = computeShader->AsConstantBuffer("CB_BlendFrame");
	}

}

ModelAnimatorPBR::~ModelAnimatorPBR()
{
	SafeDelete(model);

	SafeDeleteArray(clipTransforms);
	SafeRelease(texture);
	SafeRelease(srv);

	SafeDelete(tweenBuffer);
	SafeDelete(blendBuffer);

	SafeDelete(instanceBuffer);

	SafeDelete(computeShader);

	SafeDelete(inputWorldBuffer);
	SafeDelete(inputBoneBuffer);
	SafeDelete(outputBuffer);

	

}

void ModelAnimatorPBR::Update()
{
	

	if (texture == NULL) // clip datas srv
	{
		for (SkinMesh* mesh : model->Meshes())
		{
			mesh->SetShader(shader);
		}
		CreateTexture();

		Matrix bones[MAX_MODEL_TRANSFORMS];
		for (UINT i = 0; i < model->BoneCount(); i++)
		{
			bones[i] = model->BoneByIndex(i)->Transform(); // ���� ������
		}
		// �� �ν��Ͻ����� ������ �⺻ �� ����
		inputBoneBuffer->CopyToInput(bones); // shader���� ������ �⺻ ������
		
	}

	for (UINT i = 0; i < transforms.size(); i++)
	{
		blendDesc[i].Mode == 0 ? UpdateTweenMode(i) : UpdateBlendUpdate(i);
	}

	tweenBuffer->Render();
	blendBuffer->Render();

	// CS�� �������ӿ� ���۽��ѵ� ������ �̸ֹ��̼��� ������ ����Ʈ�� ���� ���۽�Ű�� ���� ȿ������

	frameTime += Time::Delta();
	if (frameTime > (1.0f / frameRate))
	{
		sComputeTweenBuffer->SetConstantBuffer(tweenBuffer->Buffer());
		sComputeBlendBuffer->SetConstantBuffer(blendBuffer->Buffer());
		
		// srv == clip transform
		sTransformsSRV->SetResource(srv); // ������ �ִϸ��̼� tranform ������
		sInputWorldSRV->SetResource(inputWorldBuffer->SRV());
		sInputBoneSRV->SetResource(inputBoneBuffer->SRV());

		sOutputUAV->SetUnorderedAccessView(outputBuffer->UAV());

		// Dispatch�� �׷��� �浹ü �ϳ��� ����ϹǷ� 1���� �����ϰ� ���Ŀ� �浹ü��
		// �� ���� ���� ����� �� �׷��� �ø��ϴ�.
		computeShader->Dispatch(0, 0, 1, MAX_MODEL_INSTANCE, 1);
		// �� �ν��Ͻ��� ���� ��ġ
		outputBuffer->CopyFromOutput();
	}
	frameTime = fmod(frameTime, (1.0f / frameRate));

	// compute shader debugging csv data
	/*if (Keyboard::Get()->Down(VK_SPACE))
	{
		ID3D11Texture2D* temp = outputBuffer->CopyFromOutput();

		FILE* src;
		fopen_s(&src, "../src.csv", "w");

		Matrix id[MAX_MODEL_TRANSFORMS];

		for (UINT y = 0; y < MAX_MODEL_INSTANCE; y++)
		{
			D3D11_MAPPED_SUBRESOURCE subResource;
			D3D::GetDC()->Map(temp, 0, D3D11_MAP_READ, 0, &subResource);
			{
				void* p = (BYTE *)subResource.pData + y * subResource.RowPitch;
				memcpy(id, p, MAX_MODEL_TRANSFORMS * sizeof(Matrix));
			}
			D3D::GetDC()->Unmap(temp, 0);


			for (UINT x = 0; x < MAX_MODEL_TRANSFORMS; x++)
			{
				Matrix destMatrix = id[x];

				fprintf(src, "%f,%f,%f,%f,", destMatrix._11, destMatrix._12, destMatrix._13, destMatrix._14);
				fprintf(src, "%f,%f,%f,%f,", destMatrix._21, destMatrix._22, destMatrix._23, destMatrix._24);
				fprintf(src, "%f,%f,%f,%f,", destMatrix._31, destMatrix._32, destMatrix._33, destMatrix._34);
				fprintf(src, "%f,%f,%f,%f\n", destMatrix._41, destMatrix._42, destMatrix._43, destMatrix._44);
			}
		}

		fclose(src);
	}*/

	for (SkinMesh* mesh : model->Meshes())
	{
		mesh->Update();
	}
}

void ModelAnimatorPBR::Render()
{	
	sSRV->SetResource(srv);

	sTweenBuffer->SetConstantBuffer(tweenBuffer->Buffer());
	sBlendBuffer->SetConstantBuffer(blendBuffer->Buffer());

	instanceBuffer->Render();

	// �������� ������ Model Mesh �̸� ���� �׷��� transform�� ������ �ش�.
	for (SkinMesh* mesh : model->Meshes())
	{
		mesh->Render(transforms.size());
	}
}

void ModelAnimatorPBR::UpdateTweenMode(UINT index)
{
	

	TweenDesc& desc = tweenDesc[index];
	// ���� �ִϸ��̼�
	{
		//desc.Clip = 3;
		//desc.Speed = 0.05;
		ModelClip* clip = model->ClipByIndex(desc.Curr.Clip);

		desc.Curr.RunningTime += Time::Delta();
		bool stop = Time::Stopped();

		//�ð� ����
		// 1/30 / 1
		float f = clip->FrameRate();
		// ���������� ����ð�
		float time = 1.0f / (clip->FrameRate() * desc.Curr.Speed);

		if (desc.Curr.Time >= 1.0f) // ���� ������
		{
			desc.Curr.RunningTime = 0;

			UINT i = clip->FrameCount();

			// ���� ���
			desc.Curr.CurrFrame = (desc.Curr.CurrFrame + 1) % clip->FrameCount();
			desc.Curr.NextFrame = (desc.Curr.CurrFrame + 1) % clip->FrameCount();

		}
		desc.Curr.Time = desc.Curr.RunningTime / time;


	}
	// �ٲ� ������ �����Ѵٸ�
	if (desc.Next.Clip > -1)
	{
		desc.ChangeTime += Time::Delta();
		desc.TweenTime = desc.ChangeTime / desc.TakeTime; // ����ȭ�� �ð� ( Curr �� Next �� animation transform�� lerp�ϱ� ���� �ʿ��ϴ�.

		if (desc.TweenTime >= 1.0f) // �ִϸ��̼� ��ȯ �Ϸ��
		{
			desc.Curr = desc.Next;

			desc.Next.Clip = -1;
			desc.Next.CurrFrame = 0;
			desc.Next.NextFrame = 0;
			desc.Next.Time = 0;
			desc.Next.RunningTime = 0.0f;

			desc.ChangeTime = 0.0f;
			desc.TweenTime = 0.0f;

		}
		else
		{
			ModelClip* clip = model->ClipByIndex(desc.Next.Clip); // ���� ���� Ŭ���� �����Ѵ�.
			
			desc.Next.RunningTime += Time::Delta(); // �ð��� ��� ��Ų��.

			//�ð� ����
			// 1/30 / 1
			// �� �����Ӵ��� ����ð��� ����Ѵ�. 0.3�� (if 30 frame per second )
			float time = 1.0f / clip->FrameRate() / desc.Next.Speed;

			if (desc.Next.Time >= 1.0f) // ���� ������
			{
				desc.Next.RunningTime = 0;

				// ���� ���
				desc.Next.CurrFrame = (desc.Next.CurrFrame + 1) % clip->FrameCount();
				desc.Next.NextFrame = (desc.Next.CurrFrame + 1) % clip->FrameCount();

			}
			// ���� ����� �ð� / �����Ӵ� ��� �ð�
			desc.Next.Time = desc.Next.RunningTime / time;
		}
	}
	
}

void ModelAnimatorPBR::UpdateBlendUpdate(UINT index)
{
	BlendDesc& desc = blendDesc[index];

	for (UINT i = 0; i < 3; i++)
	{
		ModelClip* clip = model->ClipByIndex(desc.Clip[i].Clip);

		desc.Clip[i].RunningTime += Time::Delta();

		float time = 1.0f / clip->FrameRate() / desc.Clip[i].Speed;

		if (desc.Clip[i].Time >= 1.0f) // ���� ������
		{
			desc.Clip[i].RunningTime = 0;

			desc.Clip[i].CurrFrame = (desc.Clip[i].CurrFrame + 1) % clip->FrameCount();
			desc.Clip[i].NextFrame = (desc.Clip[i].CurrFrame + 1) % clip->FrameCount();
		}
		desc.Clip[i].Time = desc.Clip[i].RunningTime / time;
	}
}

void ModelAnimatorPBR::ReadMesh(wstring file)
{
	model->ReadMesh(file);
}

void ModelAnimatorPBR::ReadMaterial(wstring file)
{
	model->ReadMaterial(file);
}

void ModelAnimatorPBR::ReadClip(wstring file)
{
	model->ReadClip(file);
}

void ModelAnimatorPBR::Pass(UINT pass)
{
	for (SkinMesh* mesh : model->Meshes())
	{
		mesh->Pass(pass);
	}
}

void ModelAnimatorPBR::PlayTweenMode(UINT index, UINT clip, float speed, float takeTime)
{
	blendDesc[index].Mode = 0;
			 
	tweenDesc[index].TakeTime = takeTime;
	tweenDesc[index].Next.Clip = clip;
	tweenDesc[index].Next.Speed = speed; 
}

void ModelAnimatorPBR::PlayBlendMode(UINT index, UINT clip, UINT clip1, UINT clip2)
{
	blendDesc[index].Mode = 1;
			 
	blendDesc[index].Clip[0].Clip = clip;
	blendDesc[index].Clip[1].Clip = clip1;
	blendDesc[index].Clip[2].Clip = clip2;

}

void ModelAnimatorPBR::SetBlendAlpha(UINT index ,float alpha)
{
	alpha = Math::Clamp(alpha, 0.0f, 2.0f);

	blendDesc[index].Alpha = alpha;
}


// ���� �ش� �ν��Ͻ��� ���� ��ġ
void ModelAnimatorPBR::GetAttachTransform(UINT instance, Matrix * outResult)
{
	ID3D11Texture2D* texture = outputBuffer->Result();

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(texture, 0, D3D11_MAP_READ, 0, &subResource);
	{
		// �ؽ�ó �ٴ��� �����Ҷ�, ������ ũ���� sizeof(Matrix)�� ����ϸ� �ȵȴ�.
		// �ؽ�ó�� ũ�⸦ 2�� ����� ����� ������ ���������� ������ ���� �ʴ´�.
		// RowPitch�� ����ؼ� 2�� ����� �����ϵ��� �ؾ� �Ѵ�.
		// output r32g32b32a32 = 4f
		// output width = 4 * 250
		// matrix 4 * 4 * 250
		// output(4f) * outputwidth(4 * 250) = matrix( 4 * 4 ) * 250
		memcpy(outResult, (BYTE*)subResource.pData + (instance * subResource.RowPitch), sizeof(Matrix) * MAX_MODEL_TRANSFORMS);
	}
	D3D::GetDC()->Unmap(texture, 0);
}

void ModelAnimatorPBR::CreateTexture()
{
	//Matrix matirx[MAX_MODEL_KEYFRAMES][MAX_MODEL_TRANSFORMS];//overflow

	clipTransforms = new ClipTransform[model->ClipCount()];
	for (UINT i = 0; i < model->ClipCount(); i++) // Ŭ�� ���ٸ� �� Ʈ�������� ����� �ش�.
	{
		CreateClipTransform(i);
	}

	//Create Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		// X - Transform(col)
		// Y - Keyframe(row)
		// Z - clip(slice)
		// texture format : R8G8B8A8
		// Dx11 Texture format : R32B32G32A32 - float ( 4, 4, 4, 4 )
		// �� ������ �ȼ� �ϳ��� �ִ� ũ�� (16byte)
		// ������ �̷��� �ϸ� ������ �����. (matrix : 64byte )
		// �׷��� marix�� 4���� �ɰ���.
		// [ 0, 1, 2, 3 , 4, 5, 6, 7 ,8 , 9 ,.. 11 ]
		// Width  = TrnasformSize * 4;
		desc.Width = MAX_MODEL_TRANSFORMS  * 4;
		desc.Height = MAX_MODEL_KEYFRAMES;
		desc.ArraySize = model->ClipCount(); // texture2DArray�� ��ȯ�ȴ�.
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // 16byte; * 4 = 64byte == matrix size
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		// �� �ؽ�ó �ڷῡ �����ϴ� array�� �����Ѵ�.
		// array�� �ٴ°� Dynamic���� ���� ����.
		// ���� �����ϰ� �ʹٸ�
		// ���������� BIND �� D3D11_BIND_SHADER_RESOURCE�� �ְ� �Ǹ�
		// DYNAMIC�� ���� �� �ֵ��� �Ҽ��� �ִ�.
		// �Ǵ� �Ѵܰ� �� ���� 2D�� 3D�� �ٲ㼭 ����Ҽ��� �ִ�. (dynic��밡��)
		// TODO:: clip texture file usage dynamic
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1; // MSAA ��Ƽ�˸���¡�� �����̴� �Ӽ�

		// 4:: float�� �ڷ� 4byte, 16 :: ����� ���� ���� ( 4x4 ����� ���ٿ� �����ϱ� )
		UINT pageSize = MAX_MODEL_TRANSFORMS * 4 * 16 * MAX_MODEL_KEYFRAMES;
		// malloc �� 2MB �̻� �����÷�
		// ���� �޸��� ũ��� �����쿡�� �Ϲ������� 2MB ������ �Ҵ��Ո���
		// memcpy�� ũ�⵵ ���� �޸𸮿� �����ؼ� 2MB �̻� ����� �� �����Ƿ�
		// VirtualAlloc �� ����Ѵ�.
		// void*p = malloc(pageSize *model->ClipCount()); overflow
		void* p = VirtualAlloc(NULL, pageSize * model->ClipCount() , MEM_RESERVE, PAGE_READWRITE);
		
		//MORY_BASIC_INFORMATION , VirtualQuery :: ������ ����� �˼��� �ִ�.

		for (UINT c = 0; c < model->ClipCount(); c++)
		{
			// jump to slice address
			UINT start = c * pageSize;

			for (UINT k = 0; k < MAX_MODEL_KEYFRAMES; k++)
			{
				// jump to row address
				void* temp = (BYTE *)p + MAX_MODEL_TRANSFORMS * k * sizeof(Matrix) + start;
				// �Ҵ��� �ּ� /�Ҵ��� ������ , MEM_COMMIT Ȯ�� ���⸦ ����ϰڴ�.
				VirtualAlloc(temp , MAX_MODEL_TRANSFORMS * sizeof(Matrix), MEM_COMMIT, PAGE_READWRITE);
				memcpy(temp, clipTransforms[c].Transform[k], MAX_MODEL_TRANSFORMS * sizeof(Matrix));
			} // for (k)
		} // for(c)


		D3D11_SUBRESOURCE_DATA* subResources = new D3D11_SUBRESOURCE_DATA[model->ClipCount()];
		for (UINT c = 0; c < model->ClipCount(); c++)
		{
			void* temp = (BYTE*)p + c * pageSize;

			subResources[c].pSysMem = temp;
			// ������ ũ��
			subResources[c].SysMemPitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
			// �� ���� ũ��
			subResources[c].SysMemSlicePitch = pageSize;
		}
		Check(D3D::GetDevice()->CreateTexture2D(&desc, subResources, &texture));

		SafeDeleteArray(subResources);
		VirtualFree(p, 0 , MEM_RELEASE);

	}

	// Cretae SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY; // 
		// dimesion �´� �κп� �ڷ� �ֱ�
		desc.Texture2DArray.MipLevels = 1;
		desc.Texture2DArray.ArraySize = model->ClipCount();

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &desc, &srv));

	}

}

void ModelAnimatorPBR::CreateClipTransform(UINT index) // index�� clip�� �ش�Ǵ� ���� transform�� �����Ѵ�.
{
	Matrix* bones = new Matrix[MAX_MODEL_TRANSFORMS]; // ������ �����͸� ���� ����

	ModelClip* clip = model->ClipByIndex(index); // clip �����͸� ������ �´�.

	// final pallete index matrix
	// to_root ��ȯ ��� 
	// �Ѹ� ��ȯ * �θ�ȯ
	for (UINT f = 0; f < clip->FrameCount(); f++) // ������ ���� ��� ���� �۾�
	{
		for (UINT b = 0; b < model->BoneCount(); b++) // ������ clip transform�� ����
		{
			ModelBone* bone = model->BoneByIndex(b); // �ε����� ���� ������ �����´�.

			Matrix parent;
			// invGlobal ( local * global ) => global ����
			//ReadBoneData:: bone->Transform = bone->Transform * matParent; 
			Matrix invGlobal = bone->Transform(); // �ش� ���� �����
			// ������ ��ȯ
			D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);
			

			int parentIndex = bone->ParentIndex();
			if (parentIndex < 0) // ��Ʈ ����̸� ������� Ʈ������
				D3DXMatrixIdentity(&parent);
			else // ��Ʈ ��尡 �ƴϸ� �θ� Ʈ������
				parent = bones[parentIndex];

			Matrix animation;
			ModelKeyframe* frame = clip->Keyframe(bone->Name()); // �ش��������� �ش� ���� ���� �̸����� ��������

			if (frame != NULL)
			{
				ModelKeyframeData& data = frame->Transforms[f];

				Matrix S, R, T;
				D3DXMatrixScaling(&S, data.Scale.x, data.Scale.y, data.Scale.z);
				D3DXMatrixRotationQuaternion(&R, &data.Rotation);
				D3DXMatrixTranslation(&T, data.Translation.x, data.Translation.y, data.Translation.z);

				animation = S * R* T; // clip's bone transform
			}
			else
			{
				D3DXMatrixIdentity(&animation);
			}
			// �θ� �ڽ� ���踦 �δ°Ͱ� ����
			bones[b] = animation * parent;
			// Animation ����� �ش� �����ӿ� �ش� ���� �󸶸�ŭ �̵������� ����			
			// f : frame 
			// b : bone
			// invGlobal -> (���� ���� ����� [ ��������� �������� relative ����� �ȴ� ] ) [ boneLocal * parentWorld :inverse: parentWordInv * boneLocalInv ] 
			// bones[b]�� ���� ��ȯ
			// bones[b] = animation * parent (global)
			// gloabal *  (animation * parent )[global]

			// (bonelocal * global)inv * ( animation * global);
			// (bonelcal)inv * animation ( bone local ��ȯ )

			// invGlobal�� relative tranform
			// global = relative * global;
			// final bone i transform = offset transform * to_root transform
			clipTransforms[index].Transform[f][b] = invGlobal * bones[b];
			//clipTransforms[index].Transform[f][b] = animation;


		} // for(b)
	} // for(f)

	SafeDeleteArray(bones);
}

Transform * ModelAnimatorPBR::AddTransform()
{
	Transform* transform = new Transform();
	transforms.push_back(transform);

	return transform;
}

void ModelAnimatorPBR::UpdateTransforms()
{
	for (UINT i = 0; i < transforms.size(); i++)
		memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix));

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, worlds, sizeof(Matrix) * MAX_MODEL_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);

	inputWorldBuffer->CopyToInput(worlds);
}