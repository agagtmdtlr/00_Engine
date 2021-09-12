#pragma once

class BRDFMap
{
private:
	typedef VertexTexture VertexBRDF;

public:
	BRDFMap();	
	~BRDFMap();

	void BakingBRDF();	

	void SaveBRDF();

	ID3D11ShaderResourceView* GetSRVBRDFLUT() { return srvBRDF; }


private:
	void RenderBRDF();

	void Initialize();

private:


private:
	wstring file;

	float BRDFSize = 1024;
	
	Shader* shader;
	UINT pass = 0;

	ID3D11Texture2D* txtBRDF = NULL;

	ID3D11RenderTargetView* rtvBRDF = NULL;

	ID3D11Texture2D* mapDSVBRDF;

	ID3D11DepthStencilView* dsvBRDF = NULL;

	ID3D11ShaderResourceView* srvBRDF = NULL;

	Viewport* vpBRDF = NULL;

	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;

	VertexBRDF vertices[4];
	UINT indices[6] = { 0,1,2,2,1,3 };
};