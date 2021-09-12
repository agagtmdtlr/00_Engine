#include "stdafx.h"
#include "Converter/EnvironmentMap.h"

EnvironmentMap::EnvironmentMap()
{
	Initialize();
}

EnvironmentMap::~EnvironmentMap()
{
}

void EnvironmentMap::LoadEquirectangularMap(wstring file)
{
	SafeDelete(equirectangularMap);
	equirectangularMap = new Texture(file);
	wstring ext = L"." + Path::GetExtension(file);
	String::Replace(&file, ext, L"_");
	this->file = file;
}

void EnvironmentMap::BakingCubeMap()
{
	if (equirectangularMap == NULL) assert(false);

	shader->AsSRV("EquirectangularMap")->SetResource(equirectangularMap->SRV());
	cCubeDesc->Render();
	shader->AsConstantBuffer("CB_CubeDesc")->SetConstantBuffer(cCubeDesc->Buffer());
	RenderEnvironmentMap();
	//RenderIrradianceMap();
	RenderReflectionMap();
	//RenderTestMap();
	//SaveEnvironmentMap();
	//SaveIrradianceMap();
	//SaveReflectionMap();
}

void EnvironmentMap::BakingEnvironeMentMap()
{
	if (equirectangularMap == NULL) assert(false);
	shader->AsSRV("EquirectangularMap")->SetResource(equirectangularMap->SRV());
	cCubeDesc->Render();
	shader->AsConstantBuffer("CB_CubeDesc")->SetConstantBuffer(cCubeDesc->Buffer());
	RenderEnvironmentMap();
	SaveEnvironmentMap();
}


void EnvironmentMap::BakingIrradianceMap()
{
	if (equirectangularMap == NULL) assert(false);
	shader->AsSRV("EquirectangularMap")->SetResource(equirectangularMap->SRV());
	cCubeDesc->Render();
	shader->AsConstantBuffer("CB_CubeDesc")->SetConstantBuffer(cCubeDesc->Buffer());
	RenderEnvironmentMap();
	RenderIrradianceMap();
	SaveIrradianceMap();
}

void EnvironmentMap::BakingReflectionMap()
{
	if (equirectangularMap == NULL) assert(false);

	shader->AsSRV("EquirectangularMap")->SetResource(equirectangularMap->SRV());
	cCubeDesc->Render();
	shader->AsConstantBuffer("CB_CubeDesc")->SetConstantBuffer(cCubeDesc->Buffer());
	RenderEnvironmentMap();
	RenderReflectionMap();
	SaveReflectionMap();

}

void EnvironmentMap::ConvertEquirectangularMapToCubeMap(wstring file, float size, bool correct, wstring semi)
{
	SafeDelete(convertMap);
	convertMap = new Texture(file);

	wstring ext = L"." + Path::GetExtension(file);
	String::Replace(&file, ext, L"_");	
	equirFile = file + semi;

	SafeDelete(vpConvert);
	SafeRelease(srvConvert);
	SafeRelease(dsvConvert);
	SafeRelease(rtvConvert);
	SafeRelease(dsvConvertTXT);
	SafeRelease(convertCube);

	// create texture cube
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.ArraySize = 6;
		desc.Width = (UINT)size;
		desc.Height = (UINT)size;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &convertCube));
	}

	// multi render target view resource
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvdesc;
		ZeroMemory(&rtvdesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rtvdesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		rtvdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvdesc.Texture2DArray.ArraySize = 6;

		Check(D3D::GetDevice()->CreateRenderTargetView(convertCube, &rtvdesc, &rtvConvert));
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
		ZeroMemory(&srvdesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvdesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvdesc.TextureCube.MipLevels = 1;

		Check(D3D::GetDevice()->CreateShaderResourceView(convertCube, &srvdesc, &srvConvert));
	}
	// create dsv texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.Width = (UINT)size;
		desc.Height = (UINT)size;
		desc.ArraySize = 6;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &dsvConvertTXT));
	}
	// create dsv
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvdesc;
		ZeroMemory(&dsvdesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dsvdesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvdesc.Texture2DArray.ArraySize = 6;

		Check(D3D::GetDevice()->CreateDepthStencilView(dsvConvertTXT, &dsvdesc, &dsvConvert));
	}

	{
		vpConvert = new Viewport(size, size);
	}

	D3D::Get()->SetRenderTarget(rtvConvert, dsvConvert);
	D3D::Get()->Clear(Color(0, 0, 0, 1), rtvConvert, dsvConvert);

	vpConvert->RSSetViewport();

	shader->AsSRV("EquirectangularMap")->SetResource(convertMap->SRV());
	cCubeDesc->Render();
	shader->AsConstantBuffer("CB_CubeDesc")->SetConstantBuffer(cCubeDesc->Buffer());

	cube->Update();
	cube->Pass(correct ? 2 : 0);
	cube->Render();

	wstring path  = equirFile + L"cube.dds";
	HRESULT	hr = D3DX11SaveTextureToFile(D3D::GetDC(), convertCube, D3DX11_IFF_DDS, path.c_str());
	Check(hr);
}

