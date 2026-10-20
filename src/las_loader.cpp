#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include "ply_loader.hpp"
#include "las_loader.hpp"

constexpr int MIN_HEADER_LENGTH = 227;
constexpr int POINT_DATA_RECORD_LENGTH = 105;
constexpr int HEADER_SIZE = 94;
constexpr int OFFSET_TO_POINT_DATA = 96;
constexpr int POINT_DATA_FORMAT = 104;
constexpr int XYZ_SCALE_FACTORS = 131;
constexpr int XYZ_OFFSETS = 155;
constexpr int POINT_COUNT = 247;
constexpr int MAX_START = 179;
constexpr int MIN_START = 187; 
constexpr int LEGACY_COUNT = 107;

enum class ClassificationTag {
    Never_Classified,
    Unassigned,
    Ground,
    Low_Veg,
    Med_Veg,
    High_Veg,
    Building,
    Noise,
    Water = 9,
    Rail,
    Road,
    Wire_Guard = 13,
    Wire_Conductor,
    Transmission_Tower,
    Wire_Structure,
    Bridge_Deck,
    High_Noise
};

glm::vec3 ctag_convert(uint8_t ctag){
    switch (static_cast<ClassificationTag>(ctag)) {
        case (ClassificationTag::Never_Classified):
        return glm::vec3{0.50f, 0.50f, 0.50f};
        break;
        case (ClassificationTag::Unassigned):
        return glm::vec3{0.70f, 0.70f, 0.70f};
        break;
        case (ClassificationTag::Ground):
        return glm::vec3{0.60f, 0.40f, 0.20f};
        break;
        case (ClassificationTag::Low_Veg):
        return glm::vec3{0.60f, 0.90f, 0.40f};
        break;
        case (ClassificationTag::Med_Veg):
        return glm::vec3{0.20f, 0.70f, 0.20f};
        break;
        case (ClassificationTag::High_Veg):
        return glm::vec3{0.00f, 0.40f, 0.10f};
        break;
        case (ClassificationTag::Building):
        return glm::vec3{0.90f, 0.30f, 0.30f};
        break;
        case (ClassificationTag::Noise):
        return glm::vec3{1.00f, 0.00f, 1.00f};
        break;
        case (ClassificationTag::Water):
        return glm::vec3{0.20f, 0.40f, 1.00f};
        break;
        case (ClassificationTag::Rail):
        return glm::vec3{0.40f, 0.30f, 0.30f};
        break;
        case (ClassificationTag::Road):
        return glm::vec3{0.30f, 0.30f, 0.35f};
        break;
        case (ClassificationTag::Wire_Guard):
        return glm::vec3{1.00f, 1.00f, 0.50f};
        break;
        case (ClassificationTag::Wire_Conductor):
        return glm::vec3{1.00f, 0.90f, 0.00f};
        break;
        case (ClassificationTag::Transmission_Tower):
        return glm::vec3{1.00f, 0.60f, 0.00f};
        break;
        case (ClassificationTag::Wire_Structure):
        return glm::vec3{0.90f, 0.70f, 0.40f};
        break;
        case (ClassificationTag::Bridge_Deck):
        return glm::vec3{0.50f, 0.50f, 0.70f};
        break;
        case (ClassificationTag::High_Noise):
        return glm::vec3{1.00f, 0.00f, 0.60f};
        break;
    }
}

std::vector<Point> load_las(const std::string& las_file){
    std::vector<Point> pts;
    std::ifstream file(las_file, std::ios::binary);
    if (!file){
        std::cout << "couldn't open " << las_file << '\n';
        return pts;
    }
    std::vector<char> las_data(MIN_HEADER_LENGTH);
    file.read(las_data.data(), las_data.size());
    if (file.gcount() != MIN_HEADER_LENGTH || std::string(las_data.data(), 4) != "LASF") {
        std::cout << "too short" << '\n';
        return pts;
    }

    uint8_t format = read_at<uint8_t>(las_data, POINT_DATA_FORMAT);
    uint16_t recordLength = read_at<uint16_t>(las_data, POINT_DATA_RECORD_LENGTH);
    uint16_t headerLength = read_at<uint16_t>(las_data, HEADER_SIZE);

    if (headerLength > MIN_HEADER_LENGTH){
        las_data.resize(headerLength);
        file.read(las_data.data() + MIN_HEADER_LENGTH, headerLength - MIN_HEADER_LENGTH);
        if (file.gcount() != headerLength - MIN_HEADER_LENGTH) return {};    
    }
    uint32_t dataOffset = read_at<uint32_t>(las_data, OFFSET_TO_POINT_DATA);
    uint64_t pointCount = read_at<uint32_t>(las_data, LEGACY_COUNT);
    if (headerLength >= 375) pointCount = read_at<uint64_t>(las_data, POINT_COUNT);

    double xyz_scale[3], xyz_offsets[3], center[3];
    for (int i = 0; i < 3; i++){
        xyz_offsets[i] = read_at<double>(las_data, XYZ_OFFSETS + (i * sizeof(double)));
        xyz_scale[i] = read_at<double>(las_data, XYZ_SCALE_FACTORS + (i * sizeof(double)));
    }
    constexpr size_t BOUNDS_STRIDE = 2 * sizeof(double);
    for (int i = 0; i < 3; i++){
        double mx = read_at<double>(las_data, MAX_START + (i * BOUNDS_STRIDE));
        double mn = read_at<double>(las_data, MIN_START + (i * BOUNDS_STRIDE));
        center[i] = (mx + mn)/2;
    }

    file.seekg(dataOffset);
    std::vector<char> block((size_t)pointCount * recordLength);
    file.read(block.data(), block.size());
    if ((size_t)file.gcount() != block.size()) return {};

    pts.resize(pointCount);
    for (size_t i = 0; i < pointCount; i++){
        size_t current_record = i * recordLength;

        int32_t rx = read_at<int32_t>(block, current_record + 0);
        int32_t ry = read_at<int32_t>(block, current_record + 4);
        int32_t rz = read_at<int32_t>(block, current_record + 8);

        uint8_t ctag = read_at<uint8_t>(block, current_record + 15);

        double x = rx * xyz_scale[0] + xyz_offsets[0] - center[0];
        double y = ry * xyz_scale[1] + xyz_offsets[1] - center[1];
        double z = rz * xyz_scale[2] + xyz_offsets[2] - center[2];

        Point p;
        p.ctag_rgb = ctag_convert(ctag);
        p.x = static_cast<float>(x);
        p.y = static_cast<float>(z);
        p.z = static_cast<float>(-y);
        p.intensity = read_at<uint16_t>(block, current_record + 12) / 65535.0f;
        pts[i] = p;
    }
    normalize(pts);
    return pts;
}