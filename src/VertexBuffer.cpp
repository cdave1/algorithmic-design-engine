/*
 * Copyright (c) 2017 David Petrie david@davidpetrie.com
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software. Permission is granted to anyone to use this software for
 * any purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 * that you wrote the original software. If you use this software in a product, an
 * acknowledgment in the product documentation would be appreciated but is not
 * required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "VertexBuffer.h"

#include "CommonMath.h"
#include "OpenGLShim.h"


void VertexBuffer::SetMeshSet(GLuint shaderProgram, const MeshSet &renderData) {
    m_vertexCount = 0;
    m_triangleCount = 0;

    if (m_indexBufferObject == 0)
        glGenBuffers(1, &m_indexBufferObject);

    if (m_vertexBufferObject == 0)
        glGenBuffers(1, &m_vertexBufferObject);

    if (m_vertexArrayObject == 0)
        glGenVertexArrays(1, &m_vertexArrayObject);

    for (Mesh mesh : renderData.meshes) {

        m_vertexCount += mesh.GetVertexCount();

        if (mesh.primitive == GL_TRIANGLES) {
            m_triangleCount += mesh.GetVertexCount() / 3;
        }
        else if (mesh.primitive == GL_TRIANGLE_STRIP) {
            m_triangleCount += mesh.GetVertexCount() - 2;
        }
        else if (mesh.primitive == GL_TRIANGLE_FAN) {
            m_triangleCount += mesh.GetVertexCount() - 2;
        }
        else if (mesh.primitive == GL_QUADS) {
            m_triangleCount += 2 * (mesh.GetVertexCount() / 4);
        }
    }

    size_t vertexBlockSz = sizeof(Vertex) * m_vertexCount;
    Vertex *vertexBufferData = (Vertex *)malloc(vertexBlockSz);
    Triangle *indexBufferData = (Triangle *)malloc(sizeof(Triangle) * m_triangleCount);

    int vertexes = 0;
    int triangles = 0;

    for (Mesh mesh : renderData.meshes) {
        unsigned long vertexCount = mesh.GetVertexCount();

        for (int j = 0; j < vertexCount; ++j) {
            Vertex vertex = mesh.vertices[j];
            uint32_t index = vertexes + j;
            vertexBufferData[index] = vertex;
        }

        if (mesh.primitive == GL_TRIANGLES) {
            for (int j = 0; j < vertexCount - 2; j += 3) {
                int i1 = vertexes + j;
                int i2 = vertexes + (j + 1);
                int i3 = vertexes + (j + 2);

                indexBufferData[triangles++] = Triangle(i1, i2, i3);
            }
        }
        else if (mesh.primitive == GL_TRIANGLE_STRIP) {
            for (int j = 0; j < (vertexCount - 2); j++) {
                int i1 = vertexes + j;
                int i2 = vertexes + (j + 1);
                int i3 = vertexes + (j + 2);

                indexBufferData[triangles++] = Triangle(i1, i2, i3);
            }
        }
        else if (mesh.primitive == GL_TRIANGLE_FAN) {
            for (int j = 0; j < (vertexCount - 2); j++) {
                int i1 = vertexes;
                int i2 = vertexes + (j + 1);
                int i3 = vertexes + (j + 2);

                indexBufferData[triangles++] = Triangle(i1, i2, i3);
            }
        }
        else if (mesh.primitive == GL_QUADS) {
            for (int j = 0; j < vertexCount - 3; j += 4) {
                int i1 = vertexes + j;
                int i2 = vertexes + (j + 1);
                int i3 = vertexes + (j + 2);
                int i4 = vertexes + (j + 3);

                indexBufferData[triangles++] = Triangle(i1, i2, i3);
                indexBufferData[triangles++] = Triangle(i1, i3, i4);
            }
        }
        vertexes += vertexCount;
    }
    assert(vertexes == m_vertexCount);
    assert(triangles == m_triangleCount);

    //if (m_indexBufferObject == 0)
    //    glGenBuffers(1, &m_indexBufferObject);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Triangle) * m_triangleCount, indexBufferData, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    free(indexBufferData);

    //if (m_vertexBufferObject == 0)
    //    glGenBuffers(1, &m_vertexBufferObject);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertexCount, vertexBufferData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    free(vertexBufferData);

    //if (m_vertexArrayObject == 0)
    //    glGenVertexArrays(1, &m_vertexArrayObject);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(m_vertexArrayObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glVertexAttribPointer(glGetAttribLocation(shaderProgram, "Position"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), DIFONT_BUFFER_OFFSET(0));
    glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "Position"));

    glVertexAttribPointer(glGetAttribLocation(shaderProgram, "Color"), 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), DIFONT_BUFFER_OFFSET(sizeof(Vec3)));
    glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "Color"));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void VertexBuffer::SetFontRenderData(GLuint shaderProgram, const difont::RenderData &renderData) {
    for (difont::GlyphData glyphData : renderData.GetGlyphData()) {
        for (difont::FontMesh fontMesh : glyphData.GetMeshes()) {
            m_vertexCount += fontMesh.GetVertexCount();

            if (fontMesh.primitive == GL_TRIANGLES) {
                m_triangleCount += fontMesh.GetVertexCount() / 3;
            }
            else if (fontMesh.primitive == GL_TRIANGLE_STRIP) {
                m_triangleCount += fontMesh.GetVertexCount() - 2;
            }
            else if (fontMesh.primitive == GL_TRIANGLE_FAN) {
                m_triangleCount += fontMesh.GetVertexCount() - 2;
            }
            else if (fontMesh.primitive == GL_QUADS) {
                m_triangleCount += 2 * (fontMesh.GetVertexCount() / 4);
            }
        }
    }

    size_t vertexBlockSz = sizeof(difont::FontVertex) * m_vertexCount;
    difont::FontVertex *vertexBufferData = (difont::FontVertex *)malloc(vertexBlockSz);
    Triangle *indexBufferData = (Triangle *)malloc(sizeof(Triangle) * m_triangleCount);

    int vertexes = 0;
    int triangles = 0;

    for (difont::GlyphData glyphData : renderData.GetGlyphData()) {
        for (difont::FontMesh fontMesh : glyphData.GetMeshes()) {
            uint32_t vertexCount = fontMesh.GetVertexCount();

            for (int j = 0; j < vertexCount; ++j) {
                difont::FontVertex vertex = fontMesh.vertices[j];
                uint32_t index = vertexes + j;
                vertexBufferData[index] = vertex;
            }

            if (fontMesh.primitive == GL_TRIANGLES) {
                for (int j = 0; j < vertexCount - 2; j += 3) {
                    int i1 = vertexes + j;
                    int i2 = vertexes + (j + 1);
                    int i3 = vertexes + (j + 2);

                    indexBufferData[triangles++] = Triangle(i1, i2, i3);
                }
            }
            else if (fontMesh.primitive == GL_TRIANGLE_STRIP) {
                for (int j = 0; j < (vertexCount - 2); j++) {
                    int i1 = vertexes + j;
                    int i2 = vertexes + (j + 1);
                    int i3 = vertexes + (j + 2);

                    indexBufferData[triangles++] = Triangle(i1, i2, i3);
                }
            }
            else if (fontMesh.primitive == GL_TRIANGLE_FAN) {
                for (int j = 0; j < (vertexCount - 2); j++) {
                    int i1 = vertexes;
                    int i2 = vertexes + (j + 1);
                    int i3 = vertexes + (j + 2);

                    indexBufferData[triangles++] = Triangle(i1, i2, i3);
                }
            }
            else if (fontMesh.primitive == GL_QUADS) {
                for (int j = 0; j < vertexCount - 3; j += 4) {
                    int i1 = vertexes + j;
                    int i2 = vertexes + (j + 1);
                    int i3 = vertexes + (j + 2);
                    int i4 = vertexes + (j + 3);

                    indexBufferData[triangles++] = Triangle(i1, i2, i3);
                    indexBufferData[triangles++] = Triangle(i1, i3, i4);
                }
            }
            vertexes += vertexCount;
        }
    }
    assert(vertexes == m_vertexCount);
    assert(triangles == m_triangleCount);

    if (m_indexBufferObject == 0)
        glGenBuffers(1, &m_indexBufferObject);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Triangle) * m_triangleCount, indexBufferData, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    free(indexBufferData);

    if (m_vertexBufferObject == 0)
        glGenBuffers(1, &m_vertexBufferObject);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(difont::FontVertex) * m_vertexCount, vertexBufferData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    free(vertexBufferData);

    if (m_vertexArrayObject == 0)
        glGenVertexArrays(1, &m_vertexArrayObject);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(m_vertexArrayObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glVertexAttribPointer(glGetAttribLocation(shaderProgram, "Position"), 3, GL_FLOAT, GL_FALSE, sizeof(difont::FontVertex), DIFONT_BUFFER_OFFSET(0));
    glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "Position"));
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void VertexBuffer::Render(GLuint shaderProgram) {
    float projection[16];
    float view[16];
    float world[16];

    difont::examples::Math::MatrixIdentity(view);
    difont::examples::Math::MatrixIdentity(world);

    difont::examples::Math::MatrixTranslation(world, 0.5f * m_canvasWidth - m_cameraOffset[0], 0.5f * m_canvasHeight - m_cameraOffset[1], 0.0f);
    /*
    difont::examples::Math::Ortho(projection,
                                  -0.5f * m_canvasWidth, 0.5f * m_canvasWidth,
                                  -0.5f * m_canvasHeight, 0.5f * m_canvasHeight, -1.0f, 1.0f);
*/
    difont::examples::Math::Ortho(projection,
                                  0, m_canvasWidth,
                                  0, m_canvasHeight, -1.0f, 1.0f);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "ProjectionMatrix"), 1, GL_FALSE, projection);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "ViewMatrix"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "ModelMatrix"), 1, GL_FALSE, world);

    glBindVertexArray(m_vertexArrayObject);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawRangeElements(GL_TRIANGLES, 0, m_vertexCount - 1, m_triangleCount * 3, GL_UNSIGNED_INT, NULL);
}
