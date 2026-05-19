#version 450

layout(location = 0) in vec3 vertex;

layout(binding  = 0) uniform VertexShaderUniformVariables{
    mat4 cameraViewMatrix;
    mat4 cameraProjectionMatrix;
} vs;

void main()
{
    mat4 modelMatrix = mat4(1.0f);
    gl_Position = vs.cameraProjectionMatrix * vs.cameraViewMatrix * modelMatrix *  vec4(vertex, 1.0);
}
