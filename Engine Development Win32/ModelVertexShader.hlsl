cbuffer cbPerObject
{
    float4x4 WVP;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Color : COLOR;
    float3 Nomral : NORMAL;
};

VS_OUTPUT main(float4 inPos : POSITION, float2 inColor : COLOR, float3 inNormal : NORMAL)
{
    VS_OUTPUT output;

    output.Pos = mul(float4(inPos.xyz, 1.0f), WVP);
    output.Color = inColor;
    output.Nomral = inNormal;

    return output;
}