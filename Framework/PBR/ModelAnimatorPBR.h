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
// �� ����ü

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
	// �� ����ü
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

private:// �Ѱ��� ������ ������ ���� ����ü
	struct KeyframeDesc
	{
		int Clip = 0; // ���� �÷��� Ŭ�� ��ȣ

		UINT CurrFrame = 0; // ���� ������ ��ȣ
		UINT NextFrame = 0; // ���� ������ ��ȣ

		float Time = 0.0f; // �÷������� �ð� 1 ~ 0 normalize
		float RunningTime = 0.0f; // �ִϸ��̼� ���ۺ��� �ð�

		float Speed = 1.0f; // ��� �ӵ�

		Vector2 Padding;
	}; //keyframeDesc;

	ConstantBuffer* tweenBuffer;
	ID3DX11EffectConstantBuffer* sTweenBuffer;

	struct TweenDesc // ������ �����Ӱ��� �ڿ������� ������
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
	struct BlendDesc // ���۰� ���۰��� �ڿ� ������ ��ȭ
	{
		UINT Mode = 0;
		float Alpha = 0; // speed : distinguish clip index this value
		Vector2 Padding;

		KeyframeDesc Clip[3]; // ������ ������ ����
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
	const float frameRate = 30.0f; // ������ ���� ������ ����
	float frameTime = 0.0f;


private:

	Shader* computeShader;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV;
	
	// ���۸� �������� �ѱ��
	StructuredBuffer* inputWorldBuffer = NULL;
	ID3DX11EffectShaderResourceVariable* sInputWorldSRV;

	StructuredBuffer* inputBoneBuffer;
	ID3DX11EffectShaderResourceVariable* sInputBoneSRV;


	TextureBuffer* outputBuffer;
	ID3DX11EffectUnorderedAccessViewVariable* sOutputUAV;

	ID3DX11EffectConstantBuffer* sComputeTweenBuffer;
	ID3DX11EffectConstantBuffer* sComputeBlendBuffer;





};