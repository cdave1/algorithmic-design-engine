/*
 * Copyright (c) 2016 David Petrie david@davidpetrie.com
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

#include "LogoGenerator.h"

#include "CommonMath.h"
#include "OpenGLShim.h"
#include <iostream>
#include <sstream>

static GLuint vertexArrayObject = 0;
static GLuint vertexBufferObject = 0;
static GLuint indexBufferObject = 0;
static unsigned int vertexCount = 0;
static unsigned int triangleCount = 0;


void LogoGenerator::SetConditions(float fontSize, std::string &text, const char *fontpath) {
    glEnable(GL_TEXTURE_2D);
    m_font = new difont::PolygonFont(fontpath);
    if (m_font->Error()) {
        fprintf(stderr, "Could not load font at `%s'\n", fontpath);
        delete m_font;
        m_font = NULL;
    } else {
        m_font->FaceSize(fontSize, 300);
        m_font->CharMap(FT_ENCODING_ADOBE_LATIN_1);
    }

    m_textureFont = new difont::TextureFont(fontpath);
    if (m_textureFont->Error()) {
        fprintf(stderr, "Could not load font at `%s'\n", fontpath);
        delete m_textureFont;
        m_textureFont = NULL;
    } else {
        m_textureFont->FaceSize(fontSize, 300);
        m_textureFont->CharMap(FT_ENCODING_ADOBE_LATIN_1);
    }

    m_text = text;
}


void LogoGenerator::SetupVertexArrays(GLuint shaderProgram) {
    if (m_font) {
        difont::FontMeshSet::Begin();
        m_font->Render(m_text.c_str());

        vertexCount = 0;
        triangleCount = 0;

        unsigned int fontMeshCount = difont::FontMeshSet::MeshCount();
        difont::FontMesh *meshes = difont::FontMeshSet::GetMeshes();

        for (int i = 0; i < fontMeshCount; ++i) {
            difont::FontMesh fontMesh = meshes[i];
            vertexCount += fontMesh.GetVertexCount();

            if (fontMesh.primitive == GL_TRIANGLES) {
                triangleCount += fontMesh.GetVertexCount() / 3;
            }
            else if (fontMesh.primitive == GL_TRIANGLE_STRIP) {
                triangleCount += fontMesh.GetVertexCount() - 2;
            }
            else if (fontMesh.primitive == GL_TRIANGLE_FAN) {
                triangleCount += fontMesh.GetVertexCount() - 2;
            }
            else if (fontMesh.primitive == GL_QUADS) {
                triangleCount += 2 * (fontMesh.GetVertexCount() / 4);
            }
        }

        size_t vertexBlockSz = sizeof(difont::FontVertex) * vertexCount;
        difont::FontVertex *vertexBufferData = (difont::FontVertex *)malloc(vertexBlockSz);
        Triangle *indexBufferData = (Triangle *)malloc(sizeof(Triangle) * triangleCount);

        int vertexes = 0;
        int triangles = 0;

        for (int i = 0; i < fontMeshCount; ++i) {
            difont::FontMesh fontMesh = meshes[i];
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
        assert(vertexes == vertexCount);
        assert(triangles == triangleCount);

        if (indexBufferObject == 0)
            glGenBuffers(1, &indexBufferObject);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Triangle) * triangleCount, indexBufferData, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        free(indexBufferData);

        if (vertexBufferObject == 0)
            glGenBuffers(1, &vertexBufferObject);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(difont::FontVertex) * vertexCount, vertexBufferData, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        free(vertexBufferData);

        if (vertexArrayObject == 0)
            glGenVertexArrays(1, &vertexArrayObject);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "Position"), 3, GL_FLOAT, GL_FALSE, sizeof(difont::FontVertex), DIFONT_BUFFER_OFFSET(0));
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "Position"));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        difont::FontMeshSet::End();
    }
}


void LogoGenerator::Render(GLuint shaderProgram) {
    GLfloat width = m_width;
    GLfloat height = m_height;

    float projection[16];
    float view[16];
    float world[16];

    const float WIDTH_OFFSET = 200.0f;
    const float HEIGHT_OFFSET = 200.0f;

    difont::examples::Math::MatrixIdentity(view);
    difont::examples::Math::MatrixIdentity(world);
    difont::examples::Math::Ortho(projection,
                                  -WIDTH_OFFSET, width - WIDTH_OFFSET,
                                  -HEIGHT_OFFSET, height - HEIGHT_OFFSET,
                                  -1.0f, 1.0f);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "ProjectionMatrix"), 1, GL_FALSE, projection);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "ViewMatrix"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "ModelMatrix"), 1, GL_FALSE, world);
    glBindVertexArray(vertexArrayObject);
    glDrawRangeElements(GL_TRIANGLES, 0, vertexCount - 1, triangleCount * 3, GL_UNSIGNED_INT, NULL);
}
