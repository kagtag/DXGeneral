#pragma once

#include "d3dUtil.h"

namespace Vertex
{
	struct PosNormalTexTan
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
		XMFLOAT3 TangentU;
	};


}

class NorDisInputLayoutDesc
{
public:
	//static const D3D11_INPUT_ELEMENT_DESC Pos[1];
	//static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
	static const D3D11_INPUT_ELEMENT_DESC PosNormalTexTan[4];
};

class NorDisInputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* Pos;
	static ID3D11InputLayout* Basic32;
	static ID3D11InputLayout* PosNormalTexTan;
};