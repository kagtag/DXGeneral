
#include"d3dApp.h"
#include<Windows.h> //include a set of headers
#include<sstream>

#include<windowsx.h> //useful macros for WIN32 programming

namespace
{
	//This is just used to forward Windows messages from a gloal window
	//procedure to our member function window procedure, because we cannot
	//assign a member function to WNDCLASS::lpfnwndProc
	D3DApp* gd3dApp = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//forward hwnd on because we can get messages
	//before CreateWindow returns, and thus before mhMainWnd is valid
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance)
	:m_hAppInst(hInstance),
	m_mainWndCaption(L"D3D11 Application"),
	m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	m_clientWidth(800),
	m_clientHeight(600),
	m_enable4xMsaa(false),
	m_hMainWnd(0),
	m_appPaused(false),
	m_minimized(false),
	m_maximized(false),
	m_resizing(false),
	m_4xMsaaQuality(0),

	m_d3dDevice(0),
	m_d3dImmediateContext(0),
	m_swapChain(0),
	m_depthStencilBuffer(0),
	m_renderTargetView(0),
	m_depthStencilView(0),
	m_rasterState(0)
{
	ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT)); //specifies the view port used to display the final frame
	//relative the the window.
	//also specifies max and min depth value for depth buffer.(normally 0 and 1)


	//Get a pointer to the application object so we can forward 
	//Windows messages to the object's window procedure through the 
	//glocal window procedure
	gd3dApp = this;
}

D3DApp::~D3DApp()
{
	ReleaseCOM(m_renderTargetView);
	ReleaseCOM(m_depthStencilView);
	ReleaseCOM(m_swapChain);
	ReleaseCOM(m_depthStencilBuffer);
	ReleaseCOM(m_rasterState);

	//Restore all default settings
	if (m_d3dImmediateContext)
	{
		m_d3dImmediateContext->ClearState();
	}

	ReleaseCOM(m_d3dImmediateContext);
	ReleaseCOM(m_d3dDevice);

	//Fix the display settings if leaving full screen mode
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}
}

HINSTANCE D3DApp::AppInst()const
{
	return m_hAppInst;
}

HWND D3DApp::MainWnd()const
{
	return m_hMainWnd;
}

float D3DApp::AspectRatio()const
{
	return static_cast<float>(m_clientWidth) / m_clientHeight;
}

int D3DApp::Run()
{
	MSG msg = { 0 };
	bool result;

	m_timer.Reset();
	

	while (msg.message != WM_QUIT)
	{

		//if there are window messages then process them
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg); //keyboard info translation
			DispatchMessage(&msg); //dispatch to the appropriate window procedure
		}
		else
		{
			//Otherwise, do animation/game stuff
			m_timer.Tick();

			if (!m_appPaused)
			{
				CalculateFrameStats();
				UpdateScene(m_timer.DeltaTime());
				result = DrawScene();
				if (!result)
				{
					return -1;
				}
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

bool D3DApp::Init()
{
	if (!InitMainWindow())
	{
		return false;
	}

	if (!InitDirect3D())
	{
		return false;
	}

	return true;
}

void D3DApp::OnResize()
{
	assert(m_d3dImmediateContext);
	assert(m_d3dDevice);
	assert(m_swapChain);

	//Release the old views, as they hold reference to the buffers we will be destroying.
	//Also release the old depth/stencil buffer

	ReleaseCOM(m_renderTargetView);
	ReleaseCOM(m_depthStencilView);
	ReleaseCOM(m_depthStencilBuffer);

	//Resize the swap chain and recreate the render target view
	HR(m_swapChain->ResizeBuffers(1, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	
	//Get back buffer, and create render target view and bind back buffer to it
	ID3D11Texture2D* backBuffer;
	HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_d3dDevice->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView));
	ReleaseCOM(backBuffer);

	//Create the depth/stencil buffer and view

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = m_clientWidth;
	depthStencilDesc.Height = m_clientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//Use 4x MSAA must match swap chain MSAA values
	if (m_enable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_d3dDevice->CreateTexture2D(&depthStencilDesc, 0, &m_depthStencilBuffer));
	HR(m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer, 0, &m_depthStencilView));

	//Bind the render target view and depth/stencil view to the pipeline
	m_d3dImmediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);


	//Setup Rasterizer State
	D3D11_RASTERIZER_DESC rasterDesc;

	//Setup the raster description which will determine how and what polygons will be drawn
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	//Create the rasterizer state from the description we just filled out
	HR(m_d3dDevice->CreateRasterizerState(&rasterDesc, &m_rasterState));
	m_d3dImmediateContext->RSSetState(m_rasterState);

	//Set the viewport transform
	//Occupy the entire client window
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(m_clientWidth);
	m_screenViewport.Height = static_cast<float>(m_clientHeight);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_d3dImmediateContext->RSSetViewports(1, &m_screenViewport);

}


LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		//WM_ACTIVATE is sent when the window is activated or deactivated,
		//We pause the game when the window is deactivated and unpause it when 
		//it becomes active.
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_appPaused = true;
			m_timer.Stop();
		}
		else
		{
			m_appPaused = false;
			m_timer.Start();
		}
		return 0;

		//WM_SIZE is sent when the user resizes the window
	case WM_SIZE:
		//Save the new client area dimensions
		m_clientWidth = LOWORD(lParam);
		m_clientHeight = HIWORD(lParam);
		if (m_d3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_appPaused = true;
				m_minimized = true;
				m_maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_appPaused = false;
				m_minimized = false;
				m_maximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				//Restoring from minimized state?
				if (m_minimized)
				{
					m_appPaused = false;
					m_minimized = false;
					OnResize();
				}
				//Restoring from maximized state?
				else if (m_maximized)
				{
					m_appPaused = false;
					m_maximized = false;
					OnResize();
				}
				else if (m_resizing)
				{
					//if user is dragging the resize bars, we do not resize the buffers here
					//because as the usercontinuously drags the resize bars, 
					//a stream of WM_SIZE messages are sent to the window, and it would be 
					//POINTLESS to resize for each WM_SIZE message received from dragging the 
					//resize bars. So instead, we reset after the user is done resizing the window and
					//releases the resize bars, which sends a WM_EXITSIZEMOVE message
				}
				else
				{
					//API call such as SetWindowPos or mSwapChain->SetFullscreenState.
					OnResize();
				}
			}
		}
		return 0;

		//WM_ENTERSIZEMOVE is sent when the user grabs the resize bars
	case WM_ENTERSIZEMOVE:
		m_appPaused = true;
		m_resizing = true;
		m_timer.Stop();
		return 0;

		//WM_EXITSIZEMOVE is sent when the user releases the resize bars
		//Here we reset everything based on the new window dimensions
	case WM_EXITSIZEMOVE:
		m_appPaused = false;
		m_resizing = false;
		m_timer.Start();
		OnResize();
		return 0;

		//WM_DESTROY is sent when the window is being destroyed
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		//The WM_MENUCHAR message is sent when a menu is active and the user presses
		//a key that does not correspond to any  mnemonic or accelerator key.
	case WM_MENUCHAR:
		//Don't beep when we alt-enter
		return MAKELRESULT(0, MNC_CLOSE);

		//Catch this message so to prevent the window from becoming too small
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool D3DApp::InitMainWindow()
{

	DEVMODE dmScreenSettings;
	int posX, posY;
	int width, height;

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW; //to be repainted when either the hori or the vert window size is changed
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"BasicWndClass";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	

	//Compute Window rectangle dimensions based on requested client area dimensions

	if (FULL_SCREEN)
	{
		//Fullscreen mode

		//determine the resolution of the clients desktop screen
		m_clientWidth = GetSystemMetrics(SM_CXSCREEN);
		m_clientHeight = GetSystemMetrics(SM_CYSCREEN);
		

		ZeroMemory(&dmScreenSettings, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)m_clientWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)m_clientHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		//change the display settings to full screen
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		//Set the position of the window to the top left corner
		posX = posY = 0;
		width = m_clientWidth;
		height = m_clientHeight;
	}
	else
	{
		//Windowed mode
		RECT R = { 0,0,m_clientWidth, m_clientHeight };
		AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
		width = R.right - R.left;
		height = R.bottom - R.top;

		posX = CW_USEDEFAULT;
		posY = CW_USEDEFAULT;
	}


	m_hMainWnd = CreateWindow(L"BasicWndClass", m_mainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, posX, posY, width, height, 0, 0, m_hAppInst, 0);
	if (!m_hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	//set it as main focus
	SetForegroundWindow(m_hMainWnd);
	SetFocus(m_hMainWnd);

	return true;

}

bool D3DApp::InitDirect3D()
{
	//Create the device and device context

	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice
	(	0, //default adapter
		m_d3dDriverType,
		0, //no software device
		createDeviceFlags,
		0, 0,//Default feature level array
		D3D11_SDK_VERSION,
		&m_d3dDevice,
		&featureLevel,
		&m_d3dImmediateContext
	);

	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	//Check 4x MSAA quality supprot for our back buffer format
	//All Direct3D 11 capable devices support 4x MSAA for all render
	//target formats, so we only need to check quality support
	HR(m_d3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality
	));
	assert(m_4xMsaaQuality > 0);

	//Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_clientWidth;
	sd.BufferDesc.Height = m_clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;


	//Use 4x MSAA?
	if (m_enable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = m_hMainWnd;

	if (FULL_SCREEN)
	{
		sd.Windowed = false;
	}
	else
	{
		sd.Windowed = true;
	}
	
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	//To correctly create the swap chain, we must use the IDXGIFactory that was
	//used to create the device. if we tried to use a different IDXGIFactory instance
	//(by calling CreateDXGIFactory), we get an error.
	
	IDXGIDevice* dxgiDevice = 0;
	HR(m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	HR(dxgiFactory->CreateSwapChain(m_d3dDevice, &sd, &m_swapChain));

	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	//The remaining steps that need to be carried out for d3d creation
	//also need to be executed everytime the window is resized, so
	//just call the OnResize method here to avoid code duplication

	OnResize();

	return true;
}

void D3DApp::CalculateFrameStats()
{
	//Code computes the average frames per second, and also
	//the average time it takes to render one frame, These stats
	//are append to the window caption bar

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	//Compute averages over one second period
	if ((m_timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt;  //fps = frameCnt / 1
		float mspf = 1000.0f / fps; //milli sec per frame

		std::wostringstream outs;
		outs.precision(6);
		outs << m_mainWndCaption << L"  "
			<< L"FPS: " << fps << L"  "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(m_hMainWnd, outs.str().c_str());
		
		//Reset for next average
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}
//