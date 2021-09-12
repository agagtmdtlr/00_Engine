#pragma once

class RendererPBR
{
	virtual void Update() = 0;
	virtual void Render() = 0;	
	virtual void Pass(UINT pass) = 0;

};

