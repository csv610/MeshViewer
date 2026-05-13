#version 120
attribute vec3 position;
attribute vec3 normal;
attribute vec4 color;
attribute float curvature;
attribute float faceID;

uniform mat4 mvp;
uniform mat4 modelview;
uniform mat3 normalMatrix;

varying vec3 fragNormal;
varying vec3 fragPos;
varying vec4 fragColor;
varying float fragCurvature;
varying float fragFaceID;

void main() {
    fragPos = vec3(modelview * vec4(position, 1.0));
    fragNormal = normalMatrix * normal;
    fragColor = color;
    fragCurvature = curvature;
    fragFaceID = faceID;
    gl_Position = mvp * vec4(position, 1.0);
}
