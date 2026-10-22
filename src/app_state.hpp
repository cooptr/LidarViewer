#pragma once
struct AppState {
    double x;
    double y;
    float yaw;
    float pitch;
    float distance = 3.0f;
    bool dragging = false;
    float pointSize = 3.0f;
    int colorMode = 0;
};