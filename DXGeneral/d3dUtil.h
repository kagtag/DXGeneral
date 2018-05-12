#pragma once

//Handles D3D intialization

#ifndef _D3DUTIL_H_
#define _D3DUTIL_H_

#if defined(DEBUG) | defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include<crtdbg.h>
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include"d3dx11effect.h"

//#include<d3d11.h>
#include "MathHelper.h"
#include "LightHelper.h"

#include<d3dcompiler.h>

#include<vector>
#include<string>
#include<sstream>
#include<fstream>


#include "dxerr.h"
#include "DDSTextureLoader.h"

//#include<DirectXPackedVector.h>
//using namespace DirectX;


#if defined(DEBUG) | defined(_DEBUG)
	#ifndef HR
	#define HR(x) \
	{ \
		HRESULT hr=(x); \
		if(FAILED(hr))	\
		{\
			DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true);\
		}\
	}
#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif

#define ReleaseCOM(x) {if(x) { x->Release(); x=0; }}

#define SafeDelete(x) { delete x; x=0; }


void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename);

//For chapter 11 geometry shader
class D3DHelper
{

	//
	//static ID3D11ShaderResourceView* CreateTexture2DArraySRV(
	//	ID3D11Device* device, ID3D11DeviceContext* context,
	//	std::vector<std::wstring>& filenames,
	//	
	//)

	static ID3D11ShaderResourceView* CreateRandomTexture1DSRV(ID3D11Device* device);
};

class TextHelper
{
public:
	template<typename T>
	static inline std::wstring ToString(const T& s)
	{
		std::wostringstream oss;
		oss << s;

		return oss.str();
	}

	template<typename T>
	static inline T FromString(const std::wstring& s)
	{
		T x;
		std::wistringstream iss(s);
		iss >> x;

		return x;
	}
};

void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M);

namespace Colors
{ 
	//const XMVECTOR instances should use the XMVECTORF32 type
	XMGLOBALCONST XMVECTORF32 White = { 1.0f,1.0f,1.0f,1.0f };
	XMGLOBALCONST XMVECTORF32 Black = { 0.0f,0.0f,0.0f,1.0f };
	XMGLOBALCONST XMVECTORF32 Red = { 1.0f,0.0f,0.0f,1.0f };
	XMGLOBALCONST XMVECTORF32 Green = { 0.0f,1.0f,0.0f,1.0f };
	XMGLOBALCONST XMVECTORF32 Blue = { 0.0f,0.0f,1.0f,1.0f };
	XMGLOBALCONST XMVECTORF32 Yellow = { 1.0f,1.0f,0.0f,1.0f };
	XMGLOBALCONST XMVECTORF32 Cyan = { 0.0f,1.0f,1.0f,1.0f };
	XMGLOBALCONST XMVECTORF32 Magenta = { 1.0f,0.0f,1.0f,1.0f };

	XMGLOBALCONST XMVECTORF32 Silver = { 0.75f,0.75f,0.75f,1.0f };
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = { 0.69f,0.77f,0.87f,1.0f };
}

//Utility class for comverting between types and formats
class Convert
{
public:

	//XMVECTOR TO XMCOLOR
	//unneccessary

	//XMVECTOR to XMFLOAT4
	static inline XMFLOAT4 ToXmFloat4(FXMVECTOR v)
	{
		XMFLOAT4 dest;
		XMStoreFloat4(&dest, v);
		return dest;
	}

	static inline UINT ArgbToAbgr(UINT argb)
	{
		BYTE A = (argb >> 24) & 0xff;
		BYTE R = (argb >> 16) & 0xff;
		BYTE G = (argb >> 8) & 0xff;
		BYTE B = (argb >> 0) & 0xff;

		return (A << 24) | (B << 16) | (G << 8) | (R << 0);
	}
};

#endif