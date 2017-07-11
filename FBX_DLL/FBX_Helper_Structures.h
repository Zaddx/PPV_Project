#pragma once

#include <DirectXMath.h>
#include <vector>

struct Joint
{
	DirectX::XMMATRIX pMatrix;
	DirectX::XMFLOAT4 pXAxis;		// | XX | XY | XZ | XW | X Axis of the joint in 4x4
	DirectX::XMFLOAT4 pYAxis;		// | YX | YY | YZ | YW | Y Axis of the joint in 4x4
	DirectX::XMFLOAT4 pZAxis;		// | ZX | ZY | ZZ | ZW | Z Axis of the joint in 4x4
	DirectX::XMFLOAT4 pPosition;	// | pX | pY | pZ | pW | position of the joint in 4x4

	int pParentIndex;
	std::string pName;
};

// Dont really need bone structure since 
// It's just the position of a joint to it's parents position
struct Bone
{
	Joint pStart, pEnd; // Between joint and it's parent
};

struct Mesh_Vertex
{
	DirectX::XMFLOAT4 pPosition;
	DirectX::XMFLOAT2 pUV;
	DirectX::XMFLOAT3 pNormal;
};

struct Mesh_Polygon
{
	std::vector<Mesh_Vertex> pVertices;
};

struct Mesh
{
	std::string pName;
	std::string pTextureName;
	std::vector<Mesh_Polygon> pPolygon;
	std::vector<Mesh_Vertex> pVertices;
};

struct Skeleton
{
	std::vector<Joint> pJoints;
	std::vector<Bone> pBones;	// Optional
	std::string pName;

	Mesh pMesh;
};
