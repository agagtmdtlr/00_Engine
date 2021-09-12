#include "stdafx.h"
#include "MeshWriter.h"

MeshWriter::MeshWriter()
{
}

MeshWriter::~MeshWriter()
{
}

void MeshWriter::ExportPrimitiveMesh()
{
}

void MeshWriter::WriteCube()
{
	float w = 0.5f;
	float h = 0.5f;
	float d = 0.5f;

	vector<MeshVertex> v;

	// ÅºÁ¨Æ® U (¿À¸¥ÂÊ ¹æÇâ)
	//Front
	v.push_back(MeshVertex(-w, -h, -d, 0, 1, 0, 0, -1, 1, 0, 0));
	v.push_back(MeshVertex(-w, +h, -d, 0, 0, 0, 0, -1, 1, 0, 0));
	v.push_back(MeshVertex(+w, +h, -d, 1, 0, 0, 0, -1, 1, 0, 0));
	v.push_back(MeshVertex(+w, -h, -d, 1, 1, 0, 0, -1, 1, 0, 0));

	//Back
	v.push_back(MeshVertex(-w, -h, +d, 1, 1, 0, 0, 1, -1, 0, 0));
	v.push_back(MeshVertex(+w, -h, +d, 0, 1, 0, 0, 1, -1, 0, 0));
	v.push_back(MeshVertex(+w, +h, +d, 0, 0, 0, 0, 1, -1, 0, 0));
	v.push_back(MeshVertex(-w, +h, +d, 1, 0, 0, 0, 1, -1, 0, 0));

	//Top
	v.push_back(MeshVertex(-w, +h, -d, 0, 1, 0, 1, 0, 1, 0, 0));
	v.push_back(MeshVertex(-w, +h, +d, 0, 0, 0, 1, 0, 1, 0, 0));
	v.push_back(MeshVertex(+w, +h, +d, 1, 0, 0, 1, 0, 1, 0, 0));
	v.push_back(MeshVertex(+w, +h, -d, 1, 1, 0, 1, 0, 1, 0, 0));

	//Bottom
	v.push_back(MeshVertex(-w, -h, -d, 1, 1, 0, -1, 0, -1, 0, 0));
	v.push_back(MeshVertex(+w, -h, -d, 0, 1, 0, -1, 0, -1, 0, 0));
	v.push_back(MeshVertex(+w, -h, +d, 0, 0, 0, -1, 0, -1, 0, 0));
	v.push_back(MeshVertex(-w, -h, +d, 1, 0, 0, -1, 0, -1, 0, 0));

	//Left
	v.push_back(MeshVertex(-w, -h, +d, 0, 1, -1, 0, 0, 0, 0, -1));
	v.push_back(MeshVertex(-w, +h, +d, 0, 0, -1, 0, 0, 0, 0, -1));
	v.push_back(MeshVertex(-w, +h, -d, 1, 0, -1, 0, 0, 0, 0, -1));
	v.push_back(MeshVertex(-w, -h, -d, 1, 1, -1, 0, 0, 0, 0, -1));

	//Right
	v.push_back(MeshVertex(+w, -h, -d, 0, 1, 1, 0, 0, 0, 0, 1));
	v.push_back(MeshVertex(+w, +h, -d, 0, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(MeshVertex(+w, +h, +d, 1, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(MeshVertex(+w, -h, +d, 1, 1, 1, 0, 0, 0, 0, 1));


	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();

	copy(v.begin(), v.end(), stdext::checked_array_iterator<MeshVertex *>(vertices, vertexCount));


	indexCount = 36;
	this->indices = new UINT[indexCount]
	{
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};
	wstring path = L"";

	WriteData(path);
}

void MeshWriter::WriteCylinder()
{
	float height = 1.0f;
	float radius = 0.5f;
	float topRadius = 0.5f;
	float bottomRadius = 0.5f;
	UINT stackCount = 24;
	UINT sliceCount = 24;

	vector<MeshVertex> v;

	float stackHeight = height / (float)stackCount;
	float radiusStep = (topRadius - bottomRadius) / (float)stackCount;

	UINT ringCount = stackCount + 1;
	for (UINT i = 0; i < ringCount; i++)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;
		float theta = 2.0f * Math::PI / (float)sliceCount;

		for (UINT k = 0; k <= sliceCount; k++)
		{
			float c = cosf(k * theta);
			float s = sinf(k * theta);


			MeshVertex vertex;
			vertex.Position = Vector3(r * c, y, r * s);
			vertex.Uv = Vector2((float)k / (float)sliceCount, 1.0f - (float)i / (float)stackCount) * 5.0f;

			
			Vector3 tangent = Vector3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			Vector3 biTangent = Vector3(dr * c, -height, dr * s);

			D3DXVec3Cross(&vertex.Normal, &tangent, &biTangent);
			D3DXVec3Normalize(&vertex.Normal, &vertex.Normal);

			vertex.Tangent = tangent;

			v.push_back(vertex);
		}
	}

	vector<UINT> i;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT y = 0; y < stackCount; y++)
	{
		for (UINT x = 0; x < sliceCount; x++)
		{
			i.push_back(y * ringVertexCount + x);
			i.push_back((y + 1) * ringVertexCount + x);
			i.push_back((y + 1) * ringVertexCount + (x + 1));

			i.push_back(y * ringVertexCount + x);
			i.push_back((y + 1) * ringVertexCount + x + 1);
			i.push_back(y * ringVertexCount + x + 1);
		}
	}


	BuildTopCap(v,i);
	BuildBottomCap(v,i);


	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();
	copy(v.begin(), v.end(), stdext::checked_array_iterator<MeshVertex *>(vertices, vertexCount));

	indices = new UINT[i.size()];
	indexCount = i.size();
	copy(i.begin(), i.end(), stdext::checked_array_iterator<UINT *>(indices, indexCount));

	wstring path = L"";
	WriteData(path);
}

void MeshWriter::BuildTopCap(vector<MeshVertex>& vertices, vector<UINT>& indices)
{
	float height = 1.0f;
	float radius = 0.5f;
	float topRadius = 0.5f;
	float bottomRadius = 0.5f;
	UINT stackCount = 24;
	UINT sliceCount = 24;


	float y = 0.5f * height;
	float theta = 2.0f * Math::PI / (float)sliceCount;

	for (UINT i = 0; i <= sliceCount; i++)
	{
		float x = topRadius * cosf(i * theta);
		float z = topRadius * sinf(i * theta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		vertices.push_back(MeshVertex(x, y, z, u, v, 0, 1, 0, 1, 0, 0));
	}
	vertices.push_back(MeshVertex(0, y, 0, 0.5f, 0.5f, 0, 1, 0, 1, 0, 0));


	UINT baseIndex = vertices.size() - sliceCount - 2;
	UINT centerIndex = vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i + 1);
		indices.push_back(baseIndex + i);
	}
}

void MeshWriter::BuildBottomCap(vector<MeshVertex>& vertices, vector<UINT>& indices)
{
	float height = 1.0f;
	float radius = 0.5f;
	float topRadius = 0.5f;
	float bottomRadius = 0.5f;
	UINT stackCount = 24;
	UINT sliceCount = 24;


	float y = -0.5f * height;
	float theta = 2.0f * Math::PI / (float)sliceCount;

	for (UINT i = 0; i <= sliceCount; i++)
	{
		float x = bottomRadius * cosf(i * theta);
		float z = bottomRadius * sinf(i * theta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		vertices.push_back(MeshVertex(x, y, z, u, v, 0, -1, 0, -1, 0, 0));
	}
	vertices.push_back(MeshVertex(0, y, 0, 0.5f, 0.5f, 0, -1, 0, -1, 0, 0));


	UINT baseIndex = vertices.size() - sliceCount - 2;
	UINT centerIndex = vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}
}

void MeshWriter::WriteGrid()
{
	float offsetU = 10;
	float offsetV = 10;
	UINT countX = 11;
	UINT countZ = 11;

	float w = ((float)countX - 1) * 0.5f;
	float h = ((float)countZ - 1) * 0.5f;

	vector<MeshVertex> v;
	for (UINT z = 0; z < countZ; z++)
	{
		for (UINT x = 0; x < countX; x++)
		{
			MeshVertex vertex;
			vertex.Position = Vector3((float)x - w, 0, (float)z - h);
			vertex.Normal = Vector3(0, 1, 0);
			vertex.Tangent = Vector3(1, 0, 0);
			vertex.Uv.x = (float)x / (float)(countX - 1) * offsetU;
			vertex.Uv.y = (float)z / (float)(countZ - 1) * offsetV;

			v.push_back(vertex);
		}
	}

	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();

	copy(v.begin(), v.end(), stdext::checked_array_iterator<MeshVertex *>(vertices, vertexCount));


	vector<UINT> i;
	for (UINT z = 0; z < countZ - 1; z++)
	{
		for (UINT x = 0; x < countX - 1; x++)
		{
			i.push_back(countX * z + x);
			i.push_back(countX * (z + 1) + x);
			i.push_back(countX * z + x + 1);

			i.push_back(countX * z + x + 1);
			i.push_back(countX * (z + 1) + x);
			i.push_back(countX * (z + 1) + x + 1);
		}
	}

	indices = new UINT[i.size()];
	indexCount = i.size();

	copy(i.begin(), i.end(), stdext::checked_array_iterator<UINT *>(indices, indexCount));

	wstring path = L"";
	WriteData(L"");
}


void MeshWriter::WriteQuad()
{
}

void MeshWriter::WriteSphere()
{
}

void MeshWriter::WriteHemiSphere()
{
}

void MeshWriter::WriteData(wstring path)
{
}
