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

#include "PatternIterator.h"

#include "CommonMath.h"
#include "OpenGLShim.h"
#include <iostream>
#include <sstream>

#include "mash/Matrix.h"


uint32_t _SamplePixel(const Bitmap *bitmap, uint32_t x, uint32_t y) {
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


void PatternIterator::MakeTriangle(Mesh &mesh) {
    Vertex a(0.0f, 1.0);
    Vertex b(-1.0, -1.0);
    Vertex c(1.0,  -1.0);

    mesh.AddVertex(a);
    mesh.AddVertex(b);
    mesh.AddVertex(c);
}


void PatternIterator::MakeRectangle(Mesh &mesh) {
    Vertex a(-0.5f, -0.5f, 0.0f);
    Vertex b( 0.5f, -0.5f, 0.0f);
    Vertex c( 0.5f,  0.5f, 0.0f);
    Vertex d(-0.5f,  0.5f, 0.0f);

    mesh.AddVertex(a);
    mesh.AddVertex(b);
    mesh.AddVertex(c);
    mesh.AddVertex(d);
}


void PatternIterator::AddMesh(const PatternDescription &patternDescription, int iteration) {
    Mesh mesh;

    if (patternDescription.shape == Shape::Rectangle) {
        mesh.SetPrimitive(GL_QUADS);
        MakeRectangle(mesh);
    } else if (patternDescription.shape == Shape::Triangle) {
        mesh.SetPrimitive(GL_TRIANGLES);
        MakeRectangle(mesh);
    }

    float step = mash::lerp(0.0f, 1.0f, float(iteration) / patternDescription.iterations);

    float sizeX = mash::lerp(patternDescription.sizeRange[0][0],
                             patternDescription.sizeRange[1][0],
                             step);

    float sizeY = mash::lerp(patternDescription.sizeRange[0][1],
                             patternDescription.sizeRange[1][1],
                             step);

    Matrix scale;
    Matrix::Scale(scale, sizeX, sizeY, 1.0f);

    mesh.Transform(scale);

    float distance = mash::lerp(patternDescription.distanceRange[0],
                                patternDescription.distanceRange[1],
                                step);

    float rads = mash::lerp(patternDescription.rotationRange[0],
                            patternDescription.rotationRange[1],
                            step);

#if 0
    Color4f color = mash::lerp(patternDescription.colorRange[0], patternDescription.colorRange[1], step);
    mesh.SetColor(color);
#else
    ColorHSV from = mash::RGBAtoHSV(patternDescription.colorRange[0]);
    ColorHSV to =   mash::RGBAtoHSV(patternDescription.colorRange[1]);

    ColorHSV hsv = mash::lerp(from, to, step);
    Color4f color = mash::HSVtoRGBA(hsv);
    mesh.SetColor(color);
#endif

    Matrix translation;
    Matrix::Translation(translation, distance, 0, 0);

    Matrix rotation;
    Matrix::RotationZ(rotation, rads);

    Matrix result = rotation * translation;

    mesh.Transform(result);

    meshes.AddMesh(mesh);
}


//
// Iterate from the center of the bitmap.  Increment and place a mesh if we hit a pixel.
//
void PatternIterator::GeneratePatternFromBitmap(const PatternDescription &patternDescription, const Bitmap *bitmap) {
    const int bailOut = 1024;
    unsigned meshIteration = 0;
    unsigned iteration = 0;

    while (meshIteration++ < patternDescription.iterations) {
        Matrix transform;
        Vec3 origin;
        float step = mash::lerp(0.0f, 1.0f, float(meshIteration) / patternDescription.iterations);

        float distance = mash::lerp(patternDescription.distanceRange[0],
                                    patternDescription.distanceRange[1],
                                    step);

        float rads = mash::lerp(patternDescription.rotationRange[0],
                                patternDescription.rotationRange[1],
                                step);

        Matrix translation;
        Matrix::Translation(translation, distance, 0, 0);

        Matrix rotation;
        Matrix::RotationZ(rotation, rads);

        Matrix result = rotation * translation;

        Vec3 pos;
        Matrix::Vec3Multiply(pos, pos, rotation * translation);

        uint32_t x = bitmap->GetWidth() * 0.5f + int(pos[0]);
        uint32_t y = bitmap->GetHeight() * 0.5f + int(pos[1]);

#define USE_MESH_ITERATION
#ifdef USE_MESH_ITERATION
        if (_SamplePixel(bitmap, x, y)) {
            //meshIteration++;
            AddMesh(patternDescription, meshIteration);
        }
#else
        if (_SamplePixel(bitmap, x, y)) {
            meshIteration++;
            AddMesh(patternDescription, iteration);
        }
#endif
    }
}


void PatternIterator::GeneratePatternAlt(const PatternDescription &patternDescription) {
    if (patternDescription.angularStep == 0.0f) {
        fprintf(stderr, "Angular step should not be non-zero\n");
        return;
    }

    const unsigned circles = 20;
    unsigned iterations = 0;

    for (int i = 0; i < circles; ++i) {
        float step = float(i) / circles;


        float radius = mash::lerp(patternDescription.distanceRange[0],
                                    patternDescription.distanceRange[1],
                                    step);

        float sx = mash::lerp(patternDescription.sizeRange[0][0],
                                 patternDescription.sizeRange[1][0],
                                 step);

        float arcTheta = patternDescription.angularStep * sx / radius;

        float rot = 0.0f;

        while (rot < math_2_pi && iterations < patternDescription.iterations) {

            Matrix translation;
            Matrix::Translation(translation, radius, 0, 0);

            Matrix rotation;
            Matrix::RotationZ(rotation, rot);

            { // Duplicated code
                Mesh mesh;

                if (patternDescription.shape == Shape::Rectangle) {
                    mesh.SetPrimitive(GL_QUADS);
                    MakeRectangle(mesh);
                } else if (patternDescription.shape == Shape::Triangle) {
                    mesh.SetPrimitive(GL_TRIANGLES);
                    MakeRectangle(mesh);
                }

                float sizeX = mash::lerp(patternDescription.sizeRange[0][0],
                                         patternDescription.sizeRange[1][0],
                                         step);

                float sizeY = mash::lerp(patternDescription.sizeRange[0][1],
                                         patternDescription.sizeRange[1][1],
                                         step);

                Matrix scale;
                Matrix::Scale(scale, sizeX, sizeY, 1.0f);

                mesh.Transform(scale);
                
                ColorHSV from = mash::RGBAtoHSV(patternDescription.colorRange[0]);
                ColorHSV to =   mash::RGBAtoHSV(patternDescription.colorRange[1]);
                
                ColorHSV hsv = mash::lerp(from, to, step);
                Color4f color = mash::HSVtoRGBA(hsv);
                mesh.SetColor(color);

                Matrix result = rotation * translation;

                mesh.Transform(result);
                
                meshes.AddMesh(mesh);
            }

            ++iterations;

            rot += arcTheta;
        }

        fprintf(stdout, "%d iterations on last circle\n", iterations);
    }
}


void PatternIterator::GeneratePattern(const PatternDescription &patternDescription) {
    for (int i = 0; i < patternDescription.iterations; ++i) {
        AddMesh(patternDescription, i);
    }
}

    /*
void PatternIterator::Render() {

    Matrix projection, view, world;

    const GLfloat width = m_canvasWidth * 0.5f;
    const GLfloat height = m_canvasHeight * 0.5f;

    Matrix::Identity(projection);
    Matrix::Identity(view);
    Matrix::Ortho(world, -width, width, -height, height, -1.0f, 1.0f);

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "ProjectionMatrix"), 1, GL_FALSE, projection.f);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "ViewMatrix"), 1, GL_FALSE, view.f);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "ModelMatrix"), 1, GL_FALSE, world.f);

    m_vertexBuffer.Render();

}     */


void PatternIterator::GenerateRaster(const PatternDescription &patternDescription, const Bitmap *bitmap) {
    const uint32_t STEP = 8;
    const uint32_t HORIZONTAL_STEP = STEP;
    const uint32_t VERTICAL_STEP =   STEP; //2 * STEP;

    const float FIXED_GAP = 2.0f;

    float width = STEP - FIXED_GAP;
    float height = STEP - FIXED_GAP;

    static int count = 0;

    for (uint32_t y = 0; y < bitmap->GetHeight(); y += VERTICAL_STEP) {
        for (uint32_t x = 0; x < bitmap->GetWidth(); x += HORIZONTAL_STEP) {
            count++;
            if (_SamplePixel(bitmap, x, y)) {
                Mesh mesh;

                if (patternDescription.shape == Shape::Rectangle) {
                    mesh.SetPrimitive(GL_QUADS);
                    MakeRectangle(mesh);
                } else if (patternDescription.shape == Shape::Triangle) {
                    mesh.SetPrimitive(GL_TRIANGLES);
                    MakeRectangle(mesh);
                }

                Matrix scale;
                Matrix::Scale(scale, width, height, 1.0f);

                mesh.Transform(scale);

                Matrix translation;
                Matrix::Translation(translation, x, y, 0);

                mesh.Transform(translation);

                meshes.AddMesh(mesh);
            }
        }
    }
}
