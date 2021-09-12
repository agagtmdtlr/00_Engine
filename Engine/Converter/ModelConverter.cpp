#include "stdafx.h"
#include "ModelConverter.h"
#include "ModelConverterTypes.h" // asBone // Bone
#include "Utilities/BinaryFile.h"
#include "Utilities/Xml.h"

ModelConverter::ModelConverter()
{
	importer = new Assimp::Importer();
	diffImporter = new Assimp::Importer();
}

ModelConverter::~ModelConverter()
{
	SafeDelete(importer);
	SafeDelete(diffImporter);
}

void ModelConverter::ReadFile(wstring file)
{
	// 파일은 저장할고 쓸 경로
	//this->file = L"../../_Assets/" + file;
	this->file = file;

	scene = importer->ReadFile
	(
		String::ToString(this->file),
		// 일반적으로 오른손 좌표계에 열우선을 사용하지만
		// 현재 수업 내용은 왼손좌표계에 행우선을 사용하고 있어서 변경해주는 옵션이다.
		aiProcess_ConvertToLeftHanded
		| aiProcess_Triangulate // 그리는 여러가지 방식중 삼각형을 기준으로 변환해서 달라는 뜻
		| aiProcess_GenUVCoords // 삼각형 단위에 맞게 uv를 변환해달라.
		| aiProcess_GenNormals //  삼각형 그리기 단위에 맞게 normal 벡터를 변환해 달라.
		| aiProcess_CalcTangentSpace // normal mapping할때 사용한다. tangent vector를 구해준다.
	);

	assert(scene != NULL);
}

void ModelConverter::ReadFileDiffMesh(wstring file)
{
	// 파일은 저장할고 쓸 경로
	//this->diffFile = L"../../_Assets/" + file;
	this->diffFile = file;


	diffScene = diffImporter->ReadFile
	(
		String::ToString(this->diffFile),
		// 일반적으로 오른손 좌표계에 열우선을 사용하지만
		// 현재 수업 내용은 왼손좌표계에 행우선을 사용하고 있어서 변경해주는 옵션이다.
		aiProcess_ConvertToLeftHanded
		| aiProcess_Triangulate // 그리는 여러가지 방식중 삼각형을 기준으로 변환해서 달라는 뜻
		| aiProcess_GenUVCoords // 삼각형 단위에 맞게 uv를 변환해달라.
		| aiProcess_GenNormals //  삼각형 그리기 단위에 맞게 normal 벡터를 변환해 달라.
		| aiProcess_CalcTangentSpace // normal mapping할때 사용한다. tangent vector를 구해준다.
	);

	assert(diffScene != NULL);
}

void ModelConverter::ExportMesh(wstring savePath)
{
	// 우리가 저장할 형식은 .mesh 확장좌.
	//savePath = L"../../_Models/" + savePath + L".mesh";


	ReadBoneData(scene->mRootNode, -1, -1);
	ReadSkinData();
	// csv write vertex data chart
	{
		FILE* file;
		string pth = String::ToString(Path::GetFileNameWithoutExtension(savePath));
		pth = "../" + pth + ".csv";
		fopen_s(&file, pth.c_str(), "w");

		for (asBone* bone : bones)
		{
			string name = bone->Name;
			/*if (name.find(":") != string::npos)
			{
				name = name.substr(name.find(":") + 1, -1);
			}*/
			Matrix t = bone->Transform;
			fprintf(file, "%d,%s,", bone->Index, name.c_str());			
			fprintf(file, "%f,%f,%f,", t._41, t._42, t._43);
			Vector3 s, r, p;
			Math::MatrixDecompose(t, s, r, p);
			r = Vector3(Math::ToDegree(r.x), Math::ToDegree(r.y), Math::ToDegree(r.z));
			fprintf(file, "%f,%f,%f\n", r.x, r.y, r.z);

		}

		fprintf(file, "\n");

		for (asMesh* mesh : meshes)
		{
			string name = mesh->Name;
			printf("%s\n", name.c_str());

			for (UINT i = 0; i < mesh->Vertices.size(); i++)
			{
				Vector3 p = mesh->Vertices[i].Position;
				Vector4 indices = mesh->Vertices[i].BlendIndices;
				Vector4 weights = mesh->Vertices[i].BlendWeights;

				fprintf(file, "%f,%f,%f,", p.x, p.y, p.z);
				fprintf(file, "%f,%f,%f,%f,", indices.x, indices.y, indices.z, indices.w);
				fprintf(file, "%f,%f,%f,%f\n", weights.x, weights.y, weights.z, weights.w);
			}
		}

		fclose(file);
	}


	WriteMeshData(savePath);
}

void ModelConverter::ExportStaticMesh(wstring savePath)
{
	// 우리가 저장할 형식은 .mesh 확장좌.
	//savePath = L"../../_Models/" + savePath + L".static";

	ReadStaticMeshData();
	WriteStaticMeshData(savePath);
}

