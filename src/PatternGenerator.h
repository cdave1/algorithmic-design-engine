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

#ifndef _PATTERN_GENERATOR_H_
#define _PATTERN_GENERATOR_H_

#include <stdint.h>
#include <string>
#include <difont/difont.h>
#include <difont/opengl/OpenGLInterface.h>

#include "Generators.h"
#include "Bitmap.h"
#include "VertexBuffer.h"

//
// Takes a mesh as input, and then returns a geometry pattern that covers the input mesh.
//
class PatternGenerator {
private:

    difont::Point m_offset;

    difont::Point m_meshCenter;

public:

    void GeneratePattern(const Bitmap *bitmap);

};

#endif
