#pragma once

inline constexpr const char* vertexShaderSource = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aintensity;
layout (location = 2) in float aconfidence;
layout (location = 3) in vec3 actag_rgb;

uniform mat4 viewProj;
uniform float pointSize;

out float height;
out float confidence;
out float intensity;
out vec3 ctag_rgb;

void main()
{
    gl_Position = viewProj * vec4(aPos, 1.0);
    gl_PointSize = pointSize;
    height = aPos.y;
    confidence = aconfidence;
    intensity = aintensity;
    ctag_rgb = actag_rgb;
}
)glsl";

inline constexpr const char* fragmentShaderSource = R"glsl(#version 330 core
out vec4 FragColor;

uniform int colorMode;

in float height;
in float confidence;
in float intensity;
in vec3 ctag_rgb;

void main()
{
    vec3 low  = vec3(0.1, 0.2, 0.9);
    vec3 high = vec3(1.0, 0.3, 0.1);
    vec3 v;

    if (colorMode == 0)      v = mix(low, high, clamp(height * 0.5 + 0.5, 0.0, 1.0));
    else if (colorMode == 1) v = mix(low, high, clamp(confidence, 0.0, 1.0));
    else if (colorMode == 2) v = mix(low, high, clamp(intensity, 0.0, 1.0));
    else                     v = ctag_rgb;

    FragColor = vec4(v, 1.0);
}
)glsl";