#include "SkyEffects.h"

#pragma region SkyBasicEffect

SkyBasicEffect::SkyBasicEffect(ID3D11Device* device, const std::wstring& filename)
	:BasicEffect(device, filename)
{
	CubeMap = mFX->GetVariableByName("gCubeMap")->AsShaderResource();

	Light1ReflectTech = mFX->GetTechniqueByName("Light1Reflect");
	Light2ReflectTech = mFX->GetTechniqueByName("Light2Reflect");
	Light3ReflectTech = mFX->GetTechniqueByName("Light3Reflect");

	Light0TexReflectTech = mFX->GetTechniqueByName("Light0TexReflect");
	Light1TexReflectTech = mFX->GetTechniqueByName("Light1TexReflect");
	Light2TexReflectTech = mFX->GetTechniqueByName("Light2TexReflect");
	Light3TexReflectTech = mFX->GetTechniqueByName("Light3TexReflect");

	Light0TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipReflect");
	Light1TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipReflect");
	Light2TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipReflect");
	Light3TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipReflect");

	Light1FogReflectTech = mFX->GetTechniqueByName("Light1FogReflect");
	Light2FogReflectTech = mFX->GetTechniqueByName("Light2FogReflect");
	Light3FogReflectTech = mFX->GetTechniqueByName("Light3FogReflect");

	Light0TexFogReflectTech = mFX->GetTechniqueByName("Light0TexFogReflect");
	Light1TexFogReflectTech = mFX->GetTechniqueByName("Light1TexFogReflect");
	Light2TexFogReflectTech = mFX->GetTechniqueByName("Light2TexFogReflect");
	Light3TexFogReflectTech = mFX->GetTechniqueByName("Light3TexFogReflect");

	Light0TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipFogReflect");
	Light1TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipFogReflect");
	Light2TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipFogReflect");
	Light3TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipFogReflect");
}

SkyBasicEffect::~SkyBasicEffect()
{}

#pragma endregion

#pragma region SkyEffect
SkyEffect::SkyEffect(ID3D11Device* device, const std::wstring& filename)
	:Effect(device, filename)
{
	SkyTech = mFX->GetTechniqueByName("SkyTech");
	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	CubeMap = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
}

SkyEffect::~SkyEffect()
{
}

#pragma endregion

#pragma region MyEffects

SkyBasicEffect* SkyEffects::BasicFX = 0;
SkyEffect*   SkyEffects::SkyFX = 0;

void SkyEffects::InitAll(ID3D11Device* device)
{
	BasicFX = new SkyBasicEffect(device, L"FX/Basic.cso");
	SkyFX = new SkyEffect(device, L"FX/Sky.cso");
}

void SkyEffects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(SkyFX);
}

#pragma endregion