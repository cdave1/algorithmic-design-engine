#include "Bitmap.h"

#include <stdio.h>
#include <string.h>

#include "ImageFunctions.h"

Bitmap * Bitmap::CreateBitmapWithData(uint32_t width,
                                      uint32_t height,
                                      BitmapFormat format,
                                      int len,
                                      uint8_t *data) {
    return new Bitmap(width, height, format, len, data);
}


Bitmap * Bitmap::LoadBitmap(const std::string &path) {
    std::streamsize fileLen;
    uint8_t *fileBuffer;

    std::ifstream infile;
    infile.open (path, std::ifstream::in | std::ifstream::binary);
    if (!infile.good()) {
        fprintf(stderr, "Could not read file! %s\n", path.c_str());
        return NULL;
    }

    infile.seekg(0, std::ios::end);
    fileLen = infile.tellg();
    infile.seekg(0, std::ios::beg);

    if (fileLen < BitmapHeadeLen) {
        fprintf(stderr, "File not formatted correctly.\n");
        infile.close();
        return NULL;
    }

    if (NULL == (fileBuffer = (uint8_t *)malloc(sizeof(uint8_t) * fileLen))) {
        fprintf(stderr, "Calloc.\n");
        infile.close();
        return NULL;
    }

    infile.read((char *)fileBuffer, fileLen);
    infile.close();

    Bitmap * img = Bitmap::LoadBitmapFromData((int)fileLen, fileBuffer);
    free(fileBuffer);
    return img;
}


Bitmap * Bitmap::LoadBitmapFromData(int len,
                                    uint8_t *data) {
    uint32_t w;
    uint32_t h;
    BitmapFormat format;
    uint32_t imageDataLen;
    uint8_t *imageData;
    char tmp[16];

    memcpy(tmp, data, 16);
    w = (uint32_t)strtol(tmp, NULL, 0);

    memcpy(tmp, data + 16, 16);
    h = (uint32_t)strtol(tmp, NULL, 0);

    memcpy(tmp, data + 32, 16);
    format = (BitmapFormat)strtol(tmp, NULL, 0);

    memcpy(tmp, data + 48, 16);
    imageDataLen = (uint32_t)strtol(tmp, NULL, 0);

    if (format == BitmapFormat::RGBA4) {
        if (NULL == (imageData = (uint8_t *)malloc(sizeof(unsigned short) * imageDataLen))) {
            fprintf(stderr, "Calloc.\n");
            return NULL;
        }
    } else if (format == BitmapFormat::ALPHA8) {
        if (NULL == (imageData = (uint8_t *)malloc(sizeof(char) * imageDataLen)))
        {
            fprintf(stderr, "Calloc.\n");
            return NULL;
        }
    } else if (format == BitmapFormat::RGBA5551) {
        if (NULL == (imageData = (uint8_t *)malloc(sizeof(unsigned short) * imageDataLen))) {
            fprintf(stderr, "Calloc.\n");
            return NULL;
        }
    } else if (format == BitmapFormat::LUMINANCE_ALPHA8) {
        if (NULL == (imageData = (uint8_t *)malloc(sizeof(unsigned short) * imageDataLen))) {
            fprintf(stderr, "Calloc.\n");
            return NULL;
        }
    } else {
        format = BitmapFormat::RGBA8;
        if (NULL == (imageData = (uint8_t *)malloc(sizeof(char) * imageDataLen))) {
            fprintf(stderr, "Calloc.\n");
            return NULL;
        }
    }

    memcpy(imageData, data + BitmapHeadeLen, len - BitmapHeadeLen);

    Bitmap *img = new Bitmap(w, h, format, imageDataLen, imageData);

    if (img) {
        free(imageData);
        return img;
    }

    return NULL;
}


Bitmap * Bitmap::LoadFromPNG(const std::string &path) {
    int x,y,n;
    unsigned char *data = ImageFunctions::LoadPNG(path, &x, &y, &n);

    if (data == NULL) {
        return nullptr;
    } else {
        int imageDataLen = x * y;
        BitmapFormat format = BitmapFormat::BADFORMAT;
        if (n == 3) {
            format = BitmapFormat::RGB8;
            imageDataLen = x * y * 3;
        } else if (n == 4) {
            format = BitmapFormat::RGBA8;
            imageDataLen = x * y * 4;
        } else {
            // bad format
            ImageFunctions::FreeImageData(data);
            return nullptr;
        }
        Bitmap *img = new Bitmap(x, y, format, imageDataLen, data);
        ImageFunctions::FreeImageData(data);
        return img;
    }
}


