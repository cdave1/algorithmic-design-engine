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

#ifndef _PATTERN_ITERATOR_H_
#define _PATTERN_ITERATOR_H_

#include <stdint.h>
#include <string>
#include <difont/difont.h>
#include <difont/opengl/OpenGLInterface.h>

#include "mash/Color4f.h"
#include "mash/ColorHSV.h"

#include "Generators.h"
#include "Bitmap.h"
#include "VertexBuffer.h"

enum class Shape {
    Rectangle,
    Ellipse,
    Line,
    Triangle,
    Star,
};

enum class ColorSource {
    None,
    Solid,
    Palette
};

enum class DisplayOrder {
    Ascending,
    Descending
};

enum class InterpolationStyle {
    Linear,
    SmoothStep
};


class PatternDescription {
public:

    Shape shape = Shape::Rectangle;

    Color4f colorRange[2] = { Color4f(0.0f, 1.0f, 0.0f, 1.0f), Color4f(1.0f, 0.0f, 0.0f, 1.0f) };

    uint32_t iterations = 1;

    float angularStep = 0;

    DisplayOrder displayOrder = DisplayOrder::Ascending;

    float distanceRange[2] = { 1.0f, 2.0f };

    Vec2 sizeRange[2] = { Vec2(1.0f), Vec2(2.0f) };

    float rotationRange[2] = { 0.1f, 0.2f };

    float strokeThickness[2] = { 1.0f, 1.0f };

};


//
// Takes a mesh as input, and then returns a geometry pattern that covers the input mesh.
//
class PatternIterator {
private:

    difont::Point m_offset;

    difont::Point m_meshCenter;

    MeshSet meshes;


public:
    PatternIterator() {}

    void GeneratePattern(const PatternDescription &PatternDescription);

    void GeneratePatternAlt(const PatternDescription &PatternDescription);

    void GeneratePatternFromBitmap(const PatternDescription &patternDescription, const Bitmap *bitmap);

    void GenerateRaster(const PatternDescription &patternDescription, const Bitmap *bitmap);

    MeshSet GetMeshSet() const {
        return meshes;
    }


private:

    void AddMesh(const PatternDescription &patternDescription, int iteration);

    static void MakeTriangle(Mesh &mesh);

    static void MakeRectangle(Mesh &mesh);

};

#endif
