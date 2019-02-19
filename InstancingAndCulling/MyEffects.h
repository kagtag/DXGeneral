#pragma once
#include"Effects.h"

class InstancedBasicEffect : public BasicEffect
{
public:
	InstancedBasicEffect(ID3D11Device* device, const std::wstring& filename);
	~InstancedBasicEffect();

	void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }

	ID3DX11EffectMatrixVariable* ViewProj;

};

#pragma region MyEffects
class MyEffects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static InstancedBasicEffect* InstancedBasicFX;
};
#pragma endregion