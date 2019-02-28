#pragma once

#include "d3dUtil.h"

#include "SkyEffects.h"

#pragma region NorDisBasicEffect

class NorDisBasicEffect : public SkyBasicEffect
{
public:
	NorDisBasicEffect(ID3D11Device* device, const std::wstring& filename);
	~NorDisBasicEffect();
};

#pragma endregion


#pragma region NormalMapEffect

class NormalMapEffect : public NorDisBasicEffect
{
public:
	NormalMapEffect(ID3D11Device* device, const std::wstring& filename);
	~NormalMapEffect();

	void SetNormalMap(ID3D11ShaderResourceView* tex) { NormalMap->SetResource(tex); }

	ID3DX11EffectShaderResourceVariable* NormalMap;
};

#pragma endregion


#pragma region DisplacementMapEffect

class DisplacementMapEffect : public NormalMapEffect
{
public:
	DisplacementMapEffect(ID3D11Device* device, const std::wstring& filename);
	~DisplacementMapEffect();

	void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	
	void SetHeightScale(float f) { HeightScale->SetFloat(f); }
	
	void SetMaxTessDistance(float f) { MaxTessDistance->SetFloat(f); }
	void SetMinTessDistance(float f) { MinTessDistance->SetFloat(f); }

	void SetMaxTessFactor(float f) { MaxTessFactor->SetFloat(f); }
	void SetMinTessFactor(float f) { MinTessFactor->SetFloat(f); }

	ID3DX11EffectMatrixVariable* ViewProj;

	ID3DX11EffectScalarVariable* HeightScale;
	ID3DX11EffectScalarVariable* MaxTessDistance;
	ID3DX11EffectScalarVariable* MinTessDistance;
	ID3DX11EffectScalarVariable* MaxTessFactor;
	ID3DX11EffectScalarVariable* MinTessFactor;


};

#pragma endregion


#pragma region NorDisEffects

class NorDisEffects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static NorDisBasicEffect* BasicFX;
	static NormalMapEffect* NormalMapFX;
	static DisplacementMapEffect* DisplacementMapFX;
	static SkyEffect* SkyFX;
};

#pragma endregion