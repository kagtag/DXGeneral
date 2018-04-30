//***************************************************************************************
// BoxDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates rendering a colored box.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//***************************************************************************************

#include"d3dApp.h"
#include"MathHelper.h"


struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class BoxApp :public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	~BoxApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* m_boxVB;
	ID3D11Buffer* m_boxIB;

	//No effect, technique here

	ID3D11InputLayout* m_inputLayout;
	
	XMFLOAT4X4 m_world;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;

};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	//Enable run-time memory check for debug builds
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
	BoxApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

BoxApp::BoxApp(HINSTANCE hInstance)
	:D3DApp(hInstance), m_boxVB(0), m_boxIB(0),
	m_inputLayout(0),
	m_theta(1.5f*MathHelper::Pi), m_phi(0.25*MathHelper::Pi), m_radius(5.0f)
{
	m_mainWndCaption = L"Box Demo";

	m_lastMousePos.x = 0;
	m_lastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_world, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);
}

bool BoxApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	return true;
}
