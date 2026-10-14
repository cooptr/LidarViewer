#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

struct AppState {
    double x;
    double y;
    float yaw;
    float pitch;
    float distance;
    bool dragging = false;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
void processInput(GLFWwindow *window);
void render_test(const std::string& ply_file);
#endif