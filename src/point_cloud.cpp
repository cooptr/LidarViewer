#include <vector>
#include <glad/glad.h>

#include "point_cloud.hpp"
#include "ply_loader.hpp"

PointCloud::PointCloud(){
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
}

PointCloud::~PointCloud(){
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
}

void PointCloud::upload(const std::vector<Point>& pts) {
    glBindVertexArray(vao_);
    count_ = pts.size();
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(Point), pts.data(), GL_STATIC_DRAW);

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

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
}

void PointCloud::draw() const {
    glBindVertexArray(vao_);
    glDrawArrays(GL_POINTS, 0, count_);
}

size_t PointCloud::getcount() const { return count_; }