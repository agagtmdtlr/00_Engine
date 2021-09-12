#include "stdafx.h"
#include "ModelConverter.h"
#include "Utilities/Path.h"
#include "Utilities/String.h"	
#include "ModelExportDemo.h"

void ModelExportDemo::Initialize()
{
	converter = new ModelConverter();
}

void ModelExportDemo::Destroy()
{
	SafeDelete(converter);
}

void ModelExportDemo::Update()
{
	bool b = true;
	ImGui::Begin("ModelConverter",&b);
	function<void(wstring)> loadmeshF = bind(&ModelExportDemo::LoadMesh, this, placeholders::_1);
	if(ImGui::Button("Load Mesh"))
	{
		Path::OpenFileDialog(L"", Path::FbxModelFilter, L"../../_Assets", loadmeshF, D3D::GetHandle());
	}

	function<void(wstring)> loaddiffmeshF = bind(&ModelExportDemo::LoadDiffMesh, this, placeholders::_1);
	if (ImGui::Button("Load Diff Mesh"))
	{
		Path::OpenFileDialog(L"", Path::FbxModelFilter, L"../../_Assets", loaddiffmeshF, D3D::GetHandle());
	}

	function<void(wstring)> exportMeshF = bind(&ModelExportDemo::ExportMesh, this, placeholders::_1);
	if (ImGui::Button("Export Mesh"))
	{
		Path::SaveFileDialog(L"", Path::BinModelFilter, L"../../_Models", exportMeshF, D3D::GetHandle());
	}

	function<void(wstring)> exportdiffMeshF = bind(&ModelExportDemo::ExportDiffMesh, this, placeholders::_1);
	if (ImGui::Button("Export Diff Mesh"))
	{
		Path::SaveFileDialog(L"", Path::BinModelFilter, L"../../_Models", exportdiffMeshF, D3D::GetHandle());
	}

	function<void(wstring)> exportstaticMeshF = bind(&ModelExportDemo::ExportStaticMesh, this, placeholders::_1);
	if (ImGui::Button("Export Static Mesh"))
	{
		Path::SaveFileDialog(L"", Path::BinModelFilter, L"../../_Models", exportstaticMeshF, D3D::GetHandle());
	}

	function<void(wstring)> exportMaterialF = bind(&ModelExportDemo::ExportMaterial, this, placeholders::_1);
	if (ImGui::Button("Export Material"))
	{
		Path::SaveFileDialog(L"", Path::MaterialFilter, L"../../_Textures", exportMaterialF, D3D::GetHandle());
	}

	function<void(wstring)> exportClipF = bind(&ModelExportDemo::ExportClip, this, placeholders::_1);
	if (ImGui::Button("Export Clip"))
	{
		Path::SaveFileDialog(L"", Path::ClipFilter, L"../../_Models", exportMaterialF, D3D::GetHandle());
	}
	ImGui::End();
}

void ModelExportDemo::LoadMesh(wstring file)
{
	if (Path::ExistFile(file) == true)
	{
		converter->ReadFile(file);
	}
}

void ModelExportDemo::LoadDiffMesh(wstring file)
{
	if (Path::ExistFile(file) == true)
	{
		converter->ReadFileDiffMesh(file);
	}
}

void ModelExportDemo::ExportMesh(wstring file)
{
	converter->ExportMesh(file);
}

void ModelExportDemo::ExportDiffMesh(wstring file)
{
	converter->ExportMeshDiff(file);
}

void ModelExportDemo::ExportStaticMesh(wstring file)
{
	converter->ExportStaticMesh(file);
}

void ModelExportDemo::ExportMaterial(wstring file)
{
	converter->ExportMaterialPBR(file);
}

void ModelExportDemo::ExportClip(wstring file)
{
	converter->ExportAnimClip(0, file);
}

