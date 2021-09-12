#pragma once

struct IBL_Texture
{
	void Destroy()
	{
		SafeDelete(EnvironmentMap);
		SafeDelete(IrradianceMap);
		SafeDelete(ReflectionMap);
	}
	Texture * EnvironmentMap;
	Texture * IrradianceMap;
	Texture * ReflectionMap;
};

class MaterialPBR
{
private:
	struct MapDesc // material이 포함되어있는지를 확인한다.
	{
		UINT albedo = 0;
		UINT metal = 0;
		UINT rough = 0;
		UINT ao = 0;
		UINT normal = 0;
		UINT irradiance = 0;
		UINT reflection = 0;
		UINT displacement = 0;
		UINT parallax = 0;
		float height_scale = 0.030f;
		Vector2 texOffset;
	} desc;

	struct MaterialDesc // material 정보를 저장한다. (default 포함)
	{
		Vector3 albedo = { 1,1,1 };
		float metallic = 0.0f;
		//
		float roughness = 0.0f;
		float ao = 1.0f;
		float irradiance;
		UINT reverse = 0;
		//
		Vector3 ambient;
		float padding2;
		Vector3 fresnel;
		float padding3;
	} matDesc;

public:
	MaterialPBR();
	MaterialPBR(Shader* shader);
	~MaterialPBR();

	Shader* GetShader() { return shader; }
	void SetShader(Shader* shader);

	void CopyFrom(MaterialPBR* material);

	void Name(wstring val) { name = val; }
	wstring Name() { return name; }

	void HeightSacle(float f) { desc.height_scale = f; }
	void ReverseHeight() { matDesc.reverse = (matDesc.reverse + 1) % 2; }

public:
	Texture* AlbedoMap() { return albedoMap; }
	void AlbedoMap(string file);
	void AlbedoMap(wstring file);
	Texture* MetallicMap() { return metallicMap; }
	void MetallicMap(string file);
	void MetallicMap(wstring file);
	Texture* RoughnessMap() { return roughnessMap; }
	void RoughnessMap(string file);
	void RoughnessMap(wstring file);
	Texture* AOMap() { return aoMap; }
	void AOMap(string file);
	void AOMap(wstring file);
	Texture* NormalMap() { return normalMap; }
	void NormalMap(string file);
	void NormalMap(wstring file);
	Texture* DisplacementMap() { return displacementMap; }
	void DisplacementMap(string file);
	void DisplacementMap(wstring file);
	Texture* ParallaxMap() { return parallaxMap; }
	void ParallaxMap(string file);
	void ParallaxMap(wstring file);

public://static class method
	// share environment map for each materials
	static Texture* IrradianceCube() { return irradianceCube; }
	static void IrradianceCube(string file);
	static void IrradianceCube(wstring file);
	static void IrradianceCube(Texture* texture);
	static Texture* ReflectionCube() { return reflectionCube; }
	static void ReflectionCube(string file);
	static void ReflectionCube(wstring file);
	static void ReflectionCube(Texture* texture);
	static Texture* BRDFMap() { return brdfMap; }
	static void BRDFMap(string file);
	static void BRDFMap(wstring file);
	

	void Render();

	void Destroy();

private:
	void Initialize();
private:
	Shader* shader = NULL;

private:
	wstring name;

	Texture* albedoMap = NULL;
	ID3DX11EffectShaderResourceVariable* sAlbedoMap;
	Texture* metallicMap = NULL;
	ID3DX11EffectShaderResourceVariable* sMetallicMap;
	Texture* roughnessMap = NULL;
	ID3DX11EffectShaderResourceVariable* sRoughnessMap;
	Texture* aoMap = NULL;
	ID3DX11EffectShaderResourceVariable* sAOMap;
	Texture* normalMap = NULL;
	ID3DX11EffectShaderResourceVariable* sNormalMap;
	Texture* displacementMap = NULL;
	ID3DX11EffectShaderResourceVariable* sDisplacementMap;
	Texture* parallaxMap = NULL;
	ID3DX11EffectShaderResourceVariable* sParallaxMap;

