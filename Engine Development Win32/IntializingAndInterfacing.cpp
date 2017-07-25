#include "stdafx.h"
#include "Init_and_Inter.h"

bool Init_and_Inter::InitializeDirect3d11App(HINSTANCE hInstance, HWND hWnd)
{
	// Set the Width and Height Globals
	RECT rect;
	GetClientRect(hWnd, &rect);
	Width = rect.right - rect.left;
	Height = rect.bottom - rect.top;

	HRESULT hr;

	//Describe our Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = Width;
	bufferDesc.Height = Height;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


	//Create our SwapChain
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

	//Create our BackBuffer
	ID3D11Texture2D* BackBuffer;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

	//Create our Render Target
	hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
	BackBuffer->Release();

	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = Width;
	depthStencilDesc.Height = Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	//Create the Viewport
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Width;
	viewport.Height = Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Initialize the WireFrame Rasterizer State
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = d3d11Device->CreateRasterizerState(&wfdesc, &WireFrame);
	d3d11DevCon->RSSetState(WireFrame);

	return true;
}

void Init_and_Inter::ReleaseObjects()
{
	//Release the COM Objects we created
	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	renderTargetView->Release();
	depthStencilView->Release();
	depthStencilBuffer->Release();

	cbPerObjectBuffer->Release();

	WireFrame->Release();

	// Release the cube objects
	_square.squareVertexBuffer->Release();
	_square.squareIndexBuffer->Release();
	_square.VS->Release();
	_square.PS->Release();
	_square.VS_Buffer->Release();
	_square.PS_Buffer->Release();
	_square.vertLayout->Release();

	// Release the mage model objects
	gMageModel.pVertexBuffer->Release();
	gMageModel.pVS->Release();
	gMageModel.pPS->Release();
	gMageModel.pVS_Buffer->Release();
	gMageModel.pPS_Buffer->Release();
	gMageModel.pvertLayout->Release();
}

bool Init_and_Inter::InitScene(User_Input &_input)
{
	// Pass in the User_Input
	input = _input;

#pragma region Shader Stuff

	// Compile Shaders from shader files
	hr = D3DCompileFromFile(L"SimpleVertexShader.hlsl", NULL, NULL, "main", "vs_5_0", 0, 0, &_square.VS_Buffer, NULL);
	hr = D3DCompileFromFile(L"SimplePixelShader.hlsl", NULL, NULL, "main", "ps_5_0", 0, 0, &_square.PS_Buffer, NULL);
	hr = D3DCompileFromFile(L"ModelVertexShader.hlsl", NULL, NULL, "main", "vs_5_0", 0, 0, &gMageModel.pVS_Buffer, NULL);
	hr = D3DCompileFromFile(L"ModelPixelShader.hlsl", NULL, NULL, "main", "ps_5_0", 0, 0, &gMageModel.pPS_Buffer, NULL);
	hr = D3DCompileFromFile(L"ModelVertexShader.hlsl", NULL, NULL, "main", "vs_5_0", 0, 0, &gTeddyModel.pVS_Buffer, NULL);
	hr = D3DCompileFromFile(L"ModelPixelShader.hlsl", NULL, NULL, "main", "ps_5_0", 0, 0, &gTeddyModel.pPS_Buffer, NULL);

	// Create the shader objects
	hr = d3d11Device->CreateVertexShader(_square.VS_Buffer->GetBufferPointer(), _square.VS_Buffer->GetBufferSize(), NULL, &_square.VS);
	hr = d3d11Device->CreatePixelShader(_square.PS_Buffer->GetBufferPointer(), _square.PS_Buffer->GetBufferSize(), NULL, &_square.PS);
	hr = d3d11Device->CreateVertexShader(gMageModel.pVS_Buffer->GetBufferPointer(), gMageModel.pVS_Buffer->GetBufferSize(), NULL, &gMageModel.pVS);
	hr = d3d11Device->CreatePixelShader(gMageModel.pPS_Buffer->GetBufferPointer(), gMageModel.pPS_Buffer->GetBufferSize(), NULL, &gMageModel.pPS);
	hr = d3d11Device->CreateVertexShader(gTeddyModel.pVS_Buffer->GetBufferPointer(), gTeddyModel.pVS_Buffer->GetBufferSize(), NULL, &gTeddyModel.pVS);
	hr = d3d11Device->CreatePixelShader(gTeddyModel.pPS_Buffer->GetBufferPointer(), gTeddyModel.pPS_Buffer->GetBufferSize(), NULL, &gTeddyModel.pPS);

#pragma endregion

#pragma region Cube

	//Create the vertex buffer
	Vertex v[] =
	{
		Vertex(-10.0f, 0.0f, -10.0f, 1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(-10.0f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 1.0f),
		Vertex(+10.0f, 0.0f, -10.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(+10.0f, 0.0f, -10.0f, 1.0f, 1.0f, 0.0f, 1.0f),
		Vertex(-10.0f, 0.0f, +10.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(-10.0f, 0.0f, +10.0f, 1.0f, 1.0f, 1.0f, 1.0f),
		Vertex(+10.0f, 0.0f, +10.0f, 1.0f, 0.0f, 1.0f, 1.0f),
		Vertex(+10.0f, 0.0f, +10.0f, 1.0f, 0.0f, 0.0f, 1.0f),
	};

	DWORD indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 12 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 8;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	D3D11_SUBRESOURCE_DATA iinitData;

	ZeroMemory(&iinitData, sizeof(iinitData));
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));

	vertexBufferData.pSysMem = v;
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &_square.squareVertexBuffer);

	iinitData.pSysMem = indices;
	d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &_square.squareIndexBuffer);

	// Create the Input Layout
	// Layout for describing the triangle
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	hr = d3d11Device->CreateInputLayout(layout, numElements, _square.VS_Buffer->GetBufferPointer(), _square.VS_Buffer->GetBufferSize(), &_square.vertLayout);

	//Create the buffer to send to the cbuffer in effect file
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

