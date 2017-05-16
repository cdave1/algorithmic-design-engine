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

#ifndef _VERTEX_BUFFER_H_
#define _VERTEX_BUFFER_H_

#include <stdint.h>
#include <difont/opengl/OpenGLInterface.h>

#include "Generators.h"
#include "Bitmap.h"
#include "MeshSet.h"

//
// Takes a mesh as input, and then returns a geometry pattern that covers the input mesh.
//
class VertexBuffer {
private:

    float m_canvasWidth;

    float m_canvasHeight;

    Vec2 m_cameraOffset;

    GLuint m_vertexArrayObject = 0;

    GLuint m_vertexBufferObject = 0;

    GLuint m_indexBufferObject = 0;

    unsigned int m_vertexCount = 0;

    unsigned int m_triangleCount = 0;

public:
    VertexBuffer(float canvasWidth, float canvasHeight) :
        m_canvasWidth(canvasWidth),
        m_canvasHeight(canvasHeight) {}

    void SetMeshSet(GLuint shaderProgram, const MeshSet &renderData);

    //
    // HACK HACK HACK
    //
    void SetFontRenderData(GLuint shaderProgram, const difont::RenderData &renderData);

    void SetCameraOffset(const Vec2 &offset) {
        m_cameraOffset = offset;
    }

    void Render(GLuint shaderProgram);

};

#endif
