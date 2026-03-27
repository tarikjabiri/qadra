#version 460 core

layout (location = 0) in vec2 aTextOriginWorld;
layout (location = 1) in vec2 aQuadMinLocal;
layout (location = 2) in vec2 aQuadMaxLocal;
layout (location = 3) in vec2 aUvMin;
layout (location = 4) in vec2 aUvMax;
layout (location = 5) in vec2 aRotation;
layout (location = 6) in vec4 aColor;
layout (location = 7) in uint aRenderKey;

uniform dmat4 u_viewProjection;
uniform float u_renderKeyScale;

out vec2 vUV;
out vec4 vColor;

const vec2 kCorners[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

void main() {
    vec2 corner = kCorners[gl_VertexID];
    vec2 local = mix(aQuadMinLocal, aQuadMaxLocal, corner);
    vec2 rotated = vec2(
        local.x * aRotation.x - local.y * aRotation.y,
        local.x * aRotation.y + local.y * aRotation.x
    );
    vec2 world = aTextOriginWorld + rotated;

    gl_Position = vec4(u_viewProjection * dvec4(dvec2(world), 0.0, 1.0));
    gl_Position.z = 1.0 - float(aRenderKey + 1u) * u_renderKeyScale;
    vUV = mix(aUvMin, aUvMax, corner);
    vColor = aColor;
}
