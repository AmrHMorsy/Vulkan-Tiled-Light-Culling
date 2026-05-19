#version 450
#extension GL_ARB_shader_draw_parameters : enable

layout(location = 0) in vec3 _vertex;
layout(location = 1) in vec3 _normal;
layout(location = 2) in vec2 _textureCoordinates;

layout(location = 0) out vec3 vertex;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 textureCoordinates;
layout(location = 3) out flat uint drawID;

layout(set = 0, binding = 0) uniform VertexShaderUniformVariables {
    mat4 cameraViewMatrix;
    mat4 cameraProjectionMatrix;
} vs;

void main()
{
    drawID = gl_InstanceIndex;
    
    mat4 modelMatrix = mat4(1.0f);
    
    vertex = (modelMatrix * vec4(_vertex, 1.0f)).xyz;
    normal = normalize(mat3(transpose(inverse(modelMatrix))) * _normal);
    textureCoordinates = _textureCoordinates;
    
    gl_Position = vs.cameraProjectionMatrix * vs.cameraViewMatrix * modelMatrix * vec4(_vertex, 1.0);
}
