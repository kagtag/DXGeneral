#include "AppHelper.h"

#include "SkyEffects.h"
#include "SkyVertex.h"

#include "Sky.h"

#include "Vertex.h"

class DynamicCubeMapApp : public ShapesBaseApp
{
public:
	DynamicCubeMapApp(HINSTANCE hInstance);
	~DynamicCubeMapApp();

	bool Init();
	//void OnResize();
	void UpdateScene(float dt);
	bool DrawScene();

private:
	void DrawScene(const Camera& camera, bool drawSkull);
	void BuildCubeFaceCamera(float x, float y, float z);
	void BuildDynamicCubeMapViews();

private:

	Sky* mSky;

	ID3D11DepthStencilView* mDynamicCubeMapDSV;
	ID3D11RenderTargetView* mDynamicCubeMapRTV[6];
	ID3D11ShaderResourceView* mDynamicCubeMapSRV;
	D3D11_VIEWPORT mCubeMapViewport;

	static const int CubeMapSize = 256;

	Material mCenterSphereMat;

	XMFLOAT4X4 mCenterSphereWorld;

	Camera mCubeMapCamera[6];

};

DEFAULT_WINMAIN(DynamicCubeMapApp)

DynamicCubeMapApp::DynamicCubeMapApp(HINSTANCE hInstance)
	:ShapesBaseApp(hInstance),
	mSky(0),
	mDynamicCubeMapDSV(0),
	mDynamicCubeMapSRV(0)
{
	m_mainWndCaption = L"Dynamic CubeMap Demo";

	BuildCubeFaceCamera(0.0f, 2.0f, 0.0f);

	for (int i = 0; i < 6; ++i)
	{
		mDynamicCubeMapRTV[i] = 0;
	}

	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&mCenterSphereWorld, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	mSkullMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mSkullMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSphereMat.Ambient = XMFLOAT4(0.6f, 0.8f, 1.0f, 1.0f);
	mSphereMat.Diffuse = XMFLOAT4(0.6f, 0.8f, 1.0f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mSphereMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mCenterSphereMat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mCenterSphereMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mCenterSphereMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mCenterSphereMat.Reflect = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
}

DynamicCubeMapApp::~DynamicCubeMapApp()
{
	SafeDelete(mSky);

	ReleaseCOM(mDynamicCubeMapDSV);
	ReleaseCOM(mDynamicCubeMapSRV);

	for (int i = 0; i < 6; ++i)
		ReleaseCOM(mDynamicCubeMapRTV[i]);

	SkyEffects::DestroyAll();
	SkyInputLayouts::DestroyAll();
}
bool DynamicCubeMapApp::Init()
{
	if (!ShapesBaseApp::Init())
		return false;

	SkyEffects::InitAll(m_d3dDevice);
	SkyInputLayouts::InitAll(m_d3dDevice);

	mSky = new Sky(m_d3dDevice, L"../Textures/sunsetcube1024.dds", 5000.0f);

	BuildDynamicCubeMapViews();

	return true;
}

void DynamicCubeMapApp::UpdateScene(float dt)
{
	ShapesBaseApp::UpdateScene(dt);

	//
	// Switch the number of lights based on key presses
	//

	if (GetAsyncKeyState('0') & 0x8000)
		mLightCount = 0;

	if (GetAsyncKeyState('1') & 0x8000)
		mLightCount = 1;
	
	if (GetAsyncKeyState('2') & 0x8000)
		mLightCount = 2;

	if (GetAsyncKeyState('3') & 0x8000)
		mLightCount = 3;

	//
	// Animate the skull around the center sphere
	//

	XMMATRIX skullScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
	XMMATRIX skullOffset = XMMatrixTranslation(3.0f, 2.0f, 0.0f);
	XMMATRIX skullLocalRotate = XMMatrixRotationY(2.0f * m_timer.TotalTime());
	XMMATRIX skullGlobalRotate = XMMatrixRotationY(0.5f*m_timer.TotalTime());
	XMStoreFloat4x4(&mSkullWorld, skullScale*skullLocalRotate*skullOffset*skullGlobalRotate);

}

bool DynamicCubeMapApp::DrawScene()
{
	ID3D11RenderTargetView* renderTargets[1];

	// Generate the cube map
	m_d3dImmediateContext->RSSetViewports(1, &mCubeMapViewport);
	for (int i = 0; i < 6; ++i)
	{
		// Clear cube map face and depth buffer
		PreProcessing(mDynamicCubeMapRTV[i], mDynamicCubeMapDSV);

		// Bind cube map face as render target
		renderTargets[0] = mDynamicCubeMapRTV[i];
		m_d3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDynamicCubeMapDSV);

		// Draw the scene with the exception of the center sphere to this cube map face
		DrawScene(mCubeMapCamera[i], false);
	}

	// Restore old viewport and render targets
	m_d3dImmediateContext->RSSetViewports(1, &m_screenViewport);
	renderTargets[0] = m_renderTargetView;
	m_d3dImmediateContext->OMSetRenderTargets(1, renderTargets, m_depthStencilView);

	// Have hardware generate lower mipmap levels of cube map
	m_d3dImmediateContext->GenerateMips(mDynamicCubeMapSRV);

	// Now draw the scene as normal, but with the center sphere
	PreProcessing();

	DrawScene(mCam, true);

	HR(m_swapChain->Present(0, 0));

	return true;
}

