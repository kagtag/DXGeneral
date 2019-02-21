#include"SkyVertex.h"
#include"SkyEffects.h"

#pragma region InputLayoutDesc

const D3D11_INPUT_ELEMENT_DESC SkyInputLayoutDesc::Pos[1]=
{
	{"POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D11_INPUT_PER_VERTEX_DATA,0}
};

const D3D11_INPUT_ELEMENT_DESC SkyInputLayoutDesc::Basic32[3] =
{
	{ "POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "NORMAL",0, DXGI_FORMAT_R32G32B32_FLOAT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "TEXCOORD",0, DXGI_FORMAT_R32G32_FLOAT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
};

#pragma endregion

#pragma region InputLayouts

ID3D11InputLayout* SkyInputLayouts::Pos = 0;
ID3D11InputLayout* SkyInputLayouts::Basic32 = 0;

void SkyInputLayouts::InitAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;

	//
	// Pos
	//
	
	SkyEffects::SkyFX->SkyTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(SkyInputLayoutDesc::Pos, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Pos));

	//
	// Basic32
	//
	SkyEffects::BasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(SkyInputLayoutDesc::Basic32, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Basic32));
}

void SkyInputLayouts::DestroyAll()
{
	ReleaseCOM(Pos);
	ReleaseCOM(Basic32);
}

#pragma endregion