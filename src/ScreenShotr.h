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

#ifndef _SCREENSHOTR_H_
#define _SCREENSHOTR_H_

#define MAX_PIXEL_BUFFER_OBJECTS 1

#include <stdint.h>
#include <string>
#include <difont/opengl/OpenGLInterface.h>

#include "ImageFunctions.h"

class Bitmap;

class ScreenShotr {

private:
    uint16_t m_width, m_height;

    uint32_t m_pixelCount;

    size_t m_colorPixelSize;

    uint8_t *m_colorCopyBuffer;

    uint32_t m_colorPixelBufferIds[MAX_PIXEL_BUFFER_OBJECTS];

    uint32_t m_totalColorPixelBuffers;

    int m_colorVram2sys;

    int m_colorGpu2vram;

public:
    
    ScreenShotr(uint16_t width, uint16_t height);

    Bitmap * TakeScreenshot();
    
};

#endif