void EnvironmentMap::RenderEnvironmentMap()
{
	D3D::Get()->SetRenderTarget(rtvEnvironment, dsvEnvironment);
	D3D::Get()->Clear(Color(0, 0, 0, 1), rtvEnvironment, dsvEnvironment);

	vpEnvironment->RSSetViewport();

	cube->Update();
	cube->Pass(0);
	cube->Render();
}

void EnvironmentMap::RenderIrradianceMap()
{
	D3D::Get()->SetRenderTarget(rtvIrradiance, dsvIrradiance);
	D3D::Get()->Clear(Color(0, 0, 0, 1), rtvIrradiance, dsvIrradiance);

	// Irradiance map make from environment map by Integral;
	sSRVEnvironment->SetResource(srvEnvironment);

	vpEnvironment->RSSetViewport();


	sphere->Update();
	sphere->Pass(1);
	sphere->Render();
}

void EnvironmentMap::RenderReflectionMap()
{
	sSRVEnvironment->SetResource(srvEnvironment);

	UINT maxMipLevels = 5;
	for (UINT mip = 0; mip < maxMipLevels; mip++)
	{


		// mip level viewport
		vpReflection[mip]->RSSetViewport();
		// mip level roughness
		cubeDesc.roughness = (float)mip / (float)(maxMipLevels - 1);
		cubeDesc.mip = (float)mip;
		cCubeDesc->Render();
		shader->AsConstantBuffer("CB_CubeDesc")->SetConstantBuffer(cCubeDesc->Buffer());
		
		//// mip level rtv & dsv
		//D3D::Get()->SetRenderTarget(rtvCopyToReflection[mip], dsvCopyToReflection[mip]);
		//D3D::Get()->Clear(Color(0, 0, 0, 1), rtvCopyToReflection[mip], dsvCopyToReflection[mip]);
		//
		//
		//sphere->Update();
		//sphere->Pass(4); // copy resource mipmap size cube
		//sphere->Render();
		//
		//sSRVEnvironment->SetResource(srvCopyToReflection[mip]);

		D3D::Get()->SetRenderTarget(rtvReflection[mip], dsvReflection[mip]);
		D3D::Get()->Clear(Color(0, 0, 0, 1), rtvReflection[mip], dsvReflection[mip]);

		sphere->Update();
		sphere->Pass(3); // specular map prefiltered pass
		sphere->Render();

	}
}

void EnvironmentMap::RenderTestMap()
{

	UINT maxMipLevels = 5;
	//UINT mip = 0;

	for (UINT mip = 1; mip < 2; mip++)
	{
		D3D::Get()->SetRenderTarget(rtvCopyToReflection[mip], dsvCopyToReflection[mip]);
		D3D::Get()->Clear(Color(0, 0, 0, 1), rtvCopyToReflection[mip], dsvCopyToReflection[mip]);

		sSRVEnvironment->SetResource(srvEnvironment);


		cubeDesc.roughness = (float)mip / (float)(maxMipLevels - 1);
		cCubeDesc->Render();
		shader->AsConstantBuffer("CB_CubeDesc")->SetConstantBuffer(cCubeDesc->Buffer());

		vpReflection[mip]->RSSetViewport();
		//vpTest->RSSetViewport();

		sphere->Update();
		sphere->Pass(4);
		sphere->Render();

		sSRVEnvironment->SetResource(srvCopyToReflection[mip]);

		D3D::Get()->SetRenderTarget(rtvReflection[mip], dsvReflection[mip]);
		D3D::Get()->Clear(Color(0, 0, 0, 1), rtvReflection[mip], dsvReflection[mip]);

		vpReflection[mip]->RSSetViewport();


		sphere->Update();
		sphere->Pass(3);
		sphere->Render();
	}

	

}

