#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "imgui.h"
#include <glm/glm.hpp>
#include "window.hpp"
#include <stdexcept>
#include "app_state.hpp"

void Window::framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    if (!state) return;

    if (state->dragging) {
        float dx = (float)(xpos - state->x);
        float dy = (float)(ypos - state->y);
        state->x = xpos;
        state->y = ypos;
        if (ImGui::GetIO().WantCaptureMouse) return;
        state->yaw   -= dx * 0.005f;
        state->pitch += dy * 0.005f;
        state->pitch  = glm::clamp(state->pitch, -1.5f, 1.5f);
    }
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int) {
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    if (state && button == GLFW_MOUSE_BUTTON_LEFT) {
        state->dragging = (action == GLFW_PRESS);
        if (state->dragging) glfwGetCursorPos(window, &state->x, &state->y);
    }
}

void Window::scroll_callback(GLFWwindow* window, double, double yoffset) {
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    if (!state) return;

    state->distance *= (yoffset > 0) ? 0.9f : 1.1f;
    state->distance = glm::clamp(state->distance, 0.15f, 20.0f);
}

Window::Window(const int width, const int height, const char* name, AppState* state) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window_pointer = glfwCreateWindow(width, height, name, NULL, NULL);
    if (!window_pointer) { glfwTerminate(); throw std::runtime_error("window creation failed"); }
    state_pointer = state;
    glfwSetWindowUserPointer(window_pointer, state_pointer);

    glfwMakeContextCurrent(window_pointer);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      glfwDestroyWindow(window_pointer);
      glfwTerminate();
      throw std::runtime_error("glad failed");
    }

    glfwSetFramebufferSizeCallback(window_pointer, framebuffer_size_callback);
    glfwSetCursorPosCallback(window_pointer, cursor_position_callback);
    glfwSetScrollCallback(window_pointer, scroll_callback);
    glfwSetMouseButtonCallback(window_pointer, mouse_button_callback);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);   
}

GLFWwindow* Window::get_window_ptr() const { return window_pointer; }

Window::~Window() {
    glfwDestroyWindow(window_pointer);
    glfwTerminate();
}

float Window::aspect() const {
    int w, h;
    glfwGetFramebufferSize(window_pointer, &w, &h);
    if (h == 0) h = 1;                       
    return static_cast<float>(w) / h;
}

bool Window::should_close() const {
    return glfwWindowShouldClose(window_pointer);
}

void Window::process_input(){
    if (glfwGetKey(window_pointer, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window_pointer, true);
}

void Window::poll(){
    glfwPollEvents();
}

void Window::swap(){
    glfwSwapBuffers(window_pointer);
}
