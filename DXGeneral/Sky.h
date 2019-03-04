#pragma once

#include "d3dUtil.h"

#include "SkyEffects.h"

class Camera;

class Sky
{
public:
	Sky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius);
	~Sky();

	ID3D11ShaderResourceView* CubeMapSRV();

	void Draw(ID3D11DeviceContext* dc, const Camera& camera, ID3D11InputLayout* layout, SkyEffect* fx);

private:
	Sky(const Sky& rhs);
	Sky& operator=(const Sky& rhs);

private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3D11ShaderResourceView* mCubeMapSRV;

	UINT mIndexCount;

};