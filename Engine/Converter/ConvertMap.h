#pragma once
#include "Systems/IExecute.h"

class ConvertMap : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual void ResizeScreen() override {}

	void BakeMaps(wstring env, wstring irr, wstring ref);


	void ExcuteBake();
	void EnvPath(wstring path);
	void IrrPath(wstring path);
	void RefPath(wstring path);

	void Bake1();


private:
	class EnvironmentMap* bakingMap;
	class BRDFMap* brdfMap;

	function<void(wstring)> envf;
	function<void(wstring)> irrf;
	function<void(wstring)> reff;

	wstring semi = L"";
	wstring envPath = L"";
	wstring irrPath = L"";
	wstring refPath = L"";
private:
	CubeSky* sky;
	Render2D* render2D;

	Texture* texture;
};