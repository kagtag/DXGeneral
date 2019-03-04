
#include "AppHelper.h"

#include "ParEffects.h"
#include "ParVertex.h"

#include "Sky.h"
#include "Terrain.h"
#include "ParticleSystem.h"

class ParticlesApp : public CommonApp
{
public:
	ParticlesApp(HINSTANCE hInstance);
	~ParticlesApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	bool DrawScene();

private:

	Sky* mSky;
	Terrain mTerrain;

	ID3D11ShaderResourceView* mFlareTexSRV;
	ID3D11ShaderResourceView* mRainTexSRV;
	ID3D11ShaderResourceView* mRandomTexSRV;

	ParticleSystem mFire;
	ParticleSystem mRain;

	DirectionalLight mDirLights[3];

	bool mWalkCamMode;
};

DEFAULT_WINMAIN(ParticlesApp)

ParticlesApp::ParticlesApp(HINSTANCE hInstance)
	:CommonApp(hInstance),
	mSky(0),
	mRandomTexSRV(0),
	mFlareTexSRV(0), mRainTexSRV(0),
	mWalkCamMode(false)
{
	m_mainWndCaption = L"Particles Demo";
	m_enable4xMsaa = false;

	mCam.SetPosition(0.0f, 2.0f, 100.0f);

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

ParticlesApp::~ParticlesApp()
{
	m_d3dImmediateContext->ClearState();

	ReleaseCOM(mRandomTexSRV);
	ReleaseCOM(mFlareTexSRV);
	ReleaseCOM(mRainTexSRV);

	SafeDelete(mSky);

	ParEffects::DestroyAll();
	ParInputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool ParticlesApp::Init()
{
	if (!CommonApp::Init())
		return false;

	ParEffects::InitAll(m_d3dDevice);
	ParInputLayouts::InitAll(m_d3dDevice);
	RenderStates::InitAll(m_d3dDevice);

	mSky = new Sky(m_d3dDevice, L"../Textures/grasscube1024.dds", 5000.0f);

	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"../Textures/terrain.raw";

	tii.LayerMapFilename0 = L"../Textures/terrainLayers.dds";

	tii.BlendMapFilename = L"../Textures/blend.dds";

	tii.HeightScale = 50.0f;
	tii.HeightmapWidth = 2049;
	tii.HeightmapHeight = 2049;

	tii.CellSpacing = 0.5f;

	mTerrain.Init(m_d3dDevice, m_d3dImmediateContext, tii);

	mRandomTexSRV = D3DHelper::CreateRandomTexture1DSRV(m_d3dDevice); // Generate random arrays

	//
	// Fire
	//

	std::vector<std::wstring> flares;
	flares.push_back(L"../Textures/flare0.dds");
	// Create texture2d array offline
	CreateDDSTextureFromFile(m_d3dDevice, flares[0].c_str(), 0, &mFlareTexSRV);

	mFire.Init(m_d3dDevice, ParEffects::FireFX, mFlareTexSRV, mRandomTexSRV, 500);
	mFire.SetEmitPos(XMFLOAT3(0.0f, 1.0f, 120.0f));

	//
	// Water
	//
	std::vector<std::wstring> raindrops;
	raindrops.push_back(L"../Textures/raindrop.dds");
	CreateDDSTextureFromFile(m_d3dDevice, raindrops[0].c_str(), 0, &mRainTexSRV);

	mRain.Init(m_d3dDevice, ParEffects::RainFX, mRainTexSRV, mRandomTexSRV, 10000);

	return true;
}

void ParticlesApp::OnResize()
{
	CommonApp::OnResize();

	mCam.SetLens(0.25f* MathHelper::Pi, AspectRatio(), 1.0f, 3000.0f);
}

void ParticlesApp::UpdateScene(float dt)
{
	CommonApp::UpdateScene(dt);

	// Walk/fly mode
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

	// Reset particle systems
	if (GetAsyncKeyState('R') & 0x8000)
	{
		mFire.Reset();
		mRain.Reset();
	}

	mFire.Update(dt, m_timer.TotalTime());
	mRain.Update(dt, m_timer.TotalTime());

	mCam.UpdateViewMatrix();
}

bool ParticlesApp::DrawScene()
{
	PreProcessing();

	m_d3dImmediateContext->IASetInputLayout(ParInputLayouts::Basic32);
	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (GetAsyncKeyState('1') & 0x8000)
		m_d3dImmediateContext->RSSetState(RenderStates::WireframeRS);

	mTerrain.Draw(m_d3dImmediateContext, mCam, mDirLights,
		ParInputLayouts::Terrain, ParEffects::TerrainFX);

	m_d3dImmediateContext->RSSetState(0);

	mSky->Draw(m_d3dImmediateContext, mCam,
		ParInputLayouts::Pos, ParEffects::SkyFX);

	// Draw particle systems last so it is blended with the scene
	mFire.SetEyePos(mCam.GetPosition());
	mFire.Draw(m_d3dImmediateContext, mCam);
	m_d3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff); // restore default

	mRain.SetEyePos(mCam.GetPosition());
	mRain.SetEmitPos(mCam.GetPosition());
	mRain.Draw(m_d3dImmediateContext, mCam);

	// restore default states
	m_d3dImmediateContext->RSSetState(0);
	m_d3dImmediateContext->OMSetDepthStencilState(0, 0);
	m_d3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);

	HR(m_swapChain->Present(0, 0));

	return true;
}

