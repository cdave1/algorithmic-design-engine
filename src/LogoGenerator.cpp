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
#include <iostream>
#include <sstream>

#include "ImageFunctions.h"

void LogoGenerator::Generate(const std::string &fontpath,
                             std::wstring &text,
                             float fontSize,
                             float lineHeight,
                             const difont::Point spacing) {
    m_font = new difont::PolygonFont(fontpath.c_str());

    if (m_font->Error()) {
        fprintf(stderr, "Could not load font at '%s'\n", fontpath.c_str());
        delete m_font;
        m_font = NULL;
        return;
    } else {
        m_font->FaceSize(fontSize, 300);

        auto encoding = FT_ENCODING_UNICODE;
        //m_font->CharMap(ft_encoding_unicode);//FT_ENCODING_ADOBE_LATIN_1);
        if (m_font->CharMap(encoding)) {
            fprintf(stdout, "WIDE Set encoding: %d %d\n", m_font->CharMapCount(), encoding);
        } else {
            fprintf(stdout, "WIDE Invalid encoding! %d\n", encoding);
        }
    }

    m_localFontLocation = fontpath;
    m_wtext = text;

    if (m_font) {
        difont::RenderData _renderData;
        m_font->Render(m_wtext.c_str(), _renderData, static_cast<int>(m_wtext.length()), difont::Point(), spacing);
        difont::BBox box = m_font->BBox(m_wtext.c_str());
        meshCenter = Vec2((box.Upper().X() - box.Lower().X()) * 0.5, (box.Upper().Y() - box.Lower().Y()) * 0.5);
        renderData = _renderData;
    }
}


void LogoGenerator::Generate(const std::string &fontpath,
                             std::string &text,
                             float fontSize,
                             float lineHeight,
                             const difont::Point spacing) {
    m_font = new difont::PolygonFont(fontpath.c_str());

    if (m_font->Error()) {
        fprintf(stderr, "Could not load font at '%s'\n", fontpath.c_str());
        delete m_font;
        m_font = NULL;
        return;
    } else {
        auto encoding = FT_ENCODING_UNICODE;
        //m_font->CharMap(ft_encoding_unicode);//FT_ENCODING_ADOBE_LATIN_1);
        if (m_font->CharMap(encoding)) {
            fprintf(stdout, "Set encoding: %d %d\n", m_font->CharMapCount(), encoding);
        } else {
            fprintf(stdout, "Invalid encoding! %d\n", encoding);
        }
        m_font->FaceSize(fontSize, 300);


    }

    m_localFontLocation = fontpath;
    m_text = text;

    if (m_font) {
        difont::RenderData _renderData;
        m_font->Render(m_text.c_str(), _renderData, static_cast<int>(m_text.length()), difont::Point(), spacing);
        difont::BBox box = m_font->BBox(m_text.c_str());
        meshCenter = Vec2((box.Upper().X() - box.Lower().X()) * 0.5, (box.Upper().Y() - box.Lower().Y()) * 0.5);
        renderData = _renderData;
    }
}