void ModelConverter::ExportMeshDiff(wstring savePath)
{
	//savePath = L"../../_Models/" + savePath + L".mesh";
	DiffReadBoneData(scene->mRootNode, -1, -1);
	ReadSkinData();
	WriteMeshData(savePath);
}

void ModelConverter::ReadBoneData(aiNode * node, int index, int parent)
{
	// 본 정보 일기
	asBone* bone = new asBone();
	bone->Index = index;
	bone->Parent = parent;
	bone->Name = node->mName.C_Str(); // aiString->Const char

	// aiMatrix 4*4 array 시작주소를 참고하여 그대로 복사.	
	Matrix transform(node->mTransformation[0]); // 넘어오는 데이터는 열우선이므로 전치해서 사용한다.
	// 전치행렬
	D3DXMatrixTranspose(&bone->Transform, &transform);

	// 항상 매트릭스의 곱은 왼쪽이 기준이 됩니다.
	// 왼쪽을 기점으로 얼마만큼 이동할지는 오른쪽 곱이 결정합니다.
	Matrix matParent;
	if (parent < 0)// root 노드이다.
		D3DXMatrixIdentity(&matParent);
	else
		matParent = bones[parent]->Transform;


	// 자식의 월드를 부모의 world로 변환한다.
	bone->Transform = bone->Transform * matParent; // relative * Global -> global
	// 본 정보 읽은걸 추가
	bones.push_back(bone); // 다음 노드의 인덱스
	//TODO:: read mesh data 메시 정보 읽기
	ReadMeshData(node, index);

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		// 자신의 인덱스가 자식의 부모가 됩니다.
		// bones.size() : 한번식 데이터를 읽을때 마다 증가하므로 각 bone의 고유의 인덱스가 된다.
		ReadBoneData(node->mChildren[i], bones.size(), index);
	}
}

void ModelConverter::ReadMeshData(aiNode * node, int bone)
{
	if (node->mNumMeshes < 1) return;

	

	// 메쉬 정보 가져오기 ( 한 노드에서 가지고 있는 메쉬 개수 만큼)
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		asMesh* mesh = new asMesh();
		mesh->Name = node->mName.C_Str();
		mesh->BoneIndex = bone;
		// mMeshes에는 정보가 아닌 번호를 가지고 있습니다.
		// 그배열 번호는 씬에 있는 메시 배열의 번호 입니다.
		UINT index = node->mMeshes[i]; //노드의 메쉬 데이터에는 Scenec Mesh 배열의 인덱스 번호가 담겨 있다.
		aiMesh* srcMesh = scene->mMeshes[index];

		// 정점이 0개면 스킵한다.
		if (srcMesh->mNumVertices == 0)
			continue;
		if (srcMesh->mNumFaces == 0)
			continue;

		// 매터리얼도 번호를 가지고 있으므로 씬의 매터리얼 배열에서 값을 가지고 옴
		aiMaterial* material = scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->MaterialName = material->GetName().C_Str();

		// 메시가 여러 개가 되어도 정점을 계속 이어서 저장하려고
		// 이전에 쌓아놓은 크기에서 시작한다.
		UINT startVertex = mesh->Vertices.size();
		for (UINT v = 0; v < srcMesh->mNumVertices; v++)
		{
			Model::ModelVertex vertex;
			// 메시가 존재한다면 Position은 항상 존재
			memcpy(&vertex.Position, &srcMesh->mVertices[v], sizeof(Vector3));
			// 텍스처는 여러개를 가지고 있다 ( 겹쳐서 쓰므로 데칼 레이어 )
			// 게임에서는 레이어를 0번으로 고정해서 쓴다 (일반적으로)
			if (srcMesh->HasTextureCoords(0))
				memcpy(&vertex.Uv, &srcMesh->mTextureCoords[0][v], sizeof(Vector2));
			
			if (srcMesh->HasNormals())
				memcpy(&vertex.Normal, &srcMesh->mNormals[v], sizeof(Vector3));

			if (srcMesh->HasTangentsAndBitangents())
				memcpy(&vertex.Tangent, &srcMesh->mTangents[v], sizeof(Vector3));

			mesh->Vertices.push_back(vertex);
				
		}

		for (UINT f = 0; f < srcMesh->mNumFaces; f++)
		{
			aiFace& face = srcMesh->mFaces[f];

			for (UINT k = 0; k < face.mNumIndices; k++)
			{
				//  0 + 4
				// 012213 -> 456657
				// 왜 start를 더하는가
				mesh->Indices.push_back(face.mIndices[k] + startVertex);
			}
		}
		// txtfile를 byte/binaryfile로 바꿔 읽으면 훨씬 빠ㅡㄹ다.
		//
		meshes.push_back(mesh);
	}
}

