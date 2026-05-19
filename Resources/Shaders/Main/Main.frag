#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_shader_draw_parameters : enable

const int MAX_NUM_LIGHTS = 1000;
const int MAX_NUM_MODELS = 50;

layout(early_fragment_tests) in;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textureCoordinates;
layout(location = 3) in flat uint drawID;

layout(location = 0) out vec4 fragment;

layout(set = 0, binding =  1) uniform FragmentShaderUniformVariables{
    uint numLights;
    float nearClippingPlane;
    float farClippingPlane;
    float lightCullingEnabled;
    uvec4 heatMap_numCellsPerTile_albedo_normal;
    vec2 screenResolution;
    uvec2 numTiles2D;
    vec4 cameraPosition_isDepth;
    vec4 lightPositions[MAX_NUM_LIGHTS];
    vec4 lightColors[MAX_NUM_LIGHTS];
} fs;

layout(set = 0, binding = 2) readonly buffer TileLightCullingPrepass{
    vec2 tileLightCullingPrepass[];
};
layout(set = 0, binding = 3) readonly buffer TileLightCount {
    uint tileLightCount[];
};
layout(set = 0, binding = 4) readonly buffer TileLightIndices {
    uint tileLightIndices[];
};
layout(set = 0, binding = 5) readonly buffer MaxLightCountBuffer{
    uint globalMaxLightCount;
};

layout(set = 0, binding = 6) uniform sampler2D albedo[MAX_NUM_MODELS];

const float PI = 3.14159265359;
const uvec2 tileResolution = uvec2(16, 16);

float LinearizeDepth(float depth)
{
    float nearClippingPlane = fs.nearClippingPlane;
    float farClippingPlane = fs.farClippingPlane;
    float z = depth * 2.0 - 1.0;
    
    return (2.0 * nearClippingPlane * farClippingPlane) / (farClippingPlane + nearClippingPlane - z * (farClippingPlane - nearClippingPlane));
}

/*float LinearizeDepth(float depth)
{
    return (fs.nearClippingPlane * fs.farClippingPlane) / (fs.farClippingPlane - depth * (fs.farClippingPlane - fs.nearClippingPlane));
}
*/

uint ComputeCellIndex(float depth, float minDepth, float maxDepth)
{
    float depth01 = log(depth / minDepth) / log(maxDepth / minDepth);
    uint cellIndex = uint(depth01 * float(fs.heatMap_numCellsPerTile_albedo_normal.g));
    
    return max(min(cellIndex, fs.heatMap_numCellsPerTile_albedo_normal.g - 1u), 0);
}

vec3 ComputeRadiance(vec3 normal, vec3 albedo, uint startingIndex, uint endingIndex)
{
    vec3 out_radiance = vec3(0.0);
    for(uint i = startingIndex; i < endingIndex; i++){
        uint lightIndex = tileLightIndices[i];
        vec3 light = normalize(fs.lightPositions[lightIndex].rgb - vertex);
        float dist = length(fs.lightPositions[lightIndex].rgb - vertex);
        float attenuation = 1.0 / (dist * dist);
        vec3 inRadiance = fs.lightColors[lightIndex].rgb * attenuation;
        float cos_theta = max(dot(normal, light), 0.0);
        out_radiance += albedo * inRadiance * cos_theta ;
    }

    return out_radiance;
}

vec3 ComputeRadiance(vec3 normal, vec3 albedo)
{
    vec3 out_radiance = vec3(0.0);
    for(uint i = 0; i < fs.numLights; i++){
        vec3 light = normalize(fs.lightPositions[i].rgb - vertex);
        float dist = length(fs.lightPositions[i].rgb - vertex);
        float attenuation = 1.0 / (dist * dist);
        vec3 inRadiance = fs.lightColors[i].rgb * attenuation;
        float cos_theta = max(dot(normal, light), 0.0);
        out_radiance += albedo * inRadiance * cos_theta ;
    }

    return out_radiance;
}

vec3 DisplayHeatMap(uint lightCount)
{
    float lightConcentration = (log(1+lightCount)) / (log(1+globalMaxLightCount));

    vec3 heat;
    if(lightConcentration < 0.33)
        heat = mix(vec3(0,0,1), vec3(0,1,0), lightConcentration / 0.33);
    else if (lightConcentration < 0.66)
        heat = mix(vec3(0,1,0), vec3(1,1,0), (lightConcentration - 0.33) / 0.33);
    else
        heat = mix(vec3(1,1,0), vec3(1,0,0), (lightConcentration - 0.66) / 0.33);
     
    return heat;
}

/*vec3 DisplayHeatMap(uint lightCount)
{
    float t = (log(1+lightCount)) / (log(1+globalMaxLightCount));
    
    vec3 heat;
    if(t < 0.5)
        heat = mix(vec3(0,0,1), vec3(0,1,0), t * 2.0);
    else
        heat = mix(vec3(0,1,0), vec3(1,0,0), (t - 0.5) * 2.0);
    
    return heat;
}
*/

void main()
{
    if(fs.cameraPosition_isDepth.a == 1){
        fragment = vec4(vec3(pow(gl_FragCoord.z, 30)), 1.0f);
        return;
    }
    if(fs.heatMap_numCellsPerTile_albedo_normal.a == 1){
        fragment = vec4(normal, 1.0f);
        return;
    }
    vec3 albedo = texture(albedo[drawID], textureCoordinates).rgb;
    if(fs.heatMap_numCellsPerTile_albedo_normal.b == 1){
        fragment = vec4(albedo, 1.0f);
        return;
    }
    vec3 view = normalize(fs.cameraPosition_isDepth.rgb - vertex);
    uvec2 tileUV = uvec2(gl_FragCoord.xy) / tileResolution;
    uint tileIndex = (tileUV.y * fs.numTiles2D.x) + tileUV.x;
    float depth = LinearizeDepth(gl_FragCoord.z);
    float minDepth = tileLightCullingPrepass[tileIndex].x;
    float maxDepth = tileLightCullingPrepass[tileIndex].y;
    uint tileCellIndex = ComputeCellIndex(depth, minDepth, maxDepth);
    uint lightCount = tileLightCount[(tileIndex * fs.heatMap_numCellsPerTile_albedo_normal.g) + tileCellIndex];
    uint startingIndex = (tileIndex * fs.numLights * fs.heatMap_numCellsPerTile_albedo_normal.g) + (tileCellIndex * fs.numLights);
    uint endingIndex = startingIndex + lightCount;
    
    vec3 color;
    if(fs.heatMap_numCellsPerTile_albedo_normal.r == 0){
        if(fs.lightCullingEnabled == 1)
            color = ComputeRadiance(normal, albedo, startingIndex, endingIndex);
        else
            color = ComputeRadiance(normal, albedo);
    }
    else
        color = DisplayHeatMap(lightCount);
    
    color = pow(color, vec3(1.0/2.2));
    color = color / (color + vec3(1.0));
    
    fragment = vec4(color, 1.0f);
}
