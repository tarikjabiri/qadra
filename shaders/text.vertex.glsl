#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;
layout (location = 3) in float aDepth;

uniform dmat4 u_viewProjection;

out vec2 vUV;
out vec4 vColor;

void main() {
    gl_Position = vec4(u_viewProjection * dvec4(dvec2(aPos), 0.0, 1.0));
    gl_Position.z = aDepth;
    vUV = aUV;
    vColor = aColor;
}
