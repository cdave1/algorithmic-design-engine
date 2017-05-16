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

#include "ScreenShotr.h"

#include "CommonMath.h"
#include "OpenGLShim.h"
#include "Bitmap.h"
#include <iostream>
#include <sstream>

#include "ImageFunctions.h"


ScreenShotr::ScreenShotr(uint16_t width, uint16_t height) :
    m_width(0),
m_height(0),
m_pixelCount(0),
m_colorPixelSize(sizeof(uint32_t)),
m_colorCopyBuffer(nullptr),
m_colorPixelBufferIds(),
m_totalColorPixelBuffers(MAX_PIXEL_BUFFER_OBJECTS),
m_colorVram2sys(0),
m_colorGpu2vram(0)
{
    glDeleteBuffers(m_totalColorPixelBuffers, m_colorPixelBufferIds);
    for (uint32_t i = 0; i < m_totalColorPixelBuffers; ++i) {
        m_colorPixelBufferIds[i] = 0;
    }
    m_colorVram2sys = 0;
    m_colorGpu2vram = 0;

    if (m_colorCopyBuffer) {
        free(m_colorCopyBuffer);
    }

    m_width = width;
    m_height = height;

    m_pixelCount = m_width * m_height;

    m_colorPixelSize = sizeof(uint8_t) * 4;
    if (!(m_colorCopyBuffer = (uint8_t *)calloc(1, m_colorPixelSize * m_pixelCount))) {
        fprintf(stderr, "Could not allocate bytes for color pixel buffer\n");
        return;
    }
    glGenBuffers(m_totalColorPixelBuffers, m_colorPixelBufferIds);
    for (uint32_t i = 0; i < m_totalColorPixelBuffers; ++i) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, m_colorPixelBufferIds[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, m_colorPixelSize * m_pixelCount, NULL, GL_STREAM_READ);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    m_colorVram2sys = 0;
    m_colorGpu2vram = m_totalColorPixelBuffers - 1;
}


Bitmap * ScreenShotr::TakeScreenshot() {
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_colorPixelBufferIds[m_colorGpu2vram]);
    glBufferData(GL_PIXEL_PACK_BUFFER, m_colorPixelSize * m_pixelCount, NULL, GL_STREAM_READ);
    glPixelStorei(GL_PACK_SWAP_BYTES, GL_TRUE);

#ifdef GL_BGRA
    glReadPixels(0, 0, m_width, m_height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
#else
    glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#endif
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_colorPixelBufferIds[m_colorVram2sys]);

    uint8_t *data = (uint8_t *)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, m_colorPixelSize * m_pixelCount, GL_MAP_READ_BIT);
    if (data) {
#if 0
        for (uint32_t i = 0; i < m_height; ++i) {
            memcpy(m_colorCopyBuffer + ((m_height - i) * m_width * m_colorPixelSize), data + (i * m_width * m_colorPixelSize), m_width * m_colorPixelSize);
        }
#else
        memcpy(m_colorCopyBuffer, data, m_pixelCount * m_colorPixelSize);
#endif
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    uint32_t temp = m_colorPixelBufferIds[0];
    for (uint32_t i = 1; i < m_totalColorPixelBuffers; ++i) {
        m_colorPixelBufferIds[i - 1] = m_colorPixelBufferIds[i];
    }
    m_colorPixelBufferIds[m_totalColorPixelBuffers - 1] = temp;

    Bitmap *bitmap = Bitmap::CreateBitmapWithData(m_width, m_height, BitmapFormat::RGBA8, (int)(m_colorPixelSize * m_pixelCount), m_colorCopyBuffer);
    return bitmap;
}
