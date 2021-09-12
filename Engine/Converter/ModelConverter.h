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
	// ReCursion method (sceneroot���� ��͸� Ÿ�� �� �������̴� )
	/*
	transformation	matrix
	mesh			index
	matrial			index
	*/
	void ReadBoneData(aiNode* node, int index, int parent);
	// ���ϴ� �����͸� �о� �´� (mesh)
	void ReadMeshData(aiNode* node, int bone);
	void ReadStaticMeshData();
	void ReadSkinData();
	// �о�� �����͸� ���ϴ� �������� �����Ѵ�.
	void WriteMeshData(wstring savePath);
	void WriteStaticMeshData(wstring savePath);

	void DiffReadBoneData(aiNode* node, int index, int parent);
	void DiffReadMeshData(aiNode* node, int bone);
	void DiffReadSkinData();
	void DiffWriteMeshData(wstring savePath);
	Matrix DiffMatrix(aiNode* node, string name);


public: 
	// ������/���� ��Ŀ� ���� ���� ������ �����ؾ� �Ǳ� ������ xml ���Ϸ� �����Ѵ�.
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
	// ���� ���������ʰ� �о���� clip���� �޾� �״�� ����Ѵ�.
	struct asClip* ReadClipData(aiAnimation* animation);
	void ReadKeyframeData(struct asClip* clip, aiNode* node, vector<struct asClipNode>& aiNodeInfos);
	void WriteClipData(struct asClip* clip, wstring savePath);

private:
	wstring file;
	wstring diffFile;

	Assimp::Importer* importer;
	Assimp::Importer* diffImporter;
	// ��� �����Ͱ� ����ִ�.
	const aiScene* scene;
	const aiScene* diffScene;

	// as ���λ� :�츮�� ����
	// ai ���λ� : assimp

	vector<Matrix> bonesOffset;
	vector<struct asBone *> bones;
	vector<struct asMesh *> meshes;
	vector<struct asMaterial *> materials;
	vector<struct asMaterialPBR *> materialPBRs;
	vector<struct asStaticMesh *> staticMeshes;

	vector<string> clipBoneNames;
	vector<string> clipChannelNames;
};