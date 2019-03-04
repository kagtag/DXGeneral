#include "ParEffects.h"

#pragma region ParticleEffect

ParticleEffect::ParticleEffect(ID3D11Device* device, const std::wstring& filename)
	:Effect(device, filename)
{
	StreamOutTech = mFX->GetTechniqueByName("StreamOutTech");
	DrawTech = mFX->GetTechniqueByName("DrawTech");

	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();

	GameTime = mFX->GetVariableByName("gGameTime")->AsScalar();
	TimeStep = mFX->GetVariableByName("gTimeStep")->AsScalar();

	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	EmitPosW = mFX->GetVariableByName("gEmitPosW")->AsVector();
	EmitDirW = mFX->GetVariableByName("gEmitDirW")->AsVector();

	TexArray = mFX->GetVariableByName("gTexArray")->AsShaderResource();
	RandomTex = mFX->GetVariableByName("gRandomTex")->AsShaderResource();
}

ParticleEffect::~ParticleEffect()
{}

#pragma endregion

#pragma region ParEffects

SkyBasicEffect* ParEffects::BasicFX = 0;
SkyEffect* ParEffects::SkyFX = 0;
TerrainEffect* ParEffects::TerrainFX = 0;
ParticleEffect* ParEffects::FireFX = 0;
ParticleEffect* ParEffects::RainFX = 0;

void ParEffects::InitAll(ID3D11Device* device)
{
	BasicFX = new SkyBasicEffect(device, L"FX/Basic.cso");
	SkyFX = new SkyEffect(device, L"FX/Sky.cso");
	TerrainFX = new TerrainEffect(device, L"FX/Terrain.cso");

	FireFX = new ParticleEffect(device, L"FX/Fire.cso");
	RainFX = new ParticleEffect(device, L"FX/Rain.cso");

}

void ParEffects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(SkyFX);
	SafeDelete(TerrainFX);
	SafeDelete(FireFX);
	SafeDelete(RainFX);

}
#pragma endregion