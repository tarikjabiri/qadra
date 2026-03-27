#version 460 core

in vec2 v_localWorld;
flat in float v_radiusWorld;
flat in float v_startAngle;
flat in float v_sweepAngle;
flat in vec4 v_color;

uniform float u_pixelSizeWorld;

out vec4 FragColor;

const float kTwoPi = 6.28318530717958647692;

float normalizeAngle(float angle) {
    angle = mod(angle, kTwoPi);
    if (angle < 0.0) {
        angle += kTwoPi;
    }
    return angle;
}

float ccwDelta(float from, float to) {
    return normalizeAngle(to - from);
}

bool isAngleOnArc(float angle) {
    if (abs(abs(v_sweepAngle) - kTwoPi) <= 1e-4) {
        return true;
    }

    if (v_sweepAngle >= 0.0) {
        return ccwDelta(v_startAngle, angle) <= v_sweepAngle + 1e-4;
    }

    return ccwDelta(angle, v_startAngle) <= -v_sweepAngle + 1e-4;
}

void main() {
    const float lineWidthPixels = 1.0;
    const float antiAliasPixels = 1.0;
    float halfLineWidthWorld = max(0.5 * u_pixelSizeWorld, 0.5 * lineWidthPixels * u_pixelSizeWorld);

    float distanceFromCenter = length(v_localWorld);
    float radialSignedDistance = abs(distanceFromCenter - v_radiusWorld) - halfLineWidthWorld;

    float localAngle = normalizeAngle(atan(v_localWorld.y, v_localWorld.x));
    vec2 startPoint = vec2(cos(v_startAngle), sin(v_startAngle)) * v_radiusWorld;
    vec2 endPoint = vec2(cos(v_startAngle + v_sweepAngle), sin(v_startAngle + v_sweepAngle)) * v_radiusWorld;

    float signedDistance = radialSignedDistance;
    if (!isAngleOnArc(localAngle)) {
        float startCapDistance = length(v_localWorld - startPoint) - halfLineWidthWorld;
        float endCapDistance = length(v_localWorld - endPoint) - halfLineWidthWorld;
        signedDistance = min(startCapDistance, endCapDistance);
    }

    float antiAliasWorld = max(antiAliasPixels * u_pixelSizeWorld, fwidth(signedDistance));
    antiAliasWorld = clamp(antiAliasWorld, 0.35 * u_pixelSizeWorld, 2.5 * u_pixelSizeWorld);

    float coverage = 1.0 - smoothstep(-antiAliasWorld, antiAliasWorld, signedDistance);
    coverage *= v_color.a;

    if (coverage <= 0.001) {
        discard;
    }

    FragColor = vec4(v_color.rgb, coverage);
}
