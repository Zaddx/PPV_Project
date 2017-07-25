#pragma once

#include <DirectXMath.h>
#include <vector>

// Create an ENUM for filling out Keyframe_Info
enum Vertex_Part { TRANSLATION = 0, ROTATION = 1, SCALE = 2 };
enum Vertex_Part_Data { X = 0, Y = 1, Z = 2 };

struct BlendingIndexWeightPair
{
	unsigned int pBlendingIndex;
	unsigned int pBlendingWeight;
};

struct CtrlPoint
{
	DirectX::XMFLOAT3 pPosition;
	std::vector<BlendingIndexWeightPair> pBlendingInfo;
};

// Holds the vertex of the Keyframe
struct Keyframe_Vertex
{
	int pKeytime;		// To hold the keytime

	Vertex_Part pValueType;				// 0 = Trans, 1 = Rot, 2 = Scale
	Vertex_Part_Data pValueIndex;		// 0 = X, 1 = Y, 2 = Z

	DirectX::XMMATRIX pMatrix;
	DirectX::XMFLOAT4 pPosition;
	DirectX::XMFLOAT4 pXAxis;
	DirectX::XMFLOAT4 pYAxis;
	DirectX::XMFLOAT4 pZAxis;
};

struct Keyframe_Vertex_Info_Vector
{
	std::vector<Keyframe_Vertex> pVertices;
};


struct Keyframe_Info
{
	Keyframe_Vertex pTranslation;		// Translation of keyframe
	Keyframe_Vertex pRotation;			// Rotation of keyframe
	Keyframe_Vertex pScale;				// Scale of keyframe
};

struct Joint
{
	DirectX::XMMATRIX pMatrix;
	DirectX::XMFLOAT4 pXAxis;		// | XX | XY | XZ | XW | X Axis of the joint in 4x4
	DirectX::XMFLOAT4 pYAxis;		// | YX | YY | YZ | YW | Y Axis of the joint in 4x4
	DirectX::XMFLOAT4 pZAxis;		// | ZX | ZY | ZZ | ZW | Z Axis of the joint in 4x4
	DirectX::XMFLOAT4 pPosition;	// | pX | pY | pZ | pW | position of the joint in 4x4

	int pParentIndex;
	std::string pName;

	// Stuff for animation
	std::vector<Keyframe_Info> pKeyframes;		// Each joint has it's own keyframes
	Keyframe_Vertex_Info_Vector pVertex_Info_Vector;	// Holds the vertices infos
	std::vector<int> pTranslation_KeyTimes;				// Vector of Translation keytimes
	std::vector<int> pRotation_KeyTimes;				// Vector of Rotation keytimes
	std::vector<int> pScale_KeyTimes;					// Vector of Scale keytimes
	std::vector<int> pAll_KeyTimes;						// Vector of all keytimes

	// Stuff for function use
	std::vector<Keyframe_Vertex> pTranslation_Vertices;
	std::vector<Keyframe_Vertex> pRotation_Vertices;
	std::vector<Keyframe_Vertex> pScale_Vertices;
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
	std::string pAnimationName;
	double pAnimationLength;
	std::vector<CtrlPoint*> pControlPoints;

	Mesh pMesh;
};
