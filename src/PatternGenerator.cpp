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

#include "PatternGenerator.h"

#include "CommonMath.h"
#include "OpenGLShim.h"
#include <iostream>
#include <sstream>


uint32_t SamplePixel(const Bitmap *bitmap, uint32_t x, uint32_t y) {
    const unsigned SAMPLE_SIZE = 16;
    const unsigned SAMPLE_RADIUS = 14;
    const Vec2 poissonDisk[SAMPLE_SIZE] = {
        Vec2( -0.94201624,  -0.39906216 ),
        Vec2(  0.94558609,  -0.76890725 ),
        Vec2( -0.094184101, -0.92938870 ),
        Vec2(  0.34495938,   0.29387760 ),
        Vec2( -0.91588581,   0.45771432 ),
        Vec2( -0.81544232,  -0.87912464 ),
        Vec2( -0.38277543,   0.27676845 ),
        Vec2(  0.97484398,   0.75648379 ),
        Vec2(  0.44323325,  -0.97511554 ),
        Vec2(  0.53742981,  -0.47373420 ),
        Vec2( -0.26496911,  -0.41893023 ),
        Vec2(  0.79197514,   0.19090188 ),
        Vec2( -0.24188840,   0.99706507 ),
        Vec2( -0.81409955,   0.91437590 ),
        Vec2(  0.19984126,   0.78641367 ),
        Vec2(  0.14383161,  -0.14100790 ) };

    uint32_t alpha = 0;
    for (int i = 0; i < SAMPLE_SIZE; ++i) {
        Vec2 sample = poissonDisk[i];
        uint32_t _x = x + SAMPLE_RADIUS * sample[0];
        uint32_t _y = y + SAMPLE_RADIUS * sample[1];

        uint32_t pixel = bitmap->GetPixel(_x, _y);
        uint32_t a = (pixel >> 24);

        alpha += a;
    }

    alpha = alpha / SAMPLE_SIZE;

    if (alpha > 128) {
        return true;
    } else {
        return false;
    }
}


void AddTriangle(MeshSet &renderData, uint32_t x, uint32_t y, float width, float height, int count) {
    float xStep = width;
    float yStep = height;

    Vertex a(0.0f, -yStep);
    Vertex b(-xStep, yStep);
    Vertex c(xStep, yStep);

    Vertex aa(0.0f, yStep);
    Vertex bb(-xStep, -yStep);
    Vertex cc(xStep, -yStep);

    if (count % 2 == 0) {
        a.origin.Set(a.origin[0] + x, a.origin[1] + y, 0.0f);
        b.origin.Set(b.origin[0] + x, b.origin[1] + y, 0.0f);
        c.origin.Set(c.origin[0] + x, c.origin[1] + y, 0.0f);
    } else {
        a.origin.Set(aa.origin[0] + x, aa.origin[1] + y, 0.0f);
        b.origin.Set(bb.origin[0] + x, bb.origin[1] + y, 0.0f);
        c.origin.Set(cc.origin[0] + x, cc.origin[1] + y, 0.0f);
    }

    Mesh mesh(GL_TRIANGLES);
    mesh.AddVertex(a);
    mesh.AddVertex(b);
    mesh.AddVertex(c);

    renderData.AddMesh(mesh);
}


void AddRectangle(MeshSet &renderData, uint32_t x, uint32_t y, float width, float height) {
    Vertex a(x + -width * 0.5f, y + -height * 0.5f, 0.0f);
    Vertex b(x + width * 0.5f,  y + -height * 0.5f, 0.0f);
    Vertex c(x + width * 0.5f,  y + height * 0.5f, 0.0f);
    Vertex d(x + -width * 0.5f, y + height * 0.5f, 0.0f);

    Mesh mesh(GL_QUADS);
    mesh.AddVertex(a);
    mesh.AddVertex(b);
    mesh.AddVertex(c);
    mesh.AddVertex(d);

    renderData.AddMesh(mesh);
}


void PatternGenerator::GeneratePattern(const Bitmap *bitmap) {
    const uint32_t STEP = 8;
    const uint32_t HORIZONTAL_STEP = STEP;
    const uint32_t VERTICAL_STEP =   STEP; //2 * STEP;

    const float FIXED_GAP = 2.0f;

    float width = STEP - FIXED_GAP;
    float height = STEP - FIXED_GAP;

    static int count = 0;

    MeshSet meshSet;
    for (uint32_t y = 0; y < bitmap->GetHeight(); y += VERTICAL_STEP) {
        for (uint32_t x = 0; x < bitmap->GetWidth(); x += HORIZONTAL_STEP) {
            count++;
            if (SamplePixel(bitmap, x, y)) {
                //AddTriangle(meshSet, x, y, width, height, count);
                AddRectangle(meshSet, x, y, width, height);
            }
        }
    }

    fprintf(stderr, "%lu meshes added\n", meshSet.meshes.size());
}

 /*
void PatternGenerator::Render() {

    const GLfloat width = m_canvasWidth;
    const GLfloat height = m_canvasHeight;

    float projection[16];
    float view[16];
    float world[16];

    const float WIDTH_OFFSET = m_offset.X() + m_meshCenter.X();
    const float HEIGHT_OFFSET = m_offset.Y() + m_meshCenter.Y();

    difont::examples::Math::MatrixIdentity(view);
    difont::examples::Math::MatrixIdentity(world);
    //difont::examples::Math::MatrixTranslation(world, 0.5f * width - m_meshCenter.X(), 0.5f * height - m_meshCenter.Y(), 0.0f);
    difont::examples::Math::Ortho(projection, 0.0f, width, 0.0f, height, -1.0f, 1.0f);

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "ProjectionMatrix"), 1, GL_FALSE, projection);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "ViewMatrix"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "ModelMatrix"), 1, GL_FALSE, world);

    m_vertexBuffer.Render();
  }*/

