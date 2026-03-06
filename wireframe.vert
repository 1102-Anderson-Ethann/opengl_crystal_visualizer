#version 460
layout (location = 0) in vec3 aPos;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

void main() {
    gl_Position = proj_matrix * mv_matrix * vec4(aPos, 1.0);
}