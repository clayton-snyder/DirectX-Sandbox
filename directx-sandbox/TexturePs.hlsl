Texture2D texture2d;
SamplerState sampleType;

struct PixelInputType
{
    float4 position : SV_Position;
    float2 textureCoord : TEXCOORD0;
};

float TexturePixelShader(PixelInputType input) : SV_Target
{
    // Use the SamplerState type and position in the texture to decide which pixel in the raw
    // texture resource to use. (I.e., what color to draw)
    return texture2d.Sample(sampleType, input.textureCoord);
}