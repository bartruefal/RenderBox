#version 450

const vec3 vertices[] = {
    vec3(0.5f, 0.5f, 0.0f),
    vec3(0.0f, -0.5f, 0.0f),
    vec3(-0.5f, 0.5f, 0.0f)
};

layout(set = 0, binding = 0) uniform UniformData{
    float time;
} Data;

void main(){
    mat2 rotMat = mat2(cos(Data.time), sin(Data.time), -sin(Data.time), cos(Data.time));
    vec2 pos = vertices[gl_VertexIndex].xy * rotMat;
    gl_Position = vec4(pos, 0.0f, 1.0f);
}