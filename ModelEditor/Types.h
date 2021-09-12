#pragma once
#include "stdafx.h"

// �θ� �ڽ� ���踦 ����!
// parentIndex�� -1�̸� root
struct asBone  // bone data
{
	int Index;
	string Name;

	int Parent;
	Matrix Transform;
};

//Bone : Mesh �� 1 : 1 ����
// ������ Mesh�� ���� ���� �ֽ��ϴ�. Hp bar ���� ��
struct asMesh // mesh data
{
	string Name;
	int BoneIndex;

	aiMesh* Mesh;

	string MaterialName;

	// ���� �ε���
	vector<Model::ModelVertex> Vertices;
	vector<UINT> Indices;
};

struct asStaticMesh
{
	string Name;

	aiMesh* Mesh;

	string MaterialName;

	vector<StaticMesh::MeshVertex> Vertices;
	vector<UINT> Indices;
};

struct asMaterial
{
	string Name;

	Color Ambient;
	Color Diffuse;
	Color Specular;
	Color Emissive;

	string DiffuseFile; // �ؽ�ó ���� ����
	string SpecularFile;
	string NormalFile;
};

struct asMaterialPBR
{
	string Name;

	string AlbedoFile;
	string MetallicFile;
	string RoughnessFile;
	string NormalFile;
	string AOFile; // ambient occlusion
	string DisplacementFile;
};

// ������ �����
struct asBlendWeight
{
	Vector4 Indices = Vector4(0, 0, 0, 0);
	Vector4 Weights = Vector4(0, 0, 0, 0);

	void Set(UINT index, UINT boneIndex, float weight)
	{
		float i = (float)boneIndex;
		float w = weight;

		switch (index)
		{
		case 0: Indices.x = i; Weights.x = w; break;
		case 1: Indices.y = i; Weights.y = w; break;
		case 2: Indices.z = i; Weights.z = w; break;
		case 3: Indices.w = i; Weights.w = w; break;
		}
	}
};

// �о�´����� �����ϱ�
struct asBoneWeights
{
private:
	typedef pair<int, float> Pair;
	vector<Pair> BoneWeights;

public:
	// ���� ����Ʈ��
	// ����ġ�� ū�� �տ� ������ ��ġ�Ѵ�.
	void AddWeights(UINT boneIndex, float boneWeights)
	{
		if (boneWeights <= 0.0f) return;

		bool bInsert = false;
		vector<Pair>::iterator it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			if (boneWeights > it->second)
			{
				BoneWeights.insert(it, Pair(boneIndex, boneWeights));
				bInsert = true;

				break;
			}

			it++;
		} // while(it)

		if (bInsert == false)
			BoneWeights.push_back(Pair(boneIndex, boneWeights));
	}

	// �������� ����ġ �־��ֱ�
	void GetBlendWeights(asBlendWeight& blendWeights)
	{
		for (UINT i = 0; i < BoneWeights.size(); i++)
		{
			if (i >= 4) return;

			blendWeights.Set(i, BoneWeights[i].first, BoneWeights[i].second);
		}
	}
	// ����ȭ
	void Normalize()
	{
		float totalWeight = 0.0f;

		int i = 0;
		vector<Pair>::iterator it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			if (i < 4)
			{
				totalWeight += it->second;
				i++; it++;
			}
			else
				it = BoneWeights.erase(it);
		}

		float scale = 1.0f / totalWeight;

		it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			it->second *= scale;
			it++;
		}
	}
};


struct asKeyframeData
{
	float Time;

	Vector3 Scale;
	Quaternion Rotation;
	Vector3 Translation;
};

struct asKeyframe
{
	// �� ������ transform ����
	string BoneName;
	vector<asKeyframeData> Transforms;
};

struct asClip
{
	~asClip()
	{
		for (asKeyframe* k : Keyframes)
			SafeDelete(k);
	}

	string Name;

	UINT FrameCount;
	float FrameRate;
	float Duration;

	vector<asKeyframe *> Keyframes; // 
};

//aniNode�� ���� Ű������ ����
struct asClipNode // ���Ϸ� ������ ������ ���ϰ�����
{
	aiString Name;
	vector<asKeyframeData> Keyframe; // { time , s , r , t }
};