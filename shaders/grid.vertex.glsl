#version 460 core
layout (location = 0) in dvec2 aPos;
layout (location = 1) in dvec4 aColor;

uniform dmat4 u_viewProjection;
out vec4 vColor;

void main() {
    gl_Position = vec4(u_viewProjection * dvec4(aPos, 0.0, 1.0));
    vColor = vec4(aColor);
}
