cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInput
{
    float4 position : Position;
    float2 textureCoord : TEXCOORD0;
};

struct PixelInput
{
    float4 position : SV_Position;
    float2 textureCoord : TEXCOORD0;
};

PixelInput TextureVertexShader(VertexInput input)
{
    PixelInput output;
    
    input.position.w = 1.0f; // Still don't understand why we do this
    
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.textureCoord = input.textureCoord;
    
    return output;
}