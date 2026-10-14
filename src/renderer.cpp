#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <glm/gtc/matrix_transform.hpp>   
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>    

#include "renderer.hpp"
#include "ply_loader.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    MouseState* state = (MouseState*)glfwGetWindowUserPointer(window);
    if (!state) return;

    if (state->dragging) {
        float dx = (float)(xpos - state->x);
        float dy = (float)(ypos - state->y);
        state->yaw   -= dx * 0.005f;
        state->pitch += dy * 0.005f;
        state->pitch  = glm::clamp(state->pitch, -1.5f, 1.5f);
    }
    state->x = xpos;
    state->y = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    MouseState* state = (MouseState*)glfwGetWindowUserPointer(window);
    if (state && button == GLFW_MOUSE_BUTTON_LEFT) {
        state->dragging = (action == GLFW_PRESS);
        if (state->dragging) glfwGetCursorPos(window, &state->x, &state->y);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    MouseState* state = (MouseState*)glfwGetWindowUserPointer(window);
    if (!state) return;

    state->distance *= (yoffset > 0) ? 0.9f : 1.1f;
    state->distance  = glm::clamp(state->distance, 1.5f, 20.0f);
}

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 viewProj;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = viewProj * vec4(aPos, 1.0);\n"
    "   gl_PointSize = 3.0;\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

void render_test(const std::string& ply_file){
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

    std::vector<Point> points = get_points(ply_file);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Point), points.data(), GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    glm::vec3 eye(0.0f, 0.0f, 0.0f);
    glm::vec3 target(0.0f, 0.0f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);    

    int w, h;
    MouseState state{};     
    state.distance = 3.0f;
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowUserPointer(window, &state);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
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

        int loc = glGetUniformLocation(shaderProgram, "viewProj");
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(viewProj));

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, points.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}