#pragma once
template<typename T>
class MeshPBR
{
public:
	friend class ModelRenderPBR;
	friend class ModelAnimatorPBR;
	friend class ModelPBR;

public:
	typedef T MeshVertex;

public:
	MeshPBR();
	virtual ~MeshPBR();

	virtual void SetShader(class Shader* shader);
	void Pass(UINT val) { pass = val; }
	void Topology(D3D11_PRIMITIVE_TOPOLOGY t) { topology = t; }

	virtual void Update();
	virtual void Render(UINT drawCount);

	T VertexType() { return T; }

protected:
	virtual void Create() = 0;

protected:
	class Shader* shader;
	UINT pass = 0;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	class PerFrame* perFrame = NULL;

	class VertexBuffer* vertexBuffer = NULL;
	UINT vertexCount;
	MeshVertex* vertices;

	class IndexBuffer* indexBuffer = NULL;
	UINT indexCount;
	UINT* indices;

};
////////////////////////////////////////////////////////////////////////////
// StaticMesh
////////////////////////////////////////////////////////////////////////////
class StaticMesh : public MeshPBR<VertexTextureNormalTangent>
{
public:
	friend class StaticModelRender;

	StaticMesh();
	~StaticMesh();

	wstring Name() { return name; }

private:
	void Create() override;

private:
	wstring name = L"";
	wstring materialName = L"";
	class MaterialPBR* materialPBR = NULL;
};
////////////////////////////////////////////////////////////////////////////
// SkinMesh
////////////////////////////////////////////////////////////////////////////

class SkinMesh : public MeshPBR<VertexTextureNormalTangentBlend>
{
public:
	friend class ModelRenderPBR;
	friend class ModelAnimatorPBR;
	friend class ModelPBR;

public:
	SkinMesh();
	~SkinMesh();

public:
	void Create() override;

	virtual void Update() override;
	virtual void Render(UINT drawCount) override;
	virtual void SetShader(Shader* shader) override;

public:
	wstring Name() { return name; }
	void MaterialName(wstring mname) { materialName = mname; }
	int BoneIndex() { return boneIndex; }
	class ModelBone* Bone() { return bone; }

	void Binding(ModelPBR* model);

private:
	struct BoneDesc
	{
		UINT Index;
		float Padding[3];
	} boneDesc;
private:
	wstring name;

private:
	int boneIndex;
	ModelBone * bone;

	ConstantBuffer* boneCBuffer;
	ID3DX11EffectConstantBuffer* sBoneCBuffer;

private:
	wstring materialName = L"";
	class MaterialPBR* material = NULL;

private:
	ID3D11ShaderResourceView* transformsSRV = NULL;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV = NULL;
};
