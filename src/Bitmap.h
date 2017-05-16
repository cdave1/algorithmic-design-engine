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

#ifndef _BITMAP_H_
#define _BITMAP_H_

/**
 * Stores raw, uncompressed image data.
 */

#define BitmapHeadeLen 64

#include <iostream>
#include <fstream>
#include <sstream>

enum class BitmapFormat {
    BADFORMAT,
    RGBA4,
    RGBA8,
    ALPHA8,
    LUMINANCE_ALPHA8,
    RGBA5551,
    RGB4,
    RGB8,
};


class Bitmap {
private:
    uint32_t m_width;

    uint32_t m_height;

    BitmapFormat m_format;

    int m_dataLen;

    uint8_t *m_data;


public:

    static Bitmap * CreateBitmapWithData(uint32_t width,
                                         uint32_t height,
                                         BitmapFormat format,
                                         int len,
                                         uint8_t *data);

    static Bitmap * LoadBitmap(const std::string &path);

    static Bitmap * LoadBitmapFromData(int len,
                                       uint8_t *data);

    static Bitmap * LoadFromPNG(const std::string &path);

    static void SaveAs(const Bitmap *bitmap, const std::string &dest);

    static Bitmap * ConvertPixelResolution(const Bitmap *bitmap, BitmapFormat targetFormat);

    /**
     * Find the smallest bounding box that contains all non-zero
     * pixels, and then copy the image into this box.
     */
    static Bitmap * Trim(const Bitmap *bitmap);

    /**
     *  Reduces image size by a factor of two.
     */
    static Bitmap * Downsize(const Bitmap *bitmap);

    /**
     * Remove premultiplied alpha.
     *
     * Warning: this function will not exactly return the original pixel color.
     */
    static Bitmap * DivideAlpha(const Bitmap *bitmap);

    static void DestroyBitmap(const Bitmap *bitmap);

    uint32_t GetWidth() const;

    uint32_t GetHeight() const;

    uint32_t GetBitsPerChannel() const;

    BitmapFormat GetFormat() const;

    uint32_t GetDataLength() const;

    uint8_t * GetData() const;

    uint32_t GetPixel(const uint32_t x, const uint32_t y) const;

private:

    Bitmap();

    Bitmap(uint32_t dataLen,
           char *data);

    Bitmap(uint32_t width,
           uint32_t height,
           BitmapFormat format);

    Bitmap(uint32_t width,
           uint32_t height,
           BitmapFormat format,
           int len,
           uint8_t *data);

    ~Bitmap();

    void PasteData(int len,
                   uint8_t *data);

};


#endif