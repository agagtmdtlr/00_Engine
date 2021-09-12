#include "Framework.h"
#include "Utilities/BinaryFile.h"
#include "Utilities/Xml.h"
#include "StaticModelRender.h"

StaticModelRender::StaticModelRender(Shader * shader)
	: shader(shader)
{
	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]);

	instanceBuffer = new VertexBuffer(worlds, MAX_MODEL_INSTANCE, sizeof(Matrix), 1, true);
}

StaticModelRender::~StaticModelRender()
{
	for (Transform* transform : transforms) // instance transform
		SafeDelete(transform);

	SafeDelete(instanceBuffer);
}

void StaticModelRender::Update()
{
	if (bInitialized == false)
	{
		for (StaticMesh* mesh : meshes)
			mesh->SetShader(shader);

		for (MaterialPBR* mat : materials)
			mat->SetShader(shader);

		bInitialized = true;
	}

	for (StaticMesh* mesh : meshes)
	{
		mesh->Update();
	}
}

void StaticModelRender::Render()
{
	instanceBuffer->Render();
	for (StaticMesh* mesh : meshes)
	{
		UINT instanceCount = transforms.size();

		MaterialRender(mesh->materialName);
		mesh->Render(instanceCount);
	}
}

void StaticModelRender::MaterialRender(wstring name)
{
	for (UINT i = 0; i < materials.size(); i++)
	{
		if (materials[i]->Name() == name)
		{
			materials[i]->Render();
			return;
		}
	}
}

void StaticModelRender::ReadMesh(wstring file)
{
	ReadMode2(file);
}

void StaticModelRender::ReadMaterial(wstring file)
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

}

void StaticModelRender::Pass(UINT pass)
{
	for (StaticMesh* m : meshes)
	{
		m->Pass(pass);
	}
}

void StaticModelRender::Topology(D3D11_PRIMITIVE_TOPOLOGY t)
{
	for (StaticMesh*m : meshes)
	{
		m->Topology(t);
	}
}

// transform을 갱신한다.
void StaticModelRender::UpdateTransform(UINT instanceId, Transform& transform)
{
	transforms[instanceId]->World(transform.World());
}


Transform * StaticModelRender::AddTransform()
{
	Transform* transform = new Transform();
	transforms.push_back(transform); // instance data 에 넣어준다 

	return transform;
}

// UpdateInstanceBuffer
void StaticModelRender::UpdateTransforms()
{
	// 버퍼데이터에 복사해준다.
	for (UINT i = 0; i < transforms.size(); i++)
		memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix));

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, worlds, sizeof(Matrix) * MAX_MODEL_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
}

void StaticModelRender::ReadMode(wstring file)
{
	file = L"../../_Models/" + file + L".static";
	BinaryReader* r = new BinaryReader();
	r->Open(file);

	UINT count = 0;
	count = r->UInt();

	vector<vector<StaticMesh::MeshVertex>> vecVertexVec;
	vector<vector<UINT>> vecIndexVec;
	vecVertexVec.assign(count, vector<StaticMesh::MeshVertex>());
	vecIndexVec.assign(count, vector<UINT>());

	UINT totalv = 0;
	UINT totali = 0;

	StaticMesh* mesh = new StaticMesh();
	for (UINT i = 0; i < count; i++)
	{
		mesh->name = String::ToWString(r->String());
		mesh->materialName = String::ToWString(r->String());
		{
			UINT count = r->UInt();

			totalv += count;

			//vector<StaticMesh::MeshVertex> vertices;
			vector<StaticMesh::MeshVertex> & vertices = vecVertexVec[i];
			vertices.assign(count, StaticMesh::MeshVertex());

			void* ptr = (void*)&(vertices[0]);
			r->Byte(&ptr, sizeof(StaticMesh::MeshVertex) * count);

		}
		{
			UINT count = r->UInt();


			//vector<UINT> indices;
			vector<UINT> & indices = vecIndexVec[i];
			indices.assign(count, UINT());

			void* ptr = (void *)&(indices[0]);
			r->Byte(&ptr, sizeof(UINT) * count);

			for (UINT j = 0; j < count; j++)
			{
				indices[j] += totali;
			}

			totali += count;
		}
	}

	UINT vcount = 0;
	UINT icount = 0;

	mesh->vertices = new StaticMesh::MeshVertex[totalv];
	mesh->indices = new UINT[totali];

	mesh->vertexCount = totalv;
	mesh->indexCount = totali;

	for (UINT i = 0; i < vecVertexVec.size(); i++)
	{
		UINT vsize = vecVertexVec[i].size();
		UINT isize = vecIndexVec[i].size();

		StaticMesh::MeshVertex* vptr = (StaticMesh::MeshVertex*)&(mesh->vertices[0]) + vcount;
		UINT* iptr = (UINT*)&(mesh->indices[0]) + icount;
		copy
		(
			vecVertexVec[i].begin(), vecVertexVec[i].end(),
			stdext::checked_array_iterator<StaticMesh::MeshVertex *>(vptr, vsize)
		);

		copy
		(
			vecIndexVec[i].begin(), vecIndexVec[i].end(),
			stdext::checked_array_iterator<UINT *>(iptr, isize)
		);

		vcount += vsize;
		icount += isize;
	}

	meshes.push_back(mesh);

	r->Close();
	SafeDelete(r);
}

void StaticModelRender::ReadMode2(wstring file)
{
	file = L"../../_Models/" + file + L".static";
	BinaryReader* r = new BinaryReader();
	r->Open(file);

	UINT count = 0;
	count = r->UInt();

	for (UINT i = 0; i < count; i++)
	{
		StaticMesh* mesh = new StaticMesh();
		mesh->name = String::ToWString(r->String());
		mesh->materialName = String::ToWString(r->String());
		{
			UINT count = r->UInt();

			vector<StaticMesh::MeshVertex> vertices;
			vertices.assign(count, StaticMesh::MeshVertex());

			mesh->vertices = new StaticMesh::MeshVertex[count];
			mesh->vertexCount = count;

			void* ptr = (void*)&(vertices[0]);
			r->Byte(&ptr, sizeof(StaticMesh::MeshVertex) * count);

			copy(
				vertices.begin(), vertices.end(),
				stdext::checked_array_iterator<StaticMesh::MeshVertex*>(mesh->vertices, count)
			);

		}
		{
			UINT count = r->UInt();


			vector<UINT> indices;
			indices.assign(count, UINT());

			void* ptr = (void *)&(indices[0]);
			r->Byte(&ptr, sizeof(UINT) * count);

			mesh->indices = new UINT[count];
			mesh->indexCount = count;
			copy(
				indices.begin(), indices.end(),
				stdext::checked_array_iterator<UINT*>(mesh->indices, count)
			);

		}
		meshes.push_back(mesh);
	}

	r->Close();
	SafeDelete(r);
}

