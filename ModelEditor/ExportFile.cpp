#include "stdafx.h"
#include "ExportFile.h"
#include "Converter.h"

void ExportFile::Initialize()
{
	//Airplane();
	//Tower();
	//Tank();
	//Kachujin();
	//Weapon();
	//Vanguard();
	//PBRAsset();

	AstonMartin();
	//BistroStreet();
	//Warehouse();
	//TableChair();
	//BedRoom();
	//BMW();
}

void ExportFile::Airplane()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"B787/Airplane.fbx");
	conv->ExportMesh(L"B787/Airplane");
	conv->ExportMaterial(L"B787/Airplane");
	SafeDelete(conv);
}

void ExportFile::Tower()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Tower/Tower.fbx");
	conv->ExportMesh(L"Tower/Tower");
	conv->ExportMaterial(L"Tower/Tower");
	SafeDelete(conv);
}


void ExportFile::Tank()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Tank/Tank.fbx");
	conv->ExportMesh(L"Tank/Tank"); 
	conv->ExportMaterial(L"Tank/Tank", false); // 수동으로 수정한게 있으므로
	SafeDelete(conv);
}





void ExportFile::Kachujin()
{
	Converter* conv = NULL;

	conv = new Converter();
	conv->ReadFile(L"Kachujin/Mesh.fbx");
	conv->ExportMesh(L"Kachujin/Mesh");
	conv->ExportMaterial(L"Kachujin/Mesh");
	SafeDelete(conv);
	
	//conv = new Converter();
	//conv->ReadFile(L"Kachujin/Sword And Shield Idle.fbx");
	//conv->ExportAnimClip(0, L"Kachujin/Sword And Shield Idle");
	//SafeDelete(conv);	

	//conv = new Converter();
	//conv->ReadFile(L"Kachujin/Sword And Shield Run.fbx");
	//conv->ExportAnimClip(0, L"Kachujin/Sword And Shield Run");
	//SafeDelete(conv);

	//conv = new Converter();
	//conv->ReadFile(L"Kachujin/Sword And Shield Slash.fbx");
	//conv->ExportAnimClip(0, L"Kachujin/Sword And Shield Slash");
	//SafeDelete(conv);

	//conv = new Converter();
	//conv->ReadFile(L"Kachujin/Salsa Dancing.fbx");
	//conv->ExportAnimClip(0, L"Kachujin/Salsa Dancing");
	//SafeDelete(conv);

	//conv = new Converter();
	//conv->ReadFile(L"Kachujin/Sword And Shield Walk.fbx");
	//conv->ExportAnimClip(0, L"Kachujin/Sword And Shield Walk");
	//SafeDelete(conv);



	// 한 파일 내애 여러 클립들을 모아두는 경우 리스트로 받아올수 있다.
	//vector<wstring> clipNames;
	//conv->ClipList(&clipNames);

}

void ExportFile::Weapon()
{
	vector<wstring> names;
	names.push_back(L"Cutter.fbx");
	names.push_back(L"Cutter2.fbx");
	names.push_back(L"Dagger_epic.fbx");
	names.push_back(L"Dagger_small.fbx");
	names.push_back(L"Katana.fbx");
	names.push_back(L"LongArrow.obj");
	names.push_back(L"LongBow.obj");
	names.push_back(L"Rapier.fbx");
	names.push_back(L"Sword.fbx");
	names.push_back(L"Sword2.fbx");
	names.push_back(L"Sword_epic.fbx");

	for (wstring name : names)
	{
		Converter* conv = new Converter();
		conv->ReadFile(L"Weapon/" + name);


		String::Replace(&name, L".fbx", L"");
		String::Replace(&name, L".obj", L"");

		conv->ExportMaterial(L"Weapon/" + name, false);
		conv->ExportMesh(L"Weapon/" + name);
		SafeDelete(conv);
	}
}

