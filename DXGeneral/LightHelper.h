#pragma once

#include<Windows.h>

#include<DirectXMath.h>
using namespace DirectX;

//Note: Make sure sturcture alignment agrees with HLSL structure padding rules.
// Elements are packed into 4D vectors with the restriction that an element
// cannot straddle a 4D vector boundary
//Different from C++

struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT3 Direction;
	float Pad;
};

struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	//Packed into 4D vector : (Position, Range)
	XMFLOAT3 Position;
	float Range;

	//Packed into 4D vector
	XMFLOAT3 Att;
	float Pad;
};

struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	
	XMFLOAT3 Position;
	float Range;

	XMFLOAT3 Direction;
	float Spot;

	XMFLOAT3 Att;
	float Pad;
};

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular; // w = SpecPower
	//XMFLOAT4 Reflect;

};