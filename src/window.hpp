struct GLFWwindow;
#include "renderer.hpp"
#include "app_state.hpp"
class Window {
public:
    Window(const int width, const int height, const char* name, AppState* state);
    ~Window();
    Window(const Window&) = delete;
    Window operator=(const Window&) = delete;
    
    GLFWwindow* get_window_ptr() const;
    AppState* get_state_ptr() const;
    bool should_close() const;

    void swap();

    float aspect() const;

    void process_input();

    void poll();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
private:
AppState* state_pointer;
int width_, height_;
GLFWwindow* window_pointer;
};