#pragma endregion

#pragma region Camera Setup

	//Camera information
	camPosition = XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
	camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//Set the View matrix
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	//Set the Projection matrix
	camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14f, (float)Width / Height, 1.0f, 1000.0f);

#pragma endregion

#pragma region FBX Stuff

#pragma region Mage Model

	// Load the model
	char* magefileName = "mage.fbx";
	char** magefilename = &magefileName;
	fbx_Exec.Load_FBX(magefilename);

	gMageSkeleton = fbx_Exec.getSkeleton();

	// Add the lines into the joint_debugRenderer
	for (unsigned int i = 0; i < gMageSkeleton->pJoints.size(); i++)
	{
		// Get the Position, X,Y,Z Axis
		XMFLOAT3 pPosition = XMFLOAT3(gMageSkeleton->pJoints[i].pPosition.x, gMageSkeleton->pJoints[i].pPosition.y, gMageSkeleton->pJoints[i].pPosition.z);
		XMFLOAT3 pXAxis = XMFLOAT3(gMageSkeleton->pJoints[i].pXAxis.x, gMageSkeleton->pJoints[i].pXAxis.y, gMageSkeleton->pJoints[i].pXAxis.z);
		XMFLOAT3 pYAxis = XMFLOAT3(gMageSkeleton->pJoints[i].pYAxis.x, gMageSkeleton->pJoints[i].pYAxis.y, gMageSkeleton->pJoints[i].pYAxis.z);
		XMFLOAT3 pZAxis = XMFLOAT3(gMageSkeleton->pJoints[i].pZAxis.x, gMageSkeleton->pJoints[i].pZAxis.y, gMageSkeleton->pJoints[i].pZAxis.z); 

		// Add lines to the debug renderer
		// Draw X axis in red
		XMFLOAT3 pPosToXAxis = XMFLOAT3(pPosition.x + pXAxis.x, pPosition.y + pXAxis.y, pPosition.z + pXAxis.z);
		mage_joint_debugRenderer.add_debug_line(pPosition, pPosToXAxis, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
		// Draw Y axis in green
		XMFLOAT3 pPosToYAxis = XMFLOAT3(pPosition.x + pYAxis.x, pPosition.y + pYAxis.y, pPosition.z + pYAxis.z);
		mage_joint_debugRenderer.add_debug_line(pPosition, pPosToYAxis, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
		// Draw Z axis in blue
		XMFLOAT3 pPosToZAxis = XMFLOAT3(pPosition.x + pZAxis.x, pPosition.y + pZAxis.y, pPosition.z + pZAxis.z);
		mage_joint_debugRenderer.add_debug_line(pPosition, pPosToZAxis, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	}

	// Push cpu to gpu
	mage_joint_debugRenderer.push_to_gpu(d3d11DevCon);

	// Add the bones to the bone_debugRenderer
	for (unsigned int i = 0; i < gMageSkeleton->pJoints.size(); i++)
	{
		// Create a variable to hold the parent index
		int pParentIndex = 0;

		// Get the next parent index
		pParentIndex = gMageSkeleton->pJoints[i].pParentIndex;

		// It is the root
		if (pParentIndex == -1)
			continue;

		else
		{
			// Get the line between parent to child
			XMFLOAT3 pChildPosition = XMFLOAT3(gMageSkeleton->pJoints[i].pPosition.x, gMageSkeleton->pJoints[i].pPosition.y, gMageSkeleton->pJoints[i].pPosition.z);
			XMFLOAT3 pParentPosition = XMFLOAT3(gMageSkeleton->pJoints[pParentIndex].pPosition.x, gMageSkeleton->pJoints[pParentIndex].pPosition.y, gMageSkeleton->pJoints[pParentIndex].pPosition.z);

			// Add the line to the bone_debugRenderer
			mage_bone_debugRenderer.add_debug_line(pChildPosition, pParentPosition, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	// Push cpu to gpu
	mage_bone_debugRenderer.push_to_gpu(d3d11DevCon);

	// Create the vertex buffer
	D3D11_BUFFER_DESC mage_vertexBufferDesc;
	ZeroMemory(&mage_vertexBufferDesc, sizeof(mage_vertexBufferDesc));

	int mage_vertices_size = gMageSkeleton->pMesh.pVertices.size();
	mage_vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	mage_vertexBufferDesc.ByteWidth = sizeof(Mesh_Vertex) * mage_vertices_size;
	mage_vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	mage_vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mage_vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA mage_vertexBufferData;
	ZeroMemory(&mage_vertexBufferData, sizeof(mage_vertexBufferData));

	mage_vertexBufferData.pSysMem = &gMageSkeleton->pMesh.pVertices[0];
	hr = d3d11Device->CreateBuffer(&mage_vertexBufferDesc, &mage_vertexBufferData, &gMageModel.pVertexBuffer);

	// Create the Input Layout
	// Layout for describing the triangle
	D3D11_INPUT_ELEMENT_DESC mage_layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT mage_numElements = ARRAYSIZE(mage_layout);

	hr = d3d11Device->CreateInputLayout(mage_layout, mage_numElements, gMageModel.pVS_Buffer->GetBufferPointer(), gMageModel.pVS_Buffer->GetBufferSize(), &gMageModel.pvertLayout);

	//Create the buffer to send to the cbuffer in effect file
	D3D11_BUFFER_DESC mage_cbbd;
	ZeroMemory(&mage_cbbd, sizeof(D3D11_BUFFER_DESC));

	mage_cbbd.Usage = D3D11_USAGE_DEFAULT;
	mage_cbbd.ByteWidth = sizeof(cbPerObject);
	mage_cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mage_cbbd.CPUAccessFlags = 0;
	mage_cbbd.MiscFlags = 0;

	hr = d3d11Device->CreateBuffer(&mage_cbbd, NULL, &cbPerObjectBuffer);

#pragma endregion

#pragma region Teddy Model

	// Load the model
	//char* teddyfileName = "teddy.fbx";
	//char** teddyfilename = &teddyfileName;
	//fbx_Exec.Load_FBX(teddyfilename);

	//gTeddySkeleton = fbx_Exec.getSkeleton();

	//std::vector<Joint> rootIndexedJoints;

	//for (unsigned int i = 0; i < gTeddySkeleton->pJoints.size(); i++)
	//{
	//	if (gTeddySkeleton->pJoints[i].pParentIndex == -1)
	//	{
	//		rootIndexedJoints.push_back(gTeddySkeleton->pJoints[i]);
	//	}
	//}

	//// Add the lines into the joint_debugRenderer
	//for (unsigned int i = 0; i < gTeddySkeleton->pJoints.size(); i++)
	//{
	//	// Get the Position, X,Y,Z Axis
	//	XMFLOAT3 pPosition = XMFLOAT3(gTeddySkeleton->pJoints[i].pPosition.x, gTeddySkeleton->pJoints[i].pPosition.y, gTeddySkeleton->pJoints[i].pPosition.z);
	//	XMFLOAT3 pXAxis = XMFLOAT3(gTeddySkeleton->pJoints[i].pXAxis.x, gTeddySkeleton->pJoints[i].pXAxis.y, gTeddySkeleton->pJoints[i].pXAxis.z);
	//	XMFLOAT3 pYAxis = XMFLOAT3(gTeddySkeleton->pJoints[i].pYAxis.x, gTeddySkeleton->pJoints[i].pYAxis.y, gTeddySkeleton->pJoints[i].pYAxis.z);
	//	XMFLOAT3 pZAxis = XMFLOAT3(gTeddySkeleton->pJoints[i].pZAxis.x, gTeddySkeleton->pJoints[i].pZAxis.y, gTeddySkeleton->pJoints[i].pZAxis.z);

	//	// Add lines to the debug renderer
	//	// Draw X axis in red
	//	XMFLOAT3 pPosToXAxis = XMFLOAT3(pPosition.x + pXAxis.x, pPosition.y + pXAxis.y, pPosition.z + pXAxis.z);
	//	teddy_joint_debugRenderer.add_debug_line(pPosition, pPosToXAxis, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	//	// Draw Y axis in green
	//	XMFLOAT3 pPosToYAxis = XMFLOAT3(pPosition.x + pYAxis.x, pPosition.y + pYAxis.y, pPosition.z + pYAxis.z);
	//	teddy_joint_debugRenderer.add_debug_line(pPosition, pPosToYAxis, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
	//	// Draw Z axis in blue
	//	XMFLOAT3 pPosToZAxis = XMFLOAT3(pPosition.x + pZAxis.x, pPosition.y + pZAxis.y, pPosition.z + pZAxis.z);
	//	teddy_joint_debugRenderer.add_debug_line(pPosition, pPosToZAxis, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	//}

	//// Push cpu to gpu
	//teddy_joint_debugRenderer.push_to_gpu(d3d11DevCon);

	//// Add the bones to the bone_debugRenderer
	//for (unsigned int i = 0; i < gTeddySkeleton->pJoints.size(); i++)
	//{
	//	// Create a variable to hold the parent index
	//	int pParentIndex = 0;

	//	// Get the next parent index
	//	pParentIndex = gTeddySkeleton->pJoints[i].pParentIndex;

	//	// It is the root
	//	if (pParentIndex == -1)
	//		continue;

	//	else
	//	{
	//		// Get the line between parent to child
	//		XMFLOAT3 pChildPosition = XMFLOAT3(gTeddySkeleton->pJoints[i].pPosition.x, gTeddySkeleton->pJoints[i].pPosition.y, gTeddySkeleton->pJoints[i].pPosition.z);
	//		XMFLOAT3 pParentPosition = XMFLOAT3(gTeddySkeleton->pJoints[pParentIndex].pPosition.x, gTeddySkeleton->pJoints[pParentIndex].pPosition.y, gTeddySkeleton->pJoints[pParentIndex].pPosition.z);

	//		// Add the line to the bone_debugRenderer
	//		teddy_bone_debugRenderer.add_debug_line(pChildPosition, pParentPosition, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	//	}
	//}

	//// Push cpu to gpu
	//teddy_bone_debugRenderer.push_to_gpu(d3d11DevCon);

	//// Create the vertex buffer
	//D3D11_BUFFER_DESC teddy_vertexBufferDesc;
	//ZeroMemory(&teddy_vertexBufferDesc, sizeof(teddy_vertexBufferDesc));

	//int teddy_vertices_size = gTeddySkeleton->pMesh.pVertices.size();
	//teddy_vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//teddy_vertexBufferDesc.ByteWidth = sizeof(Mesh_Vertex) * teddy_vertices_size;
	//teddy_vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//teddy_vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//teddy_vertexBufferDesc.MiscFlags = 0;

	//D3D11_SUBRESOURCE_DATA teddy_vertexBufferData;
	//ZeroMemory(&teddy_vertexBufferData, sizeof(teddy_vertexBufferData));

	//teddy_vertexBufferData.pSysMem = &gTeddySkeleton->pMesh.pVertices[0];
	//hr = d3d11Device->CreateBuffer(&teddy_vertexBufferDesc, &teddy_vertexBufferData, &gTeddyModel.pVertexBuffer);

	//// Create the Input Layout
	//// Layout for describing the triangle
	//D3D11_INPUT_ELEMENT_DESC teddy_layout[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "COLOR", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//};
	//UINT teddy_numElements = ARRAYSIZE(teddy_layout);

	//hr = d3d11Device->CreateInputLayout(teddy_layout, teddy_numElements, gTeddyModel.pVS_Buffer->GetBufferPointer(), gTeddyModel.pVS_Buffer->GetBufferSize(), &gTeddyModel.pvertLayout);

	////Create the buffer to send to the cbuffer in effect file
	//D3D11_BUFFER_DESC teddy_cbbd;
	//ZeroMemory(&teddy_cbbd, sizeof(D3D11_BUFFER_DESC));

	//teddy_cbbd.Usage = D3D11_USAGE_DEFAULT;
	//teddy_cbbd.ByteWidth = sizeof(cbPerObject);
	//teddy_cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//teddy_cbbd.CPUAccessFlags = 0;
	//teddy_cbbd.MiscFlags = 0;

	//hr = d3d11Device->CreateBuffer(&teddy_cbbd, NULL, &cbPerObjectBuffer);

#pragma endregion

#pragma endregion

	return true;
}

void Init_and_Inter::UpdateScene(double time, User_Input &_input, Timer pTimer)
{
	// Update the input
	input = _input;

	//Keep the cubes rotating
	rot += 1.0f * time;
	if (rot > 6.26f)
		rot = 0.0f;

	// Update the mage model joints/bones if the animation is scrubbing
	if (input.gAnimationScrubbing)
		UpdateKeyframe(input.gKeyframeIndex);

	else if (input.gAnimationPlaying)
		UpdateKeyframe(input.gKeyframeIndex);

	else if (input.gAnimationTweening)
		TweeningAnimation(pTimer);

	_input = input;

	//Reset cube1World
	cube1World = XMMatrixIdentity();

	//Define cube1's world space matrix
	XMVECTOR rotyaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR rotzaxis = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR rotxaxis = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	Rotation = XMMatrixRotationAxis(rotyaxis, rot);
	input.Rotationx = XMMatrixRotationAxis(rotxaxis, input.rotx);
	input.Rotationz = XMMatrixRotationAxis(rotzaxis, input.rotz);
	Translation = XMMatrixTranslation(0.0f, 0.0f, 4.0f);

	//Set cube1's world space using the transformations
	//cube1World = Translation * Rotation * input.Rotationx * input.Rotationz;
	cube1World = XMMatrixIdentity();

	//Reset cube2World
	cube2World = XMMatrixIdentity();

	//Define cube2's world space matrix
	Rotation = XMMatrixRotationAxis(rotyaxis, -rot);
	Scale = XMMatrixScaling(input.scaleX, input.scaleY, 1.3f);

	//Set cube2's world space matrix
	cube2World = Rotation * Scale;

	// Define the mage's world space matrix
	Rotation = XMMatrixRotationAxis(rotyaxis, 0);
	Scale = XMMatrixScaling(input.scaleX, input.scaleY, 1.3f);

	// Set mage's world space matrix
	mageWorld = Rotation * Scale;

	// Define the Teddy's world space matrix
	Translation = XMMatrixTranslation(4.0f, 0.0f, 0.0f);
	Rotation = XMMatrixRotationAxis(rotyaxis, 0);

	// Set Teddy's world space matrix
	teddyWorld = Rotation * Scale;

	//Update the colors of our scene
	/*red += colormodr * 0.00005f;
	green += colormodg * 0.00002f;
	blue += colormodb * 0.00001f;

	if (red >= 1.0f || red <= 0.0f)
		colormodr *= -1;
	if (green >= 1.0f || green <= 0.0f)
		colormodg *= -1;
	if (blue >= 1.0f || blue <= 0.0f)
		colormodb *= -1;*/
}

void Init_and_Inter::DrawScene()
{

#pragma region Initial Stuff (Reseting Defaults)

	//Set Primitive Topology
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Clear our backbuffer to the updated color
	// float bgColor[4] = { red, blue, green, 1.0f };
	float bgColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);

	//Refresh the Depth/Stencil view
	d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

#pragma endregion

#pragma region Cube Stuff

	// Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(_square.VS, 0, 0);
	d3d11DevCon->PSSetShader(_square.PS, 0, 0);

	d3d11DevCon->IASetIndexBuffer(_square.squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &_square.squareVertexBuffer, &stride, &offset);

	//Set the Viewport
	d3d11DevCon->RSSetViewports(1, &viewport);

	//Set the Input Layout
	d3d11DevCon->IASetInputLayout(_square.vertLayout);

	//Set the WVP matrix and send it to the constant buffer in effect file
	WVP = cube1World * camView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	//Draw the first cube
	d3d11DevCon->DrawIndexed(36, 0, 0);

#pragma endregion

#pragma region Debug Renderer Stuff

#pragma region Mage Debug Renderer

	// Setup the WVP for the mage
	WVP = mageWorld * camView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	// Draw the mage_joint_DebugRenderer Lines
	mage_joint_debugRenderer.RenderLines();

	// Draw the mage_bone_debugRenderer Lines
	mage_bone_debugRenderer.RenderLines();

	// Clear both debug renderers before next update happens
	mage_joint_debugRenderer.clear_counter();
	mage_bone_debugRenderer.clear_counter();

#pragma endregion

#pragma region Teddy Debug Renderer

	// Setup the WVP for the Teddy
	//WVP = teddyWorld * camView * camProjection;
	//cbPerObj.WVP = XMMatrixTranspose(WVP);
	//d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	//d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	//// Draw the teddy_joint_DebugRenderer Lines
	//teddy_joint_debugRenderer.RenderLines();

	//// Draw the teddy_bone_debugRenderer Lines
	//teddy_bone_debugRenderer.RenderLines();

#pragma endregion

#pragma endregion

#pragma region Meshes

#pragma region Mage Mesh

	// Set new Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(gMageModel.pVS, 0, 0);
	d3d11DevCon->PSSetShader(gMageModel.pPS, 0, 0);

	//Set the new vertex buffer
	stride = sizeof(Mesh_Vertex);
	offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &gMageModel.pVertexBuffer, &stride, &offset);

	//Set the Input Layout
	d3d11DevCon->IASetInputLayout(gMageModel.pvertLayout);

	// Set the WVP matrix and send it to the constant buffer in hlsl file
	WVP = mageWorld * camView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	//Draw the mage model mesh
	d3d11DevCon->Draw(gMageSkeleton->pMesh.pVertices.size(), 0);

#pragma endregion

#pragma region Teddy Mesh

	// Set new Vertex and Pixel Shaders
	//d3d11DevCon->VSSetShader(gTeddyModel.pVS, 0, 0);
	//d3d11DevCon->PSSetShader(gTeddyModel.pPS, 0, 0);

	////Set the new vertex buffer
	//stride = sizeof(Mesh_Vertex);
	//offset = 0;
	//d3d11DevCon->IASetVertexBuffers(0, 1, &gTeddyModel.pVertexBuffer, &stride, &offset);

	////Set the Input Layout
	//d3d11DevCon->IASetInputLayout(gTeddyModel.pvertLayout);

	//// Set the WVP matrix and send it to the constant buffer in hlsl file
	//WVP = teddyWorld * camView * camProjection;
	//cbPerObj.WVP = XMMatrixTranspose(WVP);
	//d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	//d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	////Draw the mage model mesh
	//d3d11DevCon->Draw(gTeddySkeleton->pMesh.pVertices.size(), 0);

#pragma endregion

#pragma endregion

	// Present the backbuffer to the screen
	SwapChain->Present(0, 0);
}

void Init_and_Inter::UpdateCamera()
{
	camRotationMatrix = XMMatrixRotationRollPitchYaw(input.camPitch, input.camYaw, 0);
	camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(input.camYaw);

	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);
	camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

	camPosition += input.moveLeftRight*camRight;
	camPosition += input.moveBackForward*camForward;
	camPosition += input.moveUpDown*camUp;

	camTarget = camPosition + camTarget;

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
}

void Init_and_Inter::UpdateKeyframe(int pKeyframeIndex)
{
	// Add the lines into the joint_debugRenderer
	for (unsigned int i = 0; i < gMageSkeleton->pJoints.size(); i++)
	{
		// Check to see if the index is even inside of the range
		if (gMageSkeleton->pJoints[i].pKeyframes.size() - 1 <= pKeyframeIndex)
		{
			input.gKeyframeOutOfRange = true;
			pKeyframeIndex = gMageSkeleton->pJoints[i].pKeyframes.size() - 1;
		}
		else
			input.gKeyframeOutOfRange = false;

		// Get the starting position X, Y, Z
		float lStart_X = gMageSkeleton->pJoints[i].pPosition.x;
		float lStart_Y = gMageSkeleton->pJoints[i].pPosition.y;
		float lStart_Z = gMageSkeleton->pJoints[i].pPosition.z;

		// Get the X, Y, Z translation values at the given keyframe
		float lXVal = gMageSkeleton->pJoints[i].pKeyframes[pKeyframeIndex].pTranslation.pX.pVal;
		float lYVal = gMageSkeleton->pJoints[i].pKeyframes[pKeyframeIndex].pTranslation.pY.pVal;
		float lZVal = gMageSkeleton->pJoints[i].pKeyframes[pKeyframeIndex].pTranslation.pZ.pVal;

		// Get the X, Y, Z rotation values at the given keyframe
		float lRXVal = gMageSkeleton->pJoints[i].pKeyframes[pKeyframeIndex].pRotation.pX.pVal;
		float lRYVal = gMageSkeleton->pJoints[i].pKeyframes[pKeyframeIndex].pRotation.pY.pVal;
		float lRZVal = gMageSkeleton->pJoints[i].pKeyframes[pKeyframeIndex].pRotation.pZ.pVal;

		// Add keyframe value to the starting position
		lXVal += lStart_X;
		lYVal += lStart_Y;
		lZVal += lStart_Z;

		// Get the Position, X,Y,Z Axis
		XMFLOAT3 lPosition = XMFLOAT3(lXVal, lYVal, lZVal);
		XMFLOAT3 lXAxis = XMFLOAT3(gMageSkeleton->pJoints[i].pXAxis.x, gMageSkeleton->pJoints[i].pXAxis.y, gMageSkeleton->pJoints[i].pXAxis.z);
		XMFLOAT3 lYAxis = XMFLOAT3(gMageSkeleton->pJoints[i].pYAxis.x, gMageSkeleton->pJoints[i].pYAxis.y, gMageSkeleton->pJoints[i].pYAxis.z);
		XMFLOAT3 lZAxis = XMFLOAT3(gMageSkeleton->pJoints[i].pZAxis.x, gMageSkeleton->pJoints[i].pZAxis.y, gMageSkeleton->pJoints[i].pZAxis.z);

		// Add lines to the debug renderer
		// Draw X axis in red
		XMFLOAT3 lPosToXAxis = XMFLOAT3(lPosition.x + lXAxis.x, lPosition.y + lXAxis.y, lPosition.z + lXAxis.z);
		mage_joint_debugRenderer.add_debug_line(lPosition, lPosToXAxis, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
		// Draw Y axis in green
		XMFLOAT3 lPosToYAxis = XMFLOAT3(lPosition.x + lYAxis.x, lPosition.y + lYAxis.y, lPosition.z + lYAxis.z);
		mage_joint_debugRenderer.add_debug_line(lPosition, lPosToYAxis, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
		// Draw Z axis in blue
		XMFLOAT3 lPosToZAxis = XMFLOAT3(lPosition.x + lZAxis.x, lPosition.y + lZAxis.y, lPosition.z + lZAxis.z);
		mage_joint_debugRenderer.add_debug_line(lPosition, lPosToZAxis, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	}

	// Push cpu to gpu
	mage_joint_debugRenderer.push_to_gpu(d3d11DevCon);

	// Add the bones to the bone_debugRenderer
	for (unsigned int i = 0; i < gMageSkeleton->pJoints.size(); i++)
	{
		// Create a variable to hold the parent index
		int lParentIndex = 0;

		// Get the next parent index
		lParentIndex = gMageSkeleton->pJoints[i].pParentIndex;

		// It is the root
		if (lParentIndex == -1)
			continue;

		if (gMageSkeleton->pJoints[i].pKeyframes.size() - 1 < pKeyframeIndex)
		{
			input.gKeyframeOutOfRange = true;
			pKeyframeIndex = gMageSkeleton->pJoints[i].pKeyframes.size() - 1;
		}
		else if (gMageSkeleton->pJoints[lParentIndex].pKeyframes.size() - 1 < pKeyframeIndex)
		{
			input.gKeyframeOutOfRange = true;
			pKeyframeIndex = gMageSkeleton->pJoints[lParentIndex].pKeyframes.size() - 1;
		}
		else if (gMageSkeleton->pJoints[i].pKeyframes.size() - 1 > pKeyframeIndex)
		{
			input.gKeyframeOutOfRange = false;
		}
		{
			// Get the X, Y, Z values at the given keyframe
			float lChild_XVal = gMageSkeleton->pJoints[i].pKeyframes[pKeyframeIndex].pTranslation.pX.pVal;
			float lChild_YVal = gMageSkeleton->pJoints[i].pKeyframes[pKeyframeIndex].pTranslation.pY.pVal;
			float lChild_ZVal = gMageSkeleton->pJoints[i].pKeyframes[pKeyframeIndex].pTranslation.pZ.pVal;
			float lParent_XVal = gMageSkeleton->pJoints[lParentIndex].pKeyframes[pKeyframeIndex].pTranslation.pX.pVal;
			float lParent_YVal = gMageSkeleton->pJoints[lParentIndex].pKeyframes[pKeyframeIndex].pTranslation.pY.pVal;
			float lParent_ZVal = gMageSkeleton->pJoints[lParentIndex].pKeyframes[pKeyframeIndex].pTranslation.pZ.pVal;

			// Add keyframe value to the starting position
			lChild_XVal += gMageSkeleton->pJoints[i].pPosition.x;
			lChild_YVal += gMageSkeleton->pJoints[i].pPosition.y;
			lChild_ZVal += gMageSkeleton->pJoints[i].pPosition.z;
			lParent_XVal += gMageSkeleton->pJoints[lParentIndex].pPosition.x;
			lParent_YVal += gMageSkeleton->pJoints[lParentIndex].pPosition.y;
			lParent_ZVal += gMageSkeleton->pJoints[lParentIndex].pPosition.z;

			// Get the line between parent to child
			XMFLOAT3 lChildPosition = XMFLOAT3(lChild_XVal, lChild_YVal, lChild_ZVal);
			XMFLOAT3 lParentPosition = XMFLOAT3(lParent_XVal, lParent_YVal, lParent_ZVal);

			// Add the line to the bone_debugRenderer
			mage_bone_debugRenderer.add_debug_line(lChildPosition, lParentPosition, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	// Push cpu to gpu
	mage_bone_debugRenderer.push_to_gpu(d3d11DevCon);
}

Keyframe_Vertex VertexInterpolation(Keyframe_Vertex pA, Keyframe_Vertex pB, float pRatio)
{
	// Create the keyframe to fill out and return
	Keyframe_Vertex lTemp;

	// Fill out the keytime of the vertex
	lTemp.pX.pKeytime = (pA.pX.pKeytime + pB.pX.pKeytime) / 2;
	lTemp.pY.pKeytime = (pA.pY.pKeytime + pB.pY.pKeytime) / 2;
	lTemp.pZ.pKeytime = (pA.pZ.pKeytime + pB.pZ.pKeytime) / 2;

	// Fill out the value of the vertex
	lTemp.pX.pVal = (pB.pX.pVal - pA.pX.pVal) * pRatio;
	lTemp.pY.pVal = (pB.pY.pVal - pA.pY.pVal) * pRatio;
	lTemp.pZ.pVal = (pB.pZ.pVal - pA.pZ.pVal) * pRatio;

	// Fill out the vertex part
	lTemp.pX.pValueType = pA.pX.pValueType;
	lTemp.pY.pValueType = pA.pY.pValueType;
	lTemp.pZ.pValueType = pA.pZ.pValueType;

	// FIll out the vertex part data
	lTemp.pX.pValueIndex = X;
	lTemp.pY.pValueIndex = Y;
	lTemp.pZ.pValueIndex = Z;

	// Return the newly interpolate vertex
	return lTemp;
}

void Init_and_Inter::TweeningAnimation(Timer pTimer)
{
	// Make the timer and map it to the number of keyframes of a joint
	static Timer lTimer;
	lTimer.frameCount++;
	if (lTimer.GetTime() > gMageSkeleton->pJoints[0].pKeyframes.size())
	{
		lTimer.frameCount = 0;
		lTimer.StartTimer();
	}
	lTimer.frameTime = lTimer.GetFrameTime();

	// Start by getting d (ratio)
	float lD = pTimer.GetTime();

	// Get the time (t)
	float lT = lTimer.GetTime();

	// Ints to hold the keyframe values
	int lBelow_KeyTime = 0;
	int lAbove_KeyTime = 0;
	
	// Before we loop the joints clear the debug renders
	mage_bone_debugRenderer.clear_counter();
	mage_joint_debugRenderer.clear_counter();

	// Loop through each joint
	for (unsigned int i = 0; i < gMageSkeleton->pJoints.size(); i++)
	{
		// Loop through the the joints keyframes
		for (unsigned int j = 0; j < gMageSkeleton->pJoints[i].pAll_KeyTimes.size(); j++)
		{
			// Check to see if j is equal to the last index
			// If so lerp it between the last and first index
			if (j + 1 == gMageSkeleton->pJoints[i].pAll_KeyTimes.size())
			{
				lBelow_KeyTime = gMageSkeleton->pJoints[i].pAll_KeyTimes[j] - 1;
				lAbove_KeyTime = gMageSkeleton->pJoints[i].pAll_KeyTimes[0];
			}
			// Check to see if the time is less then j and j + 1
			else if (gMageSkeleton->pJoints[i].pAll_KeyTimes[j] > lT && lT < gMageSkeleton->pJoints[i].pAll_KeyTimes[j + 1])
			{
				lBelow_KeyTime = gMageSkeleton->pJoints[i].pAll_KeyTimes[j] - 1;
				lAbove_KeyTime = gMageSkeleton->pJoints[i].pAll_KeyTimes[j + 1] - 1;
			}
		}

		Keyframe_Info lBelow_Keyframe = gMageSkeleton->pJoints[i].pKeyframes[lBelow_KeyTime];
		Keyframe_Info lAbove_Keyframe = gMageSkeleton->pJoints[i].pKeyframes[lAbove_KeyTime];
		Keyframe_Info lLerped_Keyframe;

		lLerped_Keyframe.pTranslation = VertexInterpolation(lBelow_Keyframe.pTranslation, lAbove_Keyframe.pTranslation, lD);
		lLerped_Keyframe.pRotation = VertexInterpolation(lBelow_Keyframe.pRotation, lAbove_Keyframe.pRotation, lD);
		lLerped_Keyframe.pScale = VertexInterpolation(lBelow_Keyframe.pScale, lAbove_Keyframe.pScale, lD);

		XMFLOAT3 lLerp_Translation = XMFLOAT3(lLerped_Keyframe.pTranslation.pX.pVal, lLerped_Keyframe.pTranslation.pY.pVal, lLerped_Keyframe.pTranslation.pZ.pVal);
		XMFLOAT3 lAbove_Translation = XMFLOAT3(lAbove_Keyframe.pTranslation.pX.pVal, lAbove_Keyframe.pTranslation.pY.pVal, lAbove_Keyframe.pTranslation.pZ.pVal);
		mage_bone_debugRenderer.add_debug_line(lLerp_Translation, lAbove_Translation, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// Get the X, Y, Z of lerped translation
		float lXVal = lLerped_Keyframe.pTranslation.pX.pVal;
		float lYVal = lLerped_Keyframe.pTranslation.pY.pVal;
		float lZVal = lLerped_Keyframe.pTranslation.pZ.pVal;

		// Get the Position, X,Y,Z Axis
		XMFLOAT3 lPosition = XMFLOAT3(lXVal, lYVal, lZVal);
		XMFLOAT3 lXAxis = XMFLOAT3(gMageSkeleton->pJoints[i].pXAxis.x, gMageSkeleton->pJoints[i].pXAxis.y, gMageSkeleton->pJoints[i].pXAxis.z);
		XMFLOAT3 lYAxis = XMFLOAT3(gMageSkeleton->pJoints[i].pYAxis.x, gMageSkeleton->pJoints[i].pYAxis.y, gMageSkeleton->pJoints[i].pYAxis.z);
		XMFLOAT3 lZAxis = XMFLOAT3(gMageSkeleton->pJoints[i].pZAxis.x, gMageSkeleton->pJoints[i].pZAxis.y, gMageSkeleton->pJoints[i].pZAxis.z);

		// Add lines to the debug renderer
		// Draw X axis in red
		XMFLOAT3 lPosToXAxis = XMFLOAT3(lPosition.x + lXAxis.x, lPosition.y + lXAxis.y, lPosition.z + lXAxis.z);
		mage_joint_debugRenderer.add_debug_line(lPosition, lPosToXAxis, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
		// Draw Y axis in green
		XMFLOAT3 lPosToYAxis = XMFLOAT3(lPosition.x + lYAxis.x, lPosition.y + lYAxis.y, lPosition.z + lYAxis.z);
		mage_joint_debugRenderer.add_debug_line(lPosition, lPosToYAxis, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
		// Draw Z axis in blue
		XMFLOAT3 lPosToZAxis = XMFLOAT3(lPosition.x + lZAxis.x, lPosition.y + lZAxis.y, lPosition.z + lZAxis.z);
		mage_joint_debugRenderer.add_debug_line(lPosition, lPosToZAxis, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	}

	mage_bone_debugRenderer.push_to_gpu(d3d11DevCon);
	mage_joint_debugRenderer.push_to_gpu(d3d11DevCon);


	// Update the keyframe 
	UpdateKeyframe(lBelow_KeyTime);
	UpdateKeyframe(lAbove_KeyTime);
}


void Debug_Renderer::add_debug_line(XMFLOAT3 a, XMFLOAT3 b, XMFLOAT4 color)
{
	debugRendererCPU[counter].pos = a;
	debugRendererCPU[counter].color = color;
	counter++;

	debugRendererCPU[counter].pos = b;
	debugRendererCPU[counter].color = color;

	counter++;
}

void Debug_Renderer::push_to_gpu(ID3D11DeviceContext* d3d11DevCon)
{
	int vertices = counter;
	int size = sizeof(Vertex) * vertices;

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = size;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));

	vertexBufferData.pSysMem = debugRendererCPU;

	this->d3d11DevCon = d3d11DevCon;
	ID3D11Device* dev;
	d3d11DevCon->GetDevice(&dev);
	hr = dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &debugRendererGPU);
}

void Debug_Renderer::clear_counter()
{
	counter = 0;
}

void Debug_Renderer::RenderLines()
{
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &debugRendererGPU, &stride, &offset);

	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	d3d11DevCon->Draw(counter, 0);
}

void Debug_Renderer::ChangeColor(XMFLOAT4 color)
{
	for (int i = 0; i < counter; i++)
		debugRendererCPU[i].color = color;
}

void User_Input::DetectInput(double time, Init_and_Inter& _initializer, HWND hWnd)
{
	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;
	moveUpDown = 0.0f;

	float speed = 5.0f  * time;

	// Get Async Key States
	SHORT lWKeyState = GetAsyncKeyState(0x57);		  // W Key
	SHORT lAKeyState = GetAsyncKeyState(0x41);		  // A Key
	SHORT lSKeyState = GetAsyncKeyState(0x53);		  // S Key
	SHORT lDKeyState = GetAsyncKeyState(0x44);		  // D Key
	SHORT lEscKeyState = GetAsyncKeyState(VK_ESCAPE);	// ESC Key
	SHORT lLeftCtrlKeyState = GetAsyncKeyState(VK_LCONTROL); // Left Control Key
	SHORT lSpaceKeyState = GetAsyncKeyState(VK_SPACE);// Space Key
	SHORT lRightClickKeyState = GetAsyncKeyState(VK_RBUTTON); // Right Click
	SHORT lLeftArrowKeyState = GetAsyncKeyState(VK_LEFT); // Left Arrow
	SHORT lRightArrowKeyState = GetAsyncKeyState(VK_RIGHT); // Right Arrow
	SHORT lF1KeyState = GetAsyncKeyState(VK_F1);	  // F1 Key
	SHORT lF2KeyState = GetAsyncKeyState(VK_F2);	  // F2 Key
	SHORT lF3KeyState = GetAsyncKeyState(VK_F3);	  // F3 Key

	if ((1 << 15) & lEscKeyState)
		PostMessage(hWnd, WM_CLOSE, 0, 0);

	if ((1 << 15) & lWKeyState)
		moveBackForward += speed;

	if ((1 << 15) & lAKeyState)
		moveLeftRight -= speed;

	if ((1 << 15) & lSKeyState)
		moveBackForward -= speed;

	if ((1 << 15) & lDKeyState)
		moveLeftRight += speed;

	if ((1 << 15) & lSpaceKeyState)
		moveUpDown += speed;

	if ((1 << 15) & lLeftCtrlKeyState)
		moveUpDown -= speed;

	// Get the current cursor position
	POINT lCurrentFrameCursorPos;
	ZeroMemory(&lCurrentFrameCursorPos, sizeof(lCurrentFrameCursorPos));
	int lResult = GetCursorPos(&lCurrentFrameCursorPos);

	int deltaX = lCurrentFrameCursorPos.x - gLastFrameCursorPos.x;
	int deltaY = lCurrentFrameCursorPos.y - gLastFrameCursorPos.y;

	if ((1 << 15) & lRightClickKeyState)
	{
		if (lResult)
		{
			if ((lCurrentFrameCursorPos.x != gLastFrameCursorPos.x) || (lCurrentFrameCursorPos.y != gLastFrameCursorPos.y))
			{
				camYaw += deltaX * speed;

				camPitch += deltaY * speed;

				gLastFrameCursorPos = lCurrentFrameCursorPos;
			}
		}
	}

	// Variable for debounce purposes
	static bool canUpdateIndex = true;

	// Add message boxes for each keypress for debug purposes
	if ((1 << 15) & lLeftArrowKeyState && gKeyframeIndex != 0 && canUpdateIndex)
	{
		gKeyframeIndex--;

		std::cout << "KeyFrameIndex: " << gKeyframeIndex << " \n";
	}

	if ((1 << 15) & lRightArrowKeyState && canUpdateIndex && !gKeyframeOutOfRange)
	{
		gKeyframeIndex++;

		std::cout << "KeyFrameIndex: " << gKeyframeIndex << " \n";
	}

	if ((1 << 15) & lRightArrowKeyState || (1 << 15) & lLeftArrowKeyState)
		canUpdateIndex = false;
	else
		canUpdateIndex = true;

	if ((1 << 15) & lF1KeyState && !gAnimationPlaying)
	{
		gAnimationScrubbing = false;
		gAnimationTweening = false;
		gAnimationPlaying = true;

		std::cout << "Animation Playing: " << gAnimationPlaying << " \n";
		std::cout << "Animation Scrubbing: " << gAnimationScrubbing << " \n";
		std::cout << "Animation Tweening: " << gAnimationTweening << " \n";

	}

	if ((1 << 15) & lF2KeyState && !gAnimationTweening)
	{
		gAnimationScrubbing = false;
		gAnimationTweening = true;
		gAnimationPlaying = false;

		std::cout << "Animation Tweening: " << gAnimationTweening << " \n";
		std::cout << "Animation Scrubbing: " << gAnimationScrubbing << " \n";
		std::cout << "Animation Playing: " << gAnimationPlaying << " \n";
	}

	if ((1 << 15) & lF3KeyState && !gAnimationScrubbing)
	{
		gAnimationScrubbing = true;
		gAnimationTweening = false;
		gAnimationPlaying = false;

		std::cout << "Animation Scrubbing: " << gAnimationScrubbing << " \n";
		std::cout << "Animation Playing: " << gAnimationPlaying << " \n";
		std::cout << "Animation Tweening: " << gAnimationTweening << " \n";
	}

	if (gAnimationPlaying)
	{
		gKeyframeIndex++;

		// Restart the animation
		if (gKeyframeOutOfRange)
			gKeyframeIndex = 0;

		// std::cout << "KeyFrameIndex: " << gKeyframeIndex << " \n";
	}

	//mouseLastState = mouseCurrState;
	gLastFrameCursorPos = lCurrentFrameCursorPos;

	_initializer.UpdateCamera();

	return;
}

void Timer::StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	countsPerSecond = double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	CounterStart = frequencyCount.QuadPart;
}

double Timer::GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart - CounterStart) / countsPerSecond;
}

double Timer::GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart - frameTimeOld;
	frameTimeOld = currentTime.QuadPart;

	if (tickCount < 0.0f)
		tickCount = 0.0f;

	return float(tickCount) / countsPerSecond;
}