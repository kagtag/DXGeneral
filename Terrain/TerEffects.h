#pragma once

#include "d3dUtil.h"
#include "SkyEffects.h"

#pragma region TerBasicEffect

class TerBasicEffect : public SkyBasicEffect
{
public:
	TerBasicEffect(ID3D11Device* device, const std::wstring& filename);
	~TerBasicEffect();
};

#pragma endregion

#pragma region TerrainEffect

class TerrainEffect : public Effect
{
public:
	TerrainEffect(ID3D11Device* device, const std::wstring& filename);
	~TerrainEffect();

	void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& V) { EyePosW->SetRawValue(&V, 0, sizeof(XMFLOAT3)); }
	void SetFogColor(const FXMVECTOR V) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&V)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }

	void SetMinDist(float f) { MinDist->SetFloat(f); }
	void SetMaxDist(float f) { MaxDist->SetFloat(f); }
	void SetMinTess(float f) { MinTess->SetFloat(f); }
	void SetMaxTess(float f) { MaxTess->SetFloat(f); }

	void SetTexelCellSpaceU(float f) { TexelCellSpaceU->SetFloat(f); }
	void SetTexelCellSpaceV(float f) { TexelCellSpaceV->SetFloat(f); }
	void SetWorldCellSpace(float f) { WorldCellSpace->SetFloat(f); }
	
	void SetWorldFrustumPlanes(XMFLOAT4 planes[6]) { WorldFrustumPlanes->SetFloatVectorArray(reinterpret_cast<float*>(planes), 0, 6); }

	void SetLayerMapArray(ID3D11ShaderResourceView* tex) { LayerMapArray->SetResource(tex); }
	void SetBlendMap(ID3D11ShaderResourceView* tex) { BlendMap->SetResource(tex); }
	void SetHeightMap(ID3D11ShaderResourceView* tex) { HeightMap->SetResource(tex); }

	ID3DX11EffectTechnique* Light1Tech;
	ID3DX11EffectTechnique* Light2Tech;
	ID3DX11EffectTechnique* Light3Tech;
	ID3DX11EffectTechnique* Light1FogTech;
	ID3DX11EffectTechnique* Light2FogTech;
	ID3DX11EffectTechnique* Light3FogTech;

	ID3DX11EffectMatrixVariable* ViewProj;
	//ID3DX11EffectMatrixVariable* World;
	//ID3DX11EffectMatrixVariable* WorldInvTranspose;
	//ID3DX11EffectMatrixVariable* TexTransform;

	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* FogColor;

	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;

	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectScalarVariable* MinDist;
	ID3DX11EffectScalarVariable* MaxDist;
	ID3DX11EffectScalarVariable* MinTess;
	ID3DX11EffectScalarVariable* MaxTess;

	ID3DX11EffectScalarVariable* TexelCellSpaceU;
	ID3DX11EffectScalarVariable* TexelCellSpaceV;
	ID3DX11EffectScalarVariable* WorldCellSpace;

	ID3DX11EffectVectorVariable* WorldFrustumPlanes;

	ID3DX11EffectShaderResourceVariable* LayerMapArray;
	ID3DX11EffectShaderResourceVariable* BlendMap;
	ID3DX11EffectShaderResourceVariable* HeightMap;

};

#pragma endregion

#pragma region TerEffects

class TerEffects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static TerBasicEffect* BasicFX;
	static SkyEffect* SkyFX;
	static TerrainEffect* TerrainFX;
};

#pragma endregion