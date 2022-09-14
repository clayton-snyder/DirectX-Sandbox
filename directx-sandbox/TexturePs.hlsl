Texture2D texture2d;
SamplerState sampleType;

struct PixelInput
{
    float4 position : SV_Position;
    float2 textureCoord : TEXCOORD0;
};

float4 TexturePixelShader(PixelInput input) : SV_Target
{
    // Use the SamplerState type and position in the texture to decide which pixel in the raw
    // texture resource to use. (I.e., what color to draw)
    float2 override = float2(1.0f, 1.0f);
    return texture2d.Sample(sampleType, input.textureCoord);
}