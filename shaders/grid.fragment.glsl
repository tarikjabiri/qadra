#version 460 core
flat in vec4 vColor;
noperspective in float vDistanceAlongLinePixels;
noperspective in float vSignedDistanceFromLineCenterPixels;
flat in float vLineLengthPixels;
flat in float vHalfLineWidthPixels;
flat in float vAntiAliasWidthPixels;

out vec4 FragColor;

void main() {
    float distanceFromLineSidePixels = abs(vSignedDistanceFromLineCenterPixels) - vHalfLineWidthPixels;
    float signedDistancePixels = distanceFromLineSidePixels;

    if (vDistanceAlongLinePixels < 0.0) {
        signedDistancePixels = max(distanceFromLineSidePixels, -vHalfLineWidthPixels - vDistanceAlongLinePixels);
    } else if (vDistanceAlongLinePixels > vLineLengthPixels) {
        signedDistancePixels = max(
            distanceFromLineSidePixels,
            vDistanceAlongLinePixels - (vLineLengthPixels + vHalfLineWidthPixels)
        );
    }

    if (vAntiAliasWidthPixels <= 0.001) {
        if (signedDistancePixels > 0.0) {
            discard;
        }
        FragColor = vColor;
        return;
    }

    float antiAliasBlendWidthPixels = max(vAntiAliasWidthPixels, fwidth(signedDistancePixels));
    antiAliasBlendWidthPixels = clamp(antiAliasBlendWidthPixels, 0.35, 2.5);
    float alpha = 1.0 - smoothstep(
        -antiAliasBlendWidthPixels,
        antiAliasBlendWidthPixels,
        signedDistancePixels
    );
    alpha *= vColor.a;

    if (alpha <= 0.001) {
        discard;
    }

    FragColor = vec4(vColor.rgb, alpha);
}
