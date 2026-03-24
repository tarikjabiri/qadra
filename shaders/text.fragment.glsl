#version 460 core

in vec2 vUV;
in vec4 vColor;

uniform sampler2D u_texture;
uniform float u_distanceFieldRange;

out vec4 FragColor;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPixelRange(vec2 uv) {
    vec2 unitRange = vec2(u_distanceFieldRange) / vec2(textureSize(u_texture, 0));
    vec2 screenTexSize = vec2(1.0) / max(fwidth(uv), vec2(1e-6));
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

void main() {
    vec3 msdf = texture(u_texture, vUV).rgb;
    float signedDistance = median(msdf.r, msdf.g, msdf.b) - 0.5;
    float pixelDistance = signedDistance * screenPixelRange(vUV);

    // Keep the normal 1 px MSDF transition, but soften slightly when derivatives spike.
    float transitionWidth = 0.5 * clamp(fwidth(pixelDistance), 1.0, 1.5);
    float opacity = smoothstep(-transitionWidth, transitionWidth, pixelDistance);

    if (opacity <= (1.0 / 255.0)) {
        discard;
    }

    FragColor = vec4(vColor.rgb, vColor.a * opacity);
}
