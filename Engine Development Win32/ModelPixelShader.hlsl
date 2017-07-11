struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Color : COLOR;
    float3 Nomral : NORMAL;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return float4(input.Color, 0.0f, 0.0f);
}