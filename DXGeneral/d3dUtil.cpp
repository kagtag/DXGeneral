
#include "d3dUtil.h"

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
void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M)
{

}