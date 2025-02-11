cbuffer cbPerObject
{
    float4x4 WVP;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

VS_OUTPUT main(float3 inPos : POSITION, float4 inColor : COLOR)
{
    VS_OUTPUT output;

    output.Pos = mul(float4(inPos, 1.0f), WVP);
    output.Color = inColor;

    return output;
}