#include "Framework.h"
#include "MaterialPBR.h"

Texture* MaterialPBR::irradianceCube = NULL;
Texture* MaterialPBR::reflectionCube = NULL;
Texture* MaterialPBR::brdfMap = NULL;
UINT MaterialPBR::irradiance = 0;
UINT MaterialPBR::reflection = 0;
bool MaterialPBR::bIrrCreated = false;
bool MaterialPBR::bReflectCreated = false;



MaterialPBR::MaterialPBR()
	:shader(NULL)
{
	Initialize();
}

MaterialPBR::MaterialPBR(Shader * shader)
	:shader(shader)
{
	Initialize();
}

MaterialPBR::~MaterialPBR()
{
	Destroy();
}


void MaterialPBR::AlbedoMap(string file)
{
	AlbedoMap(String::ToWString(file));
}

void MaterialPBR::AlbedoMap(wstring file)
{
	SafeDelete(albedoMap);
	albedoMap = new Texture(file);
	desc.albedo = 1;
}

void MaterialPBR::MetallicMap(string file)
{
	MetallicMap(String::ToWString(file));
}

void MaterialPBR::MetallicMap(wstring file)
{
	SafeDelete(metallicMap);
	metallicMap = new Texture(file);
	desc.metal = 1;
}

void MaterialPBR::RoughnessMap(string file)
{
	RoughnessMap(String::ToWString(file));
}

void MaterialPBR::RoughnessMap(wstring file)
{
	SafeDelete(roughnessMap);
	roughnessMap = new Texture(file);
	desc.rough = 1;
}

void MaterialPBR::AOMap(string file)
{
	AOMap(String::ToWString(file));
}

void MaterialPBR::AOMap(wstring file)
{
	SafeDelete(aoMap);
	aoMap = new Texture(file);
	desc.ao = 1;
}

void MaterialPBR::NormalMap(string file)
{
	NormalMap(String::ToWString(file));
}

void MaterialPBR::NormalMap(wstring file)
{
	SafeDelete(normalMap);
	normalMap = new Texture(file);
	desc.normal = 1;
}

void MaterialPBR::IrradianceCube(string file)
{
	IrradianceCube(String::ToWString(file));
}

void MaterialPBR::IrradianceCube(wstring file)
{
	if (bIrrCreated)SafeDelete(irradianceCube);
	irradianceCube = new Texture(file);
	irradiance = 1;
	bIrrCreated = true;
}

void MaterialPBR::IrradianceCube(Texture * texture)
{
	if(bIrrCreated)SafeDelete(irradianceCube);
	irradianceCube = texture;
	bIrrCreated = false;
	irradiance = 1;
}

void MaterialPBR::ReflectionCube(string file)
{
	ReflectionCube(String::ToWString(file));
}

void MaterialPBR::ReflectionCube(wstring file)
{
	if(bReflectCreated)SafeDelete(reflectionCube);
	reflectionCube = new Texture(file);
	reflection = 1;
	bReflectCreated = true;
}

void MaterialPBR::ReflectionCube(Texture * texture)
{
	if (bReflectCreated)SafeDelete(reflectionCube);
	reflectionCube = texture;
	bReflectCreated = false;
	reflection = 1;
}

void MaterialPBR::BRDFMap(string file)
{
	BRDFMap(String::ToWString(file));
}

void MaterialPBR::BRDFMap(wstring file)
{
	SafeDelete(brdfMap);
	brdfMap = new Texture(file);
}

void MaterialPBR::DisplacementMap(string file)
{
	DisplacementMap(String::ToWString(file));
}

void MaterialPBR::DisplacementMap(wstring file)
{
	SafeDelete(displacementMap);
	displacementMap = new Texture(file);
	desc.displacement= 1;
}

void MaterialPBR::ParallaxMap(string file)
{
	ParallaxMap(String::ToWString(file));
}

void MaterialPBR::ParallaxMap(wstring file)
{
	SafeDelete(parallaxMap);
	parallaxMap = new Texture(file);
	desc.parallax = 1;
}


