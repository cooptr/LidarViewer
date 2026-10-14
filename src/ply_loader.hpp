#ifndef PLY_LOADER_H
#define PLY_LOADER_H

#include <string>
#include <vector>

struct PlyHeader {
    size_t vertexCount = 0;
    std::vector<std::string> props;
};

struct Point {
    float x, y, z, intensity, confidence;
};

PlyHeader read_header(const std::string& ply_file);

int find_col(const PlyHeader& h, const std::string& name);

void normalize(std::vector<Point>& pts);

std::vector<Point> get_points(const std::string& ply_file);
#endif