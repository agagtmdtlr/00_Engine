#pragma once
#include "stdafx.h"

// 부모 자식 관계를 저장!
// parentIndex가 -1이면 root
struct asBone  // bone data
{
	int Index;
	string Name;

	int Parent;
	Matrix Transform;
};

//Bone : Mesh 는 1 : 1 관계
// 하지만 Mesh가 없는 본도 있습니다. Hp bar 가장 본
struct asMesh // mesh data
{
	string Name;
	int BoneIndex;

	aiMesh* Mesh;

	string MaterialName;

	// 정점 인덱스
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

	string DiffuseFile; // 텍스처 파일 원색
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

// 데이터 저장용
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

// 읽어온다음에 정리하기
struct asBoneWeights
{
private:
	typedef pair<int, float> Pair;
	vector<Pair> BoneWeights;

public:
	// 들어온 웨이트가
	// 가중치가 큰게 앞에 오도록 배치한다.
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

	// 정점에다 가중치 넣어주기
	void GetBlendWeights(asBlendWeight& blendWeights)
	{
		for (UINT i = 0; i < BoneWeights.size(); i++)
		{
			if (i >= 4) return;

			blendWeights.Set(i, BoneWeights[i].first, BoneWeights[i].second);
		}
	}
	// 정규화
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
	// 각 본마드 transform 저장
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

//aniNode의 원본 키프레임 저장
struct asClipNode // 파일로 저장할 데이터 본하고조정
{
	aiString Name;
	vector<asKeyframeData> Keyframe; // { time , s , r , t }
};