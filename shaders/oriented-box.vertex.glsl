#version 460 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec4 aColor;

uniform mat4 u_viewProjection;

out vec4 vColor;

void main() {
    gl_Position = u_viewProjection * vec4(aPosition, 0.0, 1.0);
    vColor = aColor;
}
