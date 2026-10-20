#include <iostream>
#include <string>
#include <cstring>

template <typename T>
T read_at(const std::vector<char>& buf, size_t offset) {
    T value;
    std::memcpy(&value, buf.data() + offset, sizeof(T));
    return value;
}

std::vector<Point> load_las(const std::string& las_file);

struct LasHeader {
    uint16_t recordLength, headerLength;
    uint32_t offsetLength;
};