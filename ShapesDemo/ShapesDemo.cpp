
#include "ShapesDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	//Enable run-time memory check for debug builds
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ShapesApp  theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

ShapesApp::ShapesApp(HINSTANCE hInstance)
	:D3DApp(hInstance),
	m_shapeVB(0), m_shapeIB(0),
	m_vertexShader(0), m_pixelShader(0),
	m_inputLayout(0), m_wireFrameRS(0),
	m_theta(1.5f*MathHelper::Pi), m_phi(0.1f*MathHelper::Pi), m_radius(15.0f),
	m_matrixBuffer(0)
{
	m_mainWndCaption = L"Shapes Demo";

	m_lastMousePos.x = 0;
	m_lastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	
	//Grid
	XMStoreFloat4x4(&m_gridWorld, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

	//box
	XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&m_boxWorld, XMMatrixMultiply(boxScale, boxOffset)); //only scaling and translation

	//sphere
	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&m_centerSphere, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	for (int i = 0; i < 5; ++i)
	{
		//cylinders
		//5.0f gap in z direction between each pair of cylinders
		XMStoreFloat4x4(&m_cylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f));
		//10.0f gap in x direction for each in this pair
		XMStoreFloat4x4(&m_cylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f));

		//spheres
		XMStoreFloat4x4(&m_sphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&m_sphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f));
	}
}

ShapesApp::~ShapesApp()
{
	ReleaseCOM(m_shapeVB);
	ReleaseCOM(m_shapeIB);

	ReleaseCOM(m_inputLayout);
	ReleaseCOM(m_wireFrameRS);

	ReleaseCOM(m_vertexShader);
	ReleaseCOM(m_pixelShader);
	
	ReleaseCOM(m_matrixBuffer);

}

bool ShapesApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();

	bool result = BuildShader(L"shape.vs",L"shape.ps");
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

	HR(m_d3dDevice->CreateRasterizerState(&wireframeDesc, &m_wireFrameRS));

	return true;
}

bool ShapesApp::BuildShader(WCHAR* vsFilename, WCHAR* psFilename)
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
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "ShapeVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
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
	result = D3DCompileFromFile(psFilename, NULL, NULL, "ShapePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
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

void ShapesApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_proj, P);
}

void ShapesApp::UpdateScene(float dt)
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

bool ShapesApp::DrawScene()
{
	//Begin Scene
	//Clean up the back buffer and depth/stencil buffer to get ready for the new frame
	m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	bool result;

	RenderBuffers();

	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);

	XMMATRIX gridWorld = XMLoadFloat4x4(&m_gridWorld);
	SetShaderParameters(gridWorld, view, proj, m_gridIndexCount,m_gridIndexOffset, m_gridVertexOffset);

	XMMATRIX boxWorld = XMLoadFloat4x4(&m_boxWorld);
	SetShaderParameters(boxWorld, view, proj, m_boxIndexCount, m_boxIndexOffset, m_boxVertexOffset);

	XMMATRIX centerSphereWorld = XMLoadFloat4x4(&m_centerSphere);
	SetShaderParameters(centerSphereWorld, view, proj, m_sphereIndexCount, m_sphereIndexOffset, m_sphereVertexOffset);

	XMMATRIX cylWorld[10];
	XMMATRIX sphereWorld[10];

	for (int i = 0; i < 10; ++i) 
	{
		cylWorld[i] = XMLoadFloat4x4(&m_cylWorld[i]);
		SetShaderParameters(cylWorld[i], view, proj, m_cylinderIndexCount,m_cylinderIndexOffset, m_cylinderVertexOffset);

		sphereWorld[i] = XMLoadFloat4x4(&m_sphereWorld[i]);
		SetShaderParameters(sphereWorld[i], view, proj, m_sphereIndexCount, m_sphereIndexOffset, m_sphereVertexOffset);
	}


	//RenderShader();
	
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

void ShapesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos.x = x;
	m_lastMousePos.y = y;

	if ((btnState & MK_MBUTTON) != 0)
	{
		if (solidFlag)
		{
			m_d3dImmediateContext->RSSetState(m_wireFrameRS);
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

void ShapesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void ShapesApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.01f*static_cast<float>(x - m_lastMousePos.x);
		float dy = 0.01f*static_cast<float>(y - m_lastMousePos.y);

		//Update the camera radius based on input
		m_radius += (dx - dy);

		//Restrict the radius
		m_radius = MathHelper::Clamp(m_radius, 3.0f, 200.0f);
	}

	m_lastMousePos.x = x;
	m_lastMousePos.y = y;
}

bool ShapesApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;

	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	//geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateGeosphere(0.5f, 2, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	//Cache the vertex offsets to each object in the concatenated vertex buffer
	m_boxVertexOffset = 0;
	m_gridVertexOffset = box.Vertices.size();
	m_sphereVertexOffset = m_gridVertexOffset + grid.Vertices.size();
	m_cylinderVertexOffset = m_sphereVertexOffset + sphere.Vertices.size();

	//Cache the index count of each object
	m_boxIndexCount = box.Indices.size();
	m_gridIndexCount = grid.Indices.size();
	m_sphereIndexCount = sphere.Indices.size();
	m_cylinderIndexCount = cylinder.Indices.size();

	//Cache the starting index for each object in the concatenated index buffer
	m_boxIndexOffset = 0;
	m_gridIndexOffset = m_boxIndexCount;
	m_sphereIndexOffset = m_gridIndexOffset + m_gridIndexCount;
	m_cylinderIndexOffset = m_sphereIndexOffset + m_sphereIndexCount;

	UINT totalVertexCount = 
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount = 
		m_boxIndexCount +
		m_gridIndexCount +
		m_sphereIndexCount +
		m_cylinderIndexCount;

	////Extract the vertex elements we are interested in and pack the 
	////vertices of all the meshes into one vertex buffer
	std::vector<VertexType> vertices(totalVertexCount);

	XMFLOAT4 black = XMFLOAT4(reinterpret_cast<const float*>(&Colors::Black));

	XMFLOAT4  red = XMFLOAT4(reinterpret_cast<const float*>(&Colors::Red));

	XMFLOAT4  green = XMFLOAT4(reinterpret_cast<const float*>(&Colors::Green));

	XMFLOAT4  blue = XMFLOAT4(reinterpret_cast<const float*>(&Colors::Blue));

	//XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Color = red;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Color = green;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Color = blue;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexType)*totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_d3dDevice->CreateBuffer(&vbd, &vinitData, &m_shapeVB));

	//Pack the indices of all the meshes into one index buffer
	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)*totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_d3dDevice->CreateBuffer(&ibd, &iinitData, &m_shapeIB));

	return true;
}

bool ShapesApp::SetShaderParameters(XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, int indexCount,int indexOffset, int vertexOffset)
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
	RenderShader(indexCount,indexOffset,vertexOffset);
	//

	return true;
}

void ShapesApp::RenderShader(int indexCount, int indexOffset, int vertexOffset)
{
	m_d3dImmediateContext->IASetInputLayout(m_inputLayout);

	//Set the vertex and pixel shaders that will be used to render this model
	m_d3dImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_d3dImmediateContext->PSSetShader(m_pixelShader, NULL, 0);

	m_d3dImmediateContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void ShapesApp::RenderBuffers()
{
	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_shapeVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(m_shapeIB, DXGI_FORMAT_R32_UINT, 0);

	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}