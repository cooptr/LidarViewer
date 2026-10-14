#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include <algorithm>

#include "ply_loader.hpp"

PlyHeader read_header(const std::string& ply_file) {
    PlyHeader h;
    std::ifstream file(ply_file);
    if (!file) return h;                      

    std::string line;
    bool inVertex = false;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line == "end_header") break;

        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;                      

        if (keyword == "element") {
            std::string name;
            size_t count = 0;
            iss >> name >> count;
            inVertex = (name == "vertex");
            if (inVertex) h.vertexCount = count;
        }
        else if (keyword == "property" && inVertex) {
            std::string type, prop;
            iss >> type >> prop;
            h.props.push_back(prop);
        }
    }
    file.close();
    //for (const auto& p : h.props) std::cout << p << '\n';
    return h;
}

int find_col(const PlyHeader& h, const std::string& name){
    for (size_t i = 0; i < h.props.size(); ++i){
        if (h.props[i] == name) return static_cast<size_t>(i);
    }
    return -1;
}

void normalize(std::vector<Point>& pts) {
    if (pts.empty()) return;

    glm::vec3 mn(pts[0].x, pts[0].y, pts[0].z);
    glm::vec3 mx = mn;
    for (const auto& p : pts) {
        mn = glm::min(mn, glm::vec3(p.x, p.y, p.z));
        mx = glm::max(mx, glm::vec3(p.x, p.y, p.z));
    }

    glm::vec3 center = (mn + mx) * 0.5f;
    glm::vec3 size   = mx - mn;
    float maxExtent  = std::max({size.x, size.y, size.z});
    if (maxExtent <= 0.0f) maxExtent = 1.0f;   

    float scale = 2.0f / maxExtent;              

    for (auto& p : pts) {
        p.x = (p.x - center.x) * scale;
        p.y = (p.y - center.y) * scale;
        p.z = (p.z - center.z) * scale;
    }
}

std::vector<Point> get_points(const std::string& ply_file){
    PlyHeader h = read_header(ply_file);
    std::vector<Point> result(h.vertexCount);

    int ix = find_col(h, "x");
    int iy = find_col(h, "y");
    int iz = find_col(h, "z");
    int ii = find_col(h, "intensity");
    int ic = find_col(h, "confidence");

    std::vector<std::string> row(h.props.size());
    std::ifstream file(ply_file);
    std::string line;

    size_t i = 0;
    Point p;
    bool past_headers = false;

    while(std::getline(file, line)){

        std::istringstream iss(line);
        if (i >= h.vertexCount) break;
        if (past_headers){
            for (std::string& v : row) iss >> v;
            p.x = std::stof(row[ix]);
            p.y = std::stof(row[iy]);
            p.z = std::stof(row[iz]);
            p.intensity = std::stof(row[ii]);
            p.confidence = std::stof(row[ic]);
            result[i] = p;
            ++i;
        }
        if (line == "end_header"){
            past_headers = true;
        }
    }
    //normalize(result);
    //for (size_t i = 0; i < h.vertexCount; i++){
        //std::cout << "x: " << result[i].x << "y: " << result[i].y << "z: " << result[i].z << " " << '\n';
    //}
    file.close();
    normalize(result);
    return result;
}
