#version 120
#extension GL_OES_standard_derivatives : enable

varying vec3 fragNormal;
varying vec3 fragPos;
varying vec4 fragColor;
varying float fragCurvature;

uniform vec3 lightPos;
uniform vec3 color;
uniform bool useVertexColor;
uniform bool useLighting;
uniform bool useMatCap;
uniform bool useFlatShading;
uniform bool useCurvature;
uniform bool usePicking;
uniform float pickingID; // Used if we want to draw the whole mesh with one ID (not used for per-face)
varying float fragFaceID;

void main() {
    if (usePicking) {
        // Encode fragFaceID into color
        float id = fragFaceID + 1.0; // 0 is reserved for background
        float r = floor(id / 65536.0) / 255.0;
        float g = floor(mod(id, 65536.0) / 256.0) / 255.0;
        float b = mod(id, 256.0) / 255.0;
        gl_FragColor = vec4(r, g, b, 1.0);
        return;
    }

    vec3 baseColor = useVertexColor ? fragColor.rgb : color;

    if (!useLighting) {
        gl_FragColor = vec4(baseColor, 1.0);
        return;
    }

    vec3 n;
    if (useFlatShading) {
        // Compute face normal using derivatives
        n = normalize(cross(dFdx(fragPos), dFdy(fragPos)));
    } else {
        n = normalize(fragNormal);
    }
    
    vec3 finalBaseColor = baseColor;

    if (useCurvature) {
        // Heatmap: Blue (0) to Red (1)
        float c = clamp(fragCurvature * 5.0, 0.0, 1.0); // Scaled for visibility
        vec3 blue = vec3(0.0, 0.0, 1.0);
        vec3 red = vec3(1.0, 0.0, 0.0);
        finalBaseColor = mix(blue, red, c);
    }

    if (useMatCap) {
        vec2 uv = n.xy * 0.5 + 0.5;
        float dist = length(uv - 0.5);
        float rim = pow(dist * 1.2, 3.0);
        vec3 clayColor = mix(finalBaseColor, vec3(0.1), rim);
        gl_FragColor = vec4(clayColor, 1.0);
        return;
    }

    vec3 l = normalize(lightPos - fragPos);
    float diff = max(dot(n, l), 0.0);
    vec3 diffuse = diff * finalBaseColor;
    vec3 ambient = 0.15 * finalBaseColor;
    gl_FragColor = vec4(ambient + diffuse, 1.0);
}