void ModelConverter::ReadStaticMeshData()
{
	UINT meshCounts = scene->mNumMeshes;

	for (UINT i = 0; i < meshCounts; i++)
	{
		asStaticMesh* mesh = new asStaticMesh();

		aiMesh* srcMesh = scene->mMeshes[i];

		aiMaterial* material = scene->mMaterials[srcMesh->mMaterialIndex];

		mesh->Name = srcMesh->mName.C_Str();
		mesh->MaterialName = material->GetName().C_Str();

		UINT startVertex = mesh->Vertices.size();
		mesh->Vertices.reserve(srcMesh->mNumVertices);
		for (UINT v = 0; v < srcMesh->mNumVertices; v++)
		{
			StaticMesh::MeshVertex vertex;
			memcpy(&vertex.Position, &srcMesh->mVertices[v], sizeof(Vector3));
			if (srcMesh->HasTextureCoords(0))
				memcpy(&vertex.Uv, &srcMesh->mTextureCoords[0][v], sizeof(Vector2));
			if (srcMesh->HasNormals())
				memcpy(&vertex.Normal, &srcMesh->mNormals[v], sizeof(Vector3));
			if (srcMesh->HasTangentsAndBitangents())
				memcpy(&vertex.Tangent, &srcMesh->mTangents[v], sizeof(Vector3));
			mesh->Vertices.push_back(vertex);
		}
		mesh->Indices.reserve(srcMesh->mNumFaces);
		for (UINT f = 0; f < srcMesh->mNumFaces; f++)
		{
			aiFace& face = srcMesh->mFaces[f];

			for (UINT k = 0; k < face.mNumIndices; k++)
			{
				mesh->Indices.push_back(face.mIndices[k] + startVertex);
			}
		}
		staticMeshes.push_back(mesh);
	}
	

}

void ModelConverter::ReadSkinData()
{
	for (UINT i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* aiMesh = scene->mMeshes[i]; // 메쉬마다 처리
		// 가중치 없는 매쉐도 존재 ( 본이 없기 대문에 )
		if (aiMesh->HasBones() == false) continue;

		asMesh* mesh = meshes[i]; // 메쉬 찾기

		vector<asBoneWeights> boneWeights; // 메쉬의 정점 가져오기 (aiMesh에서 본의 가중치를 받아올 컨테이너)
		boneWeights.assign(mesh->Vertices.size(), asBoneWeights());

		// 각 메쉬에는 영향을 받는 본의 리스트가 존재한다.
		for (UINT b = 0; b < aiMesh->mNumBones; b++) // 메쉬의 연결된 본마다 처리하기
		{
			aiBone* aiMeshBone = aiMesh->mBones[b]; // 메쉬의 본 가져오기

			UINT boneIndex = 0;
			for (asBone* bone : bones) // 모델네 본의 인덱스 찾기 (본의 이름으로 비교한다)
			{
				if (bone->Name == (string)aiMeshBone->mName.C_Str())
				{
					boneIndex = bone->Index;

					break;
				}
			}// for(bone)

			// 메쉬에 연결된 본의  영양력 정보 정점마다 처리하기
			for (UINT w = 0; w < aiMeshBone->mNumWeights; w++) // 
			{
				UINT index = aiMeshBone->mWeights[w].mVertexId; // 정점의 인덱스
				float weight = aiMeshBone->mWeights[w].mWeight; // 정점의 가중치

				// 영향을 받는 정점에 (영향을 주는 본의 인덱스, 가중치 ) (내부적으로 내림차순으로 정렬된다)				
				boneWeights[index].AddWeights(boneIndex, weight);
			}			

		}// for(b)

		// asMesh 데이터내내의 정점의 BonIndex와 BoneWeight 변수를 대입해준다
		// BlendIndices == BoneIndex List
		// BlendWeights == Weight List by BoneIndex Transform
		for (UINT w = 0; w < boneWeights.size(); w++)
		{
			// weight는 0 ~ 1의 범위로 총합은 1이된다.
			boneWeights[i].Normalize();

			asBlendWeight blendWeight;
			boneWeights[w].GetBlendWeights(blendWeight);

			mesh->Vertices[w].BlendIndices = blendWeight.Indices;
			mesh->Vertices[w].BlendWeights = blendWeight.Weights;
		}
	}
}

void ModelConverter::WriteMeshData(wstring savePath)
{
	// CreateFolder :: 해당 경로의 폴더만 만든다.
	// CreateFolders :: 해당 경로까지 상위경로 폴더를 차례로 만들어준다.
	Path::CreateFolders(Path::GetDirectoryName(savePath));

	BinaryWriter* w = new BinaryWriter();	
	w->Open(savePath);

	// 데이터 개수 먼저
	w->UInt(bones.size());
	// string이 없다면 구조체 Byte 단위로 쓰는 것이 훨씬 빠르다.
	for (asBone* bone : bones)
	{
		w->Int(bone->Index);
		// 고정 길이로 만들기도 한다 빠르게 구조체로 일긱 위해서
		w->String(bone->Name);
		w->Int(bone->Parent);
		w->Matrix(bone->Transform);

		SafeDelete(bone);
	}

	w->UInt(meshes.size());
	for(asMesh* meshData : meshes)
	{
		w->String(meshData->Name);
		w->Int(meshData->BoneIndex);

		w->String(meshData->MaterialName);

		w->UInt(meshData->Vertices.size());
		w->Byte(&meshData->Vertices[0], sizeof(Model::ModelVertex) * meshData->Vertices.size());

		w->UInt(meshData->Indices.size());
		w->Byte(&meshData->Indices[0], sizeof(UINT) * meshData->Indices.size());

		SafeDelete(meshData);
	}
	
	w->Close();
	SafeDelete(w);
}

