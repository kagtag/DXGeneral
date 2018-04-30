#pragma once
//Base Direct3D class
//provides functions for creating the main application window, running the application message loop, handling window meesages, and initiating Direct3D

#ifndef _D3DAPP_H_
#define _D3DAPP_H_

#include "d3dUtil.h"
#include "GameTimer.h"
#include <string>

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE AppInst() const;
	HWND MainWnd() const;
	float AspectRatio() const;

	int Run();

	//Framework methods, Derived client classes overrides these methods to
	//implement specific application requirements

	virtual bool Init();
	virtual void OnResize();
	virtual void UpdateScene(float dt) = 0;
	virtual bool DrawScene() = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);

protected:
	bool InitMainWindow();
	bool InitDirect3D();

	void CalculateFrameStats();

protected:

	HINSTANCE m_hAppInst;
	HWND m_hMainWnd;
	bool m_appPaused;
	bool m_minimized;
	bool m_maximized;
	bool m_resizing;
	UINT m_4xMsaaQuality;

	GameTimer m_timer;

	ID3D11Device* m_d3dDevice;
	ID3D11DeviceContext* m_d3dImmediateContext;
	IDXGISwapChain* m_swapChain;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;
	D3D11_VIEWPORT m_screenViewport;

	//Derived class should set these in derived constructor to customize starting values
	std::wstring m_mainWndCaption;
	D3D_DRIVER_TYPE m_d3dDriverType;
	int m_clientWidth;
	int m_clientHeight;
	bool m_enable4xMsaa;
	
	//
	const bool FULL_SCREEN = false;
	const bool VSYNC_ENABLED = true;
};

#endif
