#include "stdafx.h"
#include "Main.h"
#include "Systems/Window.h"

#include "Converter/ModelExportDemo.h"
#include "Converter/ConvertMap.h"

#include "PBR/PBRInteriorDemo.h"
#include "PBR/PBRDefferedDemo.h"
#include "PBR/PBRIBLDemo.h"
#include "PBR/PBRIrradianceDemo.h"
#include "PBR/PBRBasicDemo.h"
#include "PBR/HDRDemo.h"
#include "Terrain/TerrainLodDemo.h"



void Main::Initialize()
{
	//Push(new ModelExportDemo());
	//Push(new ConvertMap());		
	//Push(new PBRInteriorDemo());
	Push(new PBRDefferedDemo());
	//Push(new PBRIBLDemo());
	//Push(new PBRIrradianceDemo());
	//Push(new PBRBasicDemo());
	//Push(new HDRDemo());
	//Push(new TerrainLodDemo());
}

void Main::Ready()
{

}

void Main::Destroy()
{
	for (IExecute* exe : executes)
	{
		exe->Destroy();
		SafeDelete(exe);
	}
}

void Main::Update()
{
	for (IExecute* exe : executes)
		exe->Update();
}

void Main::PreRender()
{
	for (IExecute* exe : executes)
		exe->PreRender();
}

void Main::Render()
{
	for (IExecute* exe : executes)
		exe->Render();
}

void Main::PostRender()
{
	for (IExecute* exe : executes)
		exe->PostRender();
}

void Main::ResizeScreen()
{
	for (IExecute* exe : executes)
		exe->ResizeScreen();
}

void Main::Push(IExecute * execute)
{
	executes.push_back(execute);

	execute->Initialize();
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR param, int command)
{
	D3DDesc desc;
	desc.AppName = L"DirectX11 3D";
	desc.Instance = instance;
	desc.bFullScreen = false;
	desc.bVsync = false;
	desc.Handle = NULL;
	desc.Width = 1280;
	desc.Height = 720;
	//desc.Width = 1920;
	//desc.Height = 1080;
	desc.Background = Color(0.3f, 0.3f, 0.3f, 1.0f);
	D3D::SetDesc(desc);

	Main* main = new Main();
	WPARAM wParam = Window::Run(main);

	SafeDelete(main);

	return wParam;
}