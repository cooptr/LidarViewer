#include <unordered_map>
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const char* vertexSrc, const char* fragmentSrc);   
    ~Shader();                                             
    Shader(const Shader&) = delete;                           
    Shader& operator=(const Shader&) = delete;

    void use() const;
    void set_mat4(const char* name, const glm::mat4& m);
    void set_float(const char* name, float v);
    void set_int(const char* name, int v);
    int location(const char* name);
private:
    unsigned int id_ = 0;
    std::unordered_map<std::string, int> locations_;
};