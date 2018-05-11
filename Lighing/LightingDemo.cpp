#include"LightingDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	//Enable run-time memory check for debug builds
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	LightingApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

LightingApp::LightingApp(HINSTANCE hInstance)
	:D3DApp(hInstance),
	m_landVB(0), m_landIB(0),
	m_wavesVB(0), m_wavesIB(0),
	m_vertexShader(0), m_pixelShader(0),
	m_inputLayout(0), m_wireframeRS(0),
	m_theta(1.5f*MathHelper::Pi), m_phi(0.1f*MathHelper::Pi), m_radius(80.0f),
	m_matrixBuffer(0),
	//
	m_eyePosW(0.0f, 0.0f, 0.0f)
{
	m_mainWndCaption = L"Lighting Demo";

	m_lastMousePos.x = 0;
	m_lastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_landWorld, I);
	XMStoreFloat4x4(&m_wavesWorld, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

	XMStoreFloat4x4(&m_worldInvTranspose, I);

	XMMATRIX wavesOffset = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
	XMStoreFloat4x4(&m_wavesWorld, wavesOffset);

	//Directional light
	m_dirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_dirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_dirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	//Point light, 
	m_pointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_pointLight.Range = 25.0f;

	//Spot light
	m_spotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_spotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	m_spotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_spotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_spotLight.Spot = 96.0;
	m_spotLight.Range = 10000.0f;

	//Setup once and never changes
	m_landMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_landMat.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_landMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_wavesMat.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	m_wavesMat.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	m_wavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);

}

LightingApp::~LightingApp()
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


bool LightingApp::Init()
{
	if (!D3DApp::Init())
		return false;

	m_waves.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();

	bool result = BuildShader(L"lightingVS.hlsl", L"lightingPS.hlsl");
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


void LightingApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_proj, P);
}

void LightingApp::UpdateScene(float dt)
{
	//Convert Shperical to Cartesian coordinates
	float x = m_radius*sinf(m_phi)*cosf(m_theta);
	float z = m_radius*sinf(m_phi)*sinf(m_theta);
	float y = m_radius*cosf(m_phi);

	m_eyePosW = XMFLOAT3(x, y, z);

	//Build the view matrix
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, V);

	//
	//Waves update
	static float t_base = 0.0f;
	if (m_timer.TotalTime() - t_base >= 0.25f)
	{
		t_base += 0.25f;

		//don't disturb the outer 5 lines of quads, in any direction
		DWORD i = 5 + rand() % (m_waves.RowCount() - 10);
		DWORD j = 5 + rand() % (m_waves.ColumnCount() - 10);

		float r = MathHelper::RandF(1.0f, 2.0f);

		m_waves.Disturb(i, j, r);
	}

	m_waves.Update(dt);

	//Update Waves vertex buffer
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_d3dImmediateContext->Map(m_wavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	VertexType* v = reinterpret_cast<VertexType*>(mappedData.pData);
	for (UINT i = 0; i < m_waves.VertexCount(); ++i)
	{
		v[i].Pos = m_waves[i];
		v[i].Normal = m_waves.Normal(i);
	}

	m_d3dImmediateContext->Unmap(m_wavesVB, 0);

	//
	//Animate the lights
	//

	//Circle the point light over the land surface
	m_pointLight.Position.x = 70.0f*cosf(0.2f*m_timer.TotalTime());
	m_pointLight.Position.z = 70.0f*sinf(0.2f*m_timer.TotalTime());
	m_pointLight.Position.y = MathHelper::Max(
		GetHillHeight(m_pointLight.Position.x, m_pointLight.Position.z), -3.0f) + 10.0f;

	//The spot light takes on the camera position and is aimed in the
	//same direction the camera is looking. In this way, it looks like
	//we are holding a flashlight.
	m_spotLight.Position = m_eyePosW;
	XMStoreFloat3(&m_spotLight.Direction, XMVector3Normalize(target - pos));

}

bool LightingApp::DrawScene()
{
	//Begin Scene
	//Clean up the back buffer and depth/stencil buffer to get ready for the new frame
	m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	bool result;

	//Can't use RenderBuffer() here, since the Vertex and Index buffers are different

	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);

	
	UINT stride = sizeof(VertexType);
	UINT offset = 0;
	
	/////////////Land
	XMMATRIX landWorld = XMLoadFloat4x4(&m_landWorld);
	XMMATRIX landInvTrans = MathHelper::InverseTranspose(landWorld);

	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_landVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(m_landIB, DXGI_FORMAT_R32_UINT, 0);

	SetShaderParameters(landWorld, view, proj, landInvTrans, m_landMat, m_landIndexCount, 0, 0);

	/////////////Waves
	XMMATRIX wavesWorld = XMLoadFloat4x4(&m_wavesWorld);
	XMMATRIX wavesInvTrans = MathHelper::InverseTranspose(wavesWorld);

	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_wavesVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(m_wavesIB, DXGI_FORMAT_R32_UINT, 0);

	SetShaderParameters(wavesWorld, view, proj, wavesInvTrans, m_wavesMat, 3 * m_waves.TriangleCount(), 0, 0);

	
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

