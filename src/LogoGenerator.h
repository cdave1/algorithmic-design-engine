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

#ifndef _LOGO_GENERATOR_H_
#define _LOGO_GENERATOR_H_

#include <stdint.h>
#include <string>
#include <difont/difont.h>

#include "Generators.h"
#include "Bitmap.h"
#include "VertexBuffer.h"


class LogoGenerator {
public:

    Vec2 meshCenter;

    difont::RenderData renderData;

private:
    difont::PolygonFont *m_font;

    difont::Font *m_textureFont;

    std::string m_localFontLocation;

    std::string m_text;

    std::wstring m_wtext;

    difont::Point m_offset;


public:
    LogoGenerator() {}

    ~LogoGenerator() {}

    void Generate(const std::string &fontpath,
                  std::string &text,
                  float fontSize = 50.0f,
                  float lineHeight = 0.0f,
                  const difont::Point spacing = difont::Point());

    void Generate(const std::string &fontpath,
                  std::wstring &text,
                  float fontSize = 50.0f,
                  float lineHeight = 0.0f,
                  const difont::Point spacing = difont::Point());

};

#endif