	// ambient light map ( environment light map )
	static UINT irradiance;
	static UINT reflection;
	static bool bIrrCreated;
	static bool bReflectCreated;
	static Texture* irradianceCube;
	ID3DX11EffectShaderResourceVariable* sIrradianceCube;
	static Texture* reflectionCube;
	ID3DX11EffectShaderResourceVariable* sReflectionCube;
	static Texture* brdfMap;
	ID3DX11EffectShaderResourceVariable* sBRDFMap;
		
	ConstantBuffer* mapDesc = NULL;
	ID3DX11EffectConstantBuffer* sMapDesc;
	ConstantBuffer* materialDesc = NULL;
	ID3DX11EffectConstantBuffer* sMaterialDesc;
};

/////////////////////////////////////////////////////////////////////////////////
class StaticMaterialPBR
{
private:
	struct MaterialDesc // material 정보를 저장한다. (default 포함)
	{
		Vector3 albedo = { 1,1,1 };
		float metallic = 0.0f;
		//
		float roughness = 0.0f;
		float ao = 1.0f;
		float irradiance;
		UINT reverse = 0;
		//
		Vector3 ambient;
		float padding2;
		Vector3 fresnel;
		float padding3;
	} matDesc;

public:
	StaticMaterialPBR();
	StaticMaterialPBR(Shader* shader);
	~StaticMaterialPBR();

	Shader* GetShader() { return shader; }
	void SetShader(Shader* shader);
public:
	TextureArray* AlbedoMap() { return albedoMap; }
	void AlbedoMap(const vector<wstring> & file);
	TextureArray* MetallicMap() { return metallicMap; }
	void MetallicMap(const vector<wstring> & file);
	TextureArray* RoughnessMap() { return roughnessMap; }
	void RoughnessMap(const vector<wstring> & file);
	TextureArray* AOMap() { return aoMap; }
	void AOMap(const vector<wstring> & file);
	TextureArray* NormalMap() { return normalMap; }
	void NormalMap(const vector<wstring> & file);
	TextureArray* DisplacementMap() { return displacementMap; }
	void DisplacementMap(const vector<wstring> & file);
	TextureArray* ParallaxMap() { return parallaxMap; }
	void ParallaxMap(const vector<wstring> & file);

public://static class method
	void Render();
	void Destroy();

private:
	void Initialize();
private:
	Shader* shader = NULL;

private:
	vector<wstring> albedoName;
	vector<wstring> metalName;
	vector<wstring> roughnessName;
	vector<wstring> aoName;
	vector<wstring> normalName;
	vector<wstring> displacementName;
	vector<wstring> parallaxName;


	TextureArray* albedoMap = NULL;
	ID3DX11EffectShaderResourceVariable* sAlbedoMap;
	TextureArray* metallicMap = NULL;
	ID3DX11EffectShaderResourceVariable* sMetallicMap;
	TextureArray* roughnessMap = NULL;
	ID3DX11EffectShaderResourceVariable* sRoughnessMap;
	TextureArray* aoMap = NULL;
	ID3DX11EffectShaderResourceVariable* sAOMap;
	TextureArray* normalMap = NULL;
	ID3DX11EffectShaderResourceVariable* sNormalMap;
	TextureArray* displacementMap = NULL;
	ID3DX11EffectShaderResourceVariable* sDisplacementMap;
	TextureArray* parallaxMap = NULL;
	ID3DX11EffectShaderResourceVariable* sParallaxMap;

	ID3DX11EffectShaderResourceVariable* sIrradianceCube;
	ID3DX11EffectShaderResourceVariable* sReflectionCube;
	ID3DX11EffectShaderResourceVariable* sBRDFMap;

	ConstantBuffer* materialDesc = NULL;
	ID3DX11EffectConstantBuffer* sMaterialDesc;
};