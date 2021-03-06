#include "Framework.h"
#include "MeshHemiSphere.h"

MeshHemiSphere::MeshHemiSphere(UINT stackCount, UINT sliceCount)
	:stackCount(stackCount), sliceCount(sliceCount)
{
}

MeshHemiSphere::~MeshHemiSphere()
{
}

void MeshHemiSphere::Create()
{
	float radius = 0.5f;
	vector<MeshVertex> v;
	v.push_back(MeshVertex(0, radius / 2.0f, 0, 0, 0, 0, 1, 0, 1, 0, 0));

	float phiStep = Math::PI / 2.0f / stackCount ; // 90?? (?ݱ?)
	float thetaStep = Math::PI * 2.0f / sliceCount;


	for (UINT i = 1; i <= stackCount - 1; i++)
	{
		float phi = i * phiStep;

		for (UINT k = 0; k <= sliceCount; k++)
		{
			float theta = k * thetaStep;
			if (k == sliceCount)
				theta = 0.01f;

			Vector3 p = Vector3
			{
				(radius * sinf(phi) * cosf(theta)),
				(radius * cos(phi)),
				(radius * sinf(phi) * sinf(theta))
			};

			Vector3 n;
			D3DXVec3Normalize(&n, &p);

			Vector3 t = Vector3
			(
				-(radius * sinf(phi) * sinf(theta)),
				0.0f,
				(radius * sinf(phi) * cosf(theta))
			);
			D3DXVec3Normalize(&t, &t);

			p.y -= radius / 2.0f; // hemisphere located in center

			Vector2 uv = Vector2(theta / (Math::PI * 2), phi / Math::PI);
			if (k == sliceCount)
				uv = Vector2(1.0f, phi / Math::PI);

			v.push_back(MeshVertex(p.x, p.y, p.z, uv.x, uv.y, n.x, n.y, n.z, t.x, t.y, t.z));
		}
	}
	// bottom
	for (UINT k = 0; k <= sliceCount; k++)
	{
		float phi = (stackCount - 1) * phiStep;
		float theta = k * thetaStep;

		if (k == sliceCount)
			theta = 0.01f;

		Vector3 p = Vector3
		{
			(radius * sinf(phi) * cosf(theta)),
			(radius * cos(phi)),
			(radius * sinf(phi) * sinf(theta))
		};

		Vector3 n = { 0,-1,0 };
		Vector3 t = { 1, 0, 0 };
		p.y -= radius / 2.0f; // hemisphere located in center

		Vector2 uv = Vector2(theta / (Math::PI * 2), phi / Math::PI);
		if (k == sliceCount)
			uv = Vector2(1.0f, phi / Math::PI);

		v.push_back(MeshVertex(p.x, p.y, p.z, uv.x, uv.y, n.x, n.y, n.z, t.x, t.y, t.z));
	}
	// bottom center
	v.push_back(MeshVertex(0, -radius / 2.0f, 0, 0, 0, 0, -1, 0, -1, 0, 0));

	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();

	copy(v.begin(), v.end(), stdext::checked_array_iterator<MeshVertex*>(vertices, vertexCount));
	vector<UINT> i;
	for (UINT k = 1; k <= sliceCount; k++)
	{
		i.push_back(0);
		i.push_back(k + 1);
		i.push_back(k);
	}

	UINT baseIndex = 1;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT k = 0; k < stackCount - 2; k++)
	{
		for (UINT j = 0; j < sliceCount; j++)
		{
			i.push_back(baseIndex + k * ringVertexCount + j);
			i.push_back(baseIndex + k * ringVertexCount + j + 1);
			i.push_back(baseIndex + (k + 1) * ringVertexCount + j);

			i.push_back(baseIndex + (k + 1) * ringVertexCount + j);
			i.push_back(baseIndex + k * ringVertexCount + j + 1);
			i.push_back(baseIndex + (k + 1) * ringVertexCount + j + 1);
		}
	}

	UINT southPoleIndex = v.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;

	for (UINT k = 0; k < sliceCount; k++)
	{
		i.push_back(southPoleIndex);
		i.push_back(baseIndex + k);
		i.push_back(baseIndex + k + 1);
	}

	indices = new UINT[i.size()];
	indexCount = i.size();

	copy(i.begin(), i.end(), stdext::checked_array_iterator<UINT *>(indices, indexCount));

}
