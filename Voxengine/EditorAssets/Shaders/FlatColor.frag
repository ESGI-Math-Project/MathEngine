#version 450 core

struct TextureInfo
{
    int Index;
    int TexCoord;
};

struct NormalTextureInfo
{
    int Index;
    int TexCoord;
    float Scale;
};

struct OcclusionTextureInfo
{
    int Index;
    int TexCoord;
    float Strenght;
};

struct MetallicRoughtness
{
    vec4 BaseColorFactor;
    TextureInfo BaseColorTexture;
    float MetallicFactor;
    float RoughtnessFactor;
    TextureInfo MetallicRoughnessTexture;
};

struct MaterialParams
{
    MetallicRoughtness PbrMetallicRoughness;
    NormalTextureInfo NormalTexture;
    OcclusionTextureInfo OcclusionTexture;
    TextureInfo EmissiveTexture;
    vec4 EmissiveFactor;
    int AlphaMode;
    float AlphaCutoff;
    int DoubleSided;
};

#define MAX_LIGHT_COUNT 20
struct Light
{
    vec4 Color;
    vec4 Position;
    vec4 Direction;
    float Range;
    float Intensity;
    int Type; //0 = Directional ; 1 = Point ; 2 = Spot
};

struct LightData
{
    Light lights[MAX_LIGHT_COUNT];
//    Light lights;
    int lightCount;
};

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjectionMatrix;
    vec4 u_CameraPosition;
    vec4 u_CameraDirection;
};

layout(std140, binding = 1) uniform Model
{
    mat4 u_ModelMatrix;
    mat4 u_NormalMatrix;
    int u_EntityId;
};

layout(std140, binding = 2) uniform Lights
{
    LightData lights;
};

layout(std140, binding = 3) uniform MaterialParameters
{
    MaterialParams materialParameters;
};

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_Entity; // -1 = no entity.

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec4 v_Color;
layout(location = 2) in flat int v_EntityId;

void main() {
    o_Color = v_Color;
    o_Entity = v_EntityId;
}
