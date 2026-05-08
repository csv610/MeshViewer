#version 120
attribute vec3 position;
attribute vec3 normal;
attribute float curvature;
attribute float faceID;

uniform mat4 mvp;
uniform mat4 modelview;
uniform mat3 normalMatrix;

varying vec3 fragNormal;
varying vec3 fragPos;
varying float fragCurvature;
varying float fragFaceID;

void main() {
    fragPos = vec3(modelview * vec4(position, 1.0));
    fragNormal = normalMatrix * normal;
    fragCurvature = curvature;
    fragFaceID = faceID;
    gl_Position = mvp * vec4(position, 1.0);
}
