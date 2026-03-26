#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;
layout (location = 3) in uint aRenderKey;

uniform dmat4 u_viewProjection;
uniform float u_renderKeyScale;

out vec2 vUV;
out vec4 vColor;

void main() {
    gl_Position = vec4(u_viewProjection * dvec4(dvec2(aPos), 0.0, 1.0));
    gl_Position.z = 1.0 - float(aRenderKey + 1u) * u_renderKeyScale;
    vUV = aUV;
    vColor = aColor;
}
