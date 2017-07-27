#pragma once

// Includes
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <dinput.h>
#include <iostream>

// Headers
#include "Structures.h"

// My library's
#include "../FBX_DLL/FBX_Executable.h"
#include "../FBX_DLL/FBX_Helper_Structures.h"

// Other Library's
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// Forward declaring classes
class Init_and_Inter;

using namespace DirectX;

class Debug_Renderer
{
private:
	Vertex debugRendererCPU[1024];
	ID3D11Buffer* debugRendererGPU;
	ID3D11DeviceContext* d3d11DevCon;
	int counter = 0;

	HRESULT hr;

public:

	void add_debug_line(XMFLOAT3 a, XMFLOAT3 b, XMFLOAT4 color);
	void push_to_gpu(ID3D11DeviceContext* d3d11DevCon);
	void clear_counter();
	void RenderLines();
	void ChangeColor(XMFLOAT4 color);
};

class User_Input
{
private:

	HRESULT hr;

public:

	float rotx = 0;
	float rotz = 0;
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	XMMATRIX Rotationx;
	XMMATRIX Rotationz;

	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;
	float moveUpDown = 0.0f;

	float camYaw = 0.0f;
	float camPitch = 0.0f;

	// Variables for animation keypresses
	bool gAnimationScrubbing = true;
	bool gAnimationPlaying = false;
	bool gAnimationTweening = false;
	bool gKeyframeOutOfRange = false;

	// Stuff for tweening
	int gBelowKeyframe = 0;
	int gAboveKeyframes = 0;

	// Variables for camera movement (none direct input way)
	POINT gLastFrameCursorPos;

	float m_viewSmoothing = 0.5f;
	float m_viewSensitivity = 0.65f;

	// Variables for indexing
	std::vector<int> gAvaiable_Keytimes;
	unsigned int gKeyTime_Index = 0;
	int gKeyframeIndex = 0;

	bool looping = false;

	void DetectInput(double time, Init_and_Inter& _initializer, HWND hWnd);
};

class Timer
{
private:
	double countsPerSecond = 0.0f;
	__int64 CounterStart = 0;

	__int64 frameTimeOld = 0;

public:

	int frameCount = 0;
	int fps = 0;
	double frameTime;
	
	void StartTimer();
	double GetTime();
	double GetFrameTime();
};

class Init_and_Inter
{
private:
	IDXGISwapChain* SwapChain;
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DevCon;
	D3D11_VIEWPORT viewport;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* depthStencilBuffer;

	ID3D11Buffer* cbPerObjectBuffer;

	ID3D11RasterizerState* WireFrame; 

	XMMATRIX WVP;
	XMMATRIX cube1World;
	XMMATRIX cube2World;
	XMMATRIX mageWorld;
	XMMATRIX teddyWorld;
	XMMATRIX camView;
	XMMATRIX camProjection;

	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;

	XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMMATRIX camRotationMatrix;
	XMMATRIX groundWorld;

	XMMATRIX Rotation;
	XMMATRIX Scale;
	XMMATRIX Translation;
	float rot = 0.01f;

	HRESULT hr;

	float red = 0.0f;
	float green = 0.0f;
	float blue = 0.0f;
	int colormodr = 1;
	int colormodg = 1;
	int colormodb = 1;

	UINT Width = 0;
	UINT Height = 0;

	// Square Object
	Square _square;

	// Constant Buffer for every object
	cbPerObject cbPerObj;

	// Debug Renderer object
	Debug_Renderer mage_joint_debugRenderer = Debug_Renderer();
	Debug_Renderer mage_bone_debugRenderer = Debug_Renderer();
	Debug_Renderer teddy_joint_debugRenderer = Debug_Renderer();
	Debug_Renderer teddy_bone_debugRenderer = Debug_Renderer();

	// User_Input object
	User_Input input;

	// FBX_Executable instance
	FBX_EXECUTABLE fbx_Exec;

	// Skeleton Globals
	Skeleton* gMageSkeleton;
	Skeleton* gTeddySkeleton;

	// Model Globals
	Model gMageModel;
	Model gTeddyModel;

	// CB For Animated mesh (mage)
	cbAnimatedMesh gAnimated_Mesh_CB;

	// Function For Scrubbing
	void UpdateKeyframe(int pKeyframeIndex);
	void TweeningAnimation(float pTime);
	void TweeningAnimation(Timer pTimer);
public:

	bool InitializeDirect3d11App(HINSTANCE hInstance, HWND hWnd);
	void ReleaseObjects();
	bool InitScene(User_Input &_input);
	void UpdateScene(double time, User_Input &_input, Timer pTimer);
	void DrawScene();
	void UpdateCamera();
};