void Bitmap::SaveAs(const Bitmap *bitmap, const std::string &dest) {
    int fileLen;
    char *fileBuffer;

    std::ofstream outfile;
    outfile.open(dest, std::ofstream::out | std::ofstream::binary);
    if (!outfile.good()) {
        fprintf(stderr, "Could not write to file! %s\n", dest.c_str());
        return;
    }

    fileLen = bitmap->m_dataLen + BitmapHeadeLen;
    if (NULL == (fileBuffer = (char *)malloc(sizeof(char) * fileLen))) {
        fprintf(stderr, "Calloc.\n");
        return;
    }

    // Save the header
    snprintf(fileBuffer + 0,   16, "%-16d", bitmap->GetWidth());
    snprintf(fileBuffer + 16,  16, "%-16d", bitmap->GetHeight());
    snprintf(fileBuffer + 32,  16, "%-16d", (int)bitmap->GetFormat());
    snprintf(fileBuffer + 48,  16, "%-16d", bitmap->GetDataLength());

    // Copy the image data, after the header
    memcpy(fileBuffer + BitmapHeadeLen, bitmap->GetData(), bitmap->GetDataLength());

    outfile.rdbuf()->sputn(fileBuffer, fileLen);
    outfile.flush();
    outfile.close();
}


Bitmap * Bitmap::ConvertPixelResolution(const Bitmap *bitmap, BitmapFormat targetFormat) {
    /*
    void *                  tempData = NULL;
    uint32_t *          inPixel32 = NULL;
    unsigned short *outPixel16 = NULL;

    tempData = calloc(1, m_width * m_height * sizeof(unsigned short));
    inPixel32 = (uint32_t *)m_data;
    outPixel16 = (unsigned short *)tempData;

    if (targetFormat == RGBA4)
    {
        for(uint32_t i = 0; i < m_width * m_height; ++i, ++inPixel32)
        {
            *outPixel16++ =
            ((((*inPixel32 >> 0) & 0xFF) >> 4) << 12) | // R
            ((((*inPixel32 >> 8) & 0xFF) >> 4) << 8) | // G
            ((((*inPixel32 >> 16) & 0xFF) >> 4) << 4) | // B
            ((((*inPixel32 >> 24) & 0xFF) >> 4) << 0); // A
        }
    }
    else if (targetFormat == RGBA5551)
    {
        for(uint32_t i = 0; i < m_width * m_height; ++i, ++inPixel32)
        {
            *outPixel16++ =
            ((((*inPixel32 >> 0) & 0xFF) >> 3) << 11) | // R
            ((((*inPixel32 >> 8) & 0xFF) >> 3) << 6) | // G
            ((((*inPixel32 >> 16) & 0xFF) >> 3) << 1) | // B
            ((((*inPixel32 >> 24) & 0xFF) > 63 ? 1 : 0) << 0); // A
        }
    }

    free(m_data);
    m_data = (char *)tempData;
    m_dataLen = m_dataLen / 2;
    m_format = targetFormat;
     */
    return nullptr;
}


Bitmap * Bitmap::Trim(const Bitmap *bitmap) {
    /*
    int row = 0;
    int column = 0;
    char * dataPointer = NULL;

    if (this->GetHeight() == 0 || this->GetWidth() == 0) return;

    // Trim top
    for (row = 0; row < (int)this->GetHeight(); ++row)
    {
        unsigned char isBlank = 1;
        for (column = 0; column < (int)this->GetWidth(); ++column)
        {
            dataPointer = m_data + ((row * m_width * 4) + (column * 4));
            if (dataPointer[3] > 0)
            {
                isBlank = 0;
                break;
            }
        }

        if (!isBlank)
            break;
    }
    unsigned top_most = row;

    // Trim left
    for (column = 0; column < (int)this->GetWidth(); ++column)
    {
        unsigned char isBlank = 1;
        for (row = 0; row < (int)this->GetHeight(); ++row)
        {
            dataPointer = m_data + ((row * m_width * 4) + (column * 4));
            if (dataPointer[3] > 0)
            {
                isBlank = 0;
                break;
            }
        }

        if (!isBlank)
            break;
    }
    unsigned left_most = column;

    // Can't trim any more!
    if (top_most == m_height && left_most == m_width)
    {
        return;
    }

    // Trip bottom
    for (row = this->GetHeight() - 1; row != 0; --row)
    {
        unsigned char isBlank = 1;
        for (column = 0; column < (int)this->GetWidth(); ++column)
        {
            dataPointer = m_data + ((row * m_width * 4) + (column * 4));
            if (dataPointer[3] > 0)
            {
                isBlank = 0;
                break;
            }
        }

        if (!isBlank)
            break;
    }
    unsigned bottom_most = row;

    // Trip right
    for (column = this->GetWidth() - 1; column != 0; --column)
    {
        unsigned char isBlank = 1;
        for (row = 0; row < (int)this->GetHeight(); ++row)
        {
            dataPointer = m_data + ((row * m_width * 4) + (column * 4));
            if (dataPointer[3] > 0)
            {
                isBlank = 0;
                break;
            }
        }

        if (!isBlank)
            break;
    }
    int right_most = column;

    int newWidth = (right_most - left_most) + 1;
    int newHeight = (bottom_most - top_most) + 1;
    char * tempData;
    int dataLen = 4 * newWidth * newHeight;

    cdkAssert(newWidth <= (int)m_width && newWidth >= 0);
    cdkAssert(newHeight <= (int)m_height && newHeight >= 0);

    if (NULL == (tempData = (char *)calloc(1, sizeof(char) * dataLen)))
    {
        fprintf(stderr, "Calloc\n");
        return;
    }

    for (unsigned copyRow = bottom_most; copyRow > top_most; copyRow -= 1)
    {
        char * rowPtr = m_data + ((copyRow * m_width * 4) + (left_most * 4));
        memcpy(tempData + ((copyRow - top_most) * newWidth * 4), rowPtr, (4 * newWidth));
    }

    FreeSafely(m_data);
    m_data = (char *)tempData;
    m_dataLen = dataLen;
    
    m_width = newWidth;
    m_height = newHeight;
     */
    return nullptr;
}


