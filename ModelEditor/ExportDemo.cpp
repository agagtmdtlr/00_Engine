#include "stdafx.h"
#include "Converter.h"
#include "Utilities/Path.h"
#include "Utilities/String.h"	
#include "ExportDemo.h"

void ExportDemo::Initialize()
{
	converter = new Converter();
}

void ExportDemo::Destroy()
{
	SafeDelete(converter);
}

void ExportDemo::Update()
{
	function<void(wstring)> loadmeshF = bind(&ExportDemo::LoadMesh, this, placeholders::_1);
	if(ImGui::Button("Load Mesh"))
	{
		Path::OpenFileDialog(L"", Path::FbxModelFilter, L"../../_Assets", loadmeshF, D3D::GetHandle());
	}

	function<void(wstring)> loaddiffmeshF = bind(&ExportDemo::LoadDiffMesh, this, placeholders::_1);
	if (ImGui::Button("Load Diff Mesh"))
	{
		Path::OpenFileDialog(L"", Path::FbxModelFilter, L"../../_Assets", loaddiffmeshF, D3D::GetHandle());
	}

	function<void(wstring)> exportMeshF = bind(&ExportDemo::ExportMesh, this, placeholders::_1);
	if (ImGui::Button("Export Mesh"))
	{
		Path::SaveFileDialog(L"", Path::BinModelFilter, L"../../_Models", exportMeshF, D3D::GetHandle());
	}

	function<void(wstring)> exportdiffMeshF = bind(&ExportDemo::ExportDiffMesh, this, placeholders::_1);
	if (ImGui::Button("Export Diff Mesh"))
	{
		Path::SaveFileDialog(L"", Path::BinModelFilter, L"../../_Models", exportdiffMeshF, D3D::GetHandle());
	}

	function<void(wstring)> exportstaticMeshF = bind(&ExportDemo::ExportStaticMesh, this, placeholders::_1);
	if (ImGui::Button("Export Static Mesh"))
	{
		Path::SaveFileDialog(L"", Path::BinModelFilter, L"../../_Models", exportstaticMeshF, D3D::GetHandle());
	}

	function<void(wstring)> exportMaterialF = bind(&ExportDemo::ExportMaterial, this, placeholders::_1);
	if (ImGui::Button("Export Material"))
	{
		Path::SaveFileDialog(L"", Path::MaterialFilter, L"../../_Textures", exportMaterialF, D3D::GetHandle());
	}

	function<void(wstring)> exportClipF = bind(&ExportDemo::ExportClip, this, placeholders::_1);
	if (ImGui::Button("Export Clip"))
	{
		Path::SaveFileDialog(L"", Path::ClipFilter, L"../../_Models", exportMaterialF, D3D::GetHandle());
	}
}

void ExportDemo::LoadMesh(wstring file)
{
	if (Path::ExistFile(file) == true)
	{
		converter->ReadFile(file);
	}
}

void ExportDemo::LoadDiffMesh(wstring file)
{
	if (Path::ExistFile(file) == true)
	{
		converter->ReadFileDiffMesh(file);
	}
}

void ExportDemo::ExportMesh(wstring file)
{
	converter->ExportMesh(file);
}

void ExportDemo::ExportDiffMesh(wstring file)
{
	converter->ExportMeshDiff(file);
}

void ExportDemo::ExportStaticMesh(wstring file)
{
	converter->ExportStaticMesh(file);
}

void ExportDemo::ExportMaterial(wstring file)
{
	converter->ExportMaterialPBR(file);
}

void ExportDemo::ExportClip(wstring file)
{
	converter->ExportAnimClip(0, file);
}

