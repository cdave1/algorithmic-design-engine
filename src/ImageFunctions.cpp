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

#include "ImageFunctions.h"

#include "CommonMath.h"
#include "OpenGLShim.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include "mash/Geometry.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

unsigned char * ImageFunctions::LoadPNG(const std::string &filename, int *width, int *height, int *comp) {
    stbi_set_flip_vertically_on_load(true);
    return stbi_load(filename.c_str(), width, height, comp, 0);
}


void ImageFunctions::FreeImageData(void *data) {
    if (data) {
        stbi_image_free(data);
    }
}


bool ImageFunctions::WritePNG(const std::string &filename, uint8_t *pixels, uint32_t w, uint32_t h) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png)
        return false;

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, &info);
        return false;
    }

    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        png_destroy_write_struct(&png, &info);
        return false;
    }

    png_init_io(png, fp);
    png_set_IHDR(png, info, w, h, 8 /* depth */, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_colorp palette = (png_colorp)png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
    if (!palette) {
        fclose(fp);
        png_destroy_write_struct(&png, &info);
        return false;
    }
    png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);
    png_write_info(png, info);
    png_set_packing(png);

    png_bytepp rows = (png_bytepp)png_malloc(png, h * sizeof(png_bytep));
    for (int i = 0; i < h; ++i)
        rows[i] = (png_bytep)(pixels + (h - i - 1) * w * 4);;

    png_write_image(png, rows);
    png_write_end(png, info);
    png_free(png, palette);
    png_destroy_write_struct(&png, &info);

    fclose(fp);
    delete[] rows;
    return true;
}


bool ImageFunctions::WriteBMP(const std::string &filename, uint8_t *pixels, uint32_t width, uint32_t height) {
    const int STB_COMP_MONOCHROME = 1;
    const int STB_COMP_MONOCHROMEALPHA = 2;
    const int STB_COMP_RGB = 3;
    const int STB_COMP_RGBA = 4;
    int result = stbi_write_bmp(filename.c_str(), width, height, STB_COMP_RGBA, pixels);
    return result == 0;
}


bool ImageFunctions::WriteSVG(const std::string &filename, uint32_t width, uint32_t height, const difont::RenderData &renderData, bool exportMetaData) {
    std::ofstream outfile;
    outfile.open(filename, std::ofstream::out | std::ofstream::binary);
    if (!outfile.good()) {
        fprintf(stderr, "Could not write to file! %s\n", filename.c_str());
        return false;
    }

    outfile << "<svg x=\"" << -0.5 * width << "\" y=\"" << -0.5 * height << "\" width=\"" << width << "\" height=\"" << height << "\" version=\"1.1\" ";
    outfile << "xmlns=\"http://www.w3.org/2000/svg\" xlink=\"http://www.w3.org/1999/xlink\">";

    unsigned count = 0;
    for (difont::GlyphData glyph : renderData.GetGlyphData()) {
        outfile << "<g id=\"glyph" << count << "\">";
        outfile << "<path d=\"";

        for (difont::Path path : glyph.GetPaths()) {
            outfile << "M" << path.GetCurves()[0].points[0].X() << "," << path.GetCurves()[0].points[0].Y() << " ";

            for (difont::Curve curve : path.GetCurves()) {
                outfile << curve.ToSVG();
            }
        }

        //outfile << "\" stroke=\"black\" fill=\"green\" /></g>";
        outfile << "\" stroke=\"black\" fill=\"none\" />";

        if (exportMetaData) {
            difont::Point baselineStart = glyph.GetBaseline();
            difont::Point baselineEnd = baselineStart + difont::Point((glyph.GetBoundingBox().Upper() - glyph.GetBoundingBox().Lower()).X(), 0.0);
            difont::Curve baseline(baselineStart, baselineEnd);

            difont::Path boundingBox;
            difont::Point a(glyph.GetBoundingBox().Lower().X(), glyph.GetBoundingBox().Lower().Y());
            difont::Point b(glyph.GetBoundingBox().Lower().X(), glyph.GetBoundingBox().Upper().Y());
            difont::Point c(glyph.GetBoundingBox().Upper().X(), glyph.GetBoundingBox().Upper().Y());
            difont::Point d(glyph.GetBoundingBox().Upper().X(), glyph.GetBoundingBox().Lower().Y());

            boundingBox.AddLine(a, b);
            boundingBox.AddLine(b, c);
            boundingBox.AddLine(c, d);
            boundingBox.AddLine(d, a);

            outfile << "<g id=\"baseline\">";
            outfile << "<path d=\"";
            outfile << "M" << baseline.points[0].X() << "," << baseline.points[0].Y() << " ";
            outfile << baseline.ToSVG();
            outfile << "\"/>";
            outfile << "</g>";

            outfile << "<g id=\"boundingBox\">";
            outfile << "<path d=\"";
            outfile << "M" << boundingBox.GetCurves()[0].points[0].X() << "," << boundingBox.GetCurves()[0].points[0].Y() << " ";
            for (difont::Curve curve : boundingBox.GetCurves()) {
                outfile << curve.ToSVG();
            }
            outfile << "\"/>";
            outfile << "</g>";
        }

        outfile << "</g>";

        count++;
    }

    outfile << "</svg>";

    outfile.flush();
    outfile.close();

    return true;
}


bool ImageFunctions::WriteSVGMesh(const std::string &filename, uint32_t width, uint32_t height, const difont::RenderData &renderData) {
    std::ofstream outfile;
    outfile.open(filename, std::ofstream::out | std::ofstream::binary);
    if (!outfile.good()) {
        fprintf(stderr, "Could not write to file! %s\n", filename.c_str());
        return false;
    }

    outfile << "<svg x=\"0\" y=\"0\" width=\"" << width << "\" height=\"" << height << "\" version=\"1.1\" ";
    outfile << "xmlns=\"http://www.w3.org/2000/svg\" xlink=\"http://www.w3.org/1999/xlink\">";

    Vec3 offset(0, 500);

    unsigned count = 0;
    for (difont::GlyphData glyph : renderData.GetGlyphData()) {
        outfile << "<g id=\"glyph" << count++ << "\">";

        for (difont::FontMesh mesh : glyph.GetMeshes()) {
            if (mesh.primitive == GL_TRIANGLE_FAN) {
                Vec3 a(mesh.vertices[0].position[0],        offset[1] - mesh.vertices[0].position[1]);

                for (int i = 1; i < mesh.vertices.size() - 1; ++i) {
                    Vec3 b(mesh.vertices[i].position[0],    offset[1] - mesh.vertices[i].position[1]);
                    Vec3 c(mesh.vertices[i+1].position[0],  offset[1] - mesh.vertices[i+1].position[1]);

                    outfile << "<path d=\"";
                    outfile << "M" << a[0] << "," << a[1] << " ";
                    outfile << "L" << a[0] << "," << a[1] << " ";
                    outfile << "L" << b[0] << "," << b[1] << " ";
                    outfile << "L" << c[0] << "," << c[1] << " ";
                    outfile << "L" << a[0] << "," << a[1] << " ";
                    outfile << "\" fill=\"black\" />";
                }
            } else if (mesh.primitive == GL_TRIANGLE_STRIP) {
                for (int i = 0; i < mesh.vertices.size() - 2; ++i) {
                    Vec3 a(mesh.vertices[i].position[0],    offset[1] - mesh.vertices[i].position[1]);
                    Vec3 b(mesh.vertices[i+1].position[0],  offset[1] - mesh.vertices[i+1].position[1]);
                    Vec3 c(mesh.vertices[i+2].position[0],  offset[1] - mesh.vertices[i+2].position[1]);

                    outfile << "<path d=\"";
                    outfile << "M" << a[0] << "," << a[1] << " ";
                    outfile << "L" << a[0] << "," << a[1] << " ";
                    outfile << "L" << b[0] << "," << b[1] << " ";
                    outfile << "L" << c[0] << "," << c[1] << " ";
                    outfile << "L" << a[0] << "," << a[1] << " ";
                    outfile << "\" fill=\"black\" />";
                }
            } else if (mesh.primitive == GL_TRIANGLES) {
                for (int i = 0; i < mesh.vertices.size(); i += 3) {
                    Vec3 a(mesh.vertices[i].position[0],    offset[1] - mesh.vertices[i].position[1]);
                    Vec3 b(mesh.vertices[i+1].position[0],  offset[1] - mesh.vertices[i+1].position[1]);
                    Vec3 c(mesh.vertices[i+2].position[0],  offset[1] - mesh.vertices[i+2].position[1]);

                    outfile << "<path d=\"";
                    outfile << "M" << a[0] << "," << a[1] << " ";
                    outfile << "L" << a[0] << "," << a[1] << " ";
                    outfile << "L" << b[0] << "," << b[1] << " ";
                    outfile << "L" << c[0] << "," << c[1] << " ";
                    outfile << "L" << a[0] << "," << a[1] << " ";
                    outfile << "\" fill=\"black\" />";
                }
            } else {
                Vec3 a(mesh.vertices[0].position[0],        offset[1] - mesh.vertices[0].position[1]);
                outfile << "<path d=\"";
                outfile << "M" << a[0] << "," << a[1] << " ";

                for (difont::FontVertex vertex : mesh.vertices) {
                    Vec3 v(vertex.position[0],              offset[1] - vertex.position[1]);
                    outfile << "L" << v[0] << "," << v[1] << " ";
                }
                outfile << "\" fill=\"black\" />";
            }
        }

        outfile << "</g>";
    }

    outfile << "</svg>";

    outfile.flush();
    outfile.close();
    
    return true;
}
