#include "NorDisEffects.h"

#pragma region NorDisBasicEffect
NorDisBasicEffect::NorDisBasicEffect(ID3D11Device* device, const std::wstring& filename)
	:SkyBasicEffect(device, filename)
{}
NorDisBasicEffect::~NorDisBasicEffect() {}
#pragma endregion

#pragma region NormalMapEffect
NormalMapEffect::NormalMapEffect(ID3D11Device* device, const std::wstring& filename)
	:NorDisBasicEffect(device, filename)
{
	NormalMap = mFX->GetVariableByName("gNormalMap")->AsShaderResource();
}

NormalMapEffect::~NormalMapEffect()
{}

#pragma endregion

#pragma region DisplacementMapEffect

DisplacementMapEffect::DisplacementMapEffect(ID3D11Device* device, const std::wstring& filename)
	:NormalMapEffect(device, filename)
{
	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();

	HeightScale = mFX->GetVariableByName("gHeightScale")->AsScalar();

	MaxTessDistance = mFX->GetVariableByName("gMaxTessDistance")->AsScalar();
	MinTessDistance = mFX->GetVariableByName("gMinTessDistance")->AsScalar();

	MaxTessFactor = mFX->GetVariableByName("gMaxTessFactor")->AsScalar();
	MinTessFactor = mFX->GetVariableByName("gMinTessFactor")->AsScalar();
}

DisplacementMapEffect::~DisplacementMapEffect()
{}

#pragma endregion


#pragma region NorDisEffects

NorDisBasicEffect* NorDisEffects::BasicFX = 0;
NormalMapEffect* NorDisEffects::NormalMapFX = 0;
DisplacementMapEffect* NorDisEffects::DisplacementMapFX = 0;
SkyEffect* NorDisEffects::SkyFX = 0;

void NorDisEffects::InitAll(ID3D11Device* device)
{
	BasicFX = new NorDisBasicEffect(device, L"FX/Basic.cso");
	NormalMapFX = new NormalMapEffect(device, L"FX/NormalMap.cso");
	DisplacementMapFX = new DisplacementMapEffect(device, L"FX/DisplacementMap.cso");
	SkyFX = new SkyEffect(device, L"FX/Sky.cso");
}

void NorDisEffects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(NormalMapFX);
	SafeDelete(DisplacementMapFX);
	SafeDelete(SkyFX);
}
#pragma endregion