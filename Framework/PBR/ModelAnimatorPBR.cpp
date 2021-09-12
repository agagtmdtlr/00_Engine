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

		// 인스턴스의 월드 행렬(Y)
		inputWorldBuffer = new StructuredBuffer(worlds, sizeof(Matrix), MAX_MODEL_INSTANCE);
		sInputWorldSRV = computeShader->AsSRV("InputWorlds");
		// 기준 본 행렬(x)
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
		
		// 기준 본 * 애니메이션 행렬 * 월드 행렬
		// 각 인스턴스의 모든 본의 현재 transform 결과를 텍스처로 반환한다.
		outputBuffer = new TextureBuffer(texture);
		SafeRelease(texture);

		sOutputUAV = computeShader->AsUAV("Output");
		
		// 참조할 tranfrom 데이터 (애니메이션 행렬 )
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
			bones[i] = model->BoneByIndex(i)->Transform(); // 기준 본정보
		}
		// 각 인스턴스에서 참조할 기본 본 정보
		inputBoneBuffer->CopyToInput(bones); // shader에서 참조할 기본 본정보
		
	}

	for (UINT i = 0; i < transforms.size(); i++)
	{
		blendDesc[i].Mode == 0 ? UpdateTweenMode(i) : UpdateBlendUpdate(i);
	}

	tweenBuffer->Render();
	blendBuffer->Render();

	// CS를 매프레임에 동작시켜도 되지만 애미메이션의 프레임 레이트에 맞춰 동작시키는 것이 효율적임

	frameTime += Time::Delta();
	if (frameTime > (1.0f / frameRate))
	{
		sComputeTweenBuffer->SetConstantBuffer(tweenBuffer->Buffer());
		sComputeBlendBuffer->SetConstantBuffer(blendBuffer->Buffer());
		
		// srv == clip transform
		sTransformsSRV->SetResource(srv); // 참조할 애니메이션 tranform 데이터
		sInputWorldSRV->SetResource(inputWorldBuffer->SRV());
		sInputBoneSRV->SetResource(inputBoneBuffer->SRV());

		sOutputUAV->SetUnorderedAccessView(outputBuffer->UAV());

		// Dispatch의 그룹은 충돌체 하나만 사용하므로 1개만 설정하고 이후에 충돌체를
		// 한 번에 전부 계산할 때 그룹을 늘립니다.
		computeShader->Dispatch(0, 0, 1, MAX_MODEL_INSTANCE, 1);
		// 각 인스턴스의 본의 위치
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

	// 렌더링의 기준은 Model Mesh 이며 모델이 그려질 transform을 세팅해 준다.
	for (SkinMesh* mesh : model->Meshes())
	{
		mesh->Render(transforms.size());
	}
}

void ModelAnimatorPBR::UpdateTweenMode(UINT index)
{
	

	TweenDesc& desc = tweenDesc[index];
	// 현재 애니메이션
	{
		//desc.Clip = 3;
		//desc.Speed = 0.05;
		ModelClip* clip = model->ClipByIndex(desc.Curr.Clip);

		desc.Curr.RunningTime += Time::Delta();
		bool stop = Time::Stopped();

		//시간 비율
		// 1/30 / 1
		float f = clip->FrameRate();
		// 한프레임의 재생시간
		float time = 1.0f / (clip->FrameRate() * desc.Curr.Speed);

		if (desc.Curr.Time >= 1.0f) // 다음 프레임
		{
			desc.Curr.RunningTime = 0;

			UINT i = clip->FrameCount();

			// 루프 재생
			desc.Curr.CurrFrame = (desc.Curr.CurrFrame + 1) % clip->FrameCount();
			desc.Curr.NextFrame = (desc.Curr.CurrFrame + 1) % clip->FrameCount();

		}
		desc.Curr.Time = desc.Curr.RunningTime / time;


	}
	// 바뀔 동작이 존재한다면
	if (desc.Next.Clip > -1)
	{
		desc.ChangeTime += Time::Delta();
		desc.TweenTime = desc.ChangeTime / desc.TakeTime; // 정규화된 시간 ( Curr 과 Next 의 animation transform을 lerp하기 위해 필요하다.

		if (desc.TweenTime >= 1.0f) // 애니메이션 전환 완료됨
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
			ModelClip* clip = model->ClipByIndex(desc.Next.Clip); // 다음 동작 클립을 참조한다.
			
			desc.Next.RunningTime += Time::Delta(); // 시간을 경과 시킨다.

			//시간 비율
			// 1/30 / 1
			// 한 프레임당의 재생시간를 계산한다. 0.3초 (if 30 frame per second )
			float time = 1.0f / clip->FrameRate() / desc.Next.Speed;

			if (desc.Next.Time >= 1.0f) // 다음 프레임
			{
				desc.Next.RunningTime = 0;

				// 루프 재생
				desc.Next.CurrFrame = (desc.Next.CurrFrame + 1) % clip->FrameCount();
				desc.Next.NextFrame = (desc.Next.CurrFrame + 1) % clip->FrameCount();

			}
			// 현재 경과한 시간 / 프레임당 재생 시간
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

		if (desc.Clip[i].Time >= 1.0f) // 다음 프레임
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


// 계산된 해당 인스턴스의 본의 위치
void ModelAnimatorPBR::GetAttachTransform(UINT instance, Matrix * outResult)
{
	ID3D11Texture2D* texture = outputBuffer->Result();

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(texture, 0, D3D11_MAP_READ, 0, &subResource);
	{
		// 텍스처 줄단위 점프할때, 한줄의 크리고 sizeof(Matrix)를 사용하면 안된다.
		// 텍스처는 크기를 2의 배수로 만들기 때문에 정상적으로 점프가 되지 않는다.
		// RowPitch를 사용해서 2의 배수로 점프하도록 해야 한다.
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
	for (UINT i = 0; i < model->ClipCount(); i++) // 클립 마다마 본 트랜스폼을 만들어 준다.
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
		// 이 포맷이 픽셀 하나의 최대 크기 (16byte)
		// 하지만 이렇게 하면 문제가 생긴다. (matrix : 64byte )
		// 그래서 marix를 4개로 쪼갠다.
		// [ 0, 1, 2, 3 , 4, 5, 6, 7 ,8 , 9 ,.. 11 ]
		// Width  = TrnasformSize * 4;
		desc.Width = MAX_MODEL_TRANSFORMS  * 4;
		desc.Height = MAX_MODEL_KEYFRAMES;
		desc.ArraySize = model->ClipCount(); // texture2DArray로 변환된다.
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // 16byte; * 4 = 64byte == matrix size
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		// 각 텍스처 자료에 상응하는 array가 존재한다.
		// array가 붙는건 Dynamic으로 쓸수 없다.
		// 굳이 변경하고 싶다면
		// 예외적으로 BIND 에 D3D11_BIND_SHADER_RESOURCE를 주게 되면
		// DYNAMIC이 허용될 수 있도록 할수는 있다.
		// 또는 한단계 더 놓은 2D는 3D로 바꿔서 사용할수도 있다. (dynic사용가능)
		// TODO:: clip texture file usage dynamic
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1; // MSAA 안티알리어징과 관련이는 속성

		// 4:: float형 자료 4byte, 16 :: 행렬의 원소 개수 ( 4x4 행렬을 한줄에 넣으니깐 )
		UINT pageSize = MAX_MODEL_TRANSFORMS * 4 * 16 * MAX_MODEL_KEYFRAMES;
		// malloc 은 2MB 이상 오버플로
		// 스택 메모리의 크기는 윈도우에서 일반적으로 2MB 정도로 할당합닏나
		// memcpy의 크기도 스택 메모리에 의존해서 2MB 이상 사용할 수 없으므로
		// VirtualAlloc 을 사용한다.
		// void*p = malloc(pageSize *model->ClipCount()); overflow
		void* p = VirtualAlloc(NULL, pageSize * model->ClipCount() , MEM_RESERVE, PAGE_READWRITE);
		
		//MORY_BASIC_INFORMATION , VirtualQuery :: 예약한 사이즈를 알수가 있다.

		for (UINT c = 0; c < model->ClipCount(); c++)
		{
			// jump to slice address
			UINT start = c * pageSize;

			for (UINT k = 0; k < MAX_MODEL_KEYFRAMES; k++)
			{
				// jump to row address
				void* temp = (BYTE *)p + MAX_MODEL_TRANSFORMS * k * sizeof(Matrix) + start;
				// 할당할 주소 /할당할 사이즈 , MEM_COMMIT 확정 여기를 사용하겠다.
				VirtualAlloc(temp , MAX_MODEL_TRANSFORMS * sizeof(Matrix), MEM_COMMIT, PAGE_READWRITE);
				memcpy(temp, clipTransforms[c].Transform[k], MAX_MODEL_TRANSFORMS * sizeof(Matrix));
			} // for (k)
		} // for(c)


		D3D11_SUBRESOURCE_DATA* subResources = new D3D11_SUBRESOURCE_DATA[model->ClipCount()];
		for (UINT c = 0; c < model->ClipCount(); c++)
		{
			void* temp = (BYTE*)p + c * pageSize;

			subResources[c].pSysMem = temp;
			// 가로의 크기
			subResources[c].SysMemPitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
			// 한 면의 크기
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
		// dimesion 맞는 부분에 자료 넣기
		desc.Texture2DArray.MipLevels = 1;
		desc.Texture2DArray.ArraySize = model->ClipCount();

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &desc, &srv));

	}

}

void ModelAnimatorPBR::CreateClipTransform(UINT index) // index의 clip에 해당되는 본의 transform을 생성한다.
{
	Matrix* bones = new Matrix[MAX_MODEL_TRANSFORMS]; // 생성된 데이터를 담을 변수

	ModelClip* clip = model->ClipByIndex(index); // clip 데이터를 가지고 온다.

	// final pallete index matrix
	// to_root 변환 행렬 
	// 뿌리 변환 * 부모변환
	for (UINT f = 0; f < clip->FrameCount(); f++) // 프레임 마다 모든 본을 작업
	{
		for (UINT b = 0; b < model->BoneCount(); b++) // 본마다 clip transform을 적용
		{
			ModelBone* bone = model->BoneByIndex(b); // 인덱스로 본의 정보를 가져온다.

			Matrix parent;
			// invGlobal ( local * global ) => global 상태
			//ReadBoneData:: bone->Transform = bone->Transform * matParent; 
			Matrix invGlobal = bone->Transform(); // 해당 본의 역행렬
			// 오프셋 변환
			D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);
			

			int parentIndex = bone->ParentIndex();
			if (parentIndex < 0) // 루트 노드이면 단위행렬 트랜스폼
				D3DXMatrixIdentity(&parent);
			else // 루트 노드가 아니면 부모 트랜스폼
				parent = bones[parentIndex];

			Matrix animation;
			ModelKeyframe* frame = clip->Keyframe(bone->Name()); // 해당프레임의 해당 본을 본의 이름으로 가져오기

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
			// 부모 자식 관계를 맺는것과 동일
			bones[b] = animation * parent;
			// Animation 행렬은 해당 프레임에 해당 본이 얼마만큼 이동할지를 결정			
			// f : frame 
			// b : bone
			// invGlobal -> (본의 월드 역행렬 [ 월드행렬의 역행을은 relative 행렬이 된다 ] ) [ boneLocal * parentWorld :inverse: parentWordInv * boneLocalInv ] 
			// bones[b]를 통해 변환
			// bones[b] = animation * parent (global)
			// gloabal *  (animation * parent )[global]

			// (bonelocal * global)inv * ( animation * global);
			// (bonelcal)inv * animation ( bone local 변환 )

			// invGlobal은 relative tranform
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