#pragma once

class Moon : public Renderer
{
public:
	Moon(Shader* shader);
	~Moon();

	void Update();
	void Render(float theta, Vector3 position, float d);

private:
	float GetAlpha(float theta);

	Matrix GetTransform(float theta, Vector3 position , float d);
	Matrix GetGlowTransform(float theta, Vector3 position, float d);

private:
	float distance, glowDistance;

	ID3DX11EffectScalarVariable* sAlpha;

	Texture* moon;
	Texture* moonGlow;
	ID3DX11EffectShaderResourceVariable* sMoon;
};