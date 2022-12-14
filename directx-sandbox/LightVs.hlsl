cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
    float padding;
};

struct VertexInput
{
    float4 position : Position;
    float2 textureCoord : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInput
{
    float4 position : SV_Position;
    float2 textureCoord : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDir : TEXCOORD1;
};

PixelInput LightVertexShader(VertexInput vertexInput)
{
    // All we do here is same old matrix translation but including the normal this time then 
    // pass to the Pixel shader which will apply the light(s?)'s effect to the pixel
    vertexInput.position.w = 1.0f;
    
    PixelInput psInput;
    psInput.position = mul(vertexInput.position, worldMatrix);
    psInput.position = mul(psInput.position, viewMatrix);
    psInput.position = mul(psInput.position, projectionMatrix);
    psInput.textureCoord = vertexInput.textureCoord;
    psInput.normal = normalize(mul(vertexInput.normal, (float3x3) worldMatrix));
    
    float4 vertexWorldPos = mul(vertexInput.position, worldMatrix);
    psInput.viewDir = normalize(cameraPosition - vertexWorldPos.xyz);
    
    return psInput;
}