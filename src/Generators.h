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

#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include <stdint.h>
#include <string>
#include <difont/difont.h>
#include <difont/opengl/OpenGLInterface.h>

#define DBOUT( s )            \
{                             \
std::ostringstream os_;    \
os_ << s;                   \
OutputDebugStringA( os_.str().c_str() );  \
}

typedef struct Triangle {
    uint32_t indexes[3];

    Triangle(uint32_t i1, uint32_t i2, uint32_t i3) {
        indexes[0] = i1;
        indexes[1] = i2;
        indexes[2] = i3;
    }
} Triangle;

#endif
