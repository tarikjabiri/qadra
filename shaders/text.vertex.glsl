#version 460 core

layout (location = 0) in vec2 aTextOriginWorld;
layout (location = 1) in vec2 aQuadMinLocal;
layout (location = 2) in vec2 aQuadMaxLocal;
layout (location = 3) in vec2 aTextBoxMinLocal;
layout (location = 4) in vec2 aTextBoxMaxLocal;
layout (location = 5) in vec2 aUvMin;
layout (location = 6) in vec2 aUvMax;
layout (location = 7) in vec2 aRotation;
layout (location = 8) in vec4 aColor;
layout (location = 9) in uint aRenderKey;
layout (location = 10) in uint aFlags;

uniform dmat4 u_viewProjection;
uniform float u_renderKeyScale;
uniform float u_pixelSizeWorld;

out vec2 vUV;
out vec4 vColor;
flat out uint vProxyMode;

const uint kFlagAlive = 1u << 0u;
const uint kFlagMarkerAnchor = 1u << 1u;
const float kMinGlyphPixels = 3.0;
const float kMinTextBoxPixels = 2.0;
const float kMinBoxAxisPixels = 2.0;
const float kPixelMarkerPixels = 1.0;

const vec2 kCorners[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

vec2 rotateLocal(vec2 local, vec2 rotation) {
    return vec2(
        local.x * rotation.x - local.y * rotation.y,
        local.x * rotation.y + local.y * rotation.x
    );
}

void main() {
    if ((aFlags & kFlagAlive) == 0u) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        vUV = vec2(0.0);
        vColor = vec4(0.0);
        vProxyMode = 0u;
        return;
    }

    vec2 corner = kCorners[gl_VertexID];
    vColor = aColor;

    vec2 glyphSize = abs(aQuadMaxLocal - aQuadMinLocal);
    float glyphPixels = max(glyphSize.x, glyphSize.y) / max(u_pixelSizeWorld, 1e-12);

    if (glyphPixels < kMinGlyphPixels) {
        vUV = vec2(0.0);

        if ((aFlags & kFlagMarkerAnchor) == 0u) {
            gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
            vProxyMode = 0u;
            return;
        }

        vec2 textBoxSize = abs(aTextBoxMaxLocal - aTextBoxMinLocal);
        float textBoxPixels = max(textBoxSize.x, textBoxSize.y) / max(u_pixelSizeWorld, 1e-12);

        if (textBoxPixels >= kMinTextBoxPixels) {
            vProxyMode = 1u;

            vec2 boxCenter = 0.5 * (aTextBoxMinLocal + aTextBoxMaxLocal);
            vec2 halfExtent = 0.5 * max(textBoxSize, vec2(kMinBoxAxisPixels * u_pixelSizeWorld));
            vec2 proxyLocal = mix(boxCenter - halfExtent, boxCenter + halfExtent, corner);
            vec2 world = aTextOriginWorld + rotateLocal(proxyLocal, aRotation);

            gl_Position = vec4(u_viewProjection * dvec4(dvec2(world), 0.0, 1.0));
            gl_Position.z = 1.0 - float(aRenderKey + 1u) * u_renderKeyScale;
            return;
        }

        vProxyMode = 2u;
        float halfSize = 0.5 * kPixelMarkerPixels * u_pixelSizeWorld;
        vec2 proxyLocal = (corner * 2.0 - 1.0) * halfSize;
        vec2 world = aTextOriginWorld + proxyLocal;

        gl_Position = vec4(u_viewProjection * dvec4(dvec2(world), 0.0, 1.0));
        gl_Position.z = 1.0 - float(aRenderKey + 1u) * u_renderKeyScale;
        return;
    }

    vProxyMode = 0u;

    vec2 local = mix(aQuadMinLocal, aQuadMaxLocal, corner);
    vec2 world = aTextOriginWorld + rotateLocal(local, aRotation);

    gl_Position = vec4(u_viewProjection * dvec4(dvec2(world), 0.0, 1.0));
    gl_Position.z = 1.0 - float(aRenderKey + 1u) * u_renderKeyScale;
    vUV = mix(aUvMin, aUvMax, corner);
}
