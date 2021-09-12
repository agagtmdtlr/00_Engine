#pragma once

class MeshWriter
{
public:
	typedef VertexTextureNormalTangent MeshVertex;

	MeshWriter();
	~MeshWriter();

public:
	void ExportPrimitiveMesh();

private:
	void WriteCube();
	void WriteCylinder();
	void BuildTopCap(vector<MeshVertex>& vertices, vector<UINT>& indices);
	void BuildBottomCap(vector<MeshVertex>& vertices, vector<UINT>& indices);

	void WriteGrid();
	void WriteQuad();
	void WriteSphere();
	void WriteHemiSphere();

	void WriteData(wstring path);

private:
	UINT vertexCount;
	UINT indexCount;

	MeshVertex * vertices = NULL;
	UINT * indices = NULL;
};