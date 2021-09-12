#pragma once

class ModelConverter
{
public:

	ModelConverter();
	~ModelConverter();

	void ReadFile(wstring file);
	void ReadFileDiffMesh(wstring file);

public:
	void ExportMesh(wstring savePath);
	void ExportStaticMesh(wstring savePath);
	void ExportMeshDiff(wstring savePath);
private:
	// ReCursion method (sceneroot에서 재귀를 타고 들어가 데이터이다 )
	/*
	transformation	matrix
	mesh			index
	matrial			index
	*/
	void ReadBoneData(aiNode* node, int index, int parent);
	// 원하는 데이터를 읽어 온다 (mesh)
	void ReadMeshData(aiNode* node, int bone);
	void ReadStaticMeshData();
	void ReadSkinData();
	// 읽어온 데이터를 원하는 형식으로 저장한다.
	void WriteMeshData(wstring savePath);
	void WriteStaticMeshData(wstring savePath);

	void DiffReadBoneData(aiNode* node, int index, int parent);
	void DiffReadMeshData(aiNode* node, int bone);
	void DiffReadSkinData();
	void DiffWriteMeshData(wstring savePath);
	Matrix DiffMatrix(aiNode* node, string name);


public: 
	// 디자인/제작 방식에 따라 직접 파일을 수정해야 되기 때문에 xml 파일로 저장한다.
	void ExportMaterial(wstring savePath, bool bOverwrite = true);
	void ExportMaterialPBR(wstring savePath, bool bOverwrite = true);


private:
	void ReadMaterialData();
	void WriteMaterialData(wstring savePath);
	void ReadMaterialDataPBR();
	void WriteMaterialDataPBR(wstring savePath);

	// material texture write
	string WriteTexture(string saveFolder, string file);

public:
	void ClipList(vector<wstring>* list);
	void ExportAnimClip(UINT index, wstring savePath);

private:
	// 따로 저장하지않고 읽어들인 clip값을 받아 그대로 사용한다.
	struct asClip* ReadClipData(aiAnimation* animation);
	void ReadKeyframeData(struct asClip* clip, aiNode* node, vector<struct asClipNode>& aiNodeInfos);
	void WriteClipData(struct asClip* clip, wstring savePath);

private:
	wstring file;
	wstring diffFile;

	Assimp::Importer* importer;
	Assimp::Importer* diffImporter;
	// 모든 데이터가 담겨있다.
	const aiScene* scene;
	const aiScene* diffScene;

	// as 접두사 :우리가 정의
	// ai 접두사 : assimp

	vector<Matrix> bonesOffset;
	vector<struct asBone *> bones;
	vector<struct asMesh *> meshes;
	vector<struct asMaterial *> materials;
	vector<struct asMaterialPBR *> materialPBRs;
	vector<struct asStaticMesh *> staticMeshes;

	vector<string> clipBoneNames;
	vector<string> clipChannelNames;
};