#version 450

struct Vertex{
    float pos[3];
    float normal[3];
    float uv[2];
};

layout(location = 0) out vec4 color;

layout(set = 0, binding = 0) readonly buffer VerticesBuffer{
    Vertex Vertices[];
};

layout(set = 0, binding = 1) readonly buffer IndexBuffer{
    uint Indices[];
};

layout(set = 0, binding = 2) uniform UniformBuffer{
    float Time;
};

void main(){
    mat3 rotMat = mat3(cos(Time),  0.0f,  sin(Time),
                       0.0f,       1.0f,  0.0f,
                       -sin(Time), 0.0f,  cos(Time));

    Vertex vert = Vertices[Indices[gl_VertexIndex]];

    vec3 pos = vec3(vert.pos[0], vert.pos[1], vert.pos[2]);
    pos = pos * rotMat;
    pos.z = 0.5f;
    pos.y = 0.5f - pos.y;
    gl_Position = vec4(pos, 1.0f);

    vec3 normal = vec3(vert.normal[0], vert.normal[1], vert.normal[2]);
    color = vec4((normal * 0.5f) + 0.5f,  1.0f);
}