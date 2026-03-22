#version 460 core

layout (location = 0) in vec3 aPos;

uniform dmat4 u_viewProjection;

void main() {
    gl_Position = vec4(u_viewProjection * dvec4(aPos, 1.0));
}
