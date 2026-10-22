#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <glm/gtc/matrix_transform.hpp>   
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>   
#include <cstddef>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "renderer.hpp"
#include "ply_loader.hpp"
#include "las_loader.hpp"

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    if (state && button == GLFW_MOUSE_BUTTON_LEFT) {
        state->dragging = (action == GLFW_PRESS);
        if (state->dragging) glfwGetCursorPos(window, &state->x, &state->y);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    if (!state) return;

    state->distance *= (yoffset > 0) ? 0.9f : 1.1f;
    state->distance = glm::clamp(state->distance, 0.15f, 20.0f);
}

void imgui_setup(GLFWwindow* gl_window){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(gl_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in float aintensity;\n"
    "layout (location = 2) in float aconfidence;\n"
    "layout (location = 3) in vec3 actag_rgb;\n"
    "uniform mat4 viewProj;\n"
    "uniform float pointSize;\n"
    "out float height;\n"
    "out float confidence;\n"
    "out float intensity;\n"
    "out vec3 ctag_rgb;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = viewProj * vec4(aPos, 1.0);\n"
    "   gl_PointSize = pointSize;\n"
    "   height = aPos.y;\n"
    "   confidence = aconfidence;\n"
    "   intensity = aintensity;\n"
    "   ctag_rgb = actag_rgb;\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform int colorMode;\n"
    "in float height;\n"
    "in float confidence;\n"
    "in float intensity;\n"
    "in vec3 ctag_rgb;\n"
    "void main()\n"
    "{\n"
    "   vec3 low  = vec3(0.1, 0.2, 0.9);\n"
    "   vec3 high = vec3(1.0, 0.3, 0.1);\n"
    "   vec3 v;\n"
    "   if (colorMode == 0)      v = mix(low, high, clamp(height * 0.5 + 0.5, 0.0, 1.0));\n"
    "   else if (colorMode == 1) v = mix(low, high, clamp(confidence, 0.0, 1.0));\n"
    "   else if (colorMode == 2) v = mix(low, high, clamp(intensity, 0.0, 1.0));\n"
    "   else                     v = ctag_rgb;\n"
    "   FragColor = vec4(v, 1.0);\n"
    "}\n\0";

void render_test(const std::string& las_file){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LidarViewer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    std::vector<Point> points = load_las(las_file);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Point), points.data(), GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)offsetof(Point, x));
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)offsetof(Point, intensity));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)offsetof(Point, confidence));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)offsetof(Point, ctag_rgb));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
 

    glm::vec3 eye(0.0f, 0.0f, 0.0f);
    glm::vec3 target(0.0f, 0.0f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);    

    int w, h;
    AppState state{};     
    state.distance = 3.0f;
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowUserPointer(window, &state);
    int cam_loc = glGetUniformLocation(shaderProgram, "viewProj");
    int point_loc = glGetUniformLocation(shaderProgram, "pointSize");
    int color_mode_loc = glGetUniformLocation(shaderProgram, "colorMode");

    imgui_setup(window);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Controls");
        ImGui::SliderFloat("Point size", &state.pointSize, 1.0f, 10.0f);
        ImGui::Combo("Color by", &state.colorMode, "Height\0Confidence\0Intensity\0Class\0");
        ImGui::Text("%d points", (int)points.size());
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        eye.x = state.distance * cos(state.pitch) * sin(state.yaw);
        eye.y = state.distance * sin(state.pitch);
        eye.z = state.distance * cos(state.pitch) * cos(state.yaw);  
        
        glfwGetFramebufferSize(window, &w, &h);
        if (h == 0) h = 1;
        float aspect = (float)w / h;
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(eye, target, up);
        glm::mat4 viewProj = proj * view;

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(cam_loc, 1, GL_FALSE, glm::value_ptr(viewProj));
        glUniform1f(point_loc, state.pointSize);
        glUniform1i(color_mode_loc, state.colorMode);

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, points.size());

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return;
}