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
#include "shader.hpp"
#include "shaders.hpp"
#include "point_cloud.hpp"
#include "window.hpp"

void imgui_setup(GLFWwindow* w) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(w, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void render_test(const std::string& las_file){  
    AppState state{};
    Window window(800, 600, "LidarViewer", &state);
    Shader shader(vertexShaderSource, fragmentShaderSource);
    PointCloud pointcloud;
    pointcloud.upload(load_las(las_file));
 

    glm::vec3 eye(0.0f, 0.0f, 0.0f);
    glm::vec3 target(0.0f, 0.0f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    imgui_setup(window.get_window_ptr());

    while (!window.should_close())
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Controls");
        ImGui::SliderFloat("Point size", &state.pointSize, 1.0f, 10.0f);
        ImGui::Combo("Color by", &state.colorMode, "Height\0Confidence\0Intensity\0Class\0");
        ImGui::Text("%d points", (int)pointcloud.getcount());
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
        window.process_input();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        eye.x = state.distance * cos(state.pitch) * sin(state.yaw);
        eye.y = state.distance * sin(state.pitch);
        eye.z = state.distance * cos(state.pitch) * cos(state.yaw);  
        
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), window.aspect(), 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(eye, target, up);
        glm::mat4 viewProj = proj * view;

        shader.use();
        shader.set_float("pointSize", state.pointSize);
        shader.set_int("colorMode", state.colorMode);
        shader.set_mat4("viewProj", viewProj);

        pointcloud.draw();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.swap();
        window.poll();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return;
}