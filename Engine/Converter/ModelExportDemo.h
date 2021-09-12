#pragma once
#include "Systems/IExecute.h"

class ModelExportDemo : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override {}
	virtual void Render() override {}
	virtual void PostRender() override {}
	virtual void ResizeScreen() override {}

private:
	void LoadMesh(wstring file);
	void LoadDiffMesh(wstring file);
	void ExportMesh(wstring file);
	void ExportDiffMesh(wstring file);
	void ExportStaticMesh(wstring file);
	void ExportMaterial(wstring file);
	void ExportClip(wstring file);

	class ModelConverter* converter;
};