#pragma once

class ModelBone;
class ModelMesh;
class ModelClip;
class SkinMesh;

class ModelPBR
{
public :
	// animation까지 관리 필요한 데이터 구조
	typedef VertexTextureNormalTangentBlend ModelVertex;

public:
	ModelPBR();
	~ModelPBR();

	UINT BoneCount() { return bones.size(); }
	vector<ModelBone *>& Bones() { return bones; }
	ModelBone* BoneByIndex(UINT index) { return bones[index]; }
	ModelBone* BoneByName(wstring name);

	UINT MeshCount() { return meshes.size(); }
	vector<SkinMesh *>& Meshes() { return meshes; }
	SkinMesh* MeshByIndex(UINT index) { return meshes[index]; }
	SkinMesh* MeshByName(wstring name);

	UINT MaterialCount() { return materials.size(); }
	vector<MaterialPBR *>& Materials() { return materials; }
	MaterialPBR* MaterialByIndex(UINT index) { return materials[index]; }
	MaterialPBR* MaterialByName(wstring name);

	UINT ClipCount() { return clips.size(); }
	vector<ModelClip *>& Clips() { return clips; }
	ModelClip* ClipByIndex(UINT index) { return clips[index]; }
	ModelClip* ClipByName(wstring name);

public:
	void ReadMesh(wstring file);
	void ReadMaterial(wstring file);
	void ReadClip(wstring file);

private:
	void BindBone();
	void BindMesh();

private:
	ModelBone* root;
	vector<ModelBone* > bones;
	//vector<ModelMesh* > meshes;
	vector<MaterialPBR *> materials;
	vector<ModelClip*> clips;
	vector<SkinMesh* > meshes;
};