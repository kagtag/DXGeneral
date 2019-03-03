#pragma once

#include "d3dUtil.h"

#include "SkyVertex.h"

namespace Vertex
{
	struct Terrain
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
		XMFLOAT2 BoundsY;
	};
}

class TerInputLayoutDesc
{
public:
	static const D3D11_INPUT_ELEMENT_DESC Terrain[3];
};

class TerInputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* Pos;
	static ID3D11InputLayout* Basic32;
	static ID3D11InputLayout* Terrain;
};
