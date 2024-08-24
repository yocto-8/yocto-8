#version 130

const int ZOOM = 4;

const int offsetR[] = int[ZOOM](0, 0, 0, 1);
const int offsetG[] = int[ZOOM](0, 0, 0, 0);
const int offsetB[] = int[ZOOM](-1, 0, 0, 0);

uniform sampler2D unscaledFrame;
uniform sampler2D pixelGridFilter;

void main()
{
    ivec2 unscaledUv = ivec2(gl_TexCoord[0].xy * 128 * ZOOM);
    ivec2 scaledUv = unscaledUv / ZOOM;

    ivec2 offset = unscaledUv % ZOOM;
    float r = texelFetch(unscaledFrame, scaledUv + ivec2(offsetR[offset.x], 0.0), 0).r;
    float g = texelFetch(unscaledFrame, scaledUv + ivec2(offsetG[offset.x], 0.0), 0).g;
    float b = texelFetch(unscaledFrame, scaledUv + ivec2(offsetB[offset.x], 0.0), 0).b;

    vec3 colorMult = mix(
        texelFetch(pixelGridFilter, offset, 0).rgb,
        vec3(1.0),
        0.0 // higher value = closer to original
    );

    gl_FragColor.rgb = vec3(r, g, b) * colorMult;
    gl_FragColor.a = 1.0;
}