void MaterialPBR::Render()
{
	if (shader == NULL) assert(false);

	if (albedoMap != NULL)
		sAlbedoMap->SetResource(albedoMap->SRV());
	if (metallicMap != NULL)
		sMetallicMap->SetResource(metallicMap->SRV());
	if (roughnessMap != NULL)
		sRoughnessMap->SetResource(roughnessMap->SRV());
	if (aoMap != NULL)
		sAOMap->SetResource(aoMap->SRV());
	if (normalMap != NULL)
		sNormalMap->SetResource(normalMap->SRV());
	if (irradianceCube != NULL)
		sIrradianceCube->SetResource(irradianceCube->SRV());
	if (reflectionCube != NULL)
		sReflectionCube->SetResource(reflectionCube->SRV());
	if (displacementMap != NULL)
	{
		sDisplacementMap->SetResource(displacementMap->SRV());
		desc.texOffset.x = (float)displacementMap->GetWidth();
		desc.texOffset.y = (float)displacementMap->GetHeight();
	}
	if (parallaxMap != NULL)
		sParallaxMap->SetResource(parallaxMap->SRV());
	sBRDFMap->SetResource(brdfMap->SRV());

	desc.albedo = albedoMap ? 1 : 0;
	desc.metal = metallicMap ? 1 : 0;
	desc.rough = roughnessMap ? 1 : 0;
	desc.ao = aoMap ? 1 : 0;
	desc.normal = normalMap ? 1 : 0;
	desc.displacement = displacementMap ? 1 : 0;
	desc.parallax = parallaxMap ? 1 : 0;
	desc.irradiance = irradiance;
	desc.reflection = reflection;

	mapDesc->Render();
	sMapDesc->SetConstantBuffer(mapDesc->Buffer());
	materialDesc->Render();
	sMaterialDesc->SetConstantBuffer(materialDesc->Buffer());

}

void MaterialPBR::Destroy()
{
	SafeDelete(albedoMap);
	SafeDelete(metallicMap);
	SafeDelete(roughnessMap);
	SafeDelete(aoMap);
	SafeDelete(normalMap);
	SafeDelete(displacementMap);
	SafeDelete(parallaxMap);

	SafeDelete(mapDesc);
	SafeDelete(materialDesc);
}

void MaterialPBR::Initialize()
{
	name = L"";

	SafeDelete(mapDesc);
	mapDesc = new ConstantBuffer(&desc, sizeof(MapDesc));
	SafeDelete(materialDesc);
	materialDesc = new ConstantBuffer(&matDesc, sizeof(MaterialDesc));

	

	if(shader != NULL) SetShader(shader);
}


void MaterialPBR::SetShader(Shader * shader)
{
	this->shader = shader;
	sAlbedoMap = shader->AsSRV("AlbedoMap");
	sMetallicMap = shader->AsSRV("MetallicMap");
	sRoughnessMap = shader->AsSRV("RoughnessMap");
	sAOMap = shader->AsSRV("AOMap");
	sNormalMap = shader->AsSRV("NormalMap");
	sIrradianceCube = shader->AsSRV("IrradianceCube");
	sReflectionCube = shader->AsSRV("ReflectionCube");
	sBRDFMap = shader->AsSRV("BRDFMap");
	sDisplacementMap = shader->AsSRV("DisplacementMap");
	sParallaxMap = shader->AsSRV("ParallaxMap");
	sMapDesc = shader->AsConstantBuffer("CB_MapDesc");
	sMaterialDesc = shader->AsConstantBuffer("Material");

}

void MaterialPBR::CopyFrom(MaterialPBR * material)
{
	name = material->name;

	if (material->shader != NULL)
		SetShader(material->shader);

	if (material->albedoMap != NULL)
		AlbedoMap(material->albedoMap->GetFile());
	if (material->metallicMap != NULL)
		MetallicMap(material->metallicMap->GetFile());
	if (material->roughnessMap != NULL)
		RoughnessMap(material->roughnessMap->GetFile());
	if (material->normalMap != NULL)
		NormalMap(material->normalMap->GetFile());
	if (material->aoMap != NULL)
		AOMap(material->aoMap->GetFile());
	if (material->displacementMap != NULL)
		DisplacementMap(material->displacementMap->GetFile());
	if (material->parallaxMap != NULL)
		ParallaxMap(material->parallaxMap->GetFile());

}

StaticMaterialPBR::StaticMaterialPBR()
{
	Initialize();
}

StaticMaterialPBR::StaticMaterialPBR(Shader * shader)
	:shader(shader)
{
	Initialize();
}

StaticMaterialPBR::~StaticMaterialPBR()
{
	Destroy();
}

