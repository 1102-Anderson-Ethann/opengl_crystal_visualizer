#version 460
layout (location = 0) in vec3 atomCenter;
layout (location = 1) in vec2 cornerOffset;
layout (location = 2) in float atomRadius;
layout (location = 3) in vec3 atomColor;


uniform mat4 mv_matrix;
uniform mat4 proj_matrix;


out vec2 vOffset;
out vec3 vColor;

void main() {
    vec4 eyePos = mv_matrix * vec4(atomCenter, 1.0);
    eyePos.xy += cornerOffset * atomRadius;
    gl_Position = proj_matrix * eyePos;
    vOffset = cornerOffset;
    vColor = atomColor;
}