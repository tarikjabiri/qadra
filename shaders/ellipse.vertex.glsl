#version 460 core

layout(location = 0) in dvec2 a_centerWorld;
layout(location = 1) in vec2 a_boundsMinLocal;
layout(location = 2) in vec2 a_boundsMaxLocal;
layout(location = 3) in vec2 a_majorDirection;
layout(location = 4) in float a_majorRadius;
layout(location = 5) in float a_minorRadius;
layout(location = 6) in vec4 a_color;
layout(location = 7) in uint a_renderKey;
layout(location = 8) in uint a_flags;

uniform dmat4 u_viewProjection;
uniform float u_renderKeyScale;
uniform float u_pixelSizeWorld;

out vec2 v_localWorld;
flat out vec2 v_majorDirection;
flat out float v_majorRadius;
flat out float v_minorRadius;
flat out vec4 v_color;

const vec2 kCorners[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

void main() {
    if ((a_flags & 1u) == 0u) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        v_localWorld = vec2(0.0);
        v_majorDirection = vec2(1.0, 0.0);
        v_majorRadius = 1.0;
        v_minorRadius = 1.0;
        v_color = vec4(0.0);
        return;
    }

    const float proxyMarginWorld = u_pixelSizeWorld * 2.5;
    vec2 localMin = a_boundsMinLocal - vec2(proxyMarginWorld);
    vec2 localMax = a_boundsMaxLocal + vec2(proxyMarginWorld);
    vec2 local = mix(localMin, localMax, kCorners[gl_VertexID]);
    dvec2 world = a_centerWorld + dvec2(local);

    gl_Position = vec4(u_viewProjection * dvec4(world, 0.0, 1.0));
    gl_Position.z = 1.0 - float(a_renderKey + 1u) * u_renderKeyScale;

    v_localWorld = local;
    v_majorDirection = a_majorDirection;
    v_majorRadius = a_majorRadius;
    v_minorRadius = a_minorRadius;
    v_color = a_color;
}
