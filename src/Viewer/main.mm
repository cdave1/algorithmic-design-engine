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
#include "PatternGenerator.h"
#include "ScreenShotr.h"
#include "Bitmap.h"
#include "Job.h"

static std::vector<WorkOrder *> workOrders;
static uint32_t currentWorkOrder = 0;
static ScreenShotr *screenShotr = nullptr;
static GLuint shaderProgram;
static uint32_t currentFrame = 0;

///
/// How to make this work:
/// 1. Read json from stdin containing work order information
/// 2. Generate according to taste.
///
int main(int argc, const char * argv[]) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    const unsigned int PREFERRED_WINDOW_WIDTH = 2364;
    const unsigned int PREFERRED_WINDOW_HEIGHT = 1464;

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
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

    std::string lines;
#if 1
    lines = R"({"jobs":[{"category":"logo-text","text":"ocean","iteration":9,"font":"/Library/Fonts/Futura.ttc","offset":[0,0],"fontSize":452.2858517477289},{"category":"pattern","iteration":9,"inputImageLocation":"","outputImageLocation":""}]}
    )";
    
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

    /*
     {"jobs":[
        {"category":"logo-text","feature":{"category":"logo-text","preferred-generator":"logo-gen-alpha","inputs":{"text":"Ocean","font-group":"san-serif","mood":"happy"},"invariants":{}},"text":"Ocean","iteration":0,"font":"/Library/Fonts/DIN Alternate Bold.ttf","offset":[0,0],"fontSize":183.0658475169912},
        ...
        ]
     }
     */

    screenShotr = new ScreenShotr(PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT);

    NSString *resourceRootPath = @"/Users/davidpetrie/Projects/aesthetics-engine";
    NSString *fragmentShaderPath = [resourceRootPath stringByAppendingPathComponent:@"res/Shaders/quad.frag"];
    NSString *vertexShaderPath = [resourceRootPath stringByAppendingPathComponent:@"res/Shaders/quad.vert"];

    shaderProgram = difont::examples::OpenGL::loadShaderProgram([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);

#define TAKE_SCREENSHOT_AND_EXIT

    const Json::Value workOrderList = data; //["jobs"];

    for (Json::Value workOrderInfo : workOrderList) {

        const Json::Value nodes = workOrderInfo["nodes"];

        WorkOrder *workOrder = new WorkOrder();

        for (Json::Value node : nodes) {
            if (node["category"] == "logo-text") {


                std::string fontText = node["text"].asString();
                const int iteration = node["iteration"].asInt();
                const double fontSize = node["fontSize"].asDouble();
                const std::string fontPath = node["font"].asString();
                const Json::Value jsonOffset = node["offset"];
                const difont::Point offset(jsonOffset[0].asFloat(), jsonOffset[1].asFloat());

                const unsigned int MAX_PATH_LEN = 1024;
                char cwd[MAX_PATH_LEN];
                getcwd(cwd, MAX_PATH_LEN);
                const std::string CWD(cwd);

                LogoGenerator *logoGenerator = new LogoGenerator(PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT, shaderProgram);
                logoGenerator->Init();
                logoGenerator->SetConditions(fontSize, fontText, fontPath.c_str(), offset);
                logoGenerator->SetupVertexArrays();

                Job *job = new Job(node);
                job->logoGenerator = logoGenerator;
                workOrder->AddJob(job);
                
            } else if (node["category"] == "pattern") {
                PatternGenerator *patternGenerator = new PatternGenerator(PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT, shaderProgram);
                patternGenerator->Init();

                Job *job = new Job(node);
                job->patternGenerator = patternGenerator;
                workOrder->AddJob(job);
            }
        }

        workOrders.push_back(workOrder);
    }

    static BOOL clientInitted = false;

    static int step = 0;

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
            glClearColor(0.0, 0.0, 0.0, 1.0);
            clientInitted = true;
        } else {
            // The way to do this is to run through each work order off screen, and only render here once it's all been
            // completed.
            auto currentIndex = currentJob % jobs.size();
            auto workOrder = workOrders.at(currentIndex);

#if 0
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            logoGenerator->Render(shaderProgram);
            glfwSwapBuffers(window);

            logoGenerator->Render(shaderProgram);
            glfwSwapBuffers(window);

            if (currentFrame++ % 60 == 0) {
                currentJob++;
            }
#else
            if (step == 0) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                job->logoGenerator->Render(shaderProgram);
                glfwSwapBuffers(window);

                job->logoGenerator->Render(shaderProgram);
                glfwSwapBuffers(window);

                Bitmap *screenshot = screenShotr->TakeScreenshot();
                job->patternGenerator->GeneratePattern(screenshot, shaderProgram);
                Bitmap::DestroyBitmap(screenshot);

                //if (currentFrame++ % 60 == 0) {
                    currentJob++;
                //}

                if (currentJob == jobs.size() - 1) {
                    ++step;
                }

            } else {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                job->patternGenerator->Render(shaderProgram);
                glfwSwapBuffers(window);

                if (currentFrame++ % 60 == 0) {
                    currentJob++;
                    step++;
                    Bitmap *screenshot = screenShotr->TakeScreenshot();

#ifdef TAKE_SCREENSHOT_AND_EXIT
                    char filename[512];
                    snprintf(filename, 512, "%s-%d.png", "ocean", currentJob);
                    NSString *dst = [NSString pathWithComponents:@[resourceRootPath, @"output", [NSString stringWithUTF8String:filename]]];

                    ImageFunctions::WritePNG([dst UTF8String], screenshot->GetData(), screenshot->GetWidth(), screenshot->GetHeight());
                    Bitmap::DestroyBitmap(screenshot);
                    //break;
#else
                    Bitmap::DestroyBitmap(screenshot);
#endif

                }
            }
#endif
        }
    }
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);
    
    return 0;
}