void DynamicCubeMapApp::DrawScene(const Camera& camera, bool drawCenterSphere)
{
	m_d3dImmediateContext->IASetInputLayout(SkyInputLayouts::Basic32);
	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = camera.View();
	XMMATRIX proj = camera.Proj();
	XMMATRIX viewProj = camera.ViewProj();

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Set per frame constants
	SkyEffects::BasicFX->SetDirLights(mDirLights);
	SkyEffects::BasicFX->SetEyePosW(mCam.GetPosition());

	// Figure out which technique to use.   

	ID3DX11EffectTechnique* activeTexTech = SkyEffects::BasicFX->Light1TexTech;
	ID3DX11EffectTechnique* activeSkullTech = SkyEffects::BasicFX->Light1Tech;
	ID3DX11EffectTechnique* activeReflectTech = SkyEffects::BasicFX->Light1ReflectTech;
	switch (mLightCount)
	{
	case 1:
		activeTexTech = SkyEffects::BasicFX->Light1TexTech;
		activeSkullTech = SkyEffects::BasicFX->Light1Tech;
		activeReflectTech = SkyEffects::BasicFX->Light1ReflectTech;
		break;
	case 2:
		activeTexTech = SkyEffects::BasicFX->Light2TexTech;
		activeSkullTech = SkyEffects::BasicFX->Light2Tech;
		activeReflectTech = SkyEffects::BasicFX->Light2ReflectTech;
		break;
	case 3:
	default:
		activeTexTech = SkyEffects::BasicFX->Light3TexTech;
		activeSkullTech = SkyEffects::BasicFX->Light3Tech;
		activeReflectTech = SkyEffects::BasicFX->Light3ReflectTech;
		break;
	}

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	//
	// Draw the skull
	//

	D3DX11_TECHNIQUE_DESC techDesc;
	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
		m_d3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		MATRICES_SET(SkyEffects::BasicFX, mSkullWorld)

		SkyEffects::BasicFX->SetTexTransform(XMMatrixIdentity());
		SkyEffects::BasicFX->SetMaterial(mSkullMat);

		activeSkullTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
		m_d3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}


	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);
	//
	// Draw the grid, cylinders, spheres and box without any cubemap reflection
	//

	activeTexTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the grid
		MATRICES_SET(SkyEffects::BasicFX, mGridWorld)

		SkyEffects::BasicFX->SetTexTransform(XMMatrixScaling(6.0f, 8.0f, 1.0f));
		SkyEffects::BasicFX->SetMaterial(mGridMat);
		SkyEffects::BasicFX->SetDiffuseMap(mFloorTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
		m_d3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// Draw the box
		MATRICES_SET(SkyEffects::BasicFX, mBoxWorld)

		SkyEffects::BasicFX->SetTexTransform(XMMatrixIdentity());
		SkyEffects::BasicFX->SetMaterial(mBoxMat);
		SkyEffects::BasicFX->SetDiffuseMap(mStoneTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
		m_d3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// Draw the cylinders
		for (int i = 0; i < 10; ++i)
		{
			MATRICES_SET(SkyEffects::BasicFX, mCylWorld[i])

			SkyEffects::BasicFX->SetTexTransform(XMMatrixIdentity());
			SkyEffects::BasicFX->SetMaterial(mCylinderMat);
			SkyEffects::BasicFX->SetDiffuseMap(mBrickTexSRV);

			activeTexTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
			m_d3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		// Draw the spheres
		for (int i = 0; i < 10; ++i)
		{
			MATRICES_SET(SkyEffects::BasicFX, mSphereWorld[i])

			SkyEffects::BasicFX->SetTexTransform(XMMatrixIdentity());
			SkyEffects::BasicFX->SetMaterial(mSphereMat);
			SkyEffects::BasicFX->SetDiffuseMap(mStoneTexSRV);

			activeTexTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
			m_d3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	// Draw the center sphere with the dynamic cube map
	if (drawCenterSphere)
	{
		activeReflectTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			// Draw the center sphere

			MATRICES_SET(SkyEffects::BasicFX, mCenterSphereWorld)

			SkyEffects::BasicFX->SetTexTransform(XMMatrixIdentity());
			SkyEffects::BasicFX->SetMaterial(mCenterSphereMat);
			SkyEffects::BasicFX->SetDiffuseMap(mStoneTexSRV);

			SkyEffects::BasicFX->SetCubeMap(mDynamicCubeMapSRV);

			activeReflectTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
			m_d3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	mSky->Draw(m_d3dImmediateContext, camera, SkyEffects::SkyFX);

	// restore default states
	m_d3dImmediateContext->RSSetState(0);
	m_d3dImmediateContext->OMSetDepthStencilState(0, 0);
}

void DynamicCubeMapApp::BuildCubeFaceCamera(float x, float y, float z)
{
	// Generate the cube map about the given position
	XMFLOAT3 center(x, y, z);
	XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

	// Look along each coordinate axis
	XMFLOAT3 targets[6] =
	{
		XMFLOAT3(x + 1.0f, y, z), //+x
		XMFLOAT3(x - 1.0f, y, z),
		XMFLOAT3(x , y + 1.0f, z),
		XMFLOAT3(x , y - 1.0f, z),
		XMFLOAT3(x , y, z + 1.0f),
		XMFLOAT3(x , y, z - 1.0f)
	};

	// Use world up vector (0,1,0) for all directions except +Y/-Y. In these cases, we
	// are looking down +Y or -Y, so we need a different up vector
	XMFLOAT3 ups[6] =
	{
		XMFLOAT3(0.0f,1.0f,0.0f), //
		XMFLOAT3(0.0f,1.0f,0.0f),
		XMFLOAT3(0.0f,0.0f,-1.0f),
		XMFLOAT3(0.0f,0.0f,1.0f),
		XMFLOAT3(0.0f,1.0f,0.0f),
		XMFLOAT3(0.0f,1.0f,0.0f)
	};

	for (int i = 0; i < 6; ++i)
	{
		mCubeMapCamera[i].LookAt(center, targets[i], ups[i]);
		mCubeMapCamera[i].SetLens(0.5f*XM_PI, 1.0f, 0.1f, 1000.0f);
		mCubeMapCamera[i].UpdateViewMatrix();
	}

}

void DynamicCubeMapApp::BuildDynamicCubeMapViews()
{
	//
	// Cubemap is a special texture array with 6 elements
	//

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = CubeMapSize;
	texDesc.Height = CubeMapSize;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE; //

	ID3D11Texture2D* cubeTex = 0;
	HR(m_d3dDevice->CreateTexture2D(&texDesc, 0, &cubeTex));

	//
	// Create a render target view to each cube map face
	// i.e. each element in the texture array
	//

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	for (int i = 0; i < 6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		HR(m_d3dDevice->CreateRenderTargetView(cubeTex, &rtvDesc, &mDynamicCubeMapRTV[i]));
	}

	//
	// Create a shader resource view to the cube map
	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	HR(m_d3dDevice->CreateShaderResourceView(cubeTex, &srvDesc, &mDynamicCubeMapSRV));

	ReleaseCOM(cubeTex);


	// Create the depth stencil view for the entire cube
	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = CubeMapSize;
	depthTexDesc.Height = CubeMapSize;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	ID3D11Texture2D* depthTex = 0;
	HR(m_d3dDevice->CreateTexture2D(&depthTexDesc, 0, &depthTex));

	// Create the depth stencil view for the entire cube
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(m_d3dDevice->CreateDepthStencilView(depthTex, &dsvDesc, &mDynamicCubeMapDSV));

	ReleaseCOM(depthTex);

	//
	// Viewport for drawing into cubemap
	//

	mCubeMapViewport.TopLeftX = 0.0f;
	mCubeMapViewport.TopLeftY = 0.0f;
	mCubeMapViewport.Width = (float)CubeMapSize;
	mCubeMapViewport.Height = (float)CubeMapSize;
	mCubeMapViewport.MinDepth = 0.0f;
	mCubeMapViewport.MaxDepth = 1.0f;
}

