#include "NorDisVertex.h"

#include "NorDisEffects.h"

#pragma region NorDisInputLayoutDesc

const D3D11_INPUT_ELEMENT_DESC NorDisInputLayoutDesc::PosNormalTexTan[4] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

#pragma endregion

#include "Vertex.h"
#include "SkyVertex.h"

#pragma region NorDisInputLayouts

ID3D11InputLayout* NorDisInputLayouts::Pos = 0;
ID3D11InputLayout* NorDisInputLayouts::Basic32 = 0;
ID3D11InputLayout* NorDisInputLayouts::PosNormalTexTan = 0;

void NorDisInputLayouts::InitAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;

	//
	// Pos
	//
	
	NorDisEffects::SkyFX->SkyTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(SkyInputLayoutDesc::Pos, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Pos));

	// Basic32
	NorDisEffects::BasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::Basic32, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Basic32));


	// NormalMap

	NorDisEffects::NormalMapFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(NorDisInputLayoutDesc::PosNormalTexTan, 4, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosNormalTexTan));
}

void NorDisInputLayouts::DestroyAll()
{
	ReleaseCOM(Pos);
	ReleaseCOM(Basic32);
	ReleaseCOM(PosNormalTexTan);
}

#pragma endregion

