#ifndef _MESH_H_
#define _MESH_H_

#include <vector>

#include <mash/Vertex.h>
#include "mash/Matrix.h"
#include "mash/Quaternion.h"


class Mesh {
public:

    unsigned int textureId = 0;

    unsigned int primitive = 0;

    std::vector <Vertex> vertices;

public:
    Mesh() : primitive(0) {}

    Mesh(unsigned int prim) : primitive(prim) {}

    void AddVertex(Vertex vertex);

    void SetTextureId(unsigned int texId);

    void SetPrimitive(unsigned int prim);

    void SetColor(const Color4f &color);

    void Transform(const Matrix &matrix);

    void Transform(const Quaternion &quaternion);

    unsigned long GetVertexCount() const;
};

#endif