void ModelConverter::WriteStaticMeshData(wstring savePath)
{
	Path::CreateFolders(Path::GetDirectoryName(savePath));

	BinaryWriter* w = new BinaryWriter();
	w->Open(savePath);

	w->UInt(staticMeshes.size());
	for (asStaticMesh* meshData : staticMeshes)
	{
		w->String(meshData->Name);
		w->String(meshData->MaterialName);

		w->UInt(meshData->Vertices.size());
		w->Byte(&meshData->Vertices[0], sizeof(StaticMesh::MeshVertex) * meshData->Vertices.size());

		w->UInt(meshData->Indices.size());
		w->Byte(&meshData->Indices[0], sizeof(UINT) * meshData->Indices.size());

		SafeDelete(meshData);
	}

	w->Close();
	SafeDelete(w);
}

void ModelConverter::DiffReadBoneData(aiNode * node, int index, int parent)
{
	asBone* bone = new asBone();
	bone->Index = index;
	bone->Parent = parent;
	bone->Name = node->mName.C_Str(); // aiString->Const char
	Matrix transform = DiffMatrix(diffScene->mRootNode,bone->Name); // 넘어오는 데이터는 열우선이므로 전치해서 사용한다.
	D3DXMatrixTranspose(&bone->Transform, &transform);
	Matrix matParent;
	if (parent < 0)// root 노드이다.
		D3DXMatrixIdentity(&matParent);
	else
		matParent = bones[parent]->Transform;

	bone->Transform = bone->Transform * matParent; // relative * Global -> global
	bones.push_back(bone); // 다음 노드의 인덱스
	ReadMeshData(node, index);

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		DiffReadBoneData(node->mChildren[i], bones.size(), index);
	}
}

void ModelConverter::DiffReadMeshData(aiNode * node, int bone)
{
}

void ModelConverter::DiffReadSkinData()
{
	for (UINT i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* aiMesh = scene->mMeshes[i]; // 메쉬마다 처리
		// 가중치 없는 매쉐도 존재 ( 본이 없기 대문에 )
		if (aiMesh->HasBones() == false) continue;

		asMesh* mesh = meshes[i]; // 메쉬 찾기

		vector<asBoneWeights> boneWeights; // 메쉬의 정점 가져오기 (aiMesh에서 본의 가중치를 받아올 컨테이너)
		boneWeights.assign(mesh->Vertices.size(), asBoneWeights());

		// 각 메쉬에는 영향을 받는 본의 리스트가 존재한다.
		for (UINT b = 0; b < aiMesh->mNumBones; b++) // 메쉬의 연결된 본마다 처리하기
		{
			aiBone* aiMeshBone = aiMesh->mBones[b]; // 메쉬의 본 가져오기

			UINT boneIndex = 0;
			for (asBone* bone : bones) // 모델네 본의 인덱스 찾기 (본의 이름으로 비교한다)
			{
				if (bone->Name == (string)aiMeshBone->mName.C_Str())
				{
					boneIndex = bone->Index;

					break;
				}
			}// for(bone)

			// 메쉬에 연결된 본의  영양력 정보 정점마다 처리하기
			for (UINT w = 0; w < aiMeshBone->mNumWeights; w++) // 
			{
				UINT index = aiMeshBone->mWeights[w].mVertexId; // 정점의 인덱스
				float weight = aiMeshBone->mWeights[w].mWeight; // 정점의 가중치

				// 영향을 받는 정점에 (영향을 주는 본의 인덱스, 가중치 ) (내부적으로 내림차순으로 정렬된다)				
				boneWeights[index].AddWeights(boneIndex, weight);
			}

		}// for(b)

		// asMesh 데이터내내의 정점의 BonIndex와 BoneWeight 변수를 대입해준다
		// BlendIndices == BoneIndex List
		// BlendWeights == Weight List by BoneIndex Transform
		for (UINT w = 0; w < boneWeights.size(); w++)
		{
			// weight는 0 ~ 1의 범위로 총합은 1이된다.
			boneWeights[i].Normalize();

			asBlendWeight blendWeight;
			boneWeights[w].GetBlendWeights(blendWeight);

			mesh->Vertices[w].BlendIndices = blendWeight.Indices;
			mesh->Vertices[w].BlendWeights = blendWeight.Weights;
		}
	}
}

