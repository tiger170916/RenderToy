#define ONE_OVER_4PI            0.079577f

// Frame-wise constants
struct UniformFrameConstants
{
    float4x4 View;
    float4x4 InvView;
    float4x4 Projection;
    float4x4 InvProjectionMatrix;
    float4x4 ViewProjectionMatrix;
    float4x4 InvViewProjectionMatrix;
    
    float4   CameraPosition;
    float4   ForwardVector;
    
    float    PixelStepScale;
    float3   Pad0;
};