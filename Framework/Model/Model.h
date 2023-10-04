#pragma once

// 최대 본의 갯수를 지정
// 사람의 관절이 250개가 넘지 않기 때문이다.
#define MAX_MODEL_TRANSFORMS 250
#define MAX_MODEL_KEYFRAMES 500
#define MAX_MODEL_INSTANCE 500

class ModelBone;
class ModelMesh;
class ModelClip;

class Model
{
public :
	// animation까지 관리 필요한 데이터 구조
	typedef VertexTextureNormalTangentBlend ModelVertex;

public:
	Model();
	~Model();

	UINT BoneCount() { return static_cast<UINT>(bones.size()); }
	vector<ModelBone *>& Bones() { return bones; }
	ModelBone* BoneByIndex(UINT index) { return bones[index]; }
	ModelBone* BoneByName(wstring name);

	UINT MeshCount() { return  static_cast<UINT>(meshes.size()); }
	vector<ModelMesh *>& Meshes() { return meshes; }
	ModelMesh* MeshByIndex(UINT index) { return meshes[index]; }
	ModelMesh* MeshByName(wstring name);

	UINT MaterialCount() { return static_cast<UINT>(materials.size()); }
	vector<Material *>& Materials() { return materials; }
	Material* MaterialByIndex(UINT index) { return materials[index]; }
	Material* MaterialByName(wstring name);

	UINT ClipCount() { return static_cast<UINT>(clips.size()); }
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
	vector<ModelMesh* > meshes;
	vector<Material *> materials;
	vector<ModelClip*> clips;
};