void StaticMaterialPBR::SetShader(Shader * shader)
{
	this->shader = shader;
	sAlbedoMap = shader->AsSRV("AlbedoMapArray");
	sMetallicMap = shader->AsSRV("MetallicMapArray");
	sRoughnessMap = shader->AsSRV("RoughnessMapArray");
	sAOMap = shader->AsSRV("AOMapArray");
	sNormalMap = shader->AsSRV("NormalMapArray");
	sIrradianceCube = shader->AsSRV("IrradianceCube");
	sReflectionCube = shader->AsSRV("ReflectionCube");
	sBRDFMap = shader->AsSRV("BRDFMap");
	sDisplacementMap = shader->AsSRV("DisplacementMapArray");
	sParallaxMap = shader->AsSRV("ParallaxMapArray");
	sMaterialDesc = shader->AsConstantBuffer("Material");
}

void StaticMaterialPBR::AlbedoMap(const vector<wstring>& file)
{
	SafeDelete(albedoMap);
	albedoName.assign(file.begin(), file.end());
	albedoMap = new TextureArray(albedoName, 512, 512);
}

void StaticMaterialPBR::MetallicMap(const vector<wstring>& file)
{
	SafeDelete(metallicMap);
	metalName.assign(file.begin(), file.end());
	metallicMap = new TextureArray(metalName, 512, 512);
}

void StaticMaterialPBR::RoughnessMap(const vector<wstring>& file)
{
	SafeDelete(roughnessMap);
	roughnessName.assign(file.begin(), file.end());
	roughnessMap = new TextureArray(roughnessName, 512, 512);
}

void StaticMaterialPBR::AOMap(const vector<wstring>& file)
{
	SafeDelete(aoMap);
	aoName.assign(file.begin(), file.end());
	aoMap = new TextureArray(aoName, 512, 512);
}

void StaticMaterialPBR::NormalMap(const vector<wstring>& file)
{
	SafeDelete(normalMap);
	normalName.assign(file.begin(), file.end());
	normalMap = new TextureArray(normalName, 512, 512);
}

void StaticMaterialPBR::DisplacementMap(const vector<wstring>& file)
{
	SafeDelete(displacementMap);
	displacementName.assign(file.begin(), file.end());
	displacementMap = new TextureArray(displacementName, 512, 512);
}

void StaticMaterialPBR::ParallaxMap(const vector<wstring>& file)
{
	SafeDelete(parallaxMap);
	parallaxName.assign(file.begin(), file.end());
	parallaxMap = new TextureArray(parallaxName, 512, 512);
}

void StaticMaterialPBR::Render()
{
	if (shader == NULL) assert(false);

	if (albedoMap != NULL)
		sAlbedoMap->SetResource(albedoMap->SRV());
	if (metallicMap != NULL)
		sMetallicMap->SetResource(metallicMap->SRV());
	if (roughnessMap != NULL)
		sRoughnessMap->SetResource(roughnessMap->SRV());
	if (aoMap != NULL)
		sAOMap->SetResource(aoMap->SRV());
	if (normalMap != NULL)
		sNormalMap->SetResource(normalMap->SRV());
	if (MaterialPBR::IrradianceCube() != NULL)
		sIrradianceCube->SetResource(MaterialPBR::IrradianceCube()->SRV());
	if (MaterialPBR::ReflectionCube() != NULL)
		sReflectionCube->SetResource(MaterialPBR::ReflectionCube()->SRV());
	if (displacementMap != NULL)
	{
		sDisplacementMap->SetResource(displacementMap->SRV());
	}
	if (parallaxMap != NULL)
		sParallaxMap->SetResource(parallaxMap->SRV());
	sBRDFMap->SetResource(MaterialPBR::BRDFMap()->SRV());

	materialDesc->Render();
	sMaterialDesc->SetConstantBuffer(materialDesc->Buffer());
}

void StaticMaterialPBR::Destroy()
{
	SafeDelete(albedoMap);
	SafeDelete(metallicMap);
	SafeDelete(roughnessMap);
	SafeDelete(aoMap);
	SafeDelete(normalMap);
	SafeDelete(displacementMap);
	SafeDelete(parallaxMap);

	SafeDelete(materialDesc);
}

void StaticMaterialPBR::Initialize()
{
	SafeDelete(materialDesc);
	materialDesc = new ConstantBuffer(&matDesc, sizeof(MaterialDesc));

	if (shader != NULL) SetShader(shader);
}