void EnvironmentMap::RenderBRDFMap()
{
	
}



void EnvironmentMap::SaveEnvironmentMap()
{
	wstring path = file + L"Environment.dds";
	HRESULT	hr = D3DX11SaveTextureToFile(D3D::GetDC(), cubeEnvironment, D3DX11_IFF_DDS, path.c_str());
}

void EnvironmentMap::SaveIrradianceMap()
{
	wstring path = file + L"Irradiance.dds";
	HRESULT	hr = D3DX11SaveTextureToFile(D3D::GetDC(), cubeIrradiance, D3DX11_IFF_DDS, path.c_str());
}

void EnvironmentMap::SaveReflectionMap()
{
	wstring path = file + L"Reflection.dds";
	HRESULT	hr = D3DX11SaveTextureToFile(D3D::GetDC(), cubeReflection, D3DX11_IFF_DDS, path.c_str());
}

void EnvironmentMap::Initialize()
{
	shader = new Shader(L"127_BakingMap.fx");

	// create texture cube
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.ArraySize = 6;
		desc.Width = (UINT)EnvSize;
		desc.Height = (UINT)EnvSize;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &cubeEnvironment));

		desc.MipLevels = 1;
		desc.Width = (UINT)IrrSize;
		desc.Height = (UINT)IrrSize;
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &cubeIrradiance));

		desc.Width = (UINT)TestSize;
		desc.Height = (UINT)TestSize;
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &cubeTest));

		for (UINT i = 0; i < MaxMipLevel; i++)
		{
			desc.Width = (UINT)(RefSize * powf(0.5, (float)i));
			desc.Height = (UINT)(RefSize * powf(0.5, (float)i));
			Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &cubeCopyToReflection[i]));
		}
	}

	// multi render target view resource
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvdesc;
		ZeroMemory(&rtvdesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rtvdesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		rtvdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvdesc.Texture2DArray.ArraySize = 6;

		Check(D3D::GetDevice()->CreateRenderTargetView(cubeEnvironment, &rtvdesc, &rtvEnvironment));
		Check(D3D::GetDevice()->CreateRenderTargetView(cubeIrradiance, &rtvdesc, &rtvIrradiance));
		Check(D3D::GetDevice()->CreateRenderTargetView(cubeTest, &rtvdesc, &rtvTest));

		for (UINT i = 0; i < MaxMipLevel; i++)
		{
			Check(D3D::GetDevice()->CreateRenderTargetView(cubeCopyToReflection[i], &rtvdesc, &rtvCopyToReflection[i]));
		}
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
		ZeroMemory(&srvdesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvdesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvdesc.TextureCube.MipLevels = 1;

		Check(D3D::GetDevice()->CreateShaderResourceView(cubeEnvironment, &srvdesc, &srvEnvironment));
		Check(D3D::GetDevice()->CreateShaderResourceView(cubeIrradiance, &srvdesc, &srvIrradiance));
		Check(D3D::GetDevice()->CreateShaderResourceView(cubeTest, &srvdesc, &srvTest));

		for (UINT i = 0; i < MaxMipLevel; i++)
		{
			Check(D3D::GetDevice()->CreateShaderResourceView(cubeCopyToReflection[i], &srvdesc, &srvCopyToReflection[i]));
		}
	}

	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.Width = (UINT)EnvSize;
		desc.Height = (UINT)EnvSize;
		desc.ArraySize = 6;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &mapDSVEnvironment));

		desc.Width = (UINT)IrrSize;
		desc.Height = (UINT)IrrSize;
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &mapDSVIrradiance));

		desc.Width = (UINT)TestSize;
		desc.Height = (UINT)TestSize;
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &mapDSVTest));

		for (UINT i = 0; i < MaxMipLevel; i++)
		{
			desc.Width = (UINT)(RefSize * powf(0.5f, (float)i));
			desc.Height = (UINT)(RefSize * powf(0.5f, (float)i));
			Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &mapDSVCopyToReflection[i]));
		}
	}

	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvdesc;
		ZeroMemory(&dsvdesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dsvdesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvdesc.Texture2DArray.ArraySize = 6;

		Check(D3D::GetDevice()->CreateDepthStencilView(mapDSVEnvironment, &dsvdesc, &dsvEnvironment));
		Check(D3D::GetDevice()->CreateDepthStencilView(mapDSVIrradiance, &dsvdesc, &dsvIrradiance));
		Check(D3D::GetDevice()->CreateDepthStencilView(mapDSVTest, &dsvdesc, &dsvTest));

		for (UINT i = 0; i < MaxMipLevel; i++)
		{
			Check(D3D::GetDevice()->CreateDepthStencilView(mapDSVCopyToReflection[i], &dsvdesc, &dsvCopyToReflection[i]));
		}
	}

	{
		vpEnvironment = new Viewport(EnvSize, EnvSize);
		vpIrradiance = new Viewport(IrrSize, IrrSize);
		vpTest = new Viewport(TestSize, TestSize);
	}

	{
		cube = new MeshRender(shader, new MeshCube());
		Transform* transform = cube->AddTransform();
		transform->Scale(Vector3(10, 10, 10));
		cube->UpdateTransforms();
	}

	{
		sphere = new MeshRender(shader, new MeshSphere(0.5));
		Transform* transform = sphere->AddTransform();
		transform->Scale(Vector3(10, 10, 10));
		sphere->UpdateTransforms();
	}


	//Create Views
	{
		float x = 0;
		float y = 0;
		float z = 0;

		Vector3 position = { 0,0,0 };
		Vector3 scale = { 1,1,1 };

		struct LookAt
		{
			Vector3 LookAt;
			Vector3 Up;
		} lookAt[6];

		lookAt[0] = { Vector3(x + scale.x, y , z), Vector3(0,1,0) }; // right
		lookAt[1] = { Vector3(x - scale.x, y , z), Vector3(0,1,0) }; // left
		lookAt[2] = { Vector3(x , y + scale.y , z), Vector3(0,0, -1) }; // up
		lookAt[3] = { Vector3(x , y - scale.y , z), Vector3(0,0, +1) }; // bottom
		lookAt[4] = { Vector3(x , y, z + scale.z), Vector3(0,1,0) }; // front
		lookAt[5] = { Vector3(x , y, z - scale.z), Vector3(0,1,0) }; // down

		for (UINT i = 0; i < 6; i++)
		{
			D3DXMatrixLookAtLH(&cubeDesc.Views[i], &position, &lookAt[i].LookAt, &lookAt[i].Up);
		}		
	}
	// Create perspection
	{
		perspective = new Perspective(1.0f, 1.0f, 0.1f, 10, Math::PI / 2);
		perspective->GetMatrix(&cubeDesc.Projection);
	}

	cCubeDesc = new ConstantBuffer(&cubeDesc, sizeof(Desc));
	
	sSRVEnvironment = shader->AsSRV("EnvironmentMap");

	// reflection Cube mip map 5
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.ArraySize = 6;
		desc.MipLevels = 5;

		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.Height = (UINT)RefSize;
		desc.Width = (UINT)RefSize;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
		desc.SampleDesc.Count = 1;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &cubeReflection));
	}
	// pre filtered specular map srv
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MipLevels = 5;
		desc.TextureCube.MostDetailedMip = 0;

		Check(D3D::GetDevice()->CreateShaderResourceView(cubeReflection, &desc, &srvReflection));
	}
	// pre filtered specular map rtv
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = 6;

		// for each mipmap level ( 0,1,2,3,4)
		for (UINT i = 0; i < 5; i++)
		{
			desc.Texture2DArray.MipSlice = i;
			Check(D3D::GetDevice()->CreateRenderTargetView(cubeReflection, &desc, &rtvReflection[i]));
		}
	}
	// pre filtered specular map for dsv
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.Width = (UINT)RefSize;
		desc.Height = (UINT)RefSize;
		desc.ArraySize = 6;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.MipLevels = 5;
		desc.SampleDesc.Count = 1;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &mapDSVReflection));
	}
	// pre filtered specular dsv
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvdesc;
		ZeroMemory(&dsvdesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dsvdesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvdesc.Texture2DArray.ArraySize = 6;

		for (UINT i = 0; i < 5; i++)
		{
			dsvdesc.Texture2DArray.MipSlice = i;
			Check(D3D::GetDevice()->CreateDepthStencilView(mapDSVReflection,&dsvdesc,&dsvReflection[i]));
		}
	}
	// pre filtered specular viewport
	{
		for (UINT i = 0; i < 5; i++)
		{
			float vpsize = RefSize * powf(0.5, (float)i);
			vpReflection[i] = new Viewport(vpsize, vpsize);
		}
	}

	
}

