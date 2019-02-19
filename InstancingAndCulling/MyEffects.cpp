
#include"MyEffects.h"

InstancedBasicEffect::InstancedBasicEffect(ID3D11Device* device, const std::wstring& filename)
	:BasicEffect(device, filename)
{
	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();
}

InstancedBasicEffect::~InstancedBasicEffect() {}


#pragma region Effects

InstancedBasicEffect* MyEffects::InstancedBasicFX = 0;

void MyEffects::InitAll(ID3D11Device* device)
{
	InstancedBasicFX = new InstancedBasicEffect(device, L"FX/InstancedBasic.cso");
}

void MyEffects::DestroyAll()
{
	SafeDelete(InstancedBasicFX);
}
#pragma endregion
