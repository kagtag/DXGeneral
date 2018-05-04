#include"SkullDemo.h"
#include<fstream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	SkullApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

SkullApp::SkullApp(HINSTANCE hInstance)
	:D3DApp(hInstance),
	m_skullVB(0), m_skullIB(0),
	m_vertexShader(0), m_pixelShader(0),
	m_inputLayout(0), m_wireframeRS(0),
	m_theta(1.5f*MathHelper::Pi), m_phi(0.1f*MathHelper::Pi), m_radius(20.0f),
	m_matrixBuffer(0)
{
	m_mainWndCaption = L"Skull Demo";

	m_lastMousePos.x = 0;
	m_lastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

	XMMATRIX T = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
	XMStoreFloat4x4(&m_world, T);
}

SkullApp::~SkullApp()
{
	ReleaseCOM(m_skullVB);
	ReleaseCOM(m_skullIB);

	ReleaseCOM(m_inputLayout);
	ReleaseCOM(m_wireframeRS);

	ReleaseCOM(m_vertexShader);
	ReleaseCOM(m_pixelShader);

	ReleaseCOM(m_matrixBuffer);
}

bool SkullApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();

	bool result = BuildShader(L"skull.vs", L"skull.ps");
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

void SkullApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_proj, P);
}

void SkullApp::UpdateScene(float dt)
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
}

bool SkullApp::BuildShader(WCHAR* vsFilename, WCHAR* psFilename)
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
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "SkullVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
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
	result = D3DCompileFromFile(psFilename, NULL, NULL, "SkullPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
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

bool SkullApp::DrawScene()
{
	//Begin Scene
	//Clean up the back buffer and depth/stencil buffer to get ready for the new frame
	m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	bool result;

	RenderBuffers();


	XMMATRIX world = XMLoadFloat4x4(&m_world);
	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);

	SetShaderParameters(world, view, proj, m_indexCount, 0, 0);


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

void SkullApp::OnMouseDown(WPARAM btnState, int x, int y)
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

void SkullApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void SkullApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.05f*static_cast<float>(x - m_lastMousePos.x);
		float dy = 0.05f*static_cast<float>(y - m_lastMousePos.y);

		//Update the camera radius based on input
		m_radius += (dx - dy);

		//Restrict the radius
		m_radius = MathHelper::Clamp(m_radius, 5.0f, 50.0f);
	}

	m_lastMousePos.x = x;
	m_lastMousePos.y = y;
}

bool SkullApp::BuildGeometryBuffers()
{
	std::ifstream fin("Models/car.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found", 0, 0);
		return false;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	float nx, ny, nz;
	XMFLOAT4 black = XMFLOAT4(reinterpret_cast<const float*>(&Colors::Black));

	std::vector<VertexType> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;

		vertices[i].Color = black;

		//Normal not used in this demo
		fin >> nx >> ny >> nz;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	m_indexCount = 3 * tcount;
	std::vector<UINT> indices(m_indexCount);

	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();


	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexType)*vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_d3dDevice->CreateBuffer(&vbd, &vinitData, &m_skullVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)*m_indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_d3dDevice->CreateBuffer(&ibd, &iinitData, &m_skullIB));

	return true;
}

bool SkullApp::SetShaderParameters(XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
	int indexCount, int indexOffset, int vertexOffset)
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

void SkullApp::RenderShader(int indexCount, int indexOffset, int vertexOffset)
{
	m_d3dImmediateContext->IASetInputLayout(m_inputLayout);

	//Set the vertex and pixel shaders that will be used to render this model
	m_d3dImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_d3dImmediateContext->PSSetShader(m_pixelShader, NULL, 0);

	m_d3dImmediateContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void SkullApp::RenderBuffers()
{
	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_skullVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(m_skullIB, DXGI_FORMAT_R32_UINT, 0);

	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}