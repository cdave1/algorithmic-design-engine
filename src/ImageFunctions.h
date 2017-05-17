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

#ifndef _IMAGE_FUNCTIONS_H_
#define _IMAGE_FUNCTIONS_H_

#include <string>
#include <png.h>

#include <difont/difont.h>

///
/// Common image functions, generally taking raw pixel input, or raw vertex input, and generating some kind of
/// image file in a common format (PNG, SVG, EPS, PDF, etc).  May or may not use libraries.
///
class ImageFunctions {
public:

    static unsigned char * LoadPNG(const std::string &filename, int *width, int *height, int *comp);

    static void FreeImageData(void *data);

    static bool WritePNG(const std::string &filename, uint8_t *pixels, uint32_t width, uint32_t height);

    static bool WriteBMP(const std::string &filename, uint8_t *pixels, uint32_t width, uint32_t height);

    static bool WriteSVG(const std::string &filename, uint32_t width, uint32_t height, const difont::RenderData &renderData, bool exportMetaData = false);

    static bool WriteSVGMesh(const std::string &filename, uint32_t width, uint32_t height, const difont::RenderData &renderData);

};

#endif
