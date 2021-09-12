#pragma once

class Shadow
{
public:
	Shadow(Shader* shader, Vector3& position, float radius, UINT width = 1024, UINT height = 1024);
	~Shadow();

	void PreRender();

	ID3D11ShaderResourceView* SRV() { return renderTarget->SRV(); }

private:
	void CalcViewProjection();


private:
	// �׸��� �׸��� ��� 3����
	struct Desc
	{
		// �������� ��ġ�� ����
		Matrix View; // shadow view
		// �����ÿ����� ������ ����
		Matrix Projection; // shadow projection

		Vector2 MapSize; // �׸��� ������ ����
		float Bias = -0.0006f;

		UINT Quality = 2; // �׸��ڸ��� ǰ���� �ø��µ� ����
	} desc;

private:
	Shader* shader;
	// �׸��� ���� �ػ� 
	// ������ ���� ũ�⵵ ����
	UINT width, height;

	// ��� ������ ������
	Vector3 position;
	// ���� ���� [ �׸��ڰ� �� �ݰ� ] [������ü]
	float radius;

	RenderTarget* renderTarget; 
	DepthStencil* depthStencil;
	Viewport* viewport;

	ConstantBuffer* buffer; // shadow data
	ID3DX11EffectConstantBuffer* sBuffer;
	ID3DX11EffectShaderResourceVariable* sShadowMap; // 2pass�� ���̹��۸� �� ����

	ID3D11SamplerState* pcfSampler;
	ID3DX11EffectSamplerVariable* sPcfSampler;

};