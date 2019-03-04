
#include "AppHelper.h"

#include "SkyEffects.h"
#include "SkyVertex.h"

#include "Sky.h"

#include "Vertex.h"

class CubeMapApp : public ShapesBaseApp
{
public:
	CubeMapApp(HINSTANCE hInstance);
	~CubeMapApp();

	bool Init();
	//void OnResize();
	void UpdateScene(float dt);
	bool DrawScene();

private:

	Sky* mSky;

	
};

DEFAULT_WINMAIN(CubeMapApp)

CubeMapApp::CubeMapApp(HINSTANCE hInstance)
	: ShapesBaseApp(hInstance),
	mSky(0)
{
	m_mainWndCaption = L"CubeMap Demo";

	// Position the skull

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

}

CubeMapApp::~CubeMapApp()
{
	SafeDelete(mSky);


	SkyEffects::DestroyAll();
	SkyInputLayouts::DestroyAll();
}

bool CubeMapApp::Init()
{
	if (!ShapesBaseApp::Init())
		return false;

	SkyEffects::InitAll(m_d3dDevice);
	SkyInputLayouts::InitAll(m_d3dDevice);

	mSky = new Sky(m_d3dDevice, L"../Textures/grasscube1024.dds", 5000.0f);

	return true;
}



void CubeMapApp::UpdateScene(float dt)
{
	// Control camera
	ShapesBaseApp::UpdateScene(dt);

	//
	// Switch the number of lights based on key presses.
	//
	if (GetAsyncKeyState('0') & 0x8000)
		mLightCount = 0;

	if (GetAsyncKeyState('1') & 0x8000)
		mLightCount = 1;

	if (GetAsyncKeyState('2') & 0x8000)
		mLightCount = 2;

	if (GetAsyncKeyState('3') & 0x8000)
		mLightCount = 3;
}

bool CubeMapApp::DrawScene()
{
	m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_d3dImmediateContext->IASetInputLayout(SkyInputLayouts::Basic32);
	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	mCam.UpdateViewMatrix();

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	float blendFactor[] = { 0.0f,0.0f, 0.0f, 0.0f };
	
	// Set per frame constants
	SkyEffects::BasicFX->SetDirLights(mDirLights);
	SkyEffects::BasicFX->SetEyePosW(mCam.GetPosition());
	SkyEffects::BasicFX->SetCubeMap(mSky->CubeMapSRV());

	// Figure out which technique to use. Skull does not have texture coordinates.
	// so we need a seperate technique for it, and not every surface is reflective,
	// so don't pay for cubemap lookup

	ID3DX11EffectTechnique* activeTexTech = SkyEffects::BasicFX->Light1TexTech;
	ID3DX11EffectTechnique* activeReflectTech = SkyEffects::BasicFX->Light1TexReflectTech;
	ID3DX11EffectTechnique* activeSkullTech = SkyEffects::BasicFX->Light1ReflectTech;

	switch (mLightCount)
	{
	case 1:
		activeTexTech = SkyEffects::BasicFX->Light1TexTech;
		activeReflectTech = SkyEffects::BasicFX->Light1TexReflectTech;
		activeSkullTech = SkyEffects::BasicFX->Light1ReflectTech;
		break;
	case 2:
		activeTexTech = SkyEffects::BasicFX->Light2TexTech;
		activeReflectTech = SkyEffects::BasicFX->Light2TexReflectTech;
		activeSkullTech = SkyEffects::BasicFX->Light2ReflectTech;
		break;
	case 3:
	default:
		activeTexTech = SkyEffects::BasicFX->Light3TexTech;
		activeReflectTech = SkyEffects::BasicFX->Light3TexReflectTech;
		activeSkullTech = SkyEffects::BasicFX->Light3ReflectTech;
		break;
	}

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	//
	// Draw the grid, cylinders and box without any cubemap reflection.
	//
	D3DX11_TECHNIQUE_DESC techDesc;
	activeTexTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
		m_d3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

		// Draw the grid.
		world = XMLoadFloat4x4(&mGridWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		SkyEffects::BasicFX->SetWorld(world);
		SkyEffects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		SkyEffects::BasicFX->SetWorldViewProj(worldViewProj);
		SkyEffects::BasicFX->SetTexTransform(XMMatrixScaling(6.0f, 8.0f, 1.0f));
		SkyEffects::BasicFX->SetMaterial(mGridMat);
		SkyEffects::BasicFX->SetDiffuseMap(mFloorTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
		m_d3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// Draw the box.
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		SkyEffects::BasicFX->SetWorld(world);
		SkyEffects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		SkyEffects::BasicFX->SetWorldViewProj(worldViewProj);
		SkyEffects::BasicFX->SetTexTransform(XMMatrixIdentity());
		SkyEffects::BasicFX->SetMaterial(mBoxMat);
		SkyEffects::BasicFX->SetDiffuseMap(mStoneTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
		m_d3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// Draw the cylinders.
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			SkyEffects::BasicFX->SetWorld(world);
			SkyEffects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			SkyEffects::BasicFX->SetWorldViewProj(worldViewProj);
			SkyEffects::BasicFX->SetTexTransform(XMMatrixIdentity());
			SkyEffects::BasicFX->SetMaterial(mCylinderMat);
			SkyEffects::BasicFX->SetDiffuseMap(mBrickTexSRV);

			activeTexTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
			m_d3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}
	}

	//
	// Draw the sphere with cubemap reflection
	//

	activeReflectTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the spheres
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			SkyEffects::BasicFX->SetWorld(world);
			SkyEffects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			SkyEffects::BasicFX->SetWorldViewProj(worldViewProj);
			SkyEffects::BasicFX->SetTexTransform(XMMatrixIdentity());
			SkyEffects::BasicFX->SetMaterial(mSphereMat);
			SkyEffects::BasicFX->SetDiffuseMap(mStoneTexSRV);

			activeReflectTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
			m_d3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the skull.

		m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
		m_d3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&mSkullWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		SkyEffects::BasicFX->SetWorld(world);
		SkyEffects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		SkyEffects::BasicFX->SetWorldViewProj(worldViewProj);
		SkyEffects::BasicFX->SetMaterial(mSkullMat);

		activeSkullTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
		m_d3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	mSky->Draw(m_d3dImmediateContext, mCam, SkyInputLayouts::Pos, SkyEffects::SkyFX);

	// restore default states
	m_d3dImmediateContext->RSSetState(0);
	m_d3dImmediateContext->OMSetDepthStencilState(0, 0);

	HR(m_swapChain->Present(0, 0));

	return true;
}