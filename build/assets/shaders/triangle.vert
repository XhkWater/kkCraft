#version 450

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    float brightness;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outTexCoord;

void main() {
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    outColor = inColor * pc.brightness;
    outTexCoord = inTexCoord;
}
