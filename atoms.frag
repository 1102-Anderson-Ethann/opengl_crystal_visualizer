#version 460
in vec2 vOffset;
out vec4 FragColor;

in vec3 vColor;

void main() {
    float dist = length(vOffset);
    if (dist > 1.0) discard;

   
    vec3 normal = vec3(vOffset.x, vOffset.y, sqrt(1.0 - dist * dist));
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normal, lightDir), 0.0);
    float ambient = 0.3;

    FragColor = vec4(vColor * (ambient + diffuse), 1.0);
}