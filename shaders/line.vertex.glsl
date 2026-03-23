#version 460 core

layout(location = 0) in dvec2 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in float a_depth;

uniform dmat4 u_viewProjection;

out vec4 v_color;

void main() {
    gl_Position = vec4(u_viewProjection * dvec4(a_position, 0.0, 1.0));
    gl_Position.z = a_depth;
    v_color = a_color;
}
