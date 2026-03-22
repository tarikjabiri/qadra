#version 460 core

in vec2 vUV;
in vec4 vColor;

uniform sampler2D u_texture;
uniform float u_distanceFieldRangePixels;

out vec4 FragColor;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

vec2 squared(vec2 value) {
    return value * value;
}

void main() {
    vec3 msd = texture(u_texture, vUV).rgb;
    float sd = median(msd.r, msd.g, msd.b);

    vec2 unitRange = vec2(u_distanceFieldRangePixels) / vec2(textureSize(u_texture, 0));
    vec2 screenTexSize = inversesqrt(squared(dFdx(vUV)) + squared(dFdy(vUV)));
    float screenPxRange = max(0.5 * dot(unitRange, screenTexSize), 1.0);

    float screenPxDistance = screenPxRange * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

    FragColor = vec4(vColor.rgb, vColor.a * opacity);
}
