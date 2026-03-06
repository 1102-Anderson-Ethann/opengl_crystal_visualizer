#version 460 
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aIntensity;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

out float vIntensity;

void main() {
    gl_Position = proj_matrix * mv_matrix * vec4(aPos, 1.0);
    vIntensity = aIntensity;
    gl_PointSize = 4.0 + vIntensity * 8.0;
}