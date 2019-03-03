
#include "d3dUtil.h"
#include<fstream>
using std::ofstream;

//Output error message when compiling shader
void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize, i;
	ofstream fout;

	//Get a pointer to the error message text buffer
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	//Get the length of the message
	bufferSize = errorMessage->GetBufferSize();

	//Open a file to write the error message to
	fout.open("shader-error.txt");

	//write out the error message
	for (i = 0; i < bufferSize; ++i)
	{
		fout << compileErrors[i];
	}

	//Close the file
	fout.close();

	//Release the error message
	errorMessage->Release();
	errorMessage = 0;

	//Pop a message up on the screen to notify the user to check the text file for compile errors
	MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

//Use texassemble tool instead to create a texture array offline
//which would be the preferred process anyway

//ID3D11ShaderResourceView* d3dHelper::CreateTexture2DArraySRV(
//		ID3D11Device* device, ID3D11DeviceContext* context,
//		std::vector<std::wstring>& filenames,
//		DXGI_FORMAT format,
//		UINT filter, 
//		UINT mipFilter)

//for chap 20 particel system
ID3D11ShaderResourceView* D3DHelper::CreateRandomTexture1DSRV(ID3D11Device* device)
{
	//Create random data

	XMFLOAT4 randomValues[1024];
	
	//for (int i =0 ; i < 1024; ++i)
	//{
	//	randomValues[i].x=
	//}



	ID3D11ShaderResourceView* randomTexSRV = 0;


	return randomTexSRV;
}

//for chap19 Terrain rendering
void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX T)
{
	XMFLOAT4X4 M;
	XMStoreFloat4x4(&M, T);

	//
	// Left
	//
	planes[0].x = M(0, 3) + M(0, 0);
	planes[0].y = M(1, 3) + M(1, 0);
	planes[0].z = M(2, 3) + M(2, 0);
	planes[0].w = M(3, 3) + M(3, 0);

	//
	// Right
	//
	planes[1].x = M(0, 3) - M(0, 0);
	planes[1].y = M(1, 3) - M(1, 0);
	planes[1].z = M(2, 3) - M(2, 0);
	planes[1].w = M(3, 3) - M(3, 0);

	//
	// Bottom
	//
	planes[2].x = M(0, 3) + M(0, 1);
	planes[2].y = M(1, 3) + M(1, 1);
	planes[2].z = M(2, 3) + M(2, 1);
	planes[2].w = M(3, 3) + M(3, 1);

	//
	// Top
	//
	planes[3].x = M(0, 3) - M(0, 1);
	planes[3].y = M(1, 3) - M(1, 1);
	planes[3].z = M(2, 3) - M(2, 1);
	planes[3].w = M(3, 3) - M(3, 1);

	//
	// Near
	//
	planes[4].x = M(0, 2);
	planes[4].y = M(1, 2);
	planes[4].z = M(2, 2);
	planes[4].w = M(3, 2);

	//
	// Far
	//
	planes[5].x = M(0, 3) - M(0, 2);
	planes[5].y = M(1, 3) - M(1, 2);
	planes[5].z = M(2, 3) - M(2, 2);
	planes[5].w = M(3, 3) - M(3, 2);
	
	// Normalize the plane equations
	for (int i = 0; i < 6; ++i)
	{
		XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&planes[i]));
		XMStoreFloat4(&planes[i], v);
	}
}