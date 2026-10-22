#include <vector>
#include "ply_loader.hpp"

class PointCloud {
public:
    PointCloud();                                  
    ~PointCloud();                               
    PointCloud(const PointCloud&) = delete;
    PointCloud& operator=(const PointCloud&) = delete;

    size_t getcount() const;
    void upload(const std::vector<Point>& pts);    
    void draw() const;                             
    size_t size() const { return count_; }
private:
    unsigned int vao_ = 0, vbo_ = 0;
    size_t count_ = 0;
};