#include "Framework.h"
#include "Utilities/BinaryFile.h"
#include "Utilities/Xml.h"
#include "ModelPBR.h"

ModelPBR::ModelPBR()
{
}

ModelPBR::~ModelPBR()
{
	for (ModelBone* bone : bones)
		SafeDelete(bone);

	for (SkinMesh * mesh : meshes)
		SafeDelete(mesh);

	for (MaterialPBR* material : materials)
		SafeDelete(material);

	for (ModelClip* clip : clips)
		SafeDelete(clip);
}

ModelBone * ModelPBR::BoneByName(wstring name)
{	
	for (ModelBone* bone : bones)
	{
		if (bone->Name() == name)
			return bone;
	}

	return NULL;
}

SkinMesh * ModelPBR::MeshByName(wstring name)
{
	for (SkinMesh* mesh : meshes)
	{
		if (mesh->Name() == name)
			return mesh;
	}

	return NULL;
}

MaterialPBR * ModelPBR::MaterialByName(wstring name)
{
	for (MaterialPBR* material : materials)
	{
		if (material->Name() == name)
			return material;
	}

	return NULL;
}

ModelClip * ModelPBR::ClipByName(wstring name)
{

	for (ModelClip* clip : clips)
	{
		if (clip->name == name)
			return clip;
	}

	return NULL;
}

void ModelPBR::ReadMesh(wstring file)
{
	file = L"../../_Models/" + file + L".mesh";
	// 모델 메쉬의 정보가 담긴 바이너리 파일을 읽는다.
	BinaryReader* r = new BinaryReader();
	r->Open(file);

	UINT count = 0;
	count = r->UInt(); // bone의 개수;

	for (UINT i = 0; i < count; i++)
	{
		ModelBone* bone = new ModelBone();
		bone->index = r->Int();
		bone->name = String::ToWString(r->String());
		bone->parentIndex = r->Int();
		bone->transform = r->Matrix();

		bones.push_back(bone);
	}

	count = r->UInt(); // Mesh의 개수;

	for (UINT i = 0; i < count; i++)
	{
		SkinMesh* mesh = new SkinMesh();

		mesh->name = String::ToWString(r->String());
		mesh->boneIndex = r->Int();

		// 이후 해당 메쉬에 메터리얼을 입힐때 모델에서 이름으로 찾는다 ModelMesh::Bind
		mesh->materialName = String::ToWString(r->String());

		//VertexData
		{			
			UINT count = r->UInt(); // 정점의 개수

			vector<SkinMesh::MeshVertex> vertices;
			vertices.assign(count, SkinMesh::MeshVertex());

			void* ptr = (void *)&(vertices[0]); // vector의 시작주소 포인터
			r->Byte(&ptr, sizeof(SkinMesh::MeshVertex) * count); // byte read

			mesh->vertices = new SkinMesh::MeshVertex[count]; // vertices crate
			mesh->vertexCount = count;
			copy
			(
				vertices.begin(), vertices.end(),
				stdext::checked_array_iterator<SkinMesh::MeshVertex *>(mesh->vertices, count)
			);
		}

		//IndexData
		{
			UINT count = r->UInt();

			vector<UINT> indices;
			indices.assign(count, UINT());

			void* ptr = (void *)&(indices[0]);
			r->Byte(&ptr, sizeof(UINT) * count);

			mesh->indices = new UINT[count];
			mesh->indexCount = count;
			copy
			(
				indices.begin(), indices.end(),
				stdext::checked_array_iterator<UINT *>(mesh->indices, count)
			);
		}

		meshes.push_back(mesh);
	}

	r->Close();
	SafeDelete(r);

	BindBone();

}

void ModelPBR::ReadMaterial(wstring file)
{
	file = L"../../_Textures/" + file + L".materialpbr";

	Xml::XMLDocument* document = new Xml::XMLDocument();
	Xml::XMLError error = document->LoadFile(String::ToString(file).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement* root = document->FirstChildElement();
	Xml::XMLElement* materialNode = root->FirstChildElement();

	do
	{
		MaterialPBR* material = new MaterialPBR();

		Xml::XMLElement* node = NULL;

		node = materialNode->FirstChildElement();
		material->Name(String::ToWString(node->GetText()));

		wstring directory = Path::GetDirectoryName(file);
		String::Replace(&directory, L"../../_Textures", L"");


		wstring texture = L"";

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->AlbedoMap(directory + texture);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->MetallicMap(directory + texture);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->RoughnessMap(directory + texture);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->NormalMap(directory + texture);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->AOMap(directory + texture);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->DisplacementMap(directory + texture);
		

		materials.push_back(material); // 각 메터리얼를 컨테이너에 저장

		materialNode = materialNode->NextSiblingElement();
	} while (materialNode != NULL);

	// 생성한 드로잉 데이터와
	// model의 material정보를 바탕으로 각 메쉬마다 bind 처리를한다.
	// 각 메쉬에는 해당 메터리얼 매칭 정보를 materialName으로 가지고 있다
	BindMesh(); 
}

void ModelPBR::ReadClip(wstring file)
{
	file = L"../../_Models/" + file + L".clip";

	BinaryReader* r = new BinaryReader();
	r->Open(file);


	ModelClip* clip = new ModelClip();

	clip->name = String::ToWString(r->String());
	clip->duration = r->Float();
	clip->frameRate = r->Float();
	clip->frameCount = r->UInt();

	UINT keyframesCount = r->UInt();
	for (UINT i = 0; i < keyframesCount; i++)
	{
		ModelKeyframe* keyframe = new ModelKeyframe();
		keyframe->BoneName = String::ToWString(r->String());


		UINT size = r->UInt();
		if (size > 0)
		{
			keyframe->Transforms.assign(size, ModelKeyframeData());

			void* ptr = (void *)&keyframe->Transforms[0];
			r->Byte(&ptr, sizeof(ModelKeyframeData) * size);
		}

		clip->keyframeMap[keyframe->BoneName] = keyframe;
	}

	r->Close();
	SafeDelete(r);

	clips.push_back(clip);
}

// 바이너리에서 읽어오는 본의 인덱스 정보를 바탕으로
// 상속관계를(부모/자식관계)를 포인터로 연결해준다.
// 이후 bone의 transform을 갱신할때 (forward kinematic)
// top down 방식으로 부모에서 자식으로 정보를 갱신할대 사용한다.
void ModelPBR::BindBone()
{
	root = bones[0];

	// 트리 구조에 맞게 
	for (ModelBone* bone : bones)
	{
		if (bone->parentIndex > -1)
		{
			bone->parent = bones[bone->parentIndex];
			bone->parent->childs.push_back(bone);
		}
		else
		{
			bone->parent = NULL;
		}
	}

}

void ModelPBR::BindMesh()
{
	for (SkinMesh* mesh : meshes) // 각 메쉬마다 바인드해줄 데이터를 처리한다. ex(bone, material)
	{
		mesh->bone = bones[mesh->boneIndex]; // 해당 메쉬의 본 정보를 가져온다. ModelBone

		mesh->Binding(this); // 해당 모델의 메터리얼을 바탕으로 각 메쉬에 메터리얼에 바인드 해준다.
	}
}
