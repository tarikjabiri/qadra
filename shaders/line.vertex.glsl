#version 460 core

layout(location = 0) in dvec2 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in uint a_renderKey;
layout(location = 3) in uint a_flags;

uniform dmat4 u_viewProjection;
uniform float u_renderKeyScale;

out vec4 v_color;

void main() {
    if ((a_flags & 1u) == 0u) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        v_color = vec4(0.0);
        return;
    }

    gl_Position = vec4(u_viewProjection * dvec4(a_position, 0.0, 1.0));
    gl_Position.z = 1.0 - float(a_renderKey + 1u) * u_renderKeyScale;
    v_color = a_color;
}