void ExportFile::Vanguard()
{
	Converter* conv = NULL;

	conv = new Converter();
	conv->ReadFile(L"Enemy/remy.fbx");
	conv->ExportMesh(L"Enemy/k");
	conv->ExportMaterial(L"Enemy/k");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Enemy/Idle.fbx");
	conv->ExportAnimClip(0, L"Enemy/Idle");
	SafeDelete(conv);

	/*conv = new Converter();
	conv->ReadFile(L"Enemy/Reaction.fbx");
	conv->ExportAnimClip(0, L"Enemy/Reaction");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Enemy/Dying.fbx");
	conv->ExportAnimClip(0, L"Enemy/Dying");
	SafeDelete(conv);*/

	

}

void ExportFile::PBRAsset()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"SpaceSuit/A pose Space Suit Apollo 11.FBX");
	conv->ExportMesh(L"SpaceSuit/SpaceSuit2");
	//conv->ReadFileDiffMesh(L"SpaceSuit/White_Space_Suit_Apollo_11.FBX");
	//conv->ExportMeshDiff(L"SpaceSuit/SpaceSuit");
	//conv->ExportMaterial(L"SpaceSuit/SpaceSuit");
	//conv->ExportMaterialPBR(L"SpaceSuit/SpaceSuit");
	//conv->ReadFile(L"SpaceSuit/WhiteRun.FBX");
	//conv->ExportAnimClip(0,L"SpaceSuit/WhiteRun");
	//SafeDelete(conv);
	//conv = new Converter();
	//conv->ReadFile(L"SpaceSuit/A pose Space Suit Apollo 12.FBX");
	//conv->ReadFileDiffMesh(L"SpaceSuit/White_Space_Suit_Apollo_12.FBX");
	//conv->ExportMeshDiff(L"SpaceSuit/SpaceSuit");

	//SafeDelete(conv);
	//conv = new Converter();
	//conv->ReadFile(L"SpaceSuit/Silly Dancing.FBX");
	//conv->ExportAnimClip(0, L"SpaceSuit/Silly Dancing");
	//SafeDelete(conv);
	//conv = new Converter();
	//conv->ReadFile(L"SpaceSuit/Silly Dancing2.FBX");
	//conv->ExportAnimClip(0, L"SpaceSuit/Silly Dancing2");
	SafeDelete(conv);
	conv = new Converter();
	conv->ReadFile(L"SpaceSuit/Running.FBX");
	conv->ExportAnimClip(0, L"SpaceSuit/Running");
	SafeDelete(conv);
	conv = new Converter();
	conv->ReadFile(L"SpaceSuit/Walking.FBX");
	conv->ExportAnimClip(0, L"SpaceSuit/Walking");
	SafeDelete(conv);
	conv = new Converter();
	conv->ReadFile(L"SpaceSuit/Swimming.FBX");
	conv->ExportAnimClip(0, L"SpaceSuit/Swimming");
	SafeDelete(conv);
	conv = new Converter();
	conv->ReadFile(L"SpaceSuit/Idle.FBX");
	conv->ExportAnimClip(0, L"SpaceSuit/Idle");


}

void ExportFile::AstonMartin()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"astonmartin/astonmartin.fbx");
	conv->ExportStaticMesh(L"astonmartin/astonmartin");
	conv->ExportMaterialPBR(L"astonmartin/astonmartin");
}

void ExportFile::BistroStreet()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Bistro_v5_1/BistroExterior.fbx");
	//conv->ExportStaticMesh(L"Bistro_v5_1/BistroExterior");
	conv->ExportMaterialPBR(L"Bistro_v5_1/BistroExterior");
}

void ExportFile::Warehouse()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"warehouse/warehouse_fbx.fbx");
	conv->ExportStaticMesh(L"warehouse/warehouse");
	conv->ExportMaterialPBR(L"warehouse/warehouse");
}

void ExportFile::TableChair()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Table_Chair/01.fbx");
	conv->ExportMesh(L"Table_Chair/Table_Chair");
	conv->ExportMaterialPBR(L"Table_Chair/Table_Chair");
}

void ExportFile::BedRoom()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"bedroom/iscv2.obj");
	//conv->ExportStaticMesh(L"bedroom/bedroom");

	conv->ExportMaterialPBR(L"bedroom/bedroom");
}

void ExportFile::BMW()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"bmw/bmw.obj");
	conv->ExportStaticMesh(L"bmw/bmw");

	conv->ExportMaterialPBR(L"bmw/bmw");
}
