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
#include "ScreenShotr.h"

static LogoGenerator *logoGenerator;
static ScreenShotr *screenShotr = nullptr;
static GLuint shaderProgram;

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
    for (std::string line; std::getline(std::cin, line);) {
        lines += line;
    }

    Json::Value data;
    Json::Reader reader;
    if (!reader.parse(lines, data)) {
        fprintf(stderr, "Error reading input.\n");
        exit(1);
    }
    /*
     {"category":"logo-text","feature":{"category":"logo-text","preferred-generator":"logo-gen-alpha","inputs":{"text":"Ocean","font-group":"san-serif","mood":"happy"},"invariants":{}},"text":"Ocean","iteration":0,"font":"/Library/Fonts/DIN Alternate Bold.ttf","offset":[0,0],"fontSize":183.0658475169912}
     */

    std::string fontText = data["text"].asString();
    const int iteration = data["iteration"].asInt();
    const double fontSize = data["fontSize"].asDouble();
    const std::string fontPath = data["font"].asString();

    const unsigned int MAX_PATH_LEN = 1024;
    char cwd[MAX_PATH_LEN];
    getcwd(cwd, MAX_PATH_LEN);
    const std::string CWD(cwd);

    logoGenerator = new LogoGenerator(PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT);
    logoGenerator->SetConditions(fontSize, fontText, fontPath.c_str());

    screenShotr = new ScreenShotr(PREFERRED_WINDOW_WIDTH, PREFERRED_WINDOW_HEIGHT);

    NSString *resourceRootPath = @"/Users/davidpetrie/Projects/aesthetics-engine";

    NSString *fragmentShaderPath = [resourceRootPath stringByAppendingPathComponent:@"res/Shaders/quad.frag"];
    NSString *vertexShaderPath = [resourceRootPath stringByAppendingPathComponent:@"res/Shaders/quad.vert"];

    shaderProgram = difont::examples::OpenGL::loadShaderProgram([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);
    logoGenerator->SetupVertexArrays(shaderProgram);

    static BOOL clientInitted = false;
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
        }
        else {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            logoGenerator->Update(shaderProgram);
            logoGenerator->Render(shaderProgram);
            glfwSwapBuffers(window);

            logoGenerator->Update(shaderProgram);
            logoGenerator->Render(shaderProgram);
            glfwSwapBuffers(window);

            char filename[512];

            snprintf(filename, 512, "%s-%d.png", fontText.c_str(), iteration);

            NSString *dst = [NSString pathWithComponents:@[resourceRootPath, @"output", [NSString stringWithUTF8String:filename]]];
            screenShotr->TakeScreenshot([dst UTF8String]);
            break;
        }
    }
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);
    
    return 0;
}
