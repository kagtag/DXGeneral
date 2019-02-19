#pragma once

#include"d3dUtil.h"

class MyInputLayoutDesc
{
public:
	static const D3D11_INPUT_ELEMENT_DESC InstancedBasic32[8];
};

class MyInputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* InstancedBasic32;
};