Bitmap * Bitmap::Downsize(const Bitmap *bitmap) {
    /*
    uint32_t            dataLen;
    void *                  tempData;
    uint32_t *          inPixel32;

    dataLen = (m_width >> 1) * (m_height >> 1) * 4;

    if (NULL == (tempData = calloc(1, sizeof(char) * dataLen)))
    {
        fprintf(stderr, "Calloc\n");
        return;
    }

    inPixel32 = (uint32_t *)m_data;
    uint32_t * outPixel32 = (uint32_t *)tempData;

    uint32_t dheight = (this->GetHeight() >> 1) << 1;
    uint32_t dwidth =  (this->GetWidth() >> 1) << 1;

    for (unsigned y = 0; y < dheight; y += 2)
    {
        inPixel32 = ((uint32_t *)m_data) + (y * m_width);
        for (unsigned x = 0; x < dwidth; x += 2)
        {
            uint32_t p = cdkPixelAverage(*inPixel32, *(inPixel32 + 1));
            uint32_t q = cdkPixelAverage(*(inPixel32 + m_width), *(inPixel32 + 1 + m_width));
            *outPixel32++  = cdkPixelAverage(p,q);
            inPixel32 += 2;
        }
    }

    FreeSafely(m_data);
    m_data = (char *)tempData;
    m_dataLen = dataLen;

    m_width >>= 1;
    m_height >>= 1;
     */
    return nullptr;
}


Bitmap * Bitmap::DivideAlpha(const Bitmap *bitmap) {
    /*
    uint32_t            pixelCount;
    char *                  dataPointer;

    pixelCount = m_width * m_height;
    dataPointer = m_data;

    for (uint32_t px = 0; px < pixelCount; ++px)
    {
        unsigned char alpha = (unsigned char)dataPointer[3];
        if (alpha != 255 && alpha != 0)
        {
            dataPointer[0] = ((int)dataPointer[0])*255/alpha;
            dataPointer[1] = ((int)dataPointer[1])*255/alpha;
            dataPointer[2] = ((int)dataPointer[2])*255/alpha;
        }
        dataPointer += 4;
    }*/
    return nullptr;
}


void Bitmap::DestroyBitmap(const Bitmap *bitmap) {
    delete bitmap;
}


uint32_t Bitmap::GetWidth() const {
    return m_width;
}


uint32_t Bitmap::GetHeight() const {
    return m_height;
}


uint32_t Bitmap::GetBitsPerChannel() const {
    if (GetFormat() == BitmapFormat::RGBA8) return 8;
    if (GetFormat() == BitmapFormat::ALPHA8) return 8;
    if (GetFormat() == BitmapFormat::LUMINANCE_ALPHA8) return 8;
    else return 4;
}


BitmapFormat Bitmap::GetFormat() const {
    return m_format;
}


uint32_t Bitmap::GetDataLength() const {
    return m_dataLen;
}


uint8_t * Bitmap::GetData() const {
    return m_data;
}


uint32_t Bitmap::GetPixel(const uint32_t x, const uint32_t y) const {
    if (x > m_width || y > m_height) return (uint32_t)0x0;

    uint32_t pixel;
    memcpy(&pixel, m_data + ((y * m_width * 4) + (x * 4)), sizeof(uint32_t));
    return pixel;
}


Bitmap::Bitmap() :
    m_width(0),
    m_height(0),
    m_format(BitmapFormat::RGBA8),
    m_data(nullptr),
    m_dataLen(0)
{}


Bitmap::Bitmap(uint32_t width,
               uint32_t height,
               BitmapFormat format) :
    m_width(width),
    m_height(height),
    m_format(format),
    m_dataLen(0),
    m_data(nullptr)
{}


Bitmap::Bitmap(uint32_t width,
               uint32_t height,
               BitmapFormat format,
               int len,
               uint8_t *data) :
    m_width(width),
    m_height(height),
    m_format(format),
    m_dataLen(len) {
    if (NULL == (m_data = (uint8_t *)malloc(sizeof(uint8_t) * len)))
    {
        fprintf(stderr, "Calloc.\n");
        return;
    }
    memcpy(m_data, data, len);
}


Bitmap::~Bitmap() {
    if (m_data) {
        free(m_data);
    }
}


void Bitmap::PasteData(int len,
                       uint8_t *data) {
    if (m_data) {
        free(m_data);
    }
    m_dataLen = len;
    if (NULL == (m_data = (uint8_t *)malloc(sizeof(uint8_t) * len)))
    {
        fprintf(stderr, "Calloc.\n");
        return;
    }
    memcpy(m_data, data, len);
}
