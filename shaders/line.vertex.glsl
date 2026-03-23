#version 460 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec4 aColor;

uniform mat4 u_viewProjection;
uniform vec2 u_viewportSizePixels;

out vec4 vColor;

void main() {
    vec4 clip = u_viewProjection * vec4(aPosition, 0.0, 1.0);
    vec2 ndc = clip.xy / clip.w;
    vec2 pixel = (ndc * 0.5 + 0.5) * u_viewportSizePixels;
    pixel = floor(pixel) + vec2(0.5);
    ndc = (pixel / u_viewportSizePixels) * 2.0 - 1.0;
    clip.xy = ndc * clip.w;

    gl_Position = clip;
    vColor = aColor;
}