void LightingApp::OnMouseDown(WPARAM btnState, int x, int y)
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

void LightingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void LightingApp::OnMouseMove(WPARAM btnState, int x, int y)
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

float LightingApp::GetHillHeight(float x, float z)const
{
	return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}

XMFLOAT3 LightingApp::GetHillNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	//normalization
	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void LightingApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	m_landIndexCount = grid.Indices.size();

	//Extract the vertex elements we are interested and apply the height function
	//to each vertex
	std::vector<VertexType> vertices(grid.Vertices.size());

	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;
		p.y = GetHillHeight(p.x, p.z);

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
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
	ibd.ByteWidth = sizeof(UINT)*m_landIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(m_d3dDevice->CreateBuffer(&ibd, &iinitData, &m_landIB));

}

void LightingApp::BuildWaveGeometryBuffers()
{
	//Create the vertex buffer. Note that we allocate space only.
	//as we will be updating the data every time step of the simulation
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(VertexType)*m_waves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(m_d3dDevice->CreateBuffer(&vbd, 0, &m_wavesVB));

	//Index buffer, stil fixed
	std::vector<UINT> indices(3 * m_waves.TriangleCount());

	//Iterate over the quad
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


bool LightingApp::BuildShader(WCHAR* vsFilename, WCHAR* psFilename)
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
	result = D3DCompileFromFile(vsFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "LightingVertexShader", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS|D3DCOMPILE_DEBUG,
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
	result = D3DCompileFromFile(psFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "LightingPixelShader", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
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

	polygonLayout[1].SemanticName = "NORMAL";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
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
	HR(m_d3dDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer));
	

	//Setup the description of the eye dynamic constant buffer that is in the vertex shader

	//Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or
	//CreateBuffer will fail.
	D3D11_BUFFER_DESC eyeBufferDesc;
	eyeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	eyeBufferDesc.ByteWidth = sizeof(EyeBufferType);
	eyeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	eyeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	eyeBufferDesc.MiscFlags = 0;
	eyeBufferDesc.StructureByteStride = 0;

	HR(m_d3dDevice->CreateBuffer(&eyeBufferDesc, 0, &m_eyeBuffer));

	//Setup the description of the light dynamic constant buffer that is in the pixel shader
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	HR(m_d3dDevice->CreateBuffer(&lightBufferDesc, 0, &m_lightBuffer));


	return true;
}

bool LightingApp::SetShaderParameters(XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
	XMMATRIX worldInvTrans, Material material,
	int indexCount, int indexOffset, int vertexOffset)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	LightBufferType* dataPtr2;
	EyeBufferType* dataPtr3;
	

	///////////////////////////
	//////Transformation Matrix//
	///////////////////////////
	//Transpose the matrices to prepare them for the shader
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	worldInvTrans = XMMatrixTranspose(worldInvTrans);

	//Lock the constant buffer so it can be written to
	HR(m_d3dImmediateContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	//Get a pointer to the data in the constant buffer
	dataPtr = reinterpret_cast<MatrixBufferType*>(mappedResource.pData);

	//Copy the matrices into constant buffer
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	dataPtr->wInvTrans = worldInvTrans; //for normal trsnsformation



	//Unlock the constant buffer
	m_d3dImmediateContext->Unmap(m_matrixBuffer, 0);

	//Set the position of the constant buffer in the vertex shader
	bufferNumber = 0;

	//Finally set the constant buffer in the vertex shader with the updated values
	m_d3dImmediateContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);


	///////////////////////////
	/////Light Buffer/////////
	//////////////////////////
	HR(m_d3dImmediateContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	
	dataPtr2 = reinterpret_cast<LightBufferType*>(mappedResource.pData);


	dataPtr2->dir = DirectionalLight(); 
					//m_dirLight;
	dataPtr2->point = //PointLight(); 
					m_pointLight;
	dataPtr2->spot = SpotLight(); 
					 //m_spotLight;

	dataPtr2->mat = material; //actually could be static

	m_d3dImmediateContext->Unmap(m_lightBuffer, 0);

	bufferNumber = 0; //the first constant buffer in pixel shader

	m_d3dImmediateContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);


	///////////////////////////
	////Eye Buffer////////////
	//////////////////////////
	HR(m_d3dImmediateContext->Map(m_eyeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	dataPtr3 = reinterpret_cast<EyeBufferType*>(mappedResource.pData);

	dataPtr3->eye = m_eyePosW;
	dataPtr3->pad = 0.0f;

	m_d3dImmediateContext->Unmap(m_eyeBuffer, 0);

	bufferNumber = 1; //the second constant buffer in pixel shader

	m_d3dImmediateContext->PSSetConstantBuffers(bufferNumber, 1, &m_eyeBuffer);

	//
	RenderShader(indexCount, indexOffset, vertexOffset);
	//

	return true;
}

void LightingApp::RenderShader(int indexCount, int indexOffset, int vertexOffset)
{
	m_d3dImmediateContext->IASetInputLayout(m_inputLayout);

	//Set the vertex and pixel shaders that will be used to render this model
	m_d3dImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_d3dImmediateContext->PSSetShader(m_pixelShader, NULL, 0);

	m_d3dImmediateContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}