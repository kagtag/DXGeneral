#include "WavesDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	//Enable run-time memory check for debug builds
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	WavesApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

WavesApp::WavesApp(HINSTANCE hInstance)
	:D3DApp(hInstance),
	m_landVB(0), m_landIB(0),
	m_wavesVB(0), m_wavesIB(0),
	m_vertexShader(0), m_pixelShader(0),
	m_inputLayout(0), m_wireframeRS(0),
	m_theta(1.5f*MathHelper::Pi), m_phi(0.1f*MathHelper::Pi), m_radius(200.0f),
	m_matrixBuffer(0)
{
	m_mainWndCaption = L"Waves Demo";

	m_lastMousePos.x = 0;
	m_lastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_gridWorld, I);
	XMStoreFloat4x4(&m_wavesWorld, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

}

WavesApp::~WavesApp()
{
	ReleaseCOM(m_landVB);
	ReleaseCOM(m_landIB);

	ReleaseCOM(m_wavesVB);
	ReleaseCOM(m_wavesIB);

	ReleaseCOM(m_inputLayout);
	ReleaseCOM(m_wireframeRS);

	ReleaseCOM(m_vertexShader);
	ReleaseCOM(m_pixelShader);

	ReleaseCOM(m_matrixBuffer);

}

bool WavesApp::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	m_waves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

	BuildLandGeometryBuffers();
	BuildWavesGeometryBuffers();

	bool result = BuildShader(L"wavesVS.hlsl", L"wavesPS.hlsl");
	if (!result)
	{
		return false;
	}

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(m_d3dDevice->CreateRasterizerState(&wireframeDesc, &m_wireframeRS));

	return true;
}

bool WavesApp::BuildShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2]; //input layout structure
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;

	//Initialize the pointers to null
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	//compile the vertex shader code
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "WavesVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		//if the shader failed to compile, it should have written something to the errorMessage
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, m_hMainWnd, vsFilename);
		}
		else
		{
			//if there was noting in the error message then it simply could not find the shader file itself
			MessageBox(m_hMainWnd, vsFilename, L"missing Shader File", MB_OK);
		}
		return false;
	}

	//compile the pixel shader code
	result = D3DCompileFromFile(psFilename, NULL, NULL, "WavesPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		//if the shader failed to compile, it should have written something to the errorMessage
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, m_hMainWnd, psFilename);
		}
		else
		{
			//if there was noting in the error message then it simply could not find the shader file itself
			MessageBox(m_hMainWnd, psFilename, L"missing Shader File", MB_OK);
		}
		return false;
	}

	//Create the vertex shader from the buffer
	result = m_d3dDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	//Create the pixel shader from the buffer
	result = m_d3dDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(),
		NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	//create the vertex input layout description
	//This setup needs to match the Vertex structure in this class and in the shader
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	//Get a count of the elements in the layout
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	//Create the vertex input layout
	result = m_d3dDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_inputLayout);
	if (FAILED(result))
	{
		return false;
	}

	//Release the vertex shader buffer and pixel shader buffer since they are no longer needed
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	//Setup the description of the dynamic matrix constant buffer that is in the vertex shader
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	//Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class
	result = m_d3dDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void WavesApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_proj, P);
}

void WavesApp::UpdateScene(float dt)
{
	//Convert Shperical to Cartesian coordinates
	float x = m_radius*sinf(m_phi)*cosf(m_theta);
	float z = m_radius*sinf(m_phi)*sinf(m_theta);
	float y = m_radius*cosf(m_phi);

	//Build the view matrix
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, V);

	//
	//Every quarter second, generate a random wave
	//

	static float t_base = 0.0f;
	if (m_timer.TotalTime() - t_base >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;

		float r = MathHelper::RandF(0.5f, 1.0f);

		m_waves.Disturb(i, j, r);
	}

	m_waves.Update(dt);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_d3dImmediateContext->Map(m_wavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	VertexType* v = reinterpret_cast<VertexType*>(mappedData.pData);
	for (UINT i = 0; i < m_waves.VertexCount(); ++i)
	{
		v[i].Pos = m_waves[i];
		v[i].Color = XMFLOAT4(reinterpret_cast<const float*>(&Colors::Blue));
	}

	m_d3dImmediateContext->Unmap(m_wavesVB, 0);
}

bool WavesApp::DrawScene()
{
	//Begin Scene
	//Clean up the back buffer and depth/stencil buffer to get ready for the new frame
	m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	bool result;

	//Can't use RenderBuffer() here, since the Vertex and Index buffers are different
	//RenderBuffers();

	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);

	//
	XMMATRIX gridWorld = XMLoadFloat4x4(&m_gridWorld);
	XMMATRIX wavesWorld = XMLoadFloat4x4(&m_wavesWorld);


	UINT stride = sizeof(VertexType);
	UINT offset = 0;
	
	///////////Land
	//Do work like RenderBuffer();
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_landVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(m_landIB, DXGI_FORMAT_R32_UINT, 0);

	SetShaderParameters(gridWorld, view, proj, m_gridIndexCount, 0, 0);

	//////////Waves
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_wavesVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(m_wavesIB, DXGI_FORMAT_R32_UINT, 0);

	SetShaderParameters(wavesWorld, view, proj, 3 * m_waves.TriangleCount(), 0, 0);


	//End Scene
	//Present the back buffer to the screen
	if (VSYNC_ENABLED)
	{
		HR(m_swapChain->Present(1, 0));
	}
	else
	{
		//Present as fast as possible
		HR(m_swapChain->Present(0, 0));
	}

	return true;
}

static bool solidFlag = true;

void WavesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos.x = x;
	m_lastMousePos.y = y;

	if ((btnState & MK_MBUTTON) != 0)
	{
		if (solidFlag)
		{
			m_d3dImmediateContext->RSSetState(m_wireframeRS);
			solidFlag = false;
		}
		else
		{
			m_d3dImmediateContext->RSSetState(0);
			solidFlag = true;
		}
	}

	SetCapture(m_hMainWnd);
}

void WavesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void WavesApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		//Make each pixel correspond to a quarter of a degree
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - m_lastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - m_lastMousePos.y));

		//Update angles based on input to orbit camera about box
		m_theta += dx;
		m_phi += dy;

		//Restrict the angle m_phi
		m_phi = MathHelper::Clamp(m_phi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		//Make each pixel correspond to 0.2 unit in the scene
		float dx = 0.2f*static_cast<float>(x - m_lastMousePos.x);
		float dy = 0.2f*static_cast<float>(y - m_lastMousePos.y);

		//Update the camera radius based on input
		m_radius += (dx - dy);

		//Restrict the radius
		m_radius = MathHelper::Clamp(m_radius, 50.0f, 500.0f);
	}

	m_lastMousePos.x = x;
	m_lastMousePos.y = y;
}

float WavesApp::GetHeight(float x, float z)const
{
	return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}

void WavesApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
	
	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	m_gridIndexCount = grid.Indices.size();

	//Simiar to Hills demo
	std::vector<VertexType> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHeight(p.x, p.z);

		vertices[i].Pos = p;

		// Color the vertex based on its height.
		if (p.y < -10.0f)
		{
			// Sandy beach color.
			vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (p.y < 5.0f)
		{
			// Light yellow-green.
			vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (p.y < 12.0f)
		{
			// Dark yellow-green.
			vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (p.y < 20.0f)
		{
			// Dark brown.
			vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// White snow.
			vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexType)*grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_d3dDevice->CreateBuffer(&vbd, &vinitData, &m_landVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)*m_gridIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(m_d3dDevice->CreateBuffer(&ibd, &iinitData, &m_landIB));
}

void WavesApp::BuildWavesGeometryBuffers()
{
	//Create the vertex buffer. Note that we allocate space only,
	//as we will be updating the data every time step of the simulation

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(VertexType)*m_waves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(m_d3dDevice->CreateBuffer(&vbd, 0, &m_wavesVB));

	//index buffer is static, only vertex buffer need to be dynamic

	std::vector<UINT> indices(3 * m_waves.TriangleCount());

	//Ierate over each quad
	UINT m = m_waves.RowCount();
	UINT n = m_waves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i*n + j;
			indices[k + 1] = i*n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i*n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; //next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)*indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_d3dDevice->CreateBuffer(&ibd, &iinitData, &m_wavesIB));

}

bool WavesApp::SetShaderParameters(XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, int indexCount, int indexOffset, int vertexOffset)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	//Transpose the matrices to prepare them for the shader
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	//Lock the constant buffer so it can be written to
	result = m_d3dImmediateContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	//Get a pointer to the data in the constant buffer
	dataPtr = (MatrixBufferType*)mappedResource.pData;


	//Copy the matrices into constant buffer
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	//Unlock the constant buffer
	m_d3dImmediateContext->Unmap(m_matrixBuffer, 0);

	//Set the position of the constant buffer in the vertex shader
	bufferNumber = 0;

	//Finally set the constant buffer in the vertex shader with the updated values
	m_d3dImmediateContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	//
	RenderShader(indexCount, indexOffset, vertexOffset);
	//

	return true;
}

void WavesApp::RenderShader(int indexCount, int indexOffset, int vertexOffset)
{
	m_d3dImmediateContext->IASetInputLayout(m_inputLayout);

	//Set the vertex and pixel shaders that will be used to render this model
	m_d3dImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_d3dImmediateContext->PSSetShader(m_pixelShader, NULL, 0);

	m_d3dImmediateContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}