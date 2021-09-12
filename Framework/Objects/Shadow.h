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
	// 그림자 그리는 방식 3가지
	struct Desc
	{
		// 라이팅의 위치와 방향
		Matrix View; // shadow view
		// 라이팅에서의 보여줄 영역
		Matrix Projection; // shadow projection

		Vector2 MapSize; // 그림자 퀄리팅 결정
		float Bias = -0.0006f;

		UINT Quality = 2; // 그림자를의 품질을 올리는데 변수
	} desc;

private:
	Shader* shader;
	// 그림자 맵의 해상도 
	// 깊이의 버퍼 크기도 결정
	UINT width, height;

	// 어는 지점을 비출지
	Vector3 position;
	// 조명 구역 [ 그림자가 들어갈 반경 ] [직육면체]
	float radius;

	RenderTarget* renderTarget; 
	DepthStencil* depthStencil;
	Viewport* viewport;

	ConstantBuffer* buffer; // shadow data
	ID3DX11EffectConstantBuffer* sBuffer;
	ID3DX11EffectShaderResourceVariable* sShadowMap; // 2pass에 깊이버퍼를 줄 변수

	ID3D11SamplerState* pcfSampler;
	ID3DX11EffectSamplerVariable* sPcfSampler;

};