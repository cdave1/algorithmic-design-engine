//
//  main.c
//  ae
//
//  Created by David Petrie on 24/03/16.
//  Copyright © 2016 n/a. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <clocale>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_GL3
#define GLFW_NO_GLU
#define GLEW_NO_GLU

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#import <Cocoa/Cocoa.h>

#include "json.h"
#include "OpenGLShim.h"
#include "LogoGenerator.h"
#include "PatternIterator.h"
#include "ScreenShotr.h"
#include "Bitmap.h"
#include "Job.h"

static std::vector<Job *> jobs;
static uint32_t currentJobIndex = 0;
static ScreenShotr *screenShotr = nullptr;
static GLuint shaderProgram;
static uint32_t currentFrame = 0;

static const unsigned int PREFERRED_WINDOW_WIDTH = 2364;
static const unsigned int PREFERRED_WINDOW_HEIGHT = 1464;

#define PREVIEW_OUTPUT false

///
/// How to make this work:
/// 1. Read json from stdin containing work order information
/// 2. Generate according to taste.
///
int main(int argc, const char * argv[]) {
    std::string lines;
    NSString *outputBasePath = @"./";

#if 1
//#define PREVIEW_OUTPUT true
    lines = R"({"id":"323C763B-870F-46F8-9A8B-18090D29D3DA","category":"logo-text","lineHeight":0,"spacing":[0,0],"text":"OD","iteration":9,"font":"/Users/davidpetrie/Projects/aesthetics-engine/utils/tmp/futura_condensed_extra_bold.ttf","offset":[0,0],"fontSize":400,"output":[{"png":"323C763B-870F-46F8-9A8B-18090D29D3DA.png"},{"svg":"323C763B-870F-46F8-9A8B-18090D29D3DA.svg"},{"svg-mesh":"323C763B-870F-46F8-9A8B-18090D29D3DA.mesh.svg"}]}
    )";

    // unicode

#else
    for (std::string line; std::getline(std::cin, line);) {
        lines += line;
    }
#endif

    Json::Value data;
    Json::Reader reader;
    if (!reader.parse(lines, data)) {
        fprintf(stderr, "Error reading input.\n");
        exit(1);
    }

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }


    GLFWwindow* window;
    if (PREVIEW_OUTPUT) {
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


        window = glfwCreateWindow(PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT, "LogoGenerator", NULL, NULL);
        if (window == NULL){
            fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        glewExperimental = true;
        if (glewInit() != GLEW_OK) {
            fprintf(stderr, "Failed to initialize GLEW\n");
            return -1;
        }

        glfwSwapInterval(0);
    }

#define TAKE_SCREENSHOT_AND_EXIT

    fprintf(stdout, "%s\n", lines.c_str());

    const Json::Value jobInfo = data;

    Job *job = new Job(jobInfo);
    if (jobInfo["category"] == "logo-text") {
        const Json::Value text = jobInfo["text"];
        std::string fontText;
        std::wstring unicodeFontText;
        if (text.isString()) {
            fontText = jobInfo["text"].asString();
        } else if (text.isObject() && text.isMember("type") && text["type"].isString() && text["type"].asString() == "Buffer"
                   && text.isMember("data") && text["data"].isArray()) {
            const Json::Value bufferData = text["data"];

            uint32_t len = (bufferData.size() / 2) + 1;
            wchar_t *wchars = new wchar_t[len];
            bzero(wchars, sizeof(wchar_t) * len);

            for (int index = 0; index < bufferData.size(); index += 2) {
                uint16_t c1 = bufferData[index].asUInt();
                uint16_t c2 = bufferData[index+1].asUInt();
                wchars[index / 2] = wchar_t((c2 << 8) + c1);
            }
            unicodeFontText = std::wstring(wchars);
            delete[] wchars;
        }

        const int iteration = jobInfo["iteration"].asInt();
        const double fontSize = jobInfo["fontSize"].asDouble();
        const std::string fontPath = jobInfo["font"].asString();
        const float lineHeight = jobInfo["lineHeight"].asDouble();
        const Json::Value jsonSpacing = jobInfo["spacing"];
        const difont::Point spacing(jsonSpacing[0].asFloat(), jsonSpacing[1].asFloat());

        //if (jobInfo.isMember("outputBasePath")) {
        //    outputBasePath = [NSString stringWithUTF8String:jobInfo["outputBasePath"].asString().c_str()];
        //} else {
            const unsigned int MAX_PATH_LEN = 1024;
            char cwd[MAX_PATH_LEN];
            getcwd(cwd, MAX_PATH_LEN);
            outputBasePath = [NSString stringWithUTF8String:cwd];
        //}

        LogoGenerator *logoGenerator = new LogoGenerator();

        if (unicodeFontText.empty()) {
            logoGenerator->Generate(fontPath.c_str(), fontText, fontSize, lineHeight, spacing);
        } else {
            logoGenerator->Generate(fontPath.c_str(), unicodeFontText, fontSize, lineHeight, spacing);
        }

        job->logoGenerator = logoGenerator;

    } else if (jobInfo["category"] == "pattern") {
        PatternIterator *patternIterator = new PatternIterator();
        job->patternIterator = patternIterator;

        if (jobInfo.isMember("input-files") && jobInfo["input-files"].isArray()) {
            for (Json::Value inputFile : jobInfo["input-files"]) {
                if (inputFile.isMember("png")) {
                    std::string _png = inputFile["png"].asString();
                    job->pngInputLocation = _png;
                }
            }
        }
    }

    if (jobInfo.isMember("exportFontMetaData") && jobInfo["exportFontMetaData"].isBool()) {
        job->exportFontMetaData = jobInfo["exportFontMetaData"].asBool();
    } else {
        job->exportFontMetaData = false;
    }

    if (jobInfo.isMember("svgOutputUrl") && jobInfo["svgOutputUrl"].isString()) {
        job->svgOutputLocation = jobInfo["svgOutputUrl"].asString();
        fprintf(stderr, "SVG output location: %s\n", job->svgOutputLocation.c_str());
    }

    if (jobInfo.isMember("pngOutputUrl") && jobInfo["pngOutputUrl"].isString()) {
        job->pngOutputLocation = jobInfo["pngOutputUrl"].asString();
    }

    if (jobInfo.isMember("svgMeshOutputUrl") && jobInfo["svgMeshOutputUrl"].isString()) {
        job->svgMeshOutputLocation = jobInfo["svgMeshOutputUrl"].asString();
    }

    jobs.push_back(job);

    static BOOL clientInitted = false;

    PatternIterator patternIterator;
    PatternDescription description;
    description.shape = Shape::Rectangle;
    description.iterations = 10000;

    description.angularStep = 1.1f;

    description.colorRange[0] = Color4f(0.8f, 0.3f, 0.3f, 1.0f);
    description.colorRange[1] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);

    description.distanceRange[0] = 50.0f;
    description.distanceRange[1] = 1000.0f;

    description.rotationRange[0] = 0.0f;
    description.rotationRange[1] = 3.141592 * 120;

    description.sizeRange[0].Set(25.0f, 25.0f);
    description.sizeRange[1].Set(50.0f, 50.0f);


#if 0
    NSString *dst = [NSString pathWithComponents:@[resourceRootPath, @"output", [NSString stringWithUTF8String:"ocean-0.png"]]];
    Bitmap *src = Bitmap::LoadFromPNG([dst UTF8String]);

    //patternIterator.GeneratePatternFromBitmap(description, src);
    patternIterator.GeneratePatternAlt(description);
#else

    patternIterator.GeneratePattern(description);
#endif

    NSString *resourceRootPath = @"/Users/davidpetrie/Projects/aesthetics-engine";

    if (PREVIEW_OUTPUT) {
        NSString *fragmentShaderPath = [resourceRootPath stringByAppendingPathComponent:@"res/Shaders/quad.frag"];
        NSString *vertexShaderPath = [resourceRootPath stringByAppendingPathComponent:@"res/Shaders/quad.vert"];

        shaderProgram = difont::examples::OpenGL::loadShaderProgram([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);

        screenShotr = new ScreenShotr(PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT);

        for (auto job : jobs) {
            job->vertexBuffer = new VertexBuffer(PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT);
            if (job->logoGenerator) {
                job->vertexBuffer->SetFontRenderData(shaderProgram, job->logoGenerator->renderData);
                job->vertexBuffer->SetCameraOffset(job->logoGenerator->meshCenter);
            } else if (job->patternIterator) {
                job->vertexBuffer->SetMeshSet(shaderProgram, job->patternIterator->GetMeshSet());
            }
        }

        do {
            glfwPollEvents();
            if (clientInitted == false) {
                glShadeModel(GL_SMOOTH);
                glClearDepth(1.0f);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glPointSize(1.0f);
                glEnable(GL_MULTISAMPLE);
                glEnable(GL_VERTEX_ARRAY);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                glDepthMask(TRUE);
                glClearColor(1.0, 1.0, 1.0, 0.0);
                clientInitted = true;
            } else {
#if 0

                glClearColor(1.0, 1.0, 1.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                patternIterator.Render();
                glfwSwapBuffers(window);
#else
                if (currentJobIndex >= jobs.size()) {
                    //break;
                }

                auto currentIndex = currentJobIndex % jobs.size();
                auto job = jobs.at(currentIndex);


                if (job->logoGenerator) {
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                    job->vertexBuffer->Render(shaderProgram);
                    glfwSwapBuffers(window);

                    //job->vertexBuffer->Render(shaderProgram);
                    //glfwSwapBuffers(window);

                    job->pngOutputLocation = "a.png";
                    job->svgOutputLocation = "a.svg";

                    if (!job->pngOutputLocation.empty()) {
                        Bitmap *screenshot = screenShotr->TakeScreenshot();
                        std::string filename = job->pngOutputLocation;
                        NSString *dst = [NSString pathWithComponents:@[outputBasePath, @"output", [NSString stringWithUTF8String:filename.c_str()]]];
                        ImageFunctions::WritePNG([dst UTF8String], screenshot->GetData(), screenshot->GetWidth(), screenshot->GetHeight());
                        Bitmap::DestroyBitmap(screenshot);
                        fprintf(stdout, "Wrote %s\n", [dst UTF8String]);
                    }

                    if (!job->svgOutputLocation.empty()) {
                        std::string filename = job->svgOutputLocation;
                        NSString *dst = [NSString pathWithComponents:@[outputBasePath, @"output", [NSString stringWithUTF8String:filename.c_str()]]];
                        ImageFunctions::WriteSVG([dst UTF8String], PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT, job->logoGenerator->renderData);
                        fprintf(stdout, "Wrote %s\n", [dst UTF8String]);
                    }

                    if (!job->svgMeshOutputLocation.empty()) {
                        std::string filename = job->svgMeshOutputLocation;
                        NSString *dst = [NSString pathWithComponents:@[outputBasePath, @"output", [NSString stringWithUTF8String:filename.c_str()]]];
                        ImageFunctions::WriteSVGMesh([dst UTF8String], PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT, job->logoGenerator->renderData);
                        fprintf(stdout, "Wrote %s\n", [dst UTF8String]);
                    }

                } else if (job->patternIterator) {
/*
                    if (job->pngInputLocation.length() > 0) {
                        NSString *dst = [NSString pathWithComponents:@[outputPath, @"output", [NSString stringWithUTF8String:job->pngInputLocation.c_str()]]];
                        Bitmap *src = Bitmap::LoadFromPNG([dst UTF8String]);

                        job->patternIterator->GeneratePattern(src);

                        Bitmap::DestroyBitmap(src);
                    }

                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                    job->patternGenerator->Render();
                    glfwSwapBuffers(window);

                    Bitmap *screenshot = screenShotr->TakeScreenshot();

                    std::string filename = job->pngOutputLocation;
                    NSString *dst = [NSString pathWithComponents:@[outputPath, @"output", [NSString stringWithUTF8String:filename.c_str()]]];
                    
                    ImageFunctions::WritePNG([dst UTF8String], screenshot->GetData(), screenshot->GetWidth(), screenshot->GetHeight());
                    
                    Bitmap::DestroyBitmap(screenshot);*/
                }
                
                currentJobIndex++;
#endif
            }
        }
        while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
               glfwWindowShouldClose(window) == 0);
    }

    for (auto job : jobs) {
        if (job->logoGenerator) {

            NSString *pngDest = [NSString pathWithComponents:@[@"/Users/davidpetrie/Projects/aesthetics-engine", @"test", @"a.png"]];
            NSString *svgDest = [NSString pathWithComponents:@[@"/Users/davidpetrie/Projects/aesthetics-engine", @"test", @"a.svg"]];

            job->pngOutputLocation = [pngDest UTF8String];
            job->svgOutputLocation = [svgDest UTF8String];
            if (!job->svgOutputLocation.empty()) {
                ImageFunctions::WriteSVG(job->svgOutputLocation, PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT, job->logoGenerator->renderData, job->exportFontMetaData);
                fprintf(stdout, "Wrote %s\n", job->svgOutputLocation.c_str());
            }

            if (!job->svgMeshOutputLocation.empty()) {
                ImageFunctions::WriteSVGMesh(job->svgMeshOutputLocation, PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT, job->logoGenerator->renderData);
                fprintf(stdout, "Wrote %s\n", job->svgOutputLocation.c_str());
            }
        }
    }

    return 0;
}
