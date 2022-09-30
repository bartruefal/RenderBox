#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;

layout(location = 0) out vec4 color;

layout(set = 0, binding = 0) uniform UniformData{
    float time;
} Data;

void main(){
    mat3 rotMat = mat3(cos(Data.time),  0.0f,  sin(Data.time),
                       0.0f,            1.0f,  0.0f,
                       -sin(Data.time), 0.0f,  cos(Data.time));

    vec3 pos = position * rotMat;
    pos.z = 0.5f;
    pos.y = 0.5f - pos.y;
    gl_Position = vec4(pos, 1.0f);

    color = vec4((normal * 0.5f) + 0.5f,  1.0f);
}