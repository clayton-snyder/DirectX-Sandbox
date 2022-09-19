Texture2D texture2d;
SamplerState sampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
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
    
    float diffuseIntensity = saturate(dot(psInput.normal, -direction));
    // This saturate only really protects against negative diffuseColor values negating the
    // ambient light; i.e., negative diffuseColor values is basically just no diffuse light.
    float4 diffuseTotal = saturate(diffuseColor * diffuseIntensity);
    
    float4 finalColor = textureColor * (diffuseTotal + ambientColor);
    
    return finalColor;
}