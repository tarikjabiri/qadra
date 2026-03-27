#version 460 core

in vec2 v_localWorld;
flat in vec2 v_majorDirection;
flat in float v_majorRadius;
flat in float v_minorRadius;
flat in vec4 v_color;

uniform float u_pixelSizeWorld;

out vec4 FragColor;

void main() {
    const float lineWidthPixels = 1.0;
    const float antiAliasPixels = 1.0;
    float halfLineWidthWorld = max(0.5 * u_pixelSizeWorld, 0.5 * lineWidthPixels * u_pixelSizeWorld);

    vec2 majorDir = normalize(v_majorDirection);
    vec2 minorDir = vec2(-majorDir.y, majorDir.x);
    vec2 localEllipse = vec2(dot(v_localWorld, majorDir), dot(v_localWorld, minorDir));

    float a = v_majorRadius;
    float b = v_minorRadius;
    float x = localEllipse.x;
    float y = localEllipse.y;

    float invA2 = 1.0 / (a * a);
    float invB2 = 1.0 / (b * b);
    float ellipseValue = x * x * invA2 + y * y * invB2 - 1.0;
    vec2 gradient = vec2(2.0 * x * invA2, 2.0 * y * invB2);
    float gradientLength = max(length(gradient), 1e-6);
    float curveSignedDistance = ellipseValue / gradientLength;
    float signedDistance = abs(curveSignedDistance) - halfLineWidthWorld;

    float antiAliasWorld = max(antiAliasPixels * u_pixelSizeWorld, fwidth(signedDistance));
    antiAliasWorld = clamp(antiAliasWorld, 0.35 * u_pixelSizeWorld, 2.5 * u_pixelSizeWorld);

    float coverage = 1.0 - smoothstep(-antiAliasWorld, antiAliasWorld, signedDistance);
    coverage *= v_color.a;

    if (coverage <= 0.001) {
        discard;
    }

    FragColor = vec4(v_color.rgb, coverage);
}
