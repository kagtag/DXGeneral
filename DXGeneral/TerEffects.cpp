#include "TerEffects.h"

#pragma region TerBasicEffect

TerBasicEffect::TerBasicEffect(ID3D11Device* device, const std::wstring& filename)
	:SkyBasicEffect(device, filename)
{}

TerBasicEffect::~TerBasicEffect()
{}

#pragma endregion


#pragma region TerrainEffect

TerrainEffect::TerrainEffect(ID3D11Device* device, const std::wstring& filename)
	:Effect(device, filename)
{
	Light1Tech = mFX->GetTechniqueByName("Light1");
	Light2Tech = mFX->GetTechniqueByName("Light2");
	Light3Tech = mFX->GetTechniqueByName("Light3");

	Light1FogTech = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech = mFX->GetTechniqueByName("Light3Fog");

	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();


	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();

	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();

	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");

	MinDist = mFX->GetVariableByName("gMinDist")->AsScalar();
	MaxDist = mFX->GetVariableByName("gMaxDist")->AsScalar();
	MinTess = mFX->GetVariableByName("gMinTess")->AsScalar();
	MaxTess = mFX->GetVariableByName("gMaxTess")->AsScalar();

	TexelCellSpaceU = mFX->GetVariableByName("gTexelCellSpaceU")->AsScalar();
	TexelCellSpaceV = mFX->GetVariableByName("gTexelCellSpaceV")->AsScalar();
	WorldCellSpace = mFX->GetVariableByName("gWorldCellSpace")->AsScalar();

	WorldFrustumPlanes = mFX->GetVariableByName("gWorldFrustumPlanes")->AsVector();

	LayerMapArray = mFX->GetVariableByName("gLayerMapArray")->AsShaderResource();
	BlendMap = mFX->GetVariableByName("gBlendMap")->AsShaderResource();
	HeightMap = mFX->GetVariableByName("gHeightMap")->AsShaderResource();

}

TerrainEffect::~TerrainEffect()
{}

#pragma endregion

#pragma region TerEffects

TerBasicEffect* TerEffects::BasicFX = 0;
SkyEffect* TerEffects::SkyFX = 0;
TerrainEffect* TerEffects::TerrainFX = 0;

void TerEffects::InitAll(ID3D11Device* device)
{
	BasicFX = new TerBasicEffect(device, L"FX/Basic.cso");
	SkyFX = new SkyEffect(device, L"FX/Sky.cso");
	TerrainFX = new TerrainEffect(device, L"FX/Terrain.cso");
}


void TerEffects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(SkyFX);
	SafeDelete(TerrainFX);
}

#pragma endregion