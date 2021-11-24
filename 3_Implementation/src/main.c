#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <GLFW/glfw3.h>
#include <linmath/linmath.h>

#include "config.h"
#include "font.h"
#include "model.h"
#include "opengl.h"
#include "physics.h"
#include "shader.h"
#include "texture.h"

// game resources
#include "models/sprite.h"
#include "shaders/font_frag.h"
#include "shaders/font_vert.h"
#include "shaders/sprite_frag.h"
#include "shaders/sprite_vert.h"
#include "textures/bg.h"
#include "textures/bird.h"
#include "textures/pipe_bot.h"
#include "textures/pipe_top.h"
#include "play.h"



static void
print_usage(const char* arg0)
{
    printf("usage: %s [options]\n", arg0);
    printf("\n");
    printf("Options:\n");
    printf("  -h --help        print this help\n");
    printf("  -f --fullscreen  fullscreen rootwin\n");
    printf("  -v --vsync       enable vsync\n");
}

int
main(int argc, char* argv[])
{
    bool fullscreen = false;
    bool vsync = false;

    // process CLI args and update corresponding flags
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fullscreen") == 0) {
            fullscreen = true;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--vsync") == 0) {
            vsync = true;
        }
    }

    srand(time(NULL));

    if (!glfwInit()) {
        const char* error = NULL;
        glfwGetError(&error);
        fprintf(stderr, "failed to init GLFW3: %s\n", error);
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // ask for an OpenGL 3.3 Core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    // set flags for monitor details
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    GLFWwindow* rootwin = NULL;
    if (fullscreen) {
        rootwin = glfwCreateWindow(mode->width, mode->height, "Flappy Bird", monitor, NULL);
    } else {
        rootwin = glfwCreateWindow(1280, 720, "Flappy Bird", NULL, NULL);
    }

    if (rootwin == NULL) {
        const char* error = NULL;
        glfwGetError(&error);
        fprintf(stderr, "failed to create GLFW3 rootwin: %s\n", error);

        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetInputMode(rootwin, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwMakeContextCurrent(rootwin);
    glfwSwapInterval(vsync ? 1 : 0);
    opengl_load_functions();

    printf("OpenGL Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL Version:  %s\n", glGetString(GL_VERSION));
    printf("GLSL Version:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    struct FlappyBoard game = { 0 };
    start_game(&game);

    // timing vars
    double l_sec = glfwGetTime();
    double l_frme = l_sec;
    long num_frame = 0;

    // loop til exit or ESCAPE key
    while (!glfwWindowShouldClose(rootwin)) {
        double now = glfwGetTime();
        double delta = now - l_frme;
        l_frme = now;

        change_gme(&game, rootwin, delta);

        int width, height;
        glfwGetFramebufferSize(rootwin, &width, &height);
        game_render(&game, width, height);

        num_frame++;
        if (glfwGetTime() - l_sec >= 1.0) {
            printf("FPS: %ld  (%lf ms/frame)\n", num_frame, 1000.0/num_frame);
            num_frame = 0;
            l_sec += 1.0;
        }

        glfwSwapBuffers(rootwin);
        glfwPollEvents();
    }

    end_game(&game);

    // Cleanup GLFW3 resources
    glfwDestroyWindow(rootwin);
    glfwTerminate();

    return EXIT_SUCCESS;
}
