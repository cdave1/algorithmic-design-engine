#include "Mesh.h"

void Mesh::SetPrimitive(unsigned int prim) {
    primitive = prim;
    vertices.clear();
}


void Mesh::AddVertex(Vertex vertex) {
    vertices.push_back(vertex);
}


void Mesh::SetTextureId(unsigned int texId) {
    textureId = texId;
}


unsigned long Mesh::GetVertexCount() const {
    return vertices.size();
}


void Mesh::SetColor(const Color4f &color) {
    for (int i = 0; i < vertices.size(); ++i) {
        vertices[i].color = color;
    }
}


void Mesh::Transform(const Matrix &matrix) {
    for (int i = 0; i < vertices.size(); ++i) {
        Vec3 result;
        Matrix::Vec3Multiply(result, vertices[i].origin, matrix);
        vertices[i].origin = result;
    }
}


void Mesh::Transform(const Quaternion &quaternion) {
    for (Vertex vertex : vertices) {
        Vec3 result = quaternion * vertex.origin;
        vertex.origin = result;
    }
}