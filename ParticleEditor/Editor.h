#pragma once
#include "Systems/IExecute.h"

class Editor : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override {}
	virtual void Render() override;
	virtual void PostRender() override {}
	virtual void ResizeScreen() override {}

private:
	void Mesh();

private:
	void UpdateParticleList();
	void UpdateTextureList();

	void OnGUI();
	void OnGUI_List();
	void OnGUI_Settings();
	void OnGUI_Write();

	void WriteFile(wstring file);

private:
	Shader* shader;

	CubeSky* sky;

	// 편집창 사이즈
	float windowWidth = 500;

	bool bLoop = false;
	UINT maxParticle = 0;

	vector<wstring> particleList; // 파티클 폴더 파일 리스트
	vector<wstring> textureList; // 파티클 폴더 텍스처 리스트

	wstring file = L""; // 파일 저장 경로


	ParticleSystem* particleSystem = NULL;

	Material* floor;
	Material* stone;

	MeshRender* sphere;
	MeshRender* grid;
};