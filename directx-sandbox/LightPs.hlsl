Texture2D texture2d;
SamplerState sampleType;

cbuffer LightBuffer
{
    float4 diffuseColor;
    float3 direction;
    float padding; // just to add four bytes for 16-byte memory alignment
};

struct PixelInput
{
    float4 position : SV_Position;
    float2 textureCoord : TEXCOORD0;
    float3 normal : NORMAL;
};

float4 LightPixelShader(PixelInput psInput) : SV_TARGET
{
    float4 textureColor = texture2d.Sample(sampleType, psInput.textureCoord);
    float intensity = saturate(dot(psInput.normal, -direction));
    
    return textureColor * saturate(diffuseColor * intensity);
}