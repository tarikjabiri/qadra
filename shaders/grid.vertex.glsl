#version 460 core
layout (location = 0) in dvec2 aFrom;
layout (location = 1) in dvec2 aTo;
layout (location = 2) in vec4 aColor;
layout (location = 3) in float aLineWidthPixels;
layout (location = 4) in float aAntiAliasWidthPixels;

uniform dmat4 u_viewProjection;
uniform vec2 u_viewportSizePixels;

flat out vec4 vColor;
noperspective out float vDistanceAlongLinePixels;
noperspective out float vSignedDistanceFromLineCenterPixels;
flat out float vLineLengthPixels;
flat out float vHalfLineWidthPixels;
flat out float vAntiAliasWidthPixels;

void main() {
    const vec2 corners[6] = vec2[6](
        vec2(0.0, -1.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0),
        vec2(0.0, -1.0),
        vec2(1.0, 1.0),
        vec2(1.0, -1.0)
    );

    vec2 corner = corners[gl_VertexID % 6];
    float t = corner.x;
    float side = corner.y;

    vec4 clip0 = vec4(u_viewProjection * dvec4(aFrom, 0.0, 1.0));
    vec4 clip1 = vec4(u_viewProjection * dvec4(aTo, 0.0, 1.0));

    vec2 ndc0 = clip0.xy / clip0.w;
    vec2 ndc1 = clip1.xy / clip1.w;

    vec2 screenPositionStartPixels = (ndc0 * 0.5 + 0.5) * u_viewportSizePixels;
    vec2 screenPositionEndPixels = (ndc1 * 0.5 + 0.5) * u_viewportSizePixels;

    screenPositionStartPixels = floor(screenPositionStartPixels) + 0.5;
    screenPositionEndPixels = floor(screenPositionEndPixels) + 0.5;

    ndc0 = (screenPositionStartPixels / u_viewportSizePixels) * 2.0 - 1.0;
    ndc1 = (screenPositionEndPixels / u_viewportSizePixels) * 2.0 - 1.0;

    clip0.xy = ndc0 * clip0.w;
    clip1.xy = ndc1 * clip1.w;

    vec2 lineDeltaPixels = (ndc1 - ndc0) * 0.5 * u_viewportSizePixels;
    float lineLengthPixels = length(lineDeltaPixels);

    vec2 lineDirectionScreen = lineLengthPixels > 1e-5 ? (lineDeltaPixels / lineLengthPixels) : vec2(1.0, 0.0);
    vec2 lineNormalScreen = vec2(-lineDirectionScreen.y, lineDirectionScreen.x);

    float antiAliasWidthPixels = max(0.0, aAntiAliasWidthPixels);
    float halfLineWidthPixels = max(0.5, 0.5 * aLineWidthPixels);
    float halfVisualExtentPixels = halfLineWidthPixels + antiAliasWidthPixels;
    float lineCapExtensionPixels = halfLineWidthPixels;
    float distanceAlongLinePixels = mix(-lineCapExtensionPixels, lineLengthPixels + lineCapExtensionPixels, t);
    float lineCapOffsetPixels = mix(-lineCapExtensionPixels, lineCapExtensionPixels, t);

    vec2 vertexOffsetPixels =
    lineNormalScreen * (side * halfVisualExtentPixels) + lineDirectionScreen * lineCapOffsetPixels;
    vec2 ndcOffset = vertexOffsetPixels / (0.5 * u_viewportSizePixels);

    vec4 clip = mix(clip0, clip1, t);
    clip.xy += ndcOffset * clip.w;
    gl_Position = clip;

    vColor = aColor;
    vDistanceAlongLinePixels = distanceAlongLinePixels;
    vSignedDistanceFromLineCenterPixels = side * halfVisualExtentPixels;
    vLineLengthPixels = lineLengthPixels;
    vHalfLineWidthPixels = halfLineWidthPixels;
    vAntiAliasWidthPixels = antiAliasWidthPixels;
}