void ModelConverter::DiffWriteMeshData(wstring savePath)
{
}

Matrix ModelConverter::DiffMatrix(aiNode * node, string name)
{
	Matrix result;
	string cmp = node->mName.C_Str();
	if (cmp == name)
		result = Matrix(node->mTransformation[0]);
	else
		D3DXMatrixIdentity(&result);

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		result *= DiffMatrix(node->mChildren[i], name);
	}
	return result;
}

void ModelConverter::ExportMaterial(wstring savePath, bool bOverwrite)
{
	//savePath = L"../../_Textures/" + savePath + L".material"; // xml formal file == material

	if (bOverwrite == false)
	{
		if (Path::ExistFile(savePath) == true)
			return; // 덮어씌우는걸 방지한다.
	}

	ReadMaterialData();
	WriteMaterialData(savePath);
}

void ModelConverter::ExportMaterialPBR(wstring savePath, bool bOverwrite)
{
	//savePath = L"../../_Textures/" + savePath + L".materialpbr"; // xml formal file == material

	if (bOverwrite == false)
	{
		if (Path::ExistFile(savePath) == true)
			return; // 덮어씌우는걸 방지한다.
	}

	ReadMaterialDataPBR();
	WriteMaterialDataPBR(savePath);
}

void ModelConverter::ReadMaterialData()
{
	for (UINT i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* srcMaterial = scene->mMaterials[i];
		asMaterial* material = new asMaterial();

		material->Name = srcMaterial->GetName().C_Str();

		// rgb == aiColor3D
		aiColor3D color;
		// 매터리얼은 키, 값으로 구성!
		// 키를 이용해 각 정보를 가져온다.
		// AI_MATKEY_COLOR_AMBIENT 함수 인자 매크로로써 실제로는 여러개의 인자를 넘겨주는 것이다.
		// http://assimp.sourceforge.net/lib_html/materials.html
		
		// 빛은 반투명이 없으므로 네번쨰 값을 강도로 대체해서 사용하는 경우가 많다.
		srcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
		material->Ambient = Color(color.r, color.g, color.b, 1.0f);

		srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->Diffuse = Color(color.r, color.g, color.b, 1.0f);

		srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
		material->Specular = Color(color.r, color.g, color.b, 1.0f);

		// 빛의 강도 불러오기
		srcMaterial->Get(AI_MATKEY_SHININESS, material->Specular.a);

		srcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
		material->Emissive = Color(color.r, color.g, color.b, 1.0f);

		// 텍스처 불러오기
		aiString file;

		srcMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
		material->DiffuseFile = file.C_Str();

		srcMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
		material->SpecularFile = file.C_Str();

		srcMaterial->GetTexture(aiTextureType_NORMALS, 0, &file);
		material->NormalFile = file.C_Str();

		materials.push_back(material);
	}

}

void ModelConverter::WriteMaterialData(wstring savePath)
{
	string folder = String::ToString(Path::GetDirectoryName(savePath));
	// Path::GetFileName :: 확장자 포함한 파일명을 가져온다.
	// Path::GetFileNameWithoutExtension : 확장자 제외한 파일명을 가져온다.
	string file = String::ToString(Path::GetFileName(savePath));

	Path::CreateFolders(folder);

	// TinyXML은 namespace가 tinyxml2로 되어 있지만 XML로 바꿔서 사용한다. 
	// xml을 사용하는 시작 형식
	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	for (asMaterial* material : materials)
	{
		
		Xml::XMLElement* node = document->NewElement("Material");// Material이라는 노드를 만든다.
		root->LinkEndChild(node);

		Xml::XMLElement* element = NULL;
		// texture info
		element = document->NewElement("Name");
		element->SetText(material->Name.c_str()); // 다른 자료형도 넣을수 있다.
		
		//element->SetAttribute("R", 10); // particle 같은거 관리할때 또 사용한다.
		node->LinkEndChild(element);

		element = document->NewElement("DiffuseFile");
		element->SetText(WriteTexture(folder, material->DiffuseFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("SpecularFile");
		element->SetText(WriteTexture(folder, material->SpecularFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		element->SetText(WriteTexture(folder, material->NormalFile).c_str());
		node->LinkEndChild(element);

		// 색상 저장하기
		element = document->NewElement("Ambient");
		element->SetAttribute("R", material->Ambient.r);
		element->SetAttribute("G", material->Ambient.g);
		element->SetAttribute("B", material->Ambient.b);
		element->SetAttribute("A", material->Ambient.a);
		node->LinkEndChild(element);

		element = document->NewElement("Diffuse");
		element->SetAttribute("R", material->Diffuse.r);
		element->SetAttribute("G", material->Diffuse.g);
		element->SetAttribute("B", material->Diffuse.b);
		element->SetAttribute("A", material->Diffuse.a);
		node->LinkEndChild(element);

		element = document->NewElement("Specular");
		element->SetAttribute("R", material->Specular.r);
		element->SetAttribute("G", material->Specular.g);
		element->SetAttribute("B", material->Specular.b);
		element->SetAttribute("A", material->Specular.a);
		node->LinkEndChild(element);

		element = document->NewElement("Emissive");
		element->SetAttribute("R", material->Emissive.r);
		element->SetAttribute("G", material->Emissive.g);
		element->SetAttribute("B", material->Emissive.b);
		element->SetAttribute("A", material->Emissive.a);
		node->LinkEndChild(element);

		SafeDelete(material);
	}


	document->SaveFile((folder + file).c_str());
	SafeDelete(document);
}

void ModelConverter::ReadMaterialDataPBR()
{
	for (UINT i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* srcMaterial = scene->mMaterials[i];
		asMaterialPBR* materialPBR = new asMaterialPBR();

		materialPBR->Name = srcMaterial->GetName().C_Str();
		
		aiString file;

		srcMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
		materialPBR->AlbedoFile = file.C_Str();
		//srcMaterial->GetTexture(aiTextureType_METALNESS, 0, &file);
		materialPBR->MetallicFile = "";
		//srcMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &file);
		materialPBR->RoughnessFile = "";
		srcMaterial->GetTexture(aiTextureType_NORMALS, 0, &file);
		//materialPBR->NormalFile = file.C_Str();
		materialPBR->NormalFile = "";
		//srcMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &file);
		materialPBR->AOFile = "";
		srcMaterial->GetTexture(aiTextureType_DISPLACEMENT, 0, &file);
		materialPBR->DisplacementFile = "";

		materialPBRs.push_back(materialPBR);
	}
}

void ModelConverter::WriteMaterialDataPBR(wstring savePath)
{
	string folder = String::ToString(Path::GetDirectoryName(savePath));
	// Path::GetFileName :: 확장자 포함한 파일명을 가져온다.
	// Path::GetFileNameWithoutExtension : 확장자 제외한 파일명을 가져온다.
	string file = String::ToString(Path::GetFileName(savePath));

	Path::CreateFolders(folder);

	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	for (asMaterialPBR* material : materialPBRs)
	{

		Xml::XMLElement* node = document->NewElement("Material");// Material이라는 노드를 만든다.
		root->LinkEndChild(node);

		Xml::XMLElement* element = NULL;
		// texture info
		element = document->NewElement("Name");
		element->SetText(material->Name.c_str()); // 다른 자료형도 넣을수 있다.

		node->LinkEndChild(element);

		element = document->NewElement("AlbedoFile");
		element->SetText(WriteTexture(folder, material->AlbedoFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("MetallicFile");
		element->SetText(WriteTexture(folder, material->MetallicFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("RoughnessFile");
		element->SetText(WriteTexture(folder, material->RoughnessFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		element->SetText(WriteTexture(folder, material->NormalFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("AOFile");
		element->SetText(WriteTexture(folder, material->AOFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("DisplacementFile");
		element->SetText(WriteTexture(folder, material->DisplacementFile).c_str());
		node->LinkEndChild(element);		

		SafeDelete(material);
	}


	document->SaveFile((folder + file).c_str());
	SafeDelete(document);
}

string ModelConverter::WriteTexture(string saveFolder, string file)
{
	// 이미지 파일 생성또는 복사

	// 파일명이 들어오지 않는다면 임의로 파일을 써줘야 한다.
	if (file.length() < 1) return"";

	string fileName = Path::GetFileName(file);
	const aiTexture* texture = scene->GetEmbeddedTexture(file.c_str());



	string path = "";
	if (texture != NULL) // 내장된 연결 텍스처 파일을 찾은 경우
	{
		path = saveFolder + fileName;

		// image has only axis x
		if (texture->mHeight < 1)
		{
			BinaryWriter w;
			w.Open(String::ToWString(path));
			// pcData : file format(byte) * width
			w.Byte(texture->pcData, texture->mWidth);
			w.Close();
		
		}
		// image x, y
		else // create file :: 이러한 케이스는 매우 드물다
		{
			D3D11_TEXTURE2D_DESC destDesc;
			ZeroMemory(&destDesc, sizeof(D3D11_TEXTURE2D_DESC));
			destDesc.Width = texture->mWidth;
			destDesc.Height = texture->mHeight;
			destDesc.MipLevels = 1;
			destDesc.ArraySize = 1;
			destDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			destDesc.SampleDesc.Count = 1;
			destDesc.SampleDesc.Quality = 0;
			destDesc.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_SUBRESOURCE_DATA subResource = { 0 };
			subResource.pSysMem = texture->pcData;


			ID3D11Texture2D* dest;

			HRESULT hr;
			hr = D3D::GetDevice()->CreateTexture2D(&destDesc, &subResource, &dest);
			assert(SUCCEEDED(hr));

			D3DX11SaveTextureToFileA(D3D::GetDC(), dest, D3DX11_IFF_PNG, saveFolder.c_str());
		}

	}
	else // 연결 테스처가 외부에 별도 존재하는 경우
	{
		// 외부에 fileName으로 존재하는지 찾아보고 없으면 빈문자열로 처리한다
		string directory = Path::GetDirectoryName(String::ToString(this->file));
		string origin = directory + file;
		String::Replace(&origin, "\\", "/");

		if (Path::ExistFile(origin) == false)
			return "";

		path = saveFolder + fileName;
		CopyFileA(origin.c_str(), path.c_str(), FALSE);
		// 읽을때 ../../_Textures/는 자동으로 합쳐서 탐색하므로 경로에서 제거해서 저장한다.
		String::Replace(&path, "../../_Textures/", "");
	}

	//write한 fileName
	return Path::GetFileName(path);
}

void ModelConverter::ClipList(vector<wstring>* list)
{
	for (UINT i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* anim = scene->mAnimations[i];

		list->push_back(String::ToWString(anim->mName.C_Str()));
	}
}

void ModelConverter::ExportAnimClip(UINT index, wstring savePath)
{
	//savePath = L"../../_Models/" + savePath + L".clip";
	
	asClip* clip = ReadClipData(scene->mAnimations[index]);
	WriteClipData(clip, savePath);
}

asClip * ModelConverter::ReadClipData(aiAnimation * animation)
{
	// 클립저장
	asClip* clip = new asClip();
	clip->Name = animation->mName.C_Str();
	clip->FrameRate = (float)animation->mTicksPerSecond; // 프레임 비율 30frame
	clip->FrameCount = (UINT)animation->mDuration + 1; // float 형이라서 올림 +1 해주었다.

	// SRT data save ant Time
	vector<asClipNode> aniNodeInfos;
	// channel이 bone의 개념이다. channel을 통해 bone을 찾는다.
	for (UINT i = 0; i < animation->mNumChannels; i++)
	{
		aiNodeAnim* aniNode = animation->mChannels[i];
		
		asClipNode aniNodeInfo;
		// 본의 이름을 저장한다.
		aniNodeInfo.Name = aniNode->mNodeName;

		clipChannelNames.push_back(aniNodeInfo.Name.C_Str());

		// 변하는 프레임이 아닌 경오 따로 저장되어 있지 않기 때문에
		// 값이 변하기 전까지의 값을 계산해서 저장한다.
		// 가장 큰 변화갯수를 찾는다 ( position rotation scale 중에서 )
		UINT keyCount = max(aniNode->mNumPositionKeys, aniNode->mNumScalingKeys);
		keyCount = max(keyCount, aniNode->mNumRotationKeys);

		for (UINT k = 0; k < keyCount; k++)
		{
			asKeyframeData frameData;
			bool bFound = false;
			UINT t = aniNodeInfo.Keyframe.size();

			// 2byte float epsilon 0으로 가까우면 0으로 취급해라 le-6
			if (fabsf((float)aniNode->mPositionKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{
				// 시간 과 위치 값을 가지고 있음
				aiVectorKey key = aniNode->mPositionKeys[k];
				frameData.Time = (float)key.mTime;
				memcpy_s(&frameData.Translation, sizeof(Vector3), &key.mValue, sizeof(aiVector3D));

				bFound = true;
			}

			if (fabsf((float)aniNode->mRotationKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{
				aiQuatKey key = aniNode->mRotationKeys[k];
				frameData.Time = (float)key.mTime;
				// DX Queturnion memory [ x y z w ]
				// Assimp Queturnion memory [ w x y z ]
				// w x y z으로 되어 있어서
				// DX와 Assimp의 메모리 순서가 다르기 때문에 직접 대입해서 사용한다.
				frameData.Rotation.x = key.mValue.x;
				frameData.Rotation.y = key.mValue.y;
				frameData.Rotation.z = key.mValue.z;
				frameData.Rotation.w = key.mValue.w;

				bFound = true;
			}


			// 애니메이션에서는 회전을 쿼터니온을 사용합니다.
			// 연산량이 적고 ( 각 각 각 비율 ) // 짐벌락이 해결된다.
			if (fabsf((float)aniNode->mScalingKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{
				// 시간 과 위치 값을 가지고 있음
				aiVectorKey key = aniNode->mScalingKeys[k];
				frameData.Time = (float)key.mTime;
				memcpy_s(&frameData.Scale, sizeof(Vector3), &key.mValue, sizeof(aiVector3D));

				bFound = true;
			}

			if (bFound == true)
			{
				aniNodeInfo.Keyframe.push_back(frameData);
			}

		} // for (k)

		if (aniNodeInfo.Keyframe.size() < clip->FrameCount) // 원래 clip길이보다 keyframe 길이가 짧다면 뒤를 채워준다.
		{
			UINT count = clip->FrameCount - aniNodeInfo.Keyframe.size();
			asKeyframeData keyFrame = aniNodeInfo.Keyframe.back(); // 마지막 프레임으로 채워준다.

			for (UINT n = 0; n < count; n++)
			{
				aniNodeInfo.Keyframe.push_back(keyFrame);
			}
		} // full emptyframe Data

		clip->Duration = max(clip->Duration, aniNodeInfo.Keyframe.back().Time);

		aniNodeInfos.push_back(aniNodeInfo); // 채널별로 keyframe 데이터 저장
	} // for(channel)

	// 모든 본을 돌면서 애니메이션 channel과 매칭하여 저장한다.
	ReadKeyframeData(clip, scene->mRootNode, aniNodeInfos);

	return clip;
}

// 불러와 저장할 keyframe 데이터 매칭하기
// asClip : 최종 저장할 파일의 데이터 -> WriteClipData
// 모델과 리타겟팅은 여기 함수에서 해주면 된다.
// TODO:: 모델과 애니메이션 파일 리타겟팅
// 재귀를 타면선 모든 본에 대해서 애니메이션 tranform 데이터를 매칭시켜서 저장한다.
void ModelConverter::ReadKeyframeData(asClip * clip, aiNode * node, vector<struct asClipNode>& aiNodeInfos)
{
	asKeyframe* keyframe = new asKeyframe();
	keyframe->BoneName = node->mName.C_Str();
	// 모델의 bone과 animation bone을 일치시키기
	// bone name이 다를 경우는 리타겟팅 해주어야 한다.

	clipBoneNames.push_back(node->mName.C_Str());
	
	for (UINT i = 0; i < clip->FrameCount; i++) // 현재 클립 데이터와 node 비교하여 tranform 데이터 넣어주기
	{
		asClipNode* asClipNode = NULL;

		
		for (UINT n = 0; n < aiNodeInfos.size(); n++) 
		{
			// anis : aiNodeInfos[n].Name :: 애니메이션의 본이름
			// bs : node->mName :: 모델의 본이름
			string ClipBoneName = aiNodeInfos[n].Name.C_Str();
			string ModelBoneName = node->mName.C_Str();		
			
			
			if (ClipBoneName == ModelBoneName)
			{
				asClipNode = &aiNodeInfos[n];
				break;
			}
		} // for(n) 맞는 이름이 존재하는지 찾는다.

		// 본의 이름이 있을 경우는 해당 데이터를 처리며
		// 없으면 노드의 본 정보를 그대로 사용한다.
		asKeyframeData frameData;
		if (asClipNode == NULL) // 모델의 본 트랜스폼을 그래도 사용한다.
		{
			// 모델 본의 transform을 그대로 사용한다.
			Matrix transform(node->mTransformation[0]); // nodeT * ParentT
			D3DXMatrixTranspose(&transform, &transform); // 열 우선이므로 전치해서 사용한다.
			
			frameData.Time = (float)i;
			D3DXMatrixDecompose(&frameData.Scale, &frameData.Rotation, &frameData.Translation, &transform);
		}
		else // 클립의 트랜스폼을 그래도 사용한다.
		{
			frameData = asClipNode->Keyframe[i];
		}

		keyframe->Transforms.push_back(frameData);
	}

	//model 본과 타게팅이 완성된 keyframe 데이터를 clip에 넣어 준다.
	clip->Keyframes.push_back(keyframe);

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		ReadKeyframeData(clip, node->mChildren[i], aiNodeInfos);
	}
}

void ModelConverter::WriteClipData(asClip * clip, wstring savePath)
{
	Path::CreateFolders(Path::GetDirectoryName(savePath));

	BinaryWriter* w = new BinaryWriter();
	w->Open(savePath);

	w->String(clip->Name);
	w->Float(clip->Duration);
	w->Float(clip->FrameRate);
	w->UInt(clip->FrameCount);

	w->UInt(clip->Keyframes.size());

	// write clip info
	FILE* file;
	string pth  = String::ToString(Path::GetFileNameWithoutExtension(savePath));
	pth = "../" + pth + ".csv";
	fopen_s(&file, pth.c_str(), "w");
	for (asKeyframe* keyframe : clip->Keyframes)
	{
		fprintf(file, "%s,", keyframe->BoneName.c_str());
		for (UINT i = 0; i < keyframe->Transforms.size(); i++)
		{
			auto k = keyframe->Transforms[i];
			Vector3 s = k.Scale;
			Vector3 t = k.Translation;
			fprintf(file, "%f,%f,%f,",s.x,s.y,s.z);
			fprintf(file, "%f,%f,%f,",t.x,t.y,t.z);
		}
		fprintf(file, "\n");
	}
	fclose(file);

	for (asKeyframe* keyframe : clip->Keyframes)
	{
		w->String(keyframe->BoneName);
		w->UInt(keyframe->Transforms.size());
		w->Byte(&keyframe->Transforms[0], sizeof(asKeyframeData) * keyframe->Transforms.size());

		
		SafeDelete(keyframe);
	}

	
	w->Close();
	SafeDelete(w);
}
