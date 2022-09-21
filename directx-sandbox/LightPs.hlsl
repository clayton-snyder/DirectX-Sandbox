Texture2D texture2d;
SamplerState sampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 direction;
    float specularExp;
    float4 specularColor;
};

struct PixelInput
{
    float4 position : SV_Position;
    float2 textureCoord : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDir : TEXCOORD1;
};

float4 LightPixelShader(PixelInput psInput) : SV_TARGET
{
    float4 textureColor = texture2d.Sample(sampleType, psInput.textureCoord);
    
    float intensity = saturate(dot(psInput.normal, -direction));
    
    // This saturate only really protects against negative diffuseColor values negating the
    // ambient light; i.e., negative diffuseColor values is basically just no diffuse light.
    float4 diffuseTotal = saturate(diffuseColor * intensity);
    
    float3 reflectionDir = normalize(2 * intensity * psInput.normal + direction);
    float4 specularTotal = pow(saturate(dot(reflectionDir, psInput.viewDir)), specularExp);
    
    return saturate((textureColor * (diffuseTotal + ambientColor)) + specularTotal);
}