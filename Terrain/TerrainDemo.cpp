#include"AppHelper.h"

#include "Sky.h"

#include "TerVertex.h"
#include "TerEffects.h"

#include "Terrain.h"

class TerrainApp : public CommonApp
{
public:
	TerrainApp(HINSTANCE hInstance);
	~TerrainApp();

	bool Init();
	void UpdateScene(float dt);
	void OnResize();
	bool DrawScene();

private:
	Sky* mSky;
	Terrain mTerrain;

	DirectionalLight mDirLights[3];


	bool mWalkCamMode;

};

DEFAULT_WINMAIN(TerrainApp)

TerrainApp::TerrainApp(HINSTANCE hInstance)
	:CommonApp(hInstance),
	mSky(0),
	mWalkCamMode(false)
{
	m_mainWndCaption = L"Terrain Demo";

	m_enable4xMsaa = false;

	mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);
}

TerrainApp::~TerrainApp()
{
	m_d3dImmediateContext->ClearState();

	SafeDelete(mSky);

	TerEffects::DestroyAll();
	TerInputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool TerrainApp::Init()
{
	if (!CommonApp::Init())
		return false;

	// 
	TerEffects::InitAll(m_d3dDevice);
	TerInputLayouts::InitAll(m_d3dDevice);
	RenderStates::InitAll(m_d3dDevice);

	mSky = new Sky(m_d3dDevice, L"../Textures/grasscube1024.dds", 5000.0f);

	Terrain::InitInfo tii;

	tii.HeightMapFilename = L"../Textures/terrain.raw";
	
	tii.LayerMapFilename0 = L"../Textures/terrainLayers.dds";

	tii.BlendMapFilename = L"../Textures/blend.dds";

	tii.HeightScale = 50.0f;
	tii.HeightmapHeight = 2049;
	tii.HeightmapWidth = 2049;
	tii.CellSpacing = 0.5f;

	mTerrain.Init(m_d3dDevice, m_d3dImmediateContext, tii);
	return true;

}

void TerrainApp::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 3000.0f);
}

void TerrainApp::UpdateScene(float dt)
{
	CommonApp::UpdateScene(dt);

	//
	// Walk/fly mode
	//
	if (GetAsyncKeyState('2') & 0x8000)
		mWalkCamMode = true;
	if (GetAsyncKeyState('3') & 0x8000)
		mWalkCamMode = false;

	// Clamp camera to terrain surface in walk mode
	if (mWalkCamMode)
	{
		XMFLOAT3 camPos = mCam.GetPosition();
		float y = mTerrain.GetHeight(camPos.x, camPos.z);
		mCam.SetPosition(camPos.x, y + 2.0f, camPos.z);
	}

	mCam.UpdateViewMatrix();
}

bool TerrainApp::DrawScene()
{
	PreProcessing();

	m_d3dImmediateContext->IASetInputLayout(TerInputLayouts::Basic32);
	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (GetAsyncKeyState('1') & 0x8000)
		m_d3dImmediateContext->RSSetState(RenderStates::WireframeRS);

	//
	mTerrain.Draw(m_d3dImmediateContext, mCam, mDirLights, TerInputLayouts::Terrain, TerEffects::TerrainFX);

	m_d3dImmediateContext->RSSetState(0);

	//
	mSky->Draw(m_d3dImmediateContext, mCam, TerEffects::SkyFX, TerInputLayouts::Pos);

	// restore default states, as SkyFX changes them in the effect file
	m_d3dImmediateContext->RSSetState(0);
	m_d3dImmediateContext->OMSetDepthStencilState(0, 0);

	HR(m_swapChain->Present(0, 0));

	return true;
}

