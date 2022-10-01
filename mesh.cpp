#include <vector>
#include <fast_obj.h>
#include <meshoptimizer.h>

struct Vertex{
    float position[3];
    float normal[3];
    float uv[2];
};

struct Mesh{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

Mesh loadObjMesh(const char* objFile) {
    fastObjMesh* objMesh{ fast_obj_read(objFile) };
    assert(objMesh);

    uint32_t indexCount{};
    for (int i{}; i < objMesh->face_count; i++){
        indexCount += 3 * (objMesh->face_vertices[i] - 2);
    }

    uint32_t vertexOffset{}, indexOffset{};
    std::vector<Vertex> vertices(indexCount);
    for (int i{}; i < objMesh->face_count; i++){
        for (int j{}; j < objMesh->face_vertices[i]; j++){
            // Triangulate: connect current vertex with the first vertex of the face
            if (j >= 3){
                vertices[vertexOffset + 0] = vertices[vertexOffset - 3];
                vertices[vertexOffset + 1] = vertices[vertexOffset - 1];
                vertexOffset += 2;
            }

            fastObjIndex objVertID{ objMesh->indices[indexOffset + j] };

            Vertex& vertex{vertices[vertexOffset++]};
            vertex.position[0] = objMesh->positions[objVertID.p * 3 + 0];
            vertex.position[1] = objMesh->positions[objVertID.p * 3 + 1];
            vertex.position[2] = objMesh->positions[objVertID.p * 3 + 2];

            vertex.normal[0] = objMesh->normals[objVertID.n * 3 + 0];
            vertex.normal[1] = objMesh->normals[objVertID.n * 3 + 1];
            vertex.normal[2] = objMesh->normals[objVertID.n * 3 + 2];

            vertex.uv[0] = objMesh->texcoords[objVertID.t * 2 + 0];
            vertex.uv[1] = objMesh->texcoords[objVertID.t * 2 + 1];
        }

        indexOffset += objMesh->face_vertices[i];
    }

    fast_obj_destroy(objMesh);

    Mesh mesh{};

    std::vector<uint32_t> remap(vertices.size());
    size_t uniqueVertexCount{ meshopt_generateVertexRemap(remap.data(), nullptr,
                                                            vertices.size(),
                                                            vertices.data(),
                                                            vertices.size(),
                                                            sizeof(Vertex)) };

    mesh.vertices.resize(uniqueVertexCount);
    mesh.indices.resize(vertices.size());

    meshopt_remapVertexBuffer(mesh.vertices.data(),
                                vertices.data(),
                                vertices.size(),
                                sizeof(Vertex),
                                remap.data());

    meshopt_remapIndexBuffer(mesh.indices.data(),
                                nullptr,
                                vertices.size(),
                                remap.data());

    return mesh;
}