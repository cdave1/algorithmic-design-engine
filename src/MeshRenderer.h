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

#ifndef _MESH_RENDERER_H_
#define _MESH_RENDERER_H_

#include <stdint.h>
#include <string>
#include <difont/difont.h>
#include <difont/opengl/OpenGLInterface.h>

#include "Generators.h"


class MeshRenderer {
private:
    difont::Font *m_font;

    difont::Font *m_textureFont;

    float m_width, m_height;

    std::string m_localFontLocation;

    std::string m_text;

public:
    MeshRenderer(float width, float height) : m_width(width), m_height(height) {}

    ~MeshRenderer() {}

    void SetConditions(float fontSize, std::string &text, const char *fontpath);

    void SetupVertexArrays(GLuint shaderProgram);

    void Render(GLuint shaderProgram);

};

#endif
