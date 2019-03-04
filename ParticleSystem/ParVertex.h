#pragma once

#include "d3dUtil.h"

namespace Vertex
{
	struct Particle
	{
		XMFLOAT3 InitialPos;
		XMFLOAT3 InitialVel;
		XMFLOAT2 Size;
		float Age;
		unsigned int Type;
	};
}

class ParInputLayoutDesc
{
public:
	static const D3D11_INPUT_ELEMENT_DESC Particle[5];
};

class ParInputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* Pos;
	static ID3D11InputLayout* Basic32;
	static ID3D11InputLayout* Terrain;
	static ID3D11InputLayout* Particle;
};