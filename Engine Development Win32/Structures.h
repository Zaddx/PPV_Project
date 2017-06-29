#pragma once

#include <d3d11.h>

struct pipeline_state_t
{
	ID3D11InputLayout *input_layout;
	ID3D11VertexShader *vertex_shader;
	ID3D11PixelShader * pixel_shader;
	ID3D11RenderTargetView *render_target;
	ID3D11Texture2D *depthStencilBuffer;
	ID3D11DepthStencilState *depthStencilState;
	ID3D11DepthStencilView *depthStencilView;
	ID3D11RasterizerState *rasterState;
}default_pipeline;

struct Vertex	// Overloaded Vertex Structure
{
	Vertex(){}
	Vertex(float x, float y, float z,
		   float cr, float cg, float cb, float ca) 
		   : pos(x, y, z), color(cr, cg, cb, ca) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

struct Triangle
{
	ID3D11Buffer* triangleVertexBuffer;
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D10Blob* VS_Buffer;
	ID3D10Blob* PS_Buffer;
	ID3D11InputLayout* vertLayout;
};

struct Square
{
	ID3D11Buffer* squareVertexBuffer;
	ID3D11Buffer* squareIndexBuffer;
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D10Blob* VS_Buffer;
	ID3D10Blob* PS_Buffer;
	ID3D11InputLayout* vertLayout;
};

struct cbPerObject
{
	DirectX::XMMATRIX  WVP;
};