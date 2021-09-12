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
	// ������ �����Ұ� �� ���
	//this->file = L"../../_Assets/" + file;
	this->file = file;

	scene = importer->ReadFile
	(
		String::ToString(this->file),
		// �Ϲ������� ������ ��ǥ�迡 ���켱�� ���������
		// ���� ���� ������ �޼���ǥ�迡 ��켱�� ����ϰ� �־ �������ִ� �ɼ��̴�.
		aiProcess_ConvertToLeftHanded
		| aiProcess_Triangulate // �׸��� �������� ����� �ﰢ���� �������� ��ȯ�ؼ� �޶�� ��
		| aiProcess_GenUVCoords // �ﰢ�� ������ �°� uv�� ��ȯ�ش޶�.
		| aiProcess_GenNormals //  �ﰢ�� �׸��� ������ �°� normal ���͸� ��ȯ�� �޶�.
		| aiProcess_CalcTangentSpace // normal mapping�Ҷ� ����Ѵ�. tangent vector�� �����ش�.
	);

	assert(scene != NULL);
}

void ModelConverter::ReadFileDiffMesh(wstring file)
{
	// ������ �����Ұ� �� ���
	//this->diffFile = L"../../_Assets/" + file;
	this->diffFile = file;


	diffScene = diffImporter->ReadFile
	(
		String::ToString(this->diffFile),
		// �Ϲ������� ������ ��ǥ�迡 ���켱�� ���������
		// ���� ���� ������ �޼���ǥ�迡 ��켱�� ����ϰ� �־ �������ִ� �ɼ��̴�.
		aiProcess_ConvertToLeftHanded
		| aiProcess_Triangulate // �׸��� �������� ����� �ﰢ���� �������� ��ȯ�ؼ� �޶�� ��
		| aiProcess_GenUVCoords // �ﰢ�� ������ �°� uv�� ��ȯ�ش޶�.
		| aiProcess_GenNormals //  �ﰢ�� �׸��� ������ �°� normal ���͸� ��ȯ�� �޶�.
		| aiProcess_CalcTangentSpace // normal mapping�Ҷ� ����Ѵ�. tangent vector�� �����ش�.
	);

	assert(diffScene != NULL);
}

void ModelConverter::ExportMesh(wstring savePath)
{
	// �츮�� ������ ������ .mesh Ȯ����.
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
	// �츮�� ������ ������ .mesh Ȯ����.
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
	// �� ���� �ϱ�
	asBone* bone = new asBone();
	bone->Index = index;
	bone->Parent = parent;
	bone->Name = node->mName.C_Str(); // aiString->Const char

	// aiMatrix 4*4 array �����ּҸ� �����Ͽ� �״�� ����.	
	Matrix transform(node->mTransformation[0]); // �Ѿ���� �����ʹ� ���켱�̹Ƿ� ��ġ�ؼ� ����Ѵ�.
	// ��ġ���
	D3DXMatrixTranspose(&bone->Transform, &transform);

	// �׻� ��Ʈ������ ���� ������ ������ �˴ϴ�.
	// ������ �������� �󸶸�ŭ �̵������� ������ ���� �����մϴ�.
	Matrix matParent;
	if (parent < 0)// root ����̴�.
		D3DXMatrixIdentity(&matParent);
	else
		matParent = bones[parent]->Transform;


	// �ڽ��� ���带 �θ��� world�� ��ȯ�Ѵ�.
	bone->Transform = bone->Transform * matParent; // relative * Global -> global
	// �� ���� ������ �߰�
	bones.push_back(bone); // ���� ����� �ε���
	//TODO:: read mesh data �޽� ���� �б�
	ReadMeshData(node, index);

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		// �ڽ��� �ε����� �ڽ��� �θ� �˴ϴ�.
		// bones.size() : �ѹ��� �����͸� ������ ���� �����ϹǷ� �� bone�� ������ �ε����� �ȴ�.
		ReadBoneData(node->mChildren[i], bones.size(), index);
	}
}

void ModelConverter::ReadMeshData(aiNode * node, int bone)
{
	if (node->mNumMeshes < 1) return;

	

	// �޽� ���� �������� ( �� ��忡�� ������ �ִ� �޽� ���� ��ŭ)
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		asMesh* mesh = new asMesh();
		mesh->Name = node->mName.C_Str();
		mesh->BoneIndex = bone;
		// mMeshes���� ������ �ƴ� ��ȣ�� ������ �ֽ��ϴ�.
		// �׹迭 ��ȣ�� ���� �ִ� �޽� �迭�� ��ȣ �Դϴ�.
		UINT index = node->mMeshes[i]; //����� �޽� �����Ϳ��� Scenec Mesh �迭�� �ε��� ��ȣ�� ��� �ִ�.
		aiMesh* srcMesh = scene->mMeshes[index];

		// ������ 0���� ��ŵ�Ѵ�.
		if (srcMesh->mNumVertices == 0)
			continue;
		if (srcMesh->mNumFaces == 0)
			continue;

		// ���͸��� ��ȣ�� ������ �����Ƿ� ���� ���͸��� �迭���� ���� ������ ��
		aiMaterial* material = scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->MaterialName = material->GetName().C_Str();

		// �޽ð� ���� ���� �Ǿ ������ ��� �̾ �����Ϸ���
		// ������ �׾Ƴ��� ũ�⿡�� �����Ѵ�.
		UINT startVertex = mesh->Vertices.size();
		for (UINT v = 0; v < srcMesh->mNumVertices; v++)
		{
			Model::ModelVertex vertex;
			// �޽ð� �����Ѵٸ� Position�� �׻� ����
			memcpy(&vertex.Position, &srcMesh->mVertices[v], sizeof(Vector3));
			// �ؽ�ó�� �������� ������ �ִ� ( ���ļ� ���Ƿ� ��Į ���̾� )
			// ���ӿ����� ���̾ 0������ �����ؼ� ���� (�Ϲ�������)
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
				// �� start�� ���ϴ°�
				mesh->Indices.push_back(face.mIndices[k] + startVertex);
			}
		}
		// txtfile�� byte/binaryfile�� �ٲ� ������ �ξ� ���Ѥ���.
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
		aiMesh* aiMesh = scene->mMeshes[i]; // �޽����� ó��
		// ����ġ ���� �Ž��� ���� ( ���� ���� �빮�� )
		if (aiMesh->HasBones() == false) continue;

		asMesh* mesh = meshes[i]; // �޽� ã��

		vector<asBoneWeights> boneWeights; // �޽��� ���� �������� (aiMesh���� ���� ����ġ�� �޾ƿ� �����̳�)
		boneWeights.assign(mesh->Vertices.size(), asBoneWeights());

		// �� �޽����� ������ �޴� ���� ����Ʈ�� �����Ѵ�.
		for (UINT b = 0; b < aiMesh->mNumBones; b++) // �޽��� ����� ������ ó���ϱ�
		{
			aiBone* aiMeshBone = aiMesh->mBones[b]; // �޽��� �� ��������

			UINT boneIndex = 0;
			for (asBone* bone : bones) // �𵨳� ���� �ε��� ã�� (���� �̸����� ���Ѵ�)
			{
				if (bone->Name == (string)aiMeshBone->mName.C_Str())
				{
					boneIndex = bone->Index;

					break;
				}
			}// for(bone)

			// �޽��� ����� ����  ����� ���� �������� ó���ϱ�
			for (UINT w = 0; w < aiMeshBone->mNumWeights; w++) // 
			{
				UINT index = aiMeshBone->mWeights[w].mVertexId; // ������ �ε���
				float weight = aiMeshBone->mWeights[w].mWeight; // ������ ����ġ

				// ������ �޴� ������ (������ �ִ� ���� �ε���, ����ġ ) (���������� ������������ ���ĵȴ�)				
				boneWeights[index].AddWeights(boneIndex, weight);
			}			

		}// for(b)

		// asMesh �����ͳ����� ������ BonIndex�� BoneWeight ������ �������ش�
		// BlendIndices == BoneIndex List
		// BlendWeights == Weight List by BoneIndex Transform
		for (UINT w = 0; w < boneWeights.size(); w++)
		{
			// weight�� 0 ~ 1�� ������ ������ 1�̵ȴ�.
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
	// CreateFolder :: �ش� ����� ������ �����.
	// CreateFolders :: �ش� ��α��� ������� ������ ���ʷ� ������ش�.
	Path::CreateFolders(Path::GetDirectoryName(savePath));

	BinaryWriter* w = new BinaryWriter();	
	w->Open(savePath);

	// ������ ���� ����
	w->UInt(bones.size());
	// string�� ���ٸ� ����ü Byte ������ ���� ���� �ξ� ������.
	for (asBone* bone : bones)
	{
		w->Int(bone->Index);
		// ���� ���̷� ����⵵ �Ѵ� ������ ����ü�� �ϱ� ���ؼ�
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
	Matrix transform = DiffMatrix(diffScene->mRootNode,bone->Name); // �Ѿ���� �����ʹ� ���켱�̹Ƿ� ��ġ�ؼ� ����Ѵ�.
	D3DXMatrixTranspose(&bone->Transform, &transform);
	Matrix matParent;
	if (parent < 0)// root ����̴�.
		D3DXMatrixIdentity(&matParent);
	else
		matParent = bones[parent]->Transform;

	bone->Transform = bone->Transform * matParent; // relative * Global -> global
	bones.push_back(bone); // ���� ����� �ε���
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
		aiMesh* aiMesh = scene->mMeshes[i]; // �޽����� ó��
		// ����ġ ���� �Ž��� ���� ( ���� ���� �빮�� )
		if (aiMesh->HasBones() == false) continue;

		asMesh* mesh = meshes[i]; // �޽� ã��

		vector<asBoneWeights> boneWeights; // �޽��� ���� �������� (aiMesh���� ���� ����ġ�� �޾ƿ� �����̳�)
		boneWeights.assign(mesh->Vertices.size(), asBoneWeights());

		// �� �޽����� ������ �޴� ���� ����Ʈ�� �����Ѵ�.
		for (UINT b = 0; b < aiMesh->mNumBones; b++) // �޽��� ����� ������ ó���ϱ�
		{
			aiBone* aiMeshBone = aiMesh->mBones[b]; // �޽��� �� ��������

			UINT boneIndex = 0;
			for (asBone* bone : bones) // �𵨳� ���� �ε��� ã�� (���� �̸����� ���Ѵ�)
			{
				if (bone->Name == (string)aiMeshBone->mName.C_Str())
				{
					boneIndex = bone->Index;

					break;
				}
			}// for(bone)

			// �޽��� ����� ����  ����� ���� �������� ó���ϱ�
			for (UINT w = 0; w < aiMeshBone->mNumWeights; w++) // 
			{
				UINT index = aiMeshBone->mWeights[w].mVertexId; // ������ �ε���
				float weight = aiMeshBone->mWeights[w].mWeight; // ������ ����ġ

				// ������ �޴� ������ (������ �ִ� ���� �ε���, ����ġ ) (���������� ������������ ���ĵȴ�)				
				boneWeights[index].AddWeights(boneIndex, weight);
			}

		}// for(b)

		// asMesh �����ͳ����� ������ BonIndex�� BoneWeight ������ �������ش�
		// BlendIndices == BoneIndex List
		// BlendWeights == Weight List by BoneIndex Transform
		for (UINT w = 0; w < boneWeights.size(); w++)
		{
			// weight�� 0 ~ 1�� ������ ������ 1�̵ȴ�.
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
			return; // �����°� �����Ѵ�.
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
			return; // �����°� �����Ѵ�.
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
		// ���͸����� Ű, ������ ����!
		// Ű�� �̿��� �� ������ �����´�.
		// AI_MATKEY_COLOR_AMBIENT �Լ� ���� ��ũ�ην� �����δ� �������� ���ڸ� �Ѱ��ִ� ���̴�.
		// http://assimp.sourceforge.net/lib_html/materials.html
		
		// ���� �������� �����Ƿ� �׹��� ���� ������ ��ü�ؼ� ����ϴ� ��찡 ����.
		srcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
		material->Ambient = Color(color.r, color.g, color.b, 1.0f);

		srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->Diffuse = Color(color.r, color.g, color.b, 1.0f);

		srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
		material->Specular = Color(color.r, color.g, color.b, 1.0f);

		// ���� ���� �ҷ�����
		srcMaterial->Get(AI_MATKEY_SHININESS, material->Specular.a);

		srcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
		material->Emissive = Color(color.r, color.g, color.b, 1.0f);

		// �ؽ�ó �ҷ�����
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
	// Path::GetFileName :: Ȯ���� ������ ���ϸ��� �����´�.
	// Path::GetFileNameWithoutExtension : Ȯ���� ������ ���ϸ��� �����´�.
	string file = String::ToString(Path::GetFileName(savePath));

	Path::CreateFolders(folder);

	// TinyXML�� namespace�� tinyxml2�� �Ǿ� ������ XML�� �ٲ㼭 ����Ѵ�. 
	// xml�� ����ϴ� ���� ����
	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	for (asMaterial* material : materials)
	{
		
		Xml::XMLElement* node = document->NewElement("Material");// Material�̶�� ��带 �����.
		root->LinkEndChild(node);

		Xml::XMLElement* element = NULL;
		// texture info
		element = document->NewElement("Name");
		element->SetText(material->Name.c_str()); // �ٸ� �ڷ����� ������ �ִ�.
		
		//element->SetAttribute("R", 10); // particle ������ �����Ҷ� �� ����Ѵ�.
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

		// ���� �����ϱ�
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
	// Path::GetFileName :: Ȯ���� ������ ���ϸ��� �����´�.
	// Path::GetFileNameWithoutExtension : Ȯ���� ������ ���ϸ��� �����´�.
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

		Xml::XMLElement* node = document->NewElement("Material");// Material�̶�� ��带 �����.
		root->LinkEndChild(node);

		Xml::XMLElement* element = NULL;
		// texture info
		element = document->NewElement("Name");
		element->SetText(material->Name.c_str()); // �ٸ� �ڷ����� ������ �ִ�.

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
	// �̹��� ���� �����Ǵ� ����

	// ���ϸ��� ������ �ʴ´ٸ� ���Ƿ� ������ ����� �Ѵ�.
	if (file.length() < 1) return"";

	string fileName = Path::GetFileName(file);
	const aiTexture* texture = scene->GetEmbeddedTexture(file.c_str());



	string path = "";
	if (texture != NULL) // ����� ���� �ؽ�ó ������ ã�� ���
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
		else // create file :: �̷��� ���̽��� �ſ� �幰��
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
	else // ���� �׽�ó�� �ܺο� ���� �����ϴ� ���
	{
		// �ܺο� fileName���� �����ϴ��� ã�ƺ��� ������ ���ڿ��� ó���Ѵ�
		string directory = Path::GetDirectoryName(String::ToString(this->file));
		string origin = directory + file;
		String::Replace(&origin, "\\", "/");

		if (Path::ExistFile(origin) == false)
			return "";

		path = saveFolder + fileName;
		CopyFileA(origin.c_str(), path.c_str(), FALSE);
		// ������ ../../_Textures/�� �ڵ����� ���ļ� Ž���ϹǷ� ��ο��� �����ؼ� �����Ѵ�.
		String::Replace(&path, "../../_Textures/", "");
	}

	//write�� fileName
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
	// Ŭ������
	asClip* clip = new asClip();
	clip->Name = animation->mName.C_Str();
	clip->FrameRate = (float)animation->mTicksPerSecond; // ������ ���� 30frame
	clip->FrameCount = (UINT)animation->mDuration + 1; // float ���̶� �ø� +1 ���־���.

	// SRT data save ant Time
	vector<asClipNode> aniNodeInfos;
	// channel�� bone�� �����̴�. channel�� ���� bone�� ã�´�.
	for (UINT i = 0; i < animation->mNumChannels; i++)
	{
		aiNodeAnim* aniNode = animation->mChannels[i];
		
		asClipNode aniNodeInfo;
		// ���� �̸��� �����Ѵ�.
		aniNodeInfo.Name = aniNode->mNodeName;

		clipChannelNames.push_back(aniNodeInfo.Name.C_Str());

		// ���ϴ� �������� �ƴ� ��� ���� ����Ǿ� ���� �ʱ� ������
		// ���� ���ϱ� �������� ���� ����ؼ� �����Ѵ�.
		// ���� ū ��ȭ������ ã�´� ( position rotation scale �߿��� )
		UINT keyCount = max(aniNode->mNumPositionKeys, aniNode->mNumScalingKeys);
		keyCount = max(keyCount, aniNode->mNumRotationKeys);

		for (UINT k = 0; k < keyCount; k++)
		{
			asKeyframeData frameData;
			bool bFound = false;
			UINT t = aniNodeInfo.Keyframe.size();

			// 2byte float epsilon 0���� ������ 0���� ����ض� le-6
			if (fabsf((float)aniNode->mPositionKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{
				// �ð� �� ��ġ ���� ������ ����
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
				// w x y z���� �Ǿ� �־
				// DX�� Assimp�� �޸� ������ �ٸ��� ������ ���� �����ؼ� ����Ѵ�.
				frameData.Rotation.x = key.mValue.x;
				frameData.Rotation.y = key.mValue.y;
				frameData.Rotation.z = key.mValue.z;
				frameData.Rotation.w = key.mValue.w;

				bFound = true;
			}


			// �ִϸ��̼ǿ����� ȸ���� ���ʹϿ��� ����մϴ�.
			// ���귮�� ���� ( �� �� �� ���� ) // �������� �ذ�ȴ�.
			if (fabsf((float)aniNode->mScalingKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{
				// �ð� �� ��ġ ���� ������ ����
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

		if (aniNodeInfo.Keyframe.size() < clip->FrameCount) // ���� clip���̺��� keyframe ���̰� ª�ٸ� �ڸ� ä���ش�.
		{
			UINT count = clip->FrameCount - aniNodeInfo.Keyframe.size();
			asKeyframeData keyFrame = aniNodeInfo.Keyframe.back(); // ������ ���������� ä���ش�.

			for (UINT n = 0; n < count; n++)
			{
				aniNodeInfo.Keyframe.push_back(keyFrame);
			}
		} // full emptyframe Data

		clip->Duration = max(clip->Duration, aniNodeInfo.Keyframe.back().Time);

		aniNodeInfos.push_back(aniNodeInfo); // ä�κ��� keyframe ������ ����
	} // for(channel)

	// ��� ���� ���鼭 �ִϸ��̼� channel�� ��Ī�Ͽ� �����Ѵ�.
	ReadKeyframeData(clip, scene->mRootNode, aniNodeInfos);

	return clip;
}

// �ҷ��� ������ keyframe ������ ��Ī�ϱ�
// asClip : ���� ������ ������ ������ -> WriteClipData
// �𵨰� ��Ÿ������ ���� �Լ����� ���ָ� �ȴ�.
// TODO:: �𵨰� �ִϸ��̼� ���� ��Ÿ����
// ��͸� Ÿ�鼱 ��� ���� ���ؼ� �ִϸ��̼� tranform �����͸� ��Ī���Ѽ� �����Ѵ�.
void ModelConverter::ReadKeyframeData(asClip * clip, aiNode * node, vector<struct asClipNode>& aiNodeInfos)
{
	asKeyframe* keyframe = new asKeyframe();
	keyframe->BoneName = node->mName.C_Str();
	// ���� bone�� animation bone�� ��ġ��Ű��
	// bone name�� �ٸ� ���� ��Ÿ���� ���־�� �Ѵ�.

	clipBoneNames.push_back(node->mName.C_Str());
	
	for (UINT i = 0; i < clip->FrameCount; i++) // ���� Ŭ�� �����Ϳ� node ���Ͽ� tranform ������ �־��ֱ�
	{
		asClipNode* asClipNode = NULL;

		
		for (UINT n = 0; n < aiNodeInfos.size(); n++) 
		{
			// anis : aiNodeInfos[n].Name :: �ִϸ��̼��� ���̸�
			// bs : node->mName :: ���� ���̸�
			string ClipBoneName = aiNodeInfos[n].Name.C_Str();
			string ModelBoneName = node->mName.C_Str();		
			
			
			if (ClipBoneName == ModelBoneName)
			{
				asClipNode = &aiNodeInfos[n];
				break;
			}
		} // for(n) �´� �̸��� �����ϴ��� ã�´�.

		// ���� �̸��� ���� ���� �ش� �����͸� ó����
		// ������ ����� �� ������ �״�� ����Ѵ�.
		asKeyframeData frameData;
		if (asClipNode == NULL) // ���� �� Ʈ�������� �׷��� ����Ѵ�.
		{
			// �� ���� transform�� �״�� ����Ѵ�.
			Matrix transform(node->mTransformation[0]); // nodeT * ParentT
			D3DXMatrixTranspose(&transform, &transform); // �� �켱�̹Ƿ� ��ġ�ؼ� ����Ѵ�.
			
			frameData.Time = (float)i;
			D3DXMatrixDecompose(&frameData.Scale, &frameData.Rotation, &frameData.Translation, &transform);
		}
		else // Ŭ���� Ʈ�������� �׷��� ����Ѵ�.
		{
			frameData = asClipNode->Keyframe[i];
		}

		keyframe->Transforms.push_back(frameData);
	}

	//model ���� Ÿ������ �ϼ��� keyframe �����͸� clip�� �־� �ش�.
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
