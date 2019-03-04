#include "ParVertex.h"
#include "TerVertex.h"
#include "Vertex.h"

#include "ParEffects.h"

#pragma region ParInputLayoutDesc

const D3D11_INPUT_ELEMENT_DESC ParInputLayoutDesc::Particle[5] =
{
	{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
	{ "VELOCITY",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "SIZE",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "AGE",0,DXGI_FORMAT_R32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "TYPE",0,DXGI_FORMAT_R32_UINT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
};

#pragma endregion

#pragma region ParInputLayouts

ID3D11InputLayout* ParInputLayouts::Pos = 0;
ID3D11InputLayout* ParInputLayouts::Basic32 = 0;
ID3D11InputLayout* ParInputLayouts::Terrain = 0;
ID3D11InputLayout* ParInputLayouts::Particle = 0;

void ParInputLayouts::InitAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;

	// Pos
	ParEffects::SkyFX->SkyTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(SkyInputLayoutDesc::Pos, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Pos));

	// Basic32
	ParEffects::BasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::Basic32, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Basic32));
	
	// Terrain
	ParEffects::TerrainFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(TerInputLayoutDesc::Terrain, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Terrain));

	// Particle
	ParEffects::FireFX->StreamOutTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(ParInputLayoutDesc::Particle, 5, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Particle));
}

void ParInputLayouts::DestroyAll()
{
	ReleaseCOM(Pos);
	ReleaseCOM(Basic32);
	ReleaseCOM(Terrain);
	ReleaseCOM(Particle);
}

#pragma endregion


