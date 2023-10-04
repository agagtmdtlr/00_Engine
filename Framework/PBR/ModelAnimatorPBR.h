#pragma once

// cbuffer size 4999
// animation data
// transforms size 250 (bone)
// clip max size 500
// 250 * 500 = 125,000 ( 125000 > 4999 )
// send to texture data 
// 125000 * 64 = 8000000 -> 8MB ( One clip)

// matrix row = keyframe
// matrix col = bone
// slice = clip
// 면 구조체

class ModelAnimatorPBR : public RendererPBR
{
public:
	ModelAnimatorPBR(Shader* shader);
	~ModelAnimatorPBR();

	void Update();
	void Render();
	
private:
	void UpdateTweenMode(UINT index);
	void UpdateBlendUpdate(UINT index);

public:
	void ReadMesh(wstring file);
	void ReadMaterial(wstring file);
	void ReadClip(wstring file);

	ModelPBR* GetModel() { return model; }
	// add for instancing method
	Transform* AddTransform();
	Transform* GetTransform(UINT index) { return transforms[index]; }
	void UpdateTransforms();
	// add for instancing method
	UINT GetTransformCount() { return static_cast<UINT>(transforms.size()); }


	void Pass(UINT pass);

	void PlayTweenMode(UINT index, UINT clip, float speed = 1.0f,float takeTime = 1.0f);
	void PlayBlendMode(UINT index, UINT clip, UINT clip1, UINT clip2);
	void SetBlendAlpha(UINT index, float alpha);

	void GetAttachTransform(UINT instance, Matrix* outResult);
private:
	void CreateTexture();
	void CreateClipTransform(UINT index);


private:
	// matrix row = keyframe
	// matrix col = bone
	// slice = clip
	// 면 구조체
	struct ClipTransform // clip
	{
		Matrix** Transform; // slice

		ClipTransform()
		{
			Transform = new Matrix*[MAX_MODEL_KEYFRAMES];

			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				Transform[i] = new Matrix[MAX_MODEL_TRANSFORMS];
		}
		~ClipTransform()
		{
			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				SafeDeleteArray(Transform[i]);

			SafeDeleteArray(Transform);
		}
	};
	
	ClipTransform* clipTransforms = NULL;

	ID3D11Texture2D* texture = NULL;
	ID3D11ShaderResourceView* srv = NULL;
	ID3DX11EffectShaderResourceVariable* sSRV;

private:// 넘겨줄 프레임 정보에 대한 구조체
	struct KeyframeDesc
	{
		int Clip = 0; // 현재 플레이 클립 번호

		UINT CurrFrame = 0; // 현재 프레임 번호
		UINT NextFrame = 0; // 다음 프레임 번호

		float Time = 0.0f; // 플레이중인 시간 1 ~ 0 normalize
		float RunningTime = 0.0f; // 애니메이션 시작부터 시간

		float Speed = 1.0f; // 재생 속도

		Vector2 Padding;
	}; //keyframeDesc;

	ConstantBuffer* tweenBuffer;
	ID3DX11EffectConstantBuffer* sTweenBuffer;

	struct TweenDesc // 동작의 프레임간의 자연스러운 움직임
	{
		float TakeTime = 0.1f;
		float TweenTime = 0.0f;
		float ChangeTime = 0.0f;
		float Padding;
		KeyframeDesc Curr;
		KeyframeDesc Next;

		TweenDesc()
		{
			Curr.Clip = 0;
			Next.Clip = -1;
		}
	} tweenDesc[MAX_MODEL_INSTANCE];

private:
	struct BlendDesc // 동작과 동작간의 자연 스러운 변화
	{
		UINT Mode = 0;
		float Alpha = 0; // speed : distinguish clip index this value
		Vector2 Padding;

		KeyframeDesc Clip[3]; // 동작의 개수를 정의
	} blendDesc[MAX_MODEL_INSTANCE];

	ConstantBuffer* blendBuffer;
	ID3DX11EffectConstantBuffer* sBlendBuffer;


private:
	Shader* shader;
	ModelPBR* model;
	
	vector<Transform *> transforms;
	Matrix worlds[MAX_MODEL_INSTANCE];

	VertexBuffer* instanceBuffer;


private:
	// CS INput output
	struct CS_InputDesc
	{
		Matrix Bone;
	};

	struct CS_OutputDesc
	{
		Matrix Result;
	};


private:
	const float frameRate = 30.0f; // 성능을 위해 프레임 제한
	float frameTime = 0.0f;


private:

	Shader* computeShader;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV;
	
	// 버퍼를 두종류를 넘길거
	StructuredBuffer* inputWorldBuffer = NULL;
	ID3DX11EffectShaderResourceVariable* sInputWorldSRV;

	StructuredBuffer* inputBoneBuffer;
	ID3DX11EffectShaderResourceVariable* sInputBoneSRV;


	TextureBuffer* outputBuffer;
	ID3DX11EffectUnorderedAccessViewVariable* sOutputUAV;

	ID3DX11EffectConstantBuffer* sComputeTweenBuffer;
	ID3DX11EffectConstantBuffer* sComputeBlendBuffer